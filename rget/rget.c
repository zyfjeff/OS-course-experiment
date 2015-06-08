#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <libgen.h>
#include "proto.h"
#include "common.h"


int recu = RECUR_FALSE;
int force = 0;

FILE *currentfp = NULL;
char currentfile[256] = {0};


static const struct option long_options[]=
{
 {"recursive",required_argument,NULL,'r'},
 {"force",required_argument,NULL,'f'},
 {"help",required_argument,NULL,'h'},
 {"version",required_argument,NULL,'v'},
 {NULL,0,NULL,0}
};

int make_connect(const char *ip,unsigned int port)
{
        int ret;
        int sockfd = socket(PF_INET,SOCK_STREAM,0);
        if(sockfd < 0)
                ERR_EXIT("create socket");
        struct sockaddr_in addr;
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;
        if((ret = inet_pton(PF_INET,ip,&(addr.sin_addr))) < 0)
                ERR_EXIT("convert to ip");
    ret = connect(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in));
    if(ret == -1) 
        ERR_EXIT("connect to server:");

    return sockfd;
}


static void usage(void)
{
   fprintf(stderr,
    "rget [option] HOST:PATH... \n"
    "  -r|--recursive   accroding to file git to find file                        \n"
    "  -f|--force       force overlap file                                        \n"
    "  -h|--help        get help                                                  \n"
    "  -V|--version     print sfind version                                       \n"
    );
   exit(EXIT_FAILURE);
};

static void version(void)
{
    fprintf(stderr,"rget v1.0\n");
    exit(EXIT_FAILURE);
}


bool _write_to_file(int fd,FILE *fp,int datalen)
{
    char buf[8192] = {0};
    assert(datalen < sizeof(buf));
    int ret = readn(fd,buf,datalen);
    if (ret <= 0 || ret != datalen)
        return false;
    ret = fwrite(buf,1,datalen,fp);
    assert(ret == datalen);
    return true;
}

bool write_to_singlefile(int fd,FILE *fp,int datalen)
{
    int ret = 0;
    _write_to_file(fd,fp,datalen);
    struct packet pack;
    while(1) {
        ret = readn(fd,&pack,sizeof(struct packet));
        if (ret <= 0 || ret != sizeof(struct packet))
            return false;
        if (pack.Type == PACKET_DATA) {
            printf("write to file\n");
            _write_to_file(fd,fp,pack.Totallen);
            continue;
        }
        
        if (pack.Type == END_OF_FILE)
            return true;
    }
}

int main(int argc,char *argv[])
{
     if (argc == 1) {
        fprintf(stderr,"please use rget -h to find rget usage\n");
        usage();
        exit(EXIT_FAILURE);
     }
    int ret = 0;
    char host_path[256] = {0};
    char path[256] = {0};
    char host[256] = {0};
    int opt =0;
    int options_index = 0;
    //获取操作集合
     while((opt=getopt_long(argc,argv,":r:f:hv",
            long_options,&options_index))!=EOF )
    {
         switch(opt) {
         case 'r': recu = RECUR_TRUE; strncpy(host_path,optarg,strlen(optarg));break;
         case 'f': force = 1;strncpy(host_path,optarg,strlen(optarg));break;
         case 'h': usage();
         case 'v': version();
         case '?': usage();
         default: usage();
       }
    }

    if (optind == 1 && argc == 2) {
        if(!str_split(argv[1],host,path,':')) {
            usage();          
        }
    }
    if (host_path[0] != '\0') {
        if(!str_split(host_path,host,path,':')) {
            usage();          
        }
    }
//    printf("%s\n",host);
//    printf("%s\n",path);
    struct packet pack;
    //发送conn结构体
    struct conn cn;
    cn.flag = recu;
    cn.Totallen = strlen(path);
    int serverfd = make_connect(host,8000);
    ret = writen(serverfd,&cn,sizeof(cn));
    ERRRET_EXIT(ret,sizeof(struct conn));
    
    ret = writen(serverfd,path,cn.Totallen);
    ERRRET_EXIT(ret,cn.Totallen);

while(1) {
    char pathbuf[256] = {0};
    ret = readn(serverfd,&pack,sizeof(pack));
    ERRRET_EXIT(ret,sizeof(struct packet));
    if (ret == 0) {
        exit(EXIT_FAILURE);    
    }
    if (pack.Type == PACKET_ERROR) {              //错误类型的packet
        printf("%s\n",pack.content.errorinfo);
        exit(EXIT_FAILURE);
    }

    if (pack.Type == PACKET_INIT) {               //初始化类型的packet
        mkdir(pack.content.filename,755);       
        continue;
    }
    
    if (pack.Type == PACKET_DATA) {               //数据传输类型的packet
        if (currentfile[0] == '\0') {
            strncpy(currentfile,pack.content.filename,strlen(pack.content.filename)); 
        }

        if (currentfp == NULL) {
            strncpy(pathbuf,currentfile,strlen(currentfile));
            dirname(pathbuf);
            if(access(pathbuf,F_OK) == -1) { //不存在
                mkdir(pathbuf,0755);
                printf("mkdir %s\n",pathbuf);
            }
            printf("fopen:%s\n",currentfile);
            currentfp = fopen(pack.content.filename,"w");   
            if (currentfp == NULL) {
                printf("init fopen file error\n");
                exit(EXIT_FAILURE);
            }
        }
        if (!strcmp(pack.content.filename,currentfile) == 0) {
            strncpy(pathbuf,currentfile,strlen(currentfile));
            dirname(pathbuf);
            if(access(pathbuf,F_OK) == -1) //不存在
                mkdir(pathbuf,0755);
            currentfp = fopen(pack.content.filename,"w");
            if (currentfp == NULL) {
                printf("fopen file error\n");
                exit(EXIT_FAILURE);
            }
            strncpy(currentfile,pack.content.filename,strlen(pack.content.filename)); 
        }
        printf("start to write to file\n");
        printf("currentfile:%s\n",currentfile);
        if(write_to_singlefile(serverfd,currentfp,pack.Totallen) == true) {
            printf("write success %s\n",currentfile);   
        } else {
            printf("file write failue\n");
            exit(EXIT_FAILURE);
        }
        continue;
    }
}

    return 0;
}


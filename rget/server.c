#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include "libsocket.h"
#include "common.h"
#include "proto.h"

char *file_not_exists = "file or dir not exists";
char *not_allow_recursiv = "the dir not allow to recursiv copy";

bool build_and_senderror(int fd,char *errorinfo)
{
        int ret =0;
        struct packet pack;
        bzero(&pack,sizeof(struct packet));
        pack.Totallen = 0;
        pack.Type = PACKET_ERROR;
        strncpy(pack.content.errorinfo,errorinfo,strlen(errorinfo));
        ret = writen(fd,&pack,sizeof(struct packet));
        if (ret <=0 || (ret != sizeof(struct packet)))
            return false;
        return true;
}


bool build_and_senddata(int fd,char *name,char *data,int datalen)
{
        int ret = 0;
        struct packet pack;
        bzero(&pack,sizeof(struct packet));
        pack.Totallen = datalen;
        pack.Type = PACKET_DATA;
        strncpy(pack.content.filename,name,strlen(name));
        ret = writen(fd,&pack,sizeof(struct packet));
        if (ret <=0 || (ret != sizeof(struct packet)))
            return false;
        ret = writen(fd,data,datalen);
        if (ret <=0 || (ret != datalen))
            return false;
        return true;
}

bool build_and_sendeof(int fd,char *name)
{
        int ret = 0;
        struct packet pack;
        bzero(&pack,sizeof(struct packet));
        pack.Type = END_OF_FILE;
        strncpy(pack.content.filename,name,strlen(name));
        ret = writen(fd,&pack,sizeof(struct packet));
        if (ret <=0 || (ret != sizeof(struct packet)))
            return false;
        return true;
}


bool build_and_sendinit(int fd,char *name)
{
        int ret = 0;
        struct packet pack;
        bzero(&pack,sizeof(struct packet));
        pack.Type = PACKET_INIT;
        strncpy(pack.content.filename,name,strlen(name));
        pack.flaginfo.filetype = DIR_TYPE;
        ret = writen(fd,&pack,sizeof(struct packet));
        if (ret <=0 || (ret != sizeof(struct packet)))
            return false;
        return true;
}


bool build_and_send_singlefile(int fd,char *filename) //发送单个文件
{
       int ret = 0;
       char buf[4096] = {0};
       FILE *fp = fopen(filename,"r+");
       while((ret = fread(buf,1,sizeof(buf),fp)) > 0) {
            build_and_senddata(fd,filename,buf,ret);
       }
       if(!feof(fp)){
                printf("fread error\n"); 
                fclose(fp);
                return false;
       }
       build_and_sendeof(fd,filename);
       fclose(fp);
       return true;
}

bool build_and_senddir(int fd,char *filename)
{
 
    char tmp_name[MAX_BUFFER_SIZE] = {0};
    DIR *dir_stream = opendir(filename);
    if (dir_stream == NULL) {
        fprintf(stderr,"open the dir is error\n");
             return;
    }
    struct dirent *dirp;
    int count = 0;
    while((dirp = readdir(dir_stream)) != NULL)
    {   
       if (strcmp(dirp->d_name,"..") == 0)
           continue;

       if (strcmp(dirp->d_name,".")== 0) {
                build_and_sendinit(fd,filename);
                printf("%s\n",filename);
                continue;
       } else {
           memset(tmp_name,0,MAX_BUFFER_SIZE);
           con_str(tmp_name,filename,dirp->d_name);
           //printf("%s\n",tmp_name);
       }

       if (is_dir(tmp_name) == true) {
            build_and_senddir(fd,tmp_name);
       } else {
            printf("%s\n",tmp_name);
            build_and_send_singlefile(fd,tmp_name); 
      }

    }   
    return;        
}

int main()
{
   struct conn co;
   struct packet pack;
   char filename[4096] = {0};
   int sockfd = create_tcp_socket(8000,10);   
   while(1)
   {
       int fd = accept(sockfd,NULL,NULL);
       int ret = readn(fd,&co,sizeof(co));
       if(ret <= 0 || (ret != sizeof(co))) {
            printf("error read\n");
            close(fd);
            continue;
       }
       bzero(filename,sizeof(filename));
       ret = readn(fd,&filename,co.Totallen);
       printf("%s\n",filename);
       if (access(filename,F_OK) == -1) {  //文件或者目录不存在的情况
            if(!build_and_senderror(fd,file_not_exists)) {
                printf("senderror\n");
            }
            close(fd);
            continue;
       }
       
       if (!is_dir(filename)) {  //是文件
           build_and_send_singlefile(fd,filename);
           close(fd);
           continue;
       }

       if (co.flag == RECUR_FALSE) {  //是目录, 但是没有递归标志,返回错误信息
           printf("not RECUR_FALSE \n");
            if(!build_and_senderror(fd,not_allow_recursiv)) {
                printf("send error\n");
            }
            close(fd);
            continue;
       }
    
      //最后一种情况，是目录，也有递归标志 
        build_and_senddir(fd,filename);
        close(fd);   
   }
}

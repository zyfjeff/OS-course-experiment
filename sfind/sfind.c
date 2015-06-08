/*
 =====================================================================================
        Filename:  sfind.c
     	Description:  sfind实现
        Version:  1.0
        Created:  04/30/2015 10:35:44 PM
        Revision:  none
        Compiler:  gcc
        Author:  Jeff (), zyfforlinux@163.com
    	Organization:  Linux
 =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_NAME_SIZE 256
#define MAX_FILE_NUMBER 8192

//实现简易版哈希
char sfind_option[] = {'n','s','t','u','g'};

int(*fun[256])(char (*input_data)[MAX_NAME_SIZE],char *para,int size);

static const struct option long_options[]=
{
 {"name",required_argument,NULL,'n'},
 {"size",required_argument,NULL,'s'},
 {"type",required_argument,NULL,'t'},
 {"uid",required_argument,NULL,'u'},
 {"gid",required_argument,NULL,'g'},
 {"help",required_argument,NULL,'h'},
 {"version",required_argument,NULL,'v'},
 {NULL,0,NULL,0}
};

static void usage(void)
{
   fprintf(stderr,
    "sfind [option]... \n"
    "  -n|--name      acrroding to filename substr to find file                 \n"
    "  -s|--size      accroding to file size to find file(lettle and equal size)\n"
    "  -t|--type      accroding to file type to find file                       \n"
    "  -g|--gid       accroding to file git to find file                        \n"
    "  -u|--uid       accroding to file uid to find file                        \n"
    "  -h|--help      get help                                                  \n"
    "  -V|--version   print sfind version                                       \n"
    );
   exit(EXIT_FAILURE);
};

static void version(void)
{
    fprintf(stderr,"sfind v1.0\n");
    exit(EXIT_FAILURE);
}

int get_all_filename(char (*input_data)[MAX_NAME_SIZE])
{
    DIR *dir_stream = opendir(".");
    if (dir_stream == NULL) {
        fprintf(stderr,"open the dir is error\n");
        return -1;
    }
    struct dirent *dirp;
    int count = 0;
    while((dirp = readdir(dir_stream)) != NULL)
    {
       if (strcmp(dirp->d_name,".") == 0 ||
           strcmp(dirp->d_name,"..") == 0)
           continue;
        count++;
        strncpy(input_data[count],dirp->d_name,sizeof(dirp->d_name));
    }
    return count;
}


/*
 * 接收三个参数，文件名集合，用户输入参数，和 文件名集合大小
 * 返回文件名集合的最大下标
 */
int name_find(char (*input_data)[MAX_NAME_SIZE],char *para,int size)
{
    int index = -1;
    int i = 0;
    //开始处理,
    for(i = 1;i <= size;i++)
    {
        if(input_data[i][0] == '\0')
            continue;
        if(strstr(input_data[i],para) == NULL) {
            memset(input_data[i],'\0',MAX_NAME_SIZE);
            continue;
        }
        index = i; //保存最后一个文件名位置
    }
    return index;
}

int type_find(char (*input_data)[MAX_NAME_SIZE],char *para,int size)
{
    int i=1;
    int flag = 0;
    int index = -1;
    struct stat file_stat;
    for(i = 1;i <= size;i++)
    {
        flag = 0;
        if(input_data[i][0] == '\0')
            continue;
        if (stat(input_data[i],&file_stat) == -1) {
            perror("stat file error\n");
            return index;
        }
        switch(para[0]) {
        case 'd':
            {
                if(S_ISDIR(file_stat.st_mode))
                    flag = 1;
                break;
            }
        case 'c':
            {
                if(S_ISCHR(file_stat.st_mode))
                    flag = 1;
                break;
            }
        case 'b':
            {
                if(S_ISBLK(file_stat.st_mode))
                    flag = 1;
                break;
            }
        case 'l':
            {
                if(S_ISLNK(file_stat.st_mode))
                    flag = 1;
                break;
            }
        case '-':
            {
                if(S_ISREG(file_stat.st_mode))
                    flag = 1;
                break;
            }
        }
        
        if (flag == 0) {
            memset(input_data[i],'\0',MAX_NAME_SIZE);
            continue;
        }
        index = i;
    }
    return index;
}


int uid_find(char (*input_data)[MAX_NAME_SIZE],char *para,int size)
{
    int i=1;
    int index = -1;
    struct stat file_stat;
    int gid = atoi(para);
    for(i = 1;i <= size;i++)
    {
        if(input_data[i][0] == '\0')
            continue;
        if (stat(input_data[i],&file_stat) == -1) {
            perror("stat file error\n");
            return index;
        }
        if (file_stat.st_gid != gid) {
            memset(input_data[i],'\0',MAX_NAME_SIZE);
            continue;
        }
        index = i;
    }
    return index;
}

int gid_find(char (*input_data)[MAX_NAME_SIZE],char *para,int size)
{
    int i=1;
    int index = -1;
    struct stat file_stat;
    int uid = atoi(para);
    for(i = 1;i <= size;i++)
    {
        if(input_data[i][0] == '\0')
            continue;
        if (stat(input_data[i],&file_stat) == -1) {
            perror("stat file error\n");
            return index;
        }
        if (file_stat.st_uid != uid) {
            memset(input_data[i],'\0',MAX_NAME_SIZE);
            continue;
        }
        index = i;
    }
    return index;
}

int size_find(char (*input_data)[MAX_NAME_SIZE],char *para,int size)
{
    int i=1;
    int index = -1;
    struct stat file_stat;
    int filesize = atoi(para);
    filesize = filesize*1024; //转换成字节
    for(i = 1;i <= size;i++)
    {
        if(input_data[i][0] == '\0')
            continue;
        if (stat(input_data[i],&file_stat) == -1) {
            perror("stat file error\n");
            return index;
        }
        if (file_stat.st_size > filesize) {
            memset(input_data[i],'\0',MAX_NAME_SIZE);
            continue;
        }
        index = i;
    }
    return index;
}


void printf_filenameset(char (*input_data)[MAX_NAME_SIZE],int size)
{
    int i = 1;
    for(i = 1; i <= size; i++)
    {
        if (input_data[i][0]=='\0')
            continue;
        printf("%s\n",input_data[i]);
            
    }
}


void init_func()
{
    fun['n'] = name_find;
    fun['s'] = size_find;
    fun['t'] = type_find;
    fun['u'] = uid_find;
    fun['g'] = gid_find;
}

void check_digit(const char *para)
{
    const char *start = para;
    while(*start  != '\0')
    {
        if (*start >= '0' && *start <= '9') {
            start++;
            continue;
        }
        else {
            fprintf(stderr,"the para error\n");
            usage();
        }
    }
    return;
}

int main(int argc,char *argv[])
{
     if (argc == 1) {
        fprintf(stderr,"please use sfind -h to find sfind usage\n");
        usage();
        exit(EXIT_FAILURE);
     }
    
     int option_flag[256] = {0}; //和sfind_option实现简易hash
     char *option_arg[256] = {'\0'};
     int options_index;
     int opt;
     
     //初始化结果集合和标志集合
     char filename_set[MAX_FILE_NUMBER][MAX_NAME_SIZE];
     memset(filename_set,0,sizeof(filename_set));
     memset(option_flag,0,sizeof(option_flag));

    //初始化函数指针
     init_func();
 
    //获取操作集合
     while((opt=getopt_long(argc,argv,"n:s:t:g:u:hv",
            long_options,&options_index))!=EOF )
    {
         switch(opt) {
         case 'n': option_flag['n'] = 1; option_arg['n'] = optarg;break;;
         case 's': option_flag['s'] = 1; option_arg['s'] = optarg;break;
         case 't': option_flag['t'] = 1; option_arg['t'] = optarg;break;
         case 'g': option_flag['g'] = 1; check_digit(optarg);option_arg['g'] = optarg;break;
         case 'u': option_flag['u'] = 1; check_digit(optarg);option_arg['u'] = optarg;break;
         case 'h': usage();
         case 'v': version();
         case '?': usage();
         default: usage();
       }
    }

    if (optind == 1) {
        fprintf(stderr,"error argument.........\n");
        usage();
    }

    //加入参数合法性检查
    int size = get_all_filename(filename_set);
    unsigned int option_len = sizeof(sfind_option);
    int index = 0;

    for(index  = 0;index < option_len;index++)
    {
        char opt_char = sfind_option[index];
        if (option_flag[opt_char] == 1) //选项开启了  
            size = fun[opt_char](filename_set,option_arg[opt_char],size);
    }
    printf_filenameset(filename_set,size);
}


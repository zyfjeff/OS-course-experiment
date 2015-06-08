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
#include <libgen.h>

#define MAX_NAME_SIZE 256
#define MAX_FILE_NUMBER 8192
#define MAX_BUFFER_SIZE 8192

typedef enum {false=0,true=1} STATUS;

//实现简易版哈希
char sfind_option[] = {'n','s','t','u','g'};

STATUS(*fun[256])(char *filename,char *para);

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

static STATUS is_dir(char *filename)
{
    struct stat file_stat;
    if (stat(filename,&file_stat) == -1) {
        perror("stat file error:");
        return false;
    }

    if (S_ISDIR(file_stat.st_mode))
        return true;
    else
        return false;
}

static void con_str(char *dest_buf,const char *fist_str,const char *last_str)
{
    strcat(dest_buf,fist_str);
    strcat(dest_buf,"/");
    strcat(dest_buf,last_str);
    return ;
}



static STATUS filter(char *filename,const int opt_len,
                    const int *opt_flag,char **opt_arg)
{
    int index = 0;
    char opt_char;

    for(index = 0;index < opt_len;++index)
    {
        opt_char = sfind_option[index];
        if (opt_flag[opt_char] == 1) //选项开启了  
            if (fun[opt_char](filename,opt_arg[opt_char]) == false) {
//                printf("filter:%s\n",filename);
                return false;
            }
    }
    return true;
}

void filter_all_file(char *findpath,const int opt_len,
                    const int *opt_flag,char **opt_arg)
{

    char tmp_name[MAX_BUFFER_SIZE] = {0};
//    if (subpath !=  )
    DIR *dir_stream = opendir(findpath);
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
           if (filter(findpath,opt_len,opt_flag,opt_arg) == true)
                printf("%s\n",findpath);
           continue;
       } else {
           memset(tmp_name,0,MAX_BUFFER_SIZE);
           con_str(tmp_name,findpath,dirp->d_name);
           if (filter(tmp_name,opt_len,opt_flag,opt_arg) == true)
                printf("%s/%s\n",findpath,dirp->d_name);
       }
       if (is_dir(tmp_name)) {
            filter_all_file(tmp_name,opt_len,opt_flag,opt_arg);
       }
    }
    return;
}


/*
 * 接收三个参数，文件名集合，用户输入参数，和 文件名集合大小
 * 返回文件名集合的最大下标
 */
STATUS name_find(char *filename,char *para)
{
    //开始处理
    if (filename == NULL)
        return false;
    char *base_filename = basename(filename);
    if(strstr(base_filename,para) == NULL) {
        return false;
    }
    return true;
}

STATUS type_find(char *filename,char *para)
{
    int flag = 0;
    struct stat file_stat;
    if (stat(filename,&file_stat) == -1) {
        perror("stat file error\n");
        return false;
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
            return false;
        }
    return true;
}


STATUS uid_find(char *filename,char *para)
{
    struct stat file_stat;
    int gid = atoi(para);
    
    if (stat(filename,&file_stat) == -1) {
            perror("stat file error\n");
            return false;
    }
    
    if (file_stat.st_gid != gid) {
            return false;
    
    }
    return true;
}

STATUS gid_find(char *filename,char *para)
{
    struct stat file_stat;
    int uid = atoi(para);
    if (stat(filename,&file_stat) == -1) {
            perror("stat file error\n");
            return false;
    }
    if (file_stat.st_uid != uid) {
        
            return false;
    }
    return true;
}

STATUS size_find(char *filename,char *para)
{
    if (filename == NULL)
        return false;
    struct stat file_stat;
    int filesize = atoi(para);
    filesize = filesize*1024; //转换成字节
    if (stat(filename,&file_stat) == -1) {
            perror("stat file error\n");
            return false;
    }
    if (file_stat.st_size > filesize) {
        return false;
    }
    return true;
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
         case 's': option_flag['s'] = 1; check_digit(optarg);option_arg['s'] = optarg;break;
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

    unsigned int option_len = sizeof(sfind_option);
    filter_all_file(".",option_len,option_flag,option_arg);
    return 0;

}


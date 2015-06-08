#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

bool str_split(const char *str,char*left,char *right,char c)
{
    char *ret = NULL;
    if((ret = strchr(str,c)) == NULL){
        left = NULL;
        right = NULL;
        return false;
    } else {
        strncpy(left,str,ret-str);
        strncpy(right,ret+1,(strlen(str)-(ret-str)-1));
    }   
    return true;
}

bool is_dir(char *filename)
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

void con_str(char *dest_buf,const char *fist_str,const char *last_str)
{
    strcat(dest_buf,fist_str);
    strcat(dest_buf,"/");
    strcat(dest_buf,last_str);
    return ;
}

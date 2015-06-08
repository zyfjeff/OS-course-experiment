#ifndef _LIB_COMMON_H_
#define _LIB_COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_NAME_SIZE 256
#define MAX_FILE_NUMBER 8192
#define MAX_BUFFER_SIZE 8192

#define ERRRET_EXIT(ret,len) do                             \
{                                                           \
    if (ret < 0 || (ret!=0 && ret !=len)) {                 \
        printf("error writen/readn \n");                    \
        exit(EXIT_FAILURE);                                 \
    }                                                       \
}while(0)

#define ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

bool str_split(const char *str,char*left,char *right,char c);
void con_str(char *dest_buf,const char *fist_str,const char *last_str);
bool is_dir(char *filename);

#endif //end of common

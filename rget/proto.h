#ifndef _SERVER_PROTO_H_
#define _SERVER_PROTO_H_

#include <stdint.h>


#define PACKET_DATA         0
#define PACKET_INIT         1
#define PACKET_ERROR        2
#define END_OF_FILE         3

#define DIR_TYPE            0
#define FILE_TYPE           1


#define RECUR_TRUE          0
#define RECUR_FALSE         1


union info{
    char filename[256]; //DIR_TYPE的时候，放的是目录名，FILE_TYPE的时候是文件名
    char errorinfo[256]; //错误信息
};

union flag{
    uint8_t filetype; //标明其类型 DIR_TYPE 或者是FILE_TYPE
    uint8_t recur;
};

struct packet
{
    uint16_t Totallen;     //包长
    uint16_t Type;         //类别 0 初始化 1数据 error
    union info content;    //内容信息，是错误信息，还是文件名信息
    union flag flaginfo;   //标志信息，标志是否递归拷贝 和 文件类型
 }__attribute__((packed));


struct conn
{
    uint8_t flag;     //是否递归拷贝
    uint16_t Totallen;  //文件名的长度
}__attribute__((packed));

//客户端发送一个conn给给服务器端，服务器端分析其flag和Totallen
//客户端紧接着发送文件名给服务器，服务器根据Totallen接收到文件名
//服务器分析文件类型，和存在性，还有递归拷贝的标志
//分为以下几种情形:
//1.文件存在，但是是目录，并且没有拷贝标志，那么返回错误信息
//2.文件不存在，直接返回错误信息
//3.文件存在，是普通文件，开始拷贝
//4.文件存在，但是目录，也有递归拷贝标志，那么开始递归拷贝

//拷贝过程
//1.普通文件拷贝
//服务器发送一个packet告诉开始传输的文件还是目录
//如果是目录，则TotalLen是0，客户端创建目录
//如果是文件，则客户端开始接收Totlalen长度的数据，写入到指定的文件中


#endif //end of 

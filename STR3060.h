#ifndef STR3060_H_INCLUDED
#define STR3060_H_INCLUDED
#include "STR3060_Helper.h"
#ifdef WIN32
#include "windows.h"
#else
#include "unistd.h"
#endif // WIN32
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


size_t     STR3060_Open(STR3060_Context *ctx,const char * device);//打开设备（其它文件）
size_t     STR3060_Close(STR3060_Context *ctx);//关闭设备
size_t     STR3060_Send_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//发送数据帧，返回发送的字节数(在其他文件实现)
size_t     STR3060_Receive_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//接收数据帧(不处理)，返回接收的字节数(在其它文件实现)

#ifdef __cplusplus
};
#endif // __cplusplus
#endif // STR3060_H_INCLUDED

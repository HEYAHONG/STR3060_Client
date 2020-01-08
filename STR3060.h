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


size_t     STR3060_Open(STR3060_Context *ctx,const char * device);//���豸�������ļ���
size_t     STR3060_Close(STR3060_Context *ctx);//�ر��豸
size_t     STR3060_Send_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//��������֡�����ط��͵��ֽ���(�������ļ�ʵ��)
size_t     STR3060_Receive_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//��������֡(������)�����ؽ��յ��ֽ���(�������ļ�ʵ��)

#ifdef __cplusplus
};
#endif // __cplusplus
#endif // STR3060_H_INCLUDED

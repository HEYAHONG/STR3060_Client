#include "STR3060.h"


size_t     STR3060_Open(STR3060_Context *ctx,const char * device)//打开设备
{
    if(ctx==NULL || device == NULL)
    {
        return 0;
    }
    size_t ret=0;
    STR3060_Close(ctx);
    #ifdef WIN32
    ctx->handle = CreateFileA(device,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		0);
    if(ctx->handle != INVALID_HANDLE_VALUE)//句柄有效
        {
        DCB dcbSerialParams={0};
        dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
        if (!GetCommState(ctx->handle, &dcbSerialParams))
            {
            STR3060_Close(ctx);
            return ret;
            }
        dcbSerialParams.BaudRate=115200;
        dcbSerialParams.ByteSize=8;
        dcbSerialParams.StopBits=ONESTOPBIT;
        dcbSerialParams.Parity=NOPARITY;
        if(!SetCommState(ctx->handle, &dcbSerialParams))
            {
            STR3060_Close(ctx);
            return ret;
            }
        COMMTIMEOUTS timeouts={0};
        timeouts.ReadIntervalTimeout=5;
        timeouts.ReadTotalTimeoutConstant=50;
        timeouts.ReadTotalTimeoutMultiplier=1;
        timeouts.WriteTotalTimeoutConstant=50;
        timeouts.WriteTotalTimeoutMultiplier=1;
        if(!SetCommTimeouts(ctx->handle, &timeouts))
            {
            STR3060_Close(ctx);
            return ret;
            }
	}

    #endif // WIN32

    //更新CTX
    ret=STR3060_Request_Data(ctx);
    STR3060_Receive_Data(ctx);
    STR3060_Query_Success(ctx);
    return ret;
}

size_t     STR3060_Close(STR3060_Context *ctx)//关闭设备
{
    if(ctx==NULL)
        return 0;
    if(ctx->handle==NULL)
        return 0;
    if(ctx->handle!=INVALID_HANDLE_VALUE)
    {
        CloseHandle(ctx->handle);
        ctx->handle=INVALID_HANDLE_VALUE;
    }
    return 1;//关闭成功
}

size_t     STR3060_Send_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length)//发送数据帧，返回发送的字节数
{
    if(ctx==NULL || packet ==NULL || length==0)
        return 0;
    size_t ret=0;
    #ifdef WIN32
    if(ctx->handle != NULL && ctx->handle != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesWrite = 0;
        if(WriteFile(ctx->handle, packet, length, &dwBytesWrite, NULL))
        {
            ret=dwBytesWrite;
        }
    }
    #endif // WIN32
    return ret;
}
size_t     STR3060_Receive_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length)//接收数据帧(不处理)，返回接收的字节数
{
    if(ctx==NULL || packet ==NULL || length==0)
        return 0;
    size_t ret=0;
    #ifdef WIN32
    if(ctx->handle != NULL && ctx->handle != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesRead = 0;
        if(ReadFile(ctx->handle, packet, length, &dwBytesRead, NULL))
        {
            ret=dwBytesRead;
        }
    }
    #endif // WIN32
    return ret;
}

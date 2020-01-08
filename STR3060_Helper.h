#ifndef STR3060_HELPER_H_INCLUDED
#define STR3060_HELPER_H_INCLUDED
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "limits.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*
#ifdef __GNUC__
#define __weak __attribute__((weak))
#else
#error not supported,不支持GCC以外的编译器!
#endif // __GNUC__
*/

//字节数据
typedef uint8_t STR3060_BYTE;

//DWORD数据
typedef union {
uint32_t dword;
uint8_t  byte[4];
} STR3060_DWORD;

typedef union {
int32_t dword;
uint8_t  byte[4];
} STR3060_DWORD_Signed;

STR3060_DWORD STR3060_Dword2Byte(STR3060_DWORD in);//将dowrd数据按字节序排列，用于填写字节数据
STR3060_DWORD STR3060_Byte2Dword(STR3060_DWORD in);//将字节序中的数据重新排列为Dword，用于读取Dword数据
STR3060_DWORD_Signed STR3060_D2DSigned(STR3060_DWORD in);//将无符号数直接转为有符号数
STR3060_DWORD STR3060_DSigned2D(STR3060_DWORD_Signed in);//将有符号数直接转为无符号数


//打包，解包数据帧
size_t STR3060_Packet_Pack(uint8_t *cmd,size_t cmd_length,uint8_t **packet);//打包数据包，返回打包后的大小，cmd=传入的命令（包含数据）指针，packet用于传出打包好的帧
size_t STR3060_Packet_Unpack(uint8_t *packet,size_t packet_length,uint8_t**cmd);//解包操作，返回命令的大小（注意：由于没有申请新内存，读取命令时必须保证数据帧有效）
void   STR3060_Packet_Free(uint8_t *packet);//释放帧所占内存



//STR3060设备上下文,用于保存STR3060状态，所有实际的操作都需要保存到此结构体中
typedef struct {
STR3060_BYTE Mode;//交直流模式。00=交流模式，01=直流模式
STR3060_BYTE Link_Mode;//接线方式。0=三相四线正相序，1=三相三线正相序，2=三相四线逆相序，3=三相三线逆相序

/*
量程限制:
电压限制 0=380V 1=220V 2=100V 3=57.7V 4=30V 5=600V
电流限制 0=20A  1=5A 2=1A 3=0.2A 4=10A 5=60A
*/
STR3060_BYTE UA_Limit;//UA量程
STR3060_BYTE UB_Limit;//UB量程
STR3060_BYTE UC_Limit;//UC量程
STR3060_BYTE IA_Limit;//IA量程
STR3060_BYTE IB_Limit;//IB量程
STR3060_BYTE IC_Limit;//IC量程

/*
电压电流幅度
*/
STR3060_DWORD UA;
STR3060_DWORD UB;
STR3060_DWORD UC;
STR3060_DWORD IA;
STR3060_DWORD IB;
STR3060_DWORD IC;

/*
电压电流相位。倍数为1000
*/
STR3060_DWORD_Signed UA_Phase;
STR3060_DWORD_Signed UB_Phase;
STR3060_DWORD_Signed UC_Phase;
STR3060_DWORD_Signed IA_Phase;
STR3060_DWORD_Signed IB_Phase;
STR3060_DWORD_Signed IC_Phase;

/*
频率.倍率为10000
*/
STR3060_DWORD Freq;

/*
有功功率。
*/
STR3060_DWORD_Signed PA;//A相功率
STR3060_DWORD_Signed PB;//B相功率
STR3060_DWORD_Signed PC;//C相功率
STR3060_DWORD_Signed P;//总功率

/*
无功功率
*/
STR3060_DWORD_Signed QA;//A相无功功率
STR3060_DWORD_Signed QB;//B相无功功率
STR3060_DWORD_Signed QC;//C相无功功率
STR3060_DWORD_Signed Q;//总无功功率

/*
视在功率
*/
STR3060_DWORD_Signed SA;//A相视在功率
STR3060_DWORD_Signed SB;//B相视在功率
STR3060_DWORD_Signed SC;//C相视在功率
STR3060_DWORD_Signed S;//总视在功率

/*
功率因数
*/
STR3060_DWORD_Signed COSA;//A相功率因数
STR3060_DWORD_Signed COSB;//B相功率因数
STR3060_DWORD_Signed COSC;//C相功率因数
STR3060_DWORD_Signed COS;//总功率因数

//打开的句柄/文件描述符,用于连接STR3060
#ifdef WIN32
void * handle;
#else
int fd;
#endif // WIN32

//回复处理的次数，可用于查询操作是否成功
long unsigned int reply_count;

} STR3060_Context;

typedef enum
{
 STR3060_A=1,
 STR3060_B,
 STR3060_C,
 STR3060_Total
} STR3060_SYM;//线的标志(A,B,C,Total(总共))

STR3060_Context * STR3060_Context_New();//获得新上下文
void STR3060_Context_Init(STR3060_Context * ctx);//初始化上下文
void STR3060_Context_Free(STR3060_Context *ctx);//清除上下文（需要手动关闭设备）

uint8_t STR3060_Process_Reply(STR3060_Context *ctx,uint8_t *cmd);//处理命令数据,将电源的返回数据（不包括头部，长度及校验位）进行处理，返回命令字节（返回0表示失败）
uint32_t STR3060_Get_KU(STR3060_Context *ctx,STR3060_SYM sym);//获得电压的系数（1000/10000）
uint32_t STR3060_Get_KI(STR3060_Context *ctx,STR3060_SYM sym);//获得电流的系数(10000/100000/1000000)
uint32_t STR3060_Get_KPhase(STR3060_Context *ctx);//获得相位的系数(1000)
uint32_t STR3060_Get_KP(STR3060_Context *ctx,STR3060_SYM sym);//获得功率(有功，无功，视在)的系数(100/1000/10000/100000)
uint32_t STR3060_Get_KCOS(STR3060_Context *ctx);//获得功率因数的系数(100000)
uint32_t STR3060_Get_KFreq(STR3060_Context *ctx);//获得频率值的系数(10000)

STR3060_BYTE STR3060_U2ULimit(long double U);//将电压转换为量程值
STR3060_BYTE STR3060_I2ILimit(long double I);//将电流转换为量程值
long double  STR3060_ULimit2U(STR3060_BYTE U);//将量程值转化为电压值
long double  STR3060_ILimit2I(STR3060_BYTE I);//将量程值转化为电流值

long double STR3060_Get_ULimit(STR3060_Context *ctx,STR3060_SYM sym);//获得量程电压值
long double STR3060_Get_ILimit(STR3060_Context *ctx,STR3060_SYM sym);//获得量程电流值
long double STR3060_Get_U(STR3060_Context *ctx,STR3060_SYM sym);//获得电压幅值
long double STR3060_Get_I(STR3060_Context *ctx,STR3060_SYM sym);//获得电流幅值
long double STR3060_Get_Freq(STR3060_Context *ctx);//获得频率值
long double STR3060_Get_U_Phase(STR3060_Context *ctx,STR3060_SYM sym);//获得电压相位
long double STR3060_Get_I_Phase(STR3060_Context *ctx,STR3060_SYM sym);//获得电流相位
long double STR3060_Get_P(STR3060_Context *ctx,STR3060_SYM sym);//获得有功功率
long double STR3060_Get_Q(STR3060_Context *ctx,STR3060_SYM sym);//获得无功功率
long double STR3060_Get_S(STR3060_Context *ctx,STR3060_SYM sym);//获得视在功率
long double STR3060_Get_COS(STR3060_Context *ctx,STR3060_SYM sym);//获得功率因数

long unsigned int STR3060_Query_Success(STR3060_Context *ctx);//查询上次操作是否成功，通过判断reply_count实现，0表示失败

extern size_t     STR3060_Send_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//发送数据帧，返回发送的字节数(在其他文件实现)
extern size_t     STR3060_Receive_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//接收数据帧(不处理)，返回接收的字节数(在其它文件实现)
extern size_t     STR3060_Close(STR3060_Context *ctx);//关闭设备


size_t  STR3060_Request_Data(STR3060_Context *ctx);//向设备发送请求数据命令
size_t  STR3060_Output_On(STR3060_Context *ctx);//向设备发送打开输出命令（必须先设置好各项参数）
size_t  STR3060_Output_Off(STR3060_Context *ctx);//向设备发送关闭输出命令
size_t  STR3060_Reset(STR3060_Context *ctx);//向设备发送复位命令

size_t  STR3060_Receive_Data(STR3060_Context *ctx);//接收设备返回的数据（响应帧）


size_t  STR3060_Set_Mode(STR3060_Context *ctx,STR3060_BYTE data);//设置交直流模式
size_t  STR3060_Set_LinkMode(STR3060_Context *ctx,STR3060_BYTE data);//设置连线模式
size_t  STR3060_Set_Limit(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC);//设置量程
size_t  STR3060_Set_Value(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC);//设置输出电压电流值
size_t  STR3060_Set_Phase(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC);//设置相位
size_t  STR3060_Set_Freq(STR3060_Context *ctx,long double Freq);//设置频率


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // STR3060_HELPER_H_INCLUDED

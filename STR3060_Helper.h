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
#error not supported,��֧��GCC����ı�����!
#endif // __GNUC__
*/

//�ֽ�����
typedef uint8_t STR3060_BYTE;

//DWORD����
typedef union {
uint32_t dword;
uint8_t  byte[4];
} STR3060_DWORD;

typedef union {
int32_t dword;
uint8_t  byte[4];
} STR3060_DWORD_Signed;

STR3060_DWORD STR3060_Dword2Byte(STR3060_DWORD in);//��dowrd���ݰ��ֽ������У�������д�ֽ�����
STR3060_DWORD STR3060_Byte2Dword(STR3060_DWORD in);//���ֽ����е�������������ΪDword�����ڶ�ȡDword����
STR3060_DWORD_Signed STR3060_D2DSigned(STR3060_DWORD in);//���޷�����ֱ��תΪ�з�����
STR3060_DWORD STR3060_DSigned2D(STR3060_DWORD_Signed in);//���з�����ֱ��תΪ�޷�����


//������������֡
size_t STR3060_Packet_Pack(uint8_t *cmd,size_t cmd_length,uint8_t **packet);//������ݰ������ش����Ĵ�С��cmd=���������������ݣ�ָ�룬packet���ڴ�������õ�֡
size_t STR3060_Packet_Unpack(uint8_t *packet,size_t packet_length,uint8_t**cmd);//�����������������Ĵ�С��ע�⣺����û���������ڴ棬��ȡ����ʱ���뱣֤����֡��Ч��
void   STR3060_Packet_Free(uint8_t *packet);//�ͷ�֡��ռ�ڴ�



//STR3060�豸������,���ڱ���STR3060״̬������ʵ�ʵĲ�������Ҫ���浽�˽ṹ����
typedef struct {
STR3060_BYTE Mode;//��ֱ��ģʽ��00=����ģʽ��01=ֱ��ģʽ
STR3060_BYTE Link_Mode;//���߷�ʽ��0=��������������1=��������������2=��������������3=��������������

/*
��������:
��ѹ���� 0=380V 1=220V 2=100V 3=57.7V 4=30V 5=600V
�������� 0=20A  1=5A 2=1A 3=0.2A 4=10A 5=60A
*/
STR3060_BYTE UA_Limit;//UA����
STR3060_BYTE UB_Limit;//UB����
STR3060_BYTE UC_Limit;//UC����
STR3060_BYTE IA_Limit;//IA����
STR3060_BYTE IB_Limit;//IB����
STR3060_BYTE IC_Limit;//IC����

/*
��ѹ��������
*/
STR3060_DWORD UA;
STR3060_DWORD UB;
STR3060_DWORD UC;
STR3060_DWORD IA;
STR3060_DWORD IB;
STR3060_DWORD IC;

/*
��ѹ������λ������Ϊ1000
*/
STR3060_DWORD_Signed UA_Phase;
STR3060_DWORD_Signed UB_Phase;
STR3060_DWORD_Signed UC_Phase;
STR3060_DWORD_Signed IA_Phase;
STR3060_DWORD_Signed IB_Phase;
STR3060_DWORD_Signed IC_Phase;

/*
Ƶ��.����Ϊ10000
*/
STR3060_DWORD Freq;

/*
�й����ʡ�
*/
STR3060_DWORD_Signed PA;//A�๦��
STR3060_DWORD_Signed PB;//B�๦��
STR3060_DWORD_Signed PC;//C�๦��
STR3060_DWORD_Signed P;//�ܹ���

/*
�޹�����
*/
STR3060_DWORD_Signed QA;//A���޹�����
STR3060_DWORD_Signed QB;//B���޹�����
STR3060_DWORD_Signed QC;//C���޹�����
STR3060_DWORD_Signed Q;//���޹�����

/*
���ڹ���
*/
STR3060_DWORD_Signed SA;//A�����ڹ���
STR3060_DWORD_Signed SB;//B�����ڹ���
STR3060_DWORD_Signed SC;//C�����ڹ���
STR3060_DWORD_Signed S;//�����ڹ���

/*
��������
*/
STR3060_DWORD_Signed COSA;//A�๦������
STR3060_DWORD_Signed COSB;//B�๦������
STR3060_DWORD_Signed COSC;//C�๦������
STR3060_DWORD_Signed COS;//�ܹ�������

//�򿪵ľ��/�ļ�������,��������STR3060
#ifdef WIN32
void * handle;
#else
int fd;
#endif // WIN32

//�ظ�����Ĵ����������ڲ�ѯ�����Ƿ�ɹ�
long unsigned int reply_count;

} STR3060_Context;

typedef enum
{
 STR3060_A=1,
 STR3060_B,
 STR3060_C,
 STR3060_Total
} STR3060_SYM;//�ߵı�־(A,B,C,Total(�ܹ�))

STR3060_Context * STR3060_Context_New();//�����������
void STR3060_Context_Init(STR3060_Context * ctx);//��ʼ��������
void STR3060_Context_Free(STR3060_Context *ctx);//��������ģ���Ҫ�ֶ��ر��豸��

uint8_t STR3060_Process_Reply(STR3060_Context *ctx,uint8_t *cmd);//������������,����Դ�ķ������ݣ�������ͷ�������ȼ�У��λ�����д������������ֽڣ�����0��ʾʧ�ܣ�
uint32_t STR3060_Get_KU(STR3060_Context *ctx,STR3060_SYM sym);//��õ�ѹ��ϵ����1000/10000��
uint32_t STR3060_Get_KI(STR3060_Context *ctx,STR3060_SYM sym);//��õ�����ϵ��(10000/100000/1000000)
uint32_t STR3060_Get_KPhase(STR3060_Context *ctx);//�����λ��ϵ��(1000)
uint32_t STR3060_Get_KP(STR3060_Context *ctx,STR3060_SYM sym);//��ù���(�й����޹�������)��ϵ��(100/1000/10000/100000)
uint32_t STR3060_Get_KCOS(STR3060_Context *ctx);//��ù���������ϵ��(100000)
uint32_t STR3060_Get_KFreq(STR3060_Context *ctx);//���Ƶ��ֵ��ϵ��(10000)

STR3060_BYTE STR3060_U2ULimit(long double U);//����ѹת��Ϊ����ֵ
STR3060_BYTE STR3060_I2ILimit(long double I);//������ת��Ϊ����ֵ
long double  STR3060_ULimit2U(STR3060_BYTE U);//������ֵת��Ϊ��ѹֵ
long double  STR3060_ILimit2I(STR3060_BYTE I);//������ֵת��Ϊ����ֵ

long double STR3060_Get_ULimit(STR3060_Context *ctx,STR3060_SYM sym);//������̵�ѹֵ
long double STR3060_Get_ILimit(STR3060_Context *ctx,STR3060_SYM sym);//������̵���ֵ
long double STR3060_Get_U(STR3060_Context *ctx,STR3060_SYM sym);//��õ�ѹ��ֵ
long double STR3060_Get_I(STR3060_Context *ctx,STR3060_SYM sym);//��õ�����ֵ
long double STR3060_Get_Freq(STR3060_Context *ctx);//���Ƶ��ֵ
long double STR3060_Get_U_Phase(STR3060_Context *ctx,STR3060_SYM sym);//��õ�ѹ��λ
long double STR3060_Get_I_Phase(STR3060_Context *ctx,STR3060_SYM sym);//��õ�����λ
long double STR3060_Get_P(STR3060_Context *ctx,STR3060_SYM sym);//����й�����
long double STR3060_Get_Q(STR3060_Context *ctx,STR3060_SYM sym);//����޹�����
long double STR3060_Get_S(STR3060_Context *ctx,STR3060_SYM sym);//������ڹ���
long double STR3060_Get_COS(STR3060_Context *ctx,STR3060_SYM sym);//��ù�������

long unsigned int STR3060_Query_Success(STR3060_Context *ctx);//��ѯ�ϴβ����Ƿ�ɹ���ͨ���ж�reply_countʵ�֣�0��ʾʧ��

extern size_t     STR3060_Send_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//��������֡�����ط��͵��ֽ���(�������ļ�ʵ��)
extern size_t     STR3060_Receive_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length);//��������֡(������)�����ؽ��յ��ֽ���(�������ļ�ʵ��)
extern size_t     STR3060_Close(STR3060_Context *ctx);//�ر��豸


size_t  STR3060_Request_Data(STR3060_Context *ctx);//���豸����������������
size_t  STR3060_Output_On(STR3060_Context *ctx);//���豸���ʹ����������������úø��������
size_t  STR3060_Output_Off(STR3060_Context *ctx);//���豸���͹ر��������
size_t  STR3060_Reset(STR3060_Context *ctx);//���豸���͸�λ����

size_t  STR3060_Receive_Data(STR3060_Context *ctx);//�����豸���ص����ݣ���Ӧ֡��


size_t  STR3060_Set_Mode(STR3060_Context *ctx,STR3060_BYTE data);//���ý�ֱ��ģʽ
size_t  STR3060_Set_LinkMode(STR3060_Context *ctx,STR3060_BYTE data);//��������ģʽ
size_t  STR3060_Set_Limit(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC);//��������
size_t  STR3060_Set_Value(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC);//���������ѹ����ֵ
size_t  STR3060_Set_Phase(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC);//������λ
size_t  STR3060_Set_Freq(STR3060_Context *ctx,long double Freq);//����Ƶ��


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // STR3060_HELPER_H_INCLUDED

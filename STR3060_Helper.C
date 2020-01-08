#include "STR3060_Helper.h"
STR3060_DWORD STR3060_Dword2Byte(STR3060_DWORD in)//将dowrd数据按字节序排列，用于填写字节数据
{
    STR3060_DWORD ret=in,Test;

    Test.dword=1;
    if(Test.byte[3])//判断大小端
    {//大端
        ret.byte[0]=in.byte[3];
        ret.byte[1]=in.byte[2];
        ret.byte[2]=in.byte[1];
        ret.byte[3]=in.byte[0];
    }

    return ret;
}
STR3060_DWORD STR3060_Byte2Dword(STR3060_DWORD in)//将字节序中的数据重新排列为Dword，用于读取Dword数据
{
    STR3060_DWORD ret=in,Test;

    Test.dword=1;
    if(Test.byte[3])//判断大小端
    {//大端
        ret.byte[0]=in.byte[3];
        ret.byte[1]=in.byte[2];
        ret.byte[2]=in.byte[1];
        ret.byte[3]=in.byte[0];
    }

    return ret;
}

STR3060_DWORD_Signed STR3060_D2DSigned(STR3060_DWORD in)//将无符号数直接转为有符号数
{
    STR3060_DWORD_Signed ret;
    memcpy(&ret,&in,sizeof(ret));
    return ret;
}

STR3060_DWORD STR3060_DSigned2D(STR3060_DWORD_Signed in)//将有符号数直接转为无符号数
{
    STR3060_DWORD ret;
    memcpy(&ret,&in,sizeof(ret));
    return ret;
}
size_t STR3060_Packet_Pack(uint8_t *cmd,size_t cmd_length,uint8_t **packet)//打包数据包，返回打包后的大小，cmd=传入的命令（包含数据）指针，packet用于传出打包好的帧
{

        if(cmd==NULL || packet ==NULL)
            return 0;

        size_t packet_length=cmd_length+5;
        uint8_t * packet_ptr=(uint8_t *)malloc(packet_length);
        (*packet)=packet_ptr;
        {//填写头部
            packet_ptr[0]=0x81;
            packet_ptr[1]=0x00;
        }
        {//填写数据长度
            packet_ptr[2]=(packet_length&0xff);
            packet_ptr[3]=((packet_length>>8)&0xff);
        }
        {//复制命令数据
            memcpy(&packet_ptr[4],cmd,cmd_length);
        }
        {//添加校验位
            uint8_t check=0;
            for(size_t i=0;i<packet_length;i++)
            {
                if(i>0 && i<(packet_length-1))
                {
                    check^=packet_ptr[i];
                }
                if(i==(packet_length-1))
                {
                    packet_ptr[i]=check;
                }
            }
        }

        return packet_length;
}

size_t STR3060_Packet_Unpack(uint8_t *packet,size_t packet_length,uint8_t**cmd)//解包操作，返回命令的大小（注意：由于没有申请新内存，读取命令时必须保证数据帧有效）
{
    if(packet==NULL || cmd==NULL || packet_length <6)
        return 0;

    size_t packet_length_2=0;
    {
        packet_length_2=packet[2]+(((size_t)packet[3])<<8);
    }

    if(packet_length_2 > packet_length)
    {//数据包不完整
        return 0;
    }
    if(packet_length_2< 6)
    {//数据包长度异常
        return 0;
    }

    size_t cmd_length=packet_length_2-5;
    (*cmd)=&packet[4];

    {//校验校验位
            uint8_t check=0;
            for(size_t i=0;i<packet_length_2;i++)
            {
                if(i>1 && i<(packet_length_2-1))
                {
                    check^=packet[i];
                }
                if(i==(packet_length_2-1))
                {
                    if(packet[i]!=check && packet_length_2!=128)//不校验查询返回数据
                        cmd_length=0;
                }
            }
        }
    return cmd_length;
}
void   STR3060_Packet_Free(uint8_t *packet)//释放帧所占内存
{
    if(packet == NULL)
        return;
    free(packet);
}

STR3060_Context * STR3060_Context_New()//获得新上下文
{
    STR3060_Context * ctx=(STR3060_Context *)malloc(sizeof(STR3060_Context));
    STR3060_Context_Init(ctx);
    return ctx;
}

void STR3060_Context_Init(STR3060_Context * ctx)
{
    memset(ctx,0,sizeof(STR3060_Context));
    #ifdef WIN32
    ctx->handle=NULL;
    #else
    ctx->fd=-1;
    #endif // WIN32
}

void STR3060_Context_Free(STR3060_Context *ctx)
{
        STR3060_Close(ctx);
        free(ctx);
}

uint8_t STR3060_Process_Reply(STR3060_Context *ctx,uint8_t *cmd)//处理命令数据,将电源的返回的数据（不包括头部，长度及校验位）进行处理，返回命令字节（返回0表示失败）
{
    uint8_t ret=0;
    if(ctx !=NULL && cmd!=NULL)
    {
        if(cmd[0]==0x4B)
            ret=0x4B;
        if(cmd[0]==0x4D)
        {
            ret=0x4D;
            {//填写接收数据
                memcpy(ctx->Freq.byte,&cmd[1],4);

                ctx->UA_Limit=cmd[5];
                ctx->UB_Limit=cmd[6];
                ctx->UC_Limit=cmd[7];
                ctx->IA_Limit=cmd[8];
                ctx->IB_Limit=cmd[9];
                ctx->IC_Limit=cmd[10];

                memcpy(ctx->UA.byte,&cmd[11],4);
                memcpy(ctx->UB.byte,&cmd[15],4);
                memcpy(ctx->UC.byte,&cmd[19],4);
                memcpy(ctx->IA.byte,&cmd[23],4);
                memcpy(ctx->IB.byte,&cmd[27],4);
                memcpy(ctx->IC.byte,&cmd[31],4);

                memcpy(ctx->UA_Phase.byte,&cmd[35],4);
                memcpy(ctx->UB_Phase.byte,&cmd[39],4);
                memcpy(ctx->UC_Phase.byte,&cmd[43],4);
                memcpy(ctx->IA_Phase.byte,&cmd[47],4);
                memcpy(ctx->IB_Phase.byte,&cmd[51],4);
                memcpy(ctx->IC_Phase.byte,&cmd[55],4);

                memcpy(ctx->PA.byte,&cmd[59],4);
                memcpy(ctx->PB.byte,&cmd[63],4);
                memcpy(ctx->PC.byte,&cmd[67],4);
                memcpy(ctx->P.byte,&cmd[71],4);

                memcpy(ctx->QA.byte,&cmd[75],4);
                memcpy(ctx->QB.byte,&cmd[79],4);
                memcpy(ctx->QC.byte,&cmd[83],4);
                memcpy(ctx->Q.byte,&cmd[87],4);

                memcpy(ctx->SA.byte,&cmd[91],4);
                memcpy(ctx->SB.byte,&cmd[95],4);
                memcpy(ctx->SC.byte,&cmd[99],4);
                memcpy(ctx->S.byte,&cmd[103],4);

                memcpy(ctx->COSA.byte,&cmd[107],4);
                memcpy(ctx->COSB.byte,&cmd[111],4);
                memcpy(ctx->COSC.byte,&cmd[115],4);
                memcpy(ctx->COS.byte,&cmd[119],4);
            }
            {//转换字节序
                ctx->Freq=STR3060_Byte2Dword(ctx->Freq);

                ctx->UA=STR3060_Byte2Dword(ctx->UA);
                ctx->UB=STR3060_Byte2Dword(ctx->UB);
                ctx->UC=STR3060_Byte2Dword(ctx->UC);
                ctx->IA=STR3060_Byte2Dword(ctx->IA);
                ctx->IB=STR3060_Byte2Dword(ctx->IB);
                ctx->IC=STR3060_Byte2Dword(ctx->IC);

                ctx->UA_Phase=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->UA_Phase)));
                ctx->UB_Phase=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->UB_Phase)));
                ctx->UC_Phase=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->UC_Phase)));
                ctx->IA_Phase=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->IA_Phase)));
                ctx->IB_Phase=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->IB_Phase)));
                ctx->IC_Phase=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->IC_Phase)));

                ctx->PA=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D( ctx->PA)));
                ctx->PB=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->PB)));
                ctx->PC=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->PC)));
                ctx->P=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->P)));

                ctx->QA=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->QA)));
                ctx->QB=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->QB)));
                ctx->QC=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->QC)));
                ctx->Q=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->Q)));

                ctx->SA=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->SA)));
                ctx->SB=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->SB)));
                ctx->SC=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->SC)));
                ctx->S=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->S)));

                ctx->COSA=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->COSA)));
                ctx->COSB=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->COSB)));
                ctx->COSC=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->COSC)));
                ctx->COS=STR3060_D2DSigned(STR3060_Byte2Dword(STR3060_DSigned2D(ctx->COS)));
            }
        }
    }

    if(ret!=0)
        ctx->reply_count++;

    return ret;
}

uint32_t STR3060_Get_KU(STR3060_Context *ctx,STR3060_SYM sym)//获得电压的系数（1000/10000）
{
   uint32_t ret=1000;
    if(ctx==NULL)
        ret=0;
    else
    {
        switch(sym)
        {
            default:
                break;
            case STR3060_A://A相
                if(ctx->UA_Limit== 3 || ctx->UA_Limit== 4 )
                {//30V,57.7档
                    ret=10000;
                }
                else
                {
                    ret=1000;
                }
                break;
            case STR3060_B://B相
                if(ctx->UB_Limit== 3 || ctx->UB_Limit== 4 )
                {//30V,57.7档
                    ret=10000;
                }
                else
                {
                    ret=1000;
                }
                break;
            case STR3060_C://C相
                if(ctx->UC_Limit== 3 || ctx->UC_Limit== 4 )
                {//30V,57.7档
                    ret=10000;
                }
                else
                {
                    ret=1000;
                }
                break;
        }
    }
    return ret;
}
uint32_t STR3060_Get_KI(STR3060_Context *ctx,STR3060_SYM sym)//获得电流的系数(10000/100000/1000000)
{
 uint32_t ret=10000;
    if(ctx==NULL)
        ret=0;
    else
    {
        switch(sym)
        {
            default:
                break;
            case STR3060_A://A相
                if(ctx->IA_Limit== 3)
                {//0.2A档
                    ret=1000000;
                }
                else
                {
                    if(ctx->IA_Limit==1 || ctx->IA_Limit == 2)
                    {//5A/1A档
                        ret=100000;
                    }
                    else
                    {
                        ret=10000;
                    }
                }
                break;
            case STR3060_B://B相
               if(ctx->IB_Limit== 3)
                {//0.2A档
                    ret=1000000;
                }
                else
                {
                    if(ctx->IB_Limit==1 || ctx->IB_Limit == 2)
                    {//5A/1A档
                        ret=100000;
                    }
                    else
                    {
                        ret=10000;
                    }
                }
                break;
            case STR3060_C://C相
                if(ctx->IC_Limit== 3)
                {//0.2A档
                    ret=1000000;
                }
                else
                {
                    if(ctx->IC_Limit==1 || ctx->IC_Limit == 2)
                    {//5A/1A档
                        ret=100000;
                    }
                    else
                    {
                        ret=10000;
                    }
                }
                break;
        }
    }
    return ret;
}
uint32_t STR3060_Get_KPhase(STR3060_Context *ctx)//获得相位的系数(1000)
{
    uint32_t ret=1000;
    if(ctx==NULL)
        ret=0;
    return ret;
}
uint32_t STR3060_Get_KP(STR3060_Context *ctx,STR3060_SYM sym)//获得功率(有功，无功，视在)的系数(100/1000/10000/100000)
{
    uint32_t ret=100;
    if(ctx ==NULL)
        return 0;
    uint8_t U_Limit=0,I_Limit=0;
    switch(sym)
    {
        default:
        case STR3060_A:
                U_Limit=ctx->UA_Limit;
                I_Limit=ctx->IA_Limit;
                break;
        case STR3060_B:
                U_Limit=ctx->UA_Limit;
                I_Limit=ctx->IA_Limit;
                break;
        case STR3060_C:
                U_Limit=ctx->UA_Limit;
                I_Limit=ctx->IA_Limit;
                break;

    }

    if(U_Limit>5 || I_Limit>5)
        return 0;
    /*
    电压限制 0=380V 1=220V 2=100V 3=57.7V 4=30V 5=600V
    电流限制 0=20A  1=5A 2=1A 3=0.2A 4=10A 5=60A

    */
    uint32_t table[6][6]=
    {
      {100,100,100,100,1000,100},
      {100,100,1000,1000,1000,100},
      {1000,1000,1000,10000,10000,1000},
      {10000,10000,10000,10000,100000,10000},
      {100,100,100,1000,1000,100},
      {100,100,100,100,100,100},
    };
    ret=table[U_Limit][I_Limit];
    return ret;
}
uint32_t STR3060_Get_KCOS(STR3060_Context *ctx)//获得功率因数的系数(100000)
{
    uint32_t ret=100000;
    if(ctx==NULL)
        ret=0;
    return ret;
}
uint32_t STR3060_Get_KFreq(STR3060_Context *ctx)//获得频率值的系数(10000)
{
    uint32_t ret=10000;
    if(ctx==NULL)
        ret=0;
    return ret;
}

/*
    电压限制 0=380V 1=220V 2=100V 3=57.7V 4=30V 5=600V
    电流限制 0=20A  1=5A 2=1A 3=0.2A 4=10A 5=60A
*/
STR3060_BYTE STR3060_U2ULimit(long double U)//将电压转换为量程值
{
    STR3060_BYTE ret=5;
    if(U<=601)
        ret=5;
    if(U<381)
        ret=0;
    if(U<221)
        ret=1;
    if(U<101)
        ret=2;
    if(U<58.7)
        ret=3;
    if(U<31)
        ret=4;
    return ret;
}
STR3060_BYTE STR3060_I2ILimit(long double I)//将电流转换为量程值
{
    STR3060_BYTE ret=5;
    if(I<61)
        ret=5;
    if(I<21)
        ret=0;
    if(I<11)
        ret=4;
    if(I<5.1)
        ret=1;
    if(I<1.01)
        ret=2;
    if(I<0.201)
        ret=3;
    return ret;
}
long double  STR3060_ULimit2U(STR3060_BYTE U)//将量程值转化为电压值
{
    if(U>5)
        return 0.0;
    long double table[6]={380.0,220.0,100.0,57.7,30.0,600.0};
    return table[U];
}
long double  STR3060_ILimit2I(STR3060_BYTE I)//将量程值转化为电流值
{
    if(I>5)
        return 0.0;
    long double table[6]={20.0,5.0,1.0,0.2,10.0,20.0};
    return table[I];
}
long double STR3060_Get_ULimit(STR3060_Context *ctx,STR3060_SYM sym)//获得量程电压值
{
    if(ctx==NULL)
        return 0.0;
    switch(sym)
    {
        default:
        case STR3060_A:
            return STR3060_ULimit2U(ctx->UA_Limit);
            break;
        case STR3060_B:
            return STR3060_ULimit2U(ctx->UB_Limit);
            break;
        case STR3060_C:
            return STR3060_ULimit2U(ctx->UC_Limit);
            break;

    }
}
long double STR3060_Get_ILimit(STR3060_Context *ctx,STR3060_SYM sym)//获得量程电流值
{
    if(ctx==NULL)
        return 0.0;
    switch(sym)
    {
        default:
        case STR3060_A:
            return STR3060_ILimit2I(ctx->IA_Limit);
            break;
        case STR3060_B:
            return STR3060_ILimit2I(ctx->IB_Limit);
            break;
        case STR3060_C:
            return STR3060_ILimit2I(ctx->IC_Limit);
            break;

    }
}
long double STR3060_Get_U(STR3060_Context *ctx,STR3060_SYM sym)//获得电压幅值
 {
     if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
        case STR3060_A:
            return ctx->UA.dword/((long double)(STR3060_Get_KU(ctx,sym)));
            break;
        case STR3060_B:
            return ctx->UB.dword/((long double)(STR3060_Get_KU(ctx,sym)));
            break;
        case STR3060_C:
            return ctx->UC.dword/((long double)(STR3060_Get_KU(ctx,sym)));
            break;

    }
 }
long double STR3060_Get_I(STR3060_Context *ctx,STR3060_SYM sym)//获得电流幅值
{
    if(ctx==NULL)
        return 0.0;
    switch(sym)
    {
        default:
        case STR3060_A:
            return ctx->IA.dword/(long double)(STR3060_Get_KI(ctx,sym));
            break;
        case STR3060_B:
            return ctx->IB.dword/(long double)(STR3060_Get_KI(ctx,sym));
            break;
        case STR3060_C:
            return ctx->IC.dword/(long double)(STR3060_Get_KI(ctx,sym));
            break;

    }
}

long double STR3060_Get_Freq(STR3060_Context *ctx)//获得频率值
{
   if(ctx==NULL)
        return 0.0;
   return ctx->Freq.dword/(long double)(STR3060_Get_KFreq(ctx));
}

long double STR3060_Get_U_Phase(STR3060_Context *ctx,STR3060_SYM sym)//获得电压幅值相位
{
    if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
        case STR3060_A:
            return ctx->UA_Phase.dword/((long double)(STR3060_Get_KPhase(ctx)));
            break;
        case STR3060_B:
            return ctx->UB_Phase.dword/((long double)(STR3060_Get_KPhase(ctx)));
            break;
        case STR3060_C:
            return ctx->UC_Phase.dword/((long double)(STR3060_Get_KPhase(ctx)));
            break;

    }
}
long double STR3060_Get_I_Phase(STR3060_Context *ctx,STR3060_SYM sym)//获得电流幅值相位
{
    if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
        case STR3060_A:
            return ctx->IA_Phase.dword/((long double)(STR3060_Get_KPhase(ctx)));
            break;
        case STR3060_B:
            return ctx->IB_Phase.dword/((long double)(STR3060_Get_KPhase(ctx)));
            break;
        case STR3060_C:
            return ctx->IC_Phase.dword/((long double)(STR3060_Get_KPhase(ctx)));
            break;

    }
}

long double STR3060_Get_P(STR3060_Context *ctx,STR3060_SYM sym)//获得有功功率
{
    if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
            return ctx->P.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_A:
            return ctx->PA.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_B:
            return ctx->PB.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_C:
            return ctx->PC.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
    }
}
long double STR3060_Get_Q(STR3060_Context *ctx,STR3060_SYM sym)//获得无功功率
{
    if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
            return ctx->Q.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_A:
            return ctx->QA.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_B:
            return ctx->QB.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_C:
            return ctx->QC.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
    }
}
long double STR3060_Get_S(STR3060_Context *ctx,STR3060_SYM sym)//获得视在功率
{
    if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
            return ctx->S.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_A:
            return ctx->SA.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_B:
            return ctx->SB.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
        case STR3060_C:
            return ctx->SC.dword/((long double)(STR3060_Get_KP(ctx,sym)));
            break;
    }
}
long double STR3060_Get_COS(STR3060_Context *ctx,STR3060_SYM sym)//获得功率因数
{
    if(ctx==NULL)
        return 0.0;
     switch(sym)
    {
        default:
            return ctx->COS.dword/((long double)(STR3060_Get_KCOS(ctx)));
            break;
        case STR3060_A:
            return ctx->COSA.dword/((long double)(STR3060_Get_KCOS(ctx)));
            break;
        case STR3060_B:
            return ctx->COSB.dword/((long double)(STR3060_Get_KCOS(ctx)));
            break;
        case STR3060_C:
            return ctx->COSC.dword/((long double)(STR3060_Get_KCOS(ctx)));
            break;
    }
}
/*
size_t  __weak    STR3060_Open(STR3060_Context *ctx,char * device)//打开设备（其它文件）
{
    return 0;
}
size_t  __weak   STR3060_Send_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length)//发送数据帧，返回发送的字节数(在其他文件实现)
{
    return 0;
}
size_t  __weak   STR3060_Receive_Packet(STR3060_Context *ctx,uint8_t * packet,size_t length)//接收数据帧(不处理)，返回接收的字节数(在其它文件实现)
{
    return 0;
}
*/

long unsigned int STR3060_Query_Success(STR3060_Context *ctx)//查询上次操作是否成功，通过判断reply_count实现，0表示失败
{
    if(ctx==NULL)
        return 0;
    static long unsigned int last_count=0;
    long unsigned int ret=0,count=ctx->reply_count;
    if(count>=last_count)
        ret=count-last_count;
    else
        ret=(ULONG_MAX-last_count+1)+count;

    last_count=count;
    return ret;
}

size_t  STR3060_Request_Data(STR3060_Context *ctx)//向设备发送请求数据命令
{
    if(ctx==NULL)
        return 0;
    uint8_t cmd[1]={0x4D};//请求数据命令
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;
}

size_t  STR3060_Output_On(STR3060_Context *ctx)//向设备发送打开输出命令（必须先设置好各项参数）
{
   if(ctx==NULL)
        return 0;
    uint8_t cmd[1]={0x54};//打开输出命令
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;
}
size_t  STR3060_Output_Off(STR3060_Context *ctx)//向设备发送关闭输出命令
{
    if(ctx==NULL)
        return 0;
    uint8_t cmd[1]={0x4F};//关闭输出命令
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;
}
size_t  STR3060_Reset(STR3060_Context *ctx)//向设备发送复位命令
{
    if(ctx==NULL)
        return 0;
    uint8_t cmd[1]={0x52};//复位命令
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;
}

size_t  STR3060_Receive_Data(STR3060_Context *ctx)//接收设备返回的数据（响应帧）
{
    if(ctx==NULL)
        return 0;
    uint8_t packet[256]={0};//接收缓冲
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Receive_Packet(ctx,packet,sizeof(packet))))
    {
        uint8_t *cmd=NULL;
        size_t  cmd_length=0;
        cmd_length=STR3060_Packet_Unpack(packet,packet_length,&cmd);
        if(cmd_length == 0)
                return 0;
        if(STR3060_Process_Reply(ctx,cmd))
        {
                ret=cmd_length;
        }
    }
    return ret;
}

size_t  STR3060_Set_Mode(STR3060_Context *ctx,STR3060_BYTE data)//设置交直流模式
{
    if(ctx==NULL)
        return 0;
    STR3060_BYTE mode=0;
    if(data>0)
        mode=0x01;//直流模式
    ctx->Mode=mode;
    uint8_t cmd[2]={0x30,0x00};
    cmd[1]=mode;
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;

}

size_t  STR3060_Set_LinkMode(STR3060_Context *ctx,STR3060_BYTE data)//设置连线模式
{
    if(ctx==NULL)
        return 0;
    STR3060_BYTE mode=data;
    if(data>3)
        mode=0;
    ctx->Link_Mode=mode;
    uint8_t cmd[2]={0x35,0x00};
    cmd[1]=mode;
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;

}

size_t  STR3060_Set_Limit(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC)//设置量程
{
    if(ctx==NULL)
        return 0;
    if(UA>0)
        ctx->UA_Limit=STR3060_U2ULimit(UA);
    if(UB>0)
        ctx->UB_Limit=STR3060_U2ULimit(UB);
    if(UC>0)
        ctx->UC_Limit=STR3060_U2ULimit(UC);
    if(IA>0)
        ctx->IA_Limit=STR3060_I2ILimit(IA);
    if(IB>0)
        ctx->IB_Limit=STR3060_I2ILimit(IB);
    if(IC>0)
        ctx->IC_Limit=STR3060_I2ILimit(IC);
    uint8_t cmd[7]={0x31,0x00,0x00,0x00,0x00,0x00,0x00};
    cmd[1]=ctx->UA_Limit;
    cmd[2]=ctx->UB_Limit;
    cmd[3]=ctx->UC_Limit;
    cmd[4]=ctx->IA_Limit;
    cmd[5]=ctx->IB_Limit;
    cmd[6]=ctx->IC_Limit;
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;

}

size_t  STR3060_Set_Value(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC)//设置输出电压电流值
{
    if(ctx==NULL)
        return 0;
    STR3060_DWORD UA_d,UB_d,UC_d,IA_d,IB_d,IC_d;
    if(UA>=0)
    {
        ctx->UA.dword=(uint32_t)(UA*STR3060_Get_KU(ctx,STR3060_A));
    }
    UA_d=STR3060_Dword2Byte(ctx->UA);
    if(UB>=0)
    {
        ctx->UB.dword=(uint32_t)(UB*STR3060_Get_KU(ctx,STR3060_B));
    }
    UB_d=STR3060_Dword2Byte(ctx->UB);
    if(UC>=0)
    {
        ctx->UC.dword=(uint32_t)(UC*STR3060_Get_KU(ctx,STR3060_C));
    }
    UC_d=STR3060_Dword2Byte(ctx->UC);

    if(IA>=0)
    {
        ctx->IA.dword=(uint32_t)(IA*STR3060_Get_KI(ctx,STR3060_A));
    }
    IA_d=STR3060_Dword2Byte(ctx->IA);
    if(IB>=0)
    {
        ctx->IB.dword=(uint32_t)(IB*STR3060_Get_KI(ctx,STR3060_B));
    }
    IB_d=STR3060_Dword2Byte(ctx->IB);
    if(IC>=0)
    {
        ctx->IC.dword=(uint32_t)(IC*STR3060_Get_KI(ctx,STR3060_C));
    }
    IC_d=STR3060_Dword2Byte(ctx->IC);

    uint8_t cmd[25]={0x32};
    cmd[0]=0x32;
    memcpy(&cmd[1],UA_d.byte,4);
    memcpy(&cmd[5],UB_d.byte,4);
    memcpy(&cmd[9],UC_d.byte,4);
    memcpy(&cmd[13],IA_d.byte,4);
    memcpy(&cmd[17],IB_d.byte,4);
    memcpy(&cmd[21],IC_d.byte,4);
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;
}

size_t  STR3060_Set_Phase(STR3060_Context *ctx,long double UA,long double UB,long double UC,long double IA,long double IB,long double IC)//设置相位
{
   if(ctx==NULL)
        return 0;
   long double UA_P=UA,UB_P=UB,UC_P=UC,IA_P=IA,IB_P=IB,IC_P=IC;
   STR3060_DWORD_Signed UA_d,UB_d,UC_d,IA_d,IB_d,IC_d;
   if(UA_P>=0)
   {
       while(UA_P>=360.0)
            UA_P-=360.0;
       ctx->UA_Phase.dword=(uint32_t)(UA_P*STR3060_Get_KPhase(ctx));
   }
   UA_d=STR3060_D2DSigned(STR3060_Dword2Byte(STR3060_DSigned2D(ctx->UA_Phase)));
   if(UB_P>=0)
   {
       while(UB_P>=360.0)
            UB_P-=360.0;
       ctx->UB_Phase.dword=(uint32_t)(UB_P*STR3060_Get_KPhase(ctx));
   }
   UB_d=STR3060_D2DSigned(STR3060_Dword2Byte(STR3060_DSigned2D(ctx->UB_Phase)));
   if(UC_P>=0)
   {
       while(UC_P>=360.0)
            UC_P-=360.0;
       ctx->UC_Phase.dword=(uint32_t)(UC_P*STR3060_Get_KPhase(ctx));
   }
   UC_d=STR3060_D2DSigned(STR3060_Dword2Byte(STR3060_DSigned2D(ctx->UC_Phase)));

   if(IA_P>=0)
   {
       while(IA_P>=360.0)
            IA_P-=360.0;
       ctx->IA_Phase.dword=(uint32_t)(IA_P*STR3060_Get_KPhase(ctx));
   }
   IA_d=STR3060_D2DSigned(STR3060_Dword2Byte(STR3060_DSigned2D(ctx->IA_Phase)));
   if(IB_P>=0)
   {
       while(IB_P>=360.0)
            IB_P-=360.0;
       ctx->IB_Phase.dword=(uint32_t)(IB_P*STR3060_Get_KPhase(ctx));
   }
   IB_d=STR3060_D2DSigned(STR3060_Dword2Byte(STR3060_DSigned2D(ctx->IB_Phase)));
   if(IC_P>=0)
   {
       while(IC_P>=360.0)
            IC_P-=360.0;
       ctx->IC_Phase.dword=(uint32_t)(IC_P*STR3060_Get_KPhase(ctx));
   }
   IC_d=STR3060_D2DSigned(STR3060_Dword2Byte(STR3060_DSigned2D(ctx->IC_Phase)));

    uint8_t cmd[25]={0x33};
    cmd[0]=0x33;
    memcpy(&cmd[1],UA_d.byte,4);
    memcpy(&cmd[5],UB_d.byte,4);
    memcpy(&cmd[9],UC_d.byte,4);
    memcpy(&cmd[13],IA_d.byte,4);
    memcpy(&cmd[17],IB_d.byte,4);
    memcpy(&cmd[21],IC_d.byte,4);
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;

}

size_t  STR3060_Set_Freq(STR3060_Context *ctx,long double Freq)//设置频率
{
   if(ctx==NULL)
        return 0;
   if(Freq >0)
   {
       ctx->Freq.dword=(uint32_t)(Freq*STR3060_Get_KFreq(ctx));
   }
    STR3060_DWORD Freq_d=STR3060_Dword2Byte(ctx->Freq);
    uint8_t cmd[5]={0x34};
    cmd[0]=0x34;
    memcpy(&cmd[1],Freq_d.byte,4);
    uint8_t *packet=NULL;
    size_t  packet_length=0,ret=0;
    if((packet_length=STR3060_Packet_Pack(cmd,sizeof(cmd),&packet)))
    {
       ret=STR3060_Send_Packet(ctx,packet,packet_length);
    }
    STR3060_Packet_Free(packet);
    return ret;
}



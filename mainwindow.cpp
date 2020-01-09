#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(size());
    setWindowTitle("STR3060三相交流测试源");
    setWindowIcon(QIcon(":/window_resources.ico"));//设置窗口图标

    ctx=STR3060_Context_New();
    Is_SerialPort_Open=false;

    STR3060_Receiver=new std::thread(&MainWindow::STR3060_Check_Receive,this,ctx);
    STR3060_Receiver->detach();

    {
        dataview=new Dialog_DataView(ctx,this);
    }

    UI_Timer=new QTimer(this);
    {//设置刷新UI的定时器

        UI_Timer->start(800);
        UI_Timer->stop();
         connect(UI_Timer,SIGNAL(timeout()),this,SLOT(UI_Timer_timeout()));
    }
    Request_Timer=new QTimer(this);
    {//设置请求数据的定时器

        Request_Timer->start(3500);
        Request_Timer->stop();
        connect(Request_Timer,SIGNAL(timeout()),this,SLOT(Request_Timer_timeout()));
    }
    {//秒节拍设置
        second_tick_count=0;
        Second_Tick_Timer=new QTimer(this);
        Second_Tick_Timer->start(1000);
        connect(Second_Tick_Timer,SIGNAL(timeout()),this,SLOT(Second_Tick_Timer_timeout()));
    }

    {//设置Tab页面内容
        ui->tabWidget->setTabText(0,"概览");
        ui->tabWidget->setTabText(1,"STR3060设置");
        ui->tabWidget->setTabText(2,"电压设置");
        ui->tabWidget->setTabText(3,"电流设置");
        ui->tabWidget->setTabText(4,"精确电压电流设置");


        ui->Generic->setWindowTitle("概览");
        ui->Settings->setWindowTitle("STR3060设置");
        ui->Settings->setWindowTitle("电压设置");
        ui->Settings->setWindowTitle("电流设置");
        ui->Settings->setWindowTitle("精确电压电流设置");


        ui->tabWidget->setTabEnabled(2,false);//设置电压设置页面
        ui->tabWidget->setTabEnabled(3,false);//设置电流设置页面
        ui->tabWidget->setTabEnabled(4,false);//精确电压电流设置页面
        ui->tabWidget->setCurrentIndex(1);
    }

    {//设置量程
        ui->S_U_Limit->addItem("380");
        ui->S_U_Limit->addItem("220");
        ui->S_U_Limit->addItem("100");
        ui->S_U_Limit->addItem("57.7");
        ui->S_U_Limit->addItem("30");
        ui->S_U_Limit->addItem("600");
        ui->S_U_Limit->setCurrentText("220");

        ui->S_I_Limit->addItem("20");
        ui->S_I_Limit->addItem("5");
        ui->S_I_Limit->addItem("1");
        ui->S_I_Limit->addItem("0.2");
        ui->S_I_Limit->addItem("10");
        ui->S_I_Limit->addItem("60");
        ui->S_I_Limit->setCurrentText("5");
    }

    Update_Serialport();
    connect(ui->SerialPort_Open,SIGNAL(clicked()),this,SLOT(Open_SerialPort()));
    connect(ui->SerialPort_Close,SIGNAL(clicked()),this,SLOT(Close_SerialPort()));
    connect(ui->ToTab1,SIGNAL(clicked()),this,SLOT(ToTab1_clicked()));
    connect(ui->S_Phase,SIGNAL(clicked()),this,SLOT(S_Phase_Clicked()));
    connect(ui->S_Reset,SIGNAL(clicked()),this,SLOT(S_Reset_clicked()));

    connect(ui->S_Init_3,SIGNAL(clicked()),this,SLOT(S_Init_3_Clicked()));
    connect(ui->S_Init_1,SIGNAL(clicked()),this,SLOT(S_Init_1_Clicked()));

    connect(ui->S_Output_On,SIGNAL(clicked()),this,SLOT(S_Output_On_clicked()));
    connect(ui->S_Output_Off,SIGNAL(clicked()),this,SLOT(S_Output_Off_clicked()));
    connect(ui->S_Link_0,SIGNAL(toggled(bool)),this,SLOT(S_Link_toggled(bool)));
    connect(ui->S_Link_1,SIGNAL(toggled(bool)),this,SLOT(S_Link_toggled(bool)));
    connect(ui->S_Link_2,SIGNAL(toggled(bool)),this,SLOT(S_Link_toggled(bool)));
    connect(ui->S_Link_3,SIGNAL(toggled(bool)),this,SLOT(S_Link_toggled(bool)));

    connect(ui->S_U_Limit,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(S_U_Limits_Changed(const QString &)));
    connect(ui->S_I_Limit,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(S_I_Limits_Changed(const QString &)));

    connect(ui->S_A_Phase,SIGNAL(textChanged(const QString &)),this,SLOT(S_Phase_Check_double(const QString &)));
    connect(ui->S_B_Phase,SIGNAL(textChanged(const QString &)),this,SLOT(S_Phase_Check_double(const QString &)));
    connect(ui->S_C_Phase,SIGNAL(textChanged(const QString &)),this,SLOT(S_Phase_Check_double(const QString &)));

    connect(ui->S_Freq,SIGNAL(textChanged(const QString &)),this,SLOT(S_Freq_Changed(const QString &)));

}

void MainWindow::Update_Serialport()
{
    ui->SerialPort_name_cmb->clear();

    {//添加串口名
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            qDebug()<<"Name:"<<info.portName();
            qDebug()<<"Description:"<<info.description();
            qDebug()<<"Manufacturer:"<<info.manufacturer();

            //这里相当于自动识别串口号之后添加到了cmb，如果要手动选择可以用下面列表的方式添加进去
            QSerialPort serial;
            serial.setPort(info);
            if(serial.open(QIODevice::ReadWrite))
            {
                //将串口号添加到cmb
                ui->SerialPort_name_cmb->addItem(info.portName());
                //关闭串口等待人为(打开串口按钮)打开
                serial.close();
            }
        }
      }
}

void MainWindow::Open_SerialPort()
{
    if(ui->SerialPort_name_cmb->currentText().length()<=0)
    {
        ui->statusbar->showMessage("未找到串口，打开失败!\n\r");
        Update_Serialport();
        return;
    }
    if(STR3060_Open(ctx,ui->SerialPort_name_cmb->currentText().toStdString().c_str()))
    {
      ui->SerialPort_Open->setEnabled(false);
      ui->SerialPort_Close->setEnabled(true);
      ui->group_STR3060->setEnabled(true);
      ui->tabWidget->setTabEnabled(2,true);//设置电压设置页面
      ui->tabWidget->setTabEnabled(3,true);//设置电流设置页面
       ui->tabWidget->setTabEnabled(4,true);//精确电压电流设置页面
      ui->statusbar->showMessage("打开串口成功!\n\r");
      Is_SerialPort_Open=true;
      {//请求数据
          STR3060_Request_Data(ctx);
      }
      {//打开定时器
          UI_Timer->start();
          Request_Timer->start();
      }
    }
    else
    {
        Update_Serialport();
        ui->statusbar->showMessage("打开串口异常!\n\r");
    }
}

void MainWindow::Close_SerialPort()
{
   if(STR3060_Close(ctx))
   {
       ui->statusbar->showMessage("关闭串口成功!\n\r");
       ui->SerialPort_Open->setEnabled(true);
       ui->SerialPort_Close->setEnabled(false);
       ui->group_STR3060->setEnabled(false);
       ui->tabWidget->setTabEnabled(2,false);//设置电压设置页面
       ui->tabWidget->setTabEnabled(3,false);//设置电流设置页面
       ui->tabWidget->setTabEnabled(4,false);//精确电压电流设置页面
       Update_Serialport();
       Is_SerialPort_Open=false;
       {//关闭定时器
           UI_Timer->stop();
           Request_Timer->stop();
       }
   }
   else
   {
        ui->statusbar->showMessage("关闭串口失败!\n\r");
   }

}

void MainWindow::ToTab1_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
    {//打开预览窗口
        dataview->show();
        dataview->activateWindow();
    }
}

void MainWindow::S_Reset_clicked()
{
    if(Is_SerialPort_Open)
    {
        if(STR3060_Reset(ctx))
        {
            ui->statusbar->showMessage("发送复位指令完成\n\r");
        }
    }
}

void MainWindow::STR3060_Check_Receive(STR3060_Context *ctx)
{

    while(1)
    {
        #ifdef WIN32
        //WaitForSingleObject(ctx->handle,INFINITE);
        #endif
        if(Is_SerialPort_Open)
        {
            if(STR3060_Receive_Data(ctx))
            {
                qDebug()<<"接收STR3060消息";
            }
            else
            {
#ifdef  WIN32
            Sleep(5);
#else
#error  不支持
#endif
            }

        }
        else
        {
#ifdef  WIN32
            Sleep(5);
#else
#error  不支持
#endif
        }
    }
}

void MainWindow::S_Link_toggled(bool checked)
{
    if(checked)
    {//选中的选项
        if(ui->S_Link_0->isChecked())
        {
            STR3060_Set_LinkMode(ctx,0);
            ui->statusbar->showMessage("设置为三相四线正相序\n\r");
        }
        if(ui->S_Link_1->isChecked())
        {
            STR3060_Set_LinkMode(ctx,1);
            ui->statusbar->showMessage("设置为三相三线正相序\n\r");
        }
        if(ui->S_Link_2->isChecked())
        {
            STR3060_Set_LinkMode(ctx,2);
            ui->statusbar->showMessage("设置为三相四线逆相序\n\r");
        }
        if(ui->S_Link_3->isChecked())
        {
            STR3060_Set_LinkMode(ctx,3);
            ui->statusbar->showMessage("设置为三相三线逆相序\n\r");
        }

    }
}

void MainWindow::S_U_Limits_Changed(const QString &text)
{
    long double U=text.toDouble();
    STR3060_Set_Limit(ctx,U,U,U,-1,-1,-1);
    ui->statusbar->showMessage("电压量程命令发送完成!\n\r");
}
void MainWindow::S_I_Limits_Changed(const QString &text)
{
    long double I=text.toDouble();
    STR3060_Set_Limit(ctx,-1,-1,-1,I,I,I);
     ui->statusbar->showMessage("电流量程命令发送完成!\n\r");
}

void MainWindow::S_Output_On_clicked()
{
    STR3060_Output_On(ctx);
    ui->statusbar->showMessage("打开输出命令发送完成!\n\r");
}

void MainWindow::S_Output_Off_clicked()
{
    STR3060_Output_Off(ctx);
    ui->statusbar->showMessage("关闭输出命令发送完成!\n\r");
}

void MainWindow::S_Phase_Check_double(const QString &text)
{
    bool is_success=false;
    text.toDouble(&is_success);
    if(!is_success)
    {
        ui->S_A_Phase->setText("0.0");
        ui->S_B_Phase->setText("120.0");
        ui->S_C_Phase->setText("240.0");
    }
}

void MainWindow::S_Phase_Clicked()
{
    long double A=ui->S_A_Phase->text().toDouble(),B=ui->S_B_Phase->text().toDouble(),C=ui->S_C_Phase->text().toDouble();
    STR3060_Set_Phase(ctx,A,B,C,A,B,C);
    ui->statusbar->showMessage("相位设置命令发送成功!\n\r");
}

void MainWindow::S_Init_3_Clicked()
{
   STR3060_Reset(ctx);
#ifdef  WIN32
            Sleep(1000);
#else
#error  不支持
#endif
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
        count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Set_Limit(ctx,220,220,220,5,5,5);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
        count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Set_LinkMode(ctx,0);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
         count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Set_Freq(ctx,50);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
         count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Set_Phase(ctx,0,120,240,180,120,240);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
         count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Set_Value(ctx,220,220,220,5,5,5);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
         count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Output_On(ctx);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
         count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   STR3060_Set_Phase(ctx,0,120,240,0,120,240);
   {
    long unsigned count=0;
    while((!STR3060_Query_Success(ctx))&&(count<3000))
    {
         count++;
#ifdef  WIN32
            Sleep(1);
#else
#error  不支持
#endif
    }
   }

   ui->statusbar->showMessage("快速初始化(三相)完成!\n\r");
}

void MainWindow::S_Init_1_Clicked()
{
    STR3060_Reset(ctx);
#ifdef  WIN32
            Sleep(1000);
#else
#error  不支持
#endif

    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    STR3060_Set_Limit(ctx,220,220,220,5,5,5);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    STR3060_Set_LinkMode(ctx,0);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    STR3060_Set_Freq(ctx,50);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    STR3060_Set_Phase(ctx,0,120,240,180,120,240);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }


    STR3060_Set_Value(ctx,220,0,0,5,0,0);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    STR3060_Output_On(ctx);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    STR3060_Set_Phase(ctx,0,120,240,0,120,240);
    {
     long unsigned count=0;
     while((!STR3060_Query_Success(ctx))&&(count<3000))
     {
          count++;
 #ifdef  WIN32
             Sleep(1);
 #else
 #error  不支持
 #endif
     }
    }

    ui->statusbar->showMessage("快速初始化（单相）完成!\n\r");
}

void MainWindow::S_Freq_Changed(const QString &text)
{
    bool is_success=false;
    text.toDouble(&is_success);
    if(is_success)
    {
        STR3060_Set_Freq(ctx,text.toDouble());
        ui->statusbar->showMessage("频率设置命令发送成功!\n\r");
    }
    else
    {
        ui->S_Freq->setText("50.0");
    }
}

void MainWindow::UI_Timer_timeout()
{//更新UI
    {//概览
        {//A
            ui->S_G_UA->display((double)STR3060_Get_U(ctx,STR3060_A));
            ui->S_G_UA_Phase->display((double)STR3060_Get_U_Phase(ctx,STR3060_A));
            ui->S_G_IA->display((double)STR3060_Get_I(ctx,STR3060_A));
            ui->S_G_IA_Phase->display((double)STR3060_Get_I_Phase(ctx,STR3060_A));
            ui->S_G_PA->display((double)STR3060_Get_P(ctx,STR3060_A));
            ui->S_G_QA->display((double)STR3060_Get_Q(ctx,STR3060_A));
            ui->S_G_SA->display((double)STR3060_Get_S(ctx,STR3060_A));
            ui->S_G_COSA->display((double)STR3060_Get_COS(ctx,STR3060_A));
        }

        {//B
            ui->S_G_UB->display((double)STR3060_Get_U(ctx,STR3060_B));
            ui->S_G_UB_Phase->display((double)STR3060_Get_U_Phase(ctx,STR3060_B));
            ui->S_G_IB->display((double)STR3060_Get_I(ctx,STR3060_B));
            ui->S_G_IB_Phase->display((double)STR3060_Get_I_Phase(ctx,STR3060_B));
            ui->S_G_PB->display((double)STR3060_Get_P(ctx,STR3060_B));
            ui->S_G_QB->display((double)STR3060_Get_Q(ctx,STR3060_B));
            ui->S_G_SB->display((double)STR3060_Get_S(ctx,STR3060_B));
            ui->S_G_COSB->display((double)STR3060_Get_COS(ctx,STR3060_B));
        }

        {//C
            ui->S_G_UC->display((double)STR3060_Get_U(ctx,STR3060_C));
            ui->S_G_UC_Phase->display((double)STR3060_Get_U_Phase(ctx,STR3060_C));
            ui->S_G_IC->display((double)STR3060_Get_I(ctx,STR3060_C));
            ui->S_G_IC_Phase->display((double)STR3060_Get_I_Phase(ctx,STR3060_C));
            ui->S_G_PC->display((double)STR3060_Get_P(ctx,STR3060_C));
            ui->S_G_QC->display((double)STR3060_Get_Q(ctx,STR3060_C));
            ui->S_G_SC->display((double)STR3060_Get_S(ctx,STR3060_C));
            ui->S_G_COSC->display((double)STR3060_Get_COS(ctx,STR3060_C));
        }
        {//杂项
            ui->S_G_P->display((double)STR3060_Get_P(ctx,STR3060_Total));
            ui->S_G_Q->display((double)STR3060_Get_Q(ctx,STR3060_Total));
            ui->S_G_S->display((double)STR3060_Get_S(ctx,STR3060_Total));
            ui->S_G_COS->display((double)STR3060_Get_COS(ctx,STR3060_Total));
            ui->S_G_Freq->display((double)STR3060_Get_Freq(ctx));

        }

    }
    {//STR3060设置界面
        static uint32_t last_second=0;
        if(Get_second_tick(last_second)>=20)
        {//间隔一段时间更新,或者刚初始化
            {//连接模式设置
                switch(ctx->Link_Mode)
                {
                default:
                    break;
                case 0:
                    if(!ui->S_Link_0->isChecked())
                    {
                        ui->S_Link_0->setChecked(true);
                    }
                    break;
                case 1:
                    if(!ui->S_Link_1->isChecked())
                    {
                        ui->S_Link_1->setChecked(true);
                    }
                    break;
                case 2:
                    if(!ui->S_Link_2->isChecked())
                    {
                        ui->S_Link_2->setChecked(true);
                    }
                    break;
                case 3:
                    if(!ui->S_Link_3->isChecked())
                    {
                        ui->S_Link_3->setChecked(true);
                    }
                    break;
                }
            }

            {//量程设定
                if((STR3060_Get_ULimit(ctx,STR3060_A)==STR3060_Get_ULimit(ctx,STR3060_B)) && (STR3060_Get_ULimit(ctx,STR3060_A)==STR3060_Get_ULimit(ctx,STR3060_B)))
                {
                    if((STR3060_Get_ULimit(ctx,STR3060_A)< ui->S_U_Limit->currentText().toDouble()*0.95) || (STR3060_Get_ULimit(ctx,STR3060_A)> ui->S_U_Limit->currentText().toDouble()*1.05))
                    {

                        ui->S_U_Limit->setCurrentIndex(ctx->UA_Limit);

                    }
                }

                if((STR3060_Get_ILimit(ctx,STR3060_A)==STR3060_Get_ILimit(ctx,STR3060_B)) && (STR3060_Get_ILimit(ctx,STR3060_A)==STR3060_Get_ILimit(ctx,STR3060_B)))
                {
                    if((STR3060_Get_ILimit(ctx,STR3060_A)< ui->S_I_Limit->currentText().toDouble()*0.95) || (STR3060_Get_ILimit(ctx,STR3060_A)> ui->S_I_Limit->currentText().toDouble()*1.05))
                    {

                        ui->S_I_Limit->setCurrentIndex(ctx->IA_Limit);

                    }
                }
            }
            {
                if((STR3060_Get_Freq(ctx)*0.99 >=ui->S_Freq->text().toDouble()) ||(STR3060_Get_Freq(ctx)*1.01 <=ui->S_Freq->text().toDouble()) )
                {
                    char buff[50]={0};
                    sprintf(buff,"%Lf",STR3060_Get_Freq(ctx));
                    ui->S_Freq->setText(buff);
                }
            }

            last_second=Get_second_tick();
        }

    }

    {//电压界面
        {//A
            {//滑块最大值设定
                int U_MAX=STR3060_Get_KU(ctx,STR3060_A)*STR3060_Get_ULimit(ctx,STR3060_A)*1.2;
                int U_Phase_MAX=STR3060_Get_KPhase(ctx)*360.0;
                if(ui->S_U_UA->maximum() != U_MAX)
                {
                    ui->S_U_UA->setMaximum(U_MAX);
                    ui->S_U_UA->setSingleStep(1);
                    ui->S_U_UA->setMinimum(0);
                    ui->S_U_UA->setValue(ctx->UA.dword);
                    ui->S_U_UA_V->display((double)STR3060_Get_U(ctx,STR3060_A));
                }

                if(ui->S_U_UA_Phase->maximum() != U_Phase_MAX)
                {
                    ui->S_U_UA_Phase->setMaximum( U_Phase_MAX);
                    ui->S_U_UA->setSingleStep(1);
                    ui->S_U_UA_Phase->setMinimum(0);
                    ui->S_U_UA_Phase->setValue(ctx->UA_Phase.dword);
                    ui->S_U_UA_Phase_V->display((double)STR3060_Get_U_Phase(ctx,STR3060_A));
                }

            }

            {//滑块值更新
                static uint32_t last_second_count=0;

                if(Get_second_tick(last_second_count)>=4)//4秒一次
                {
                    static int U_Last=ctx->UA.dword,U_Phase_Last=ctx->UA_Phase.dword;



                     if((ctx->UA.dword+STR3060_Get_KU(ctx,STR3060_A) <= (U_Last)) ||(ctx->UA.dword >= (U_Last+STR3060_Get_KU(ctx,STR3060_A))))
                     {
                         U_Last=ctx->UA.dword;
                         ui->S_U_UA->setValue(ctx->UA.dword);
                         ui->S_U_UA_V->display((double)STR3060_Get_U(ctx,STR3060_A));
                     }


                     if((ctx->UA_Phase.dword+STR3060_Get_KPhase(ctx) <= (U_Phase_Last)) ||(ctx->UA_Phase.dword >= (U_Phase_Last+STR3060_Get_KPhase(ctx))))
                     {
                         U_Phase_Last=ctx->UA_Phase.dword;
                         ui->S_U_UA_Phase->setValue(ctx->UA_Phase.dword);
                         ui->S_U_UA_Phase_V->display((double)STR3060_Get_U_Phase(ctx,STR3060_A));
                     }

                     last_second_count=Get_second_tick();

                }

            }
        }


        {//B
            {//滑块最大值设定
                int U_MAX=STR3060_Get_KU(ctx,STR3060_B)*STR3060_Get_ULimit(ctx,STR3060_B)*1.2;
                int U_Phase_MAX=STR3060_Get_KPhase(ctx)*360.0;
                if(ui->S_U_UB->maximum() != U_MAX)
                {
                    ui->S_U_UB->setMaximum(U_MAX);
                    ui->S_U_UB->setSingleStep(1);
                    ui->S_U_UB->setMinimum(0);
                    ui->S_U_UB->setValue(ctx->UB.dword);
                    ui->S_U_UB_V->display((double)STR3060_Get_U(ctx,STR3060_B));
                }

                if(ui->S_U_UB_Phase->maximum() != U_Phase_MAX)
                {
                    ui->S_U_UB_Phase->setMaximum( U_Phase_MAX);
                    ui->S_U_UB->setSingleStep(1);
                    ui->S_U_UB_Phase->setMinimum(0);
                    ui->S_U_UB_Phase->setValue(ctx->UB_Phase.dword);
                    ui->S_U_UB_Phase_V->display((double)STR3060_Get_U_Phase(ctx,STR3060_B));
                }

            }

            {//滑块值更新
                static uint32_t last_second_count=0;

                if(Get_second_tick(last_second_count)>10)//10秒一次
                {
                    static int U_Last=ctx->UB.dword,U_Phase_Last=ctx->UB_Phase.dword;



                     if((ctx->UB.dword+STR3060_Get_KU(ctx,STR3060_B) <= (U_Last)) ||(ctx->UB.dword >= (U_Last+STR3060_Get_KU(ctx,STR3060_B))))
                     {
                         U_Last=ctx->UB.dword;
                         ui->S_U_UB->setValue(ctx->UB.dword);
                         ui->S_U_UB_V->display((double)STR3060_Get_U(ctx,STR3060_B));
                     }


                     if((ctx->UB_Phase.dword+STR3060_Get_KPhase(ctx) <= (U_Phase_Last)) ||(ctx->UB_Phase.dword >= (U_Phase_Last+STR3060_Get_KPhase(ctx))))
                     {
                         U_Phase_Last=ctx->UB_Phase.dword;
                         ui->S_U_UB_Phase->setValue(ctx->UB_Phase.dword);
                         ui->S_U_UB_Phase_V->display((double)STR3060_Get_U_Phase(ctx,STR3060_B));
                     }

                     last_second_count=Get_second_tick();
                }
             }
           }


                {//C
                    {//滑块最大值设定
                        int U_MAX=STR3060_Get_KU(ctx,STR3060_C)*STR3060_Get_ULimit(ctx,STR3060_C)*1.2;
                        int U_Phase_MAX=STR3060_Get_KPhase(ctx)*360.0;
                        if(ui->S_U_UC->maximum() != U_MAX)
                        {
                            ui->S_U_UC->setMaximum(U_MAX);
                            ui->S_U_UC->setSingleStep(1);
                            ui->S_U_UC->setMinimum(0);
                            ui->S_U_UC->setValue(ctx->UC.dword);
                            ui->S_U_UC_V->display((double)STR3060_Get_U(ctx,STR3060_C));
                        }

                        if(ui->S_U_UC_Phase->maximum() != U_Phase_MAX)
                        {
                            ui->S_U_UC_Phase->setMaximum( U_Phase_MAX);
                            ui->S_U_UC->setSingleStep(1);
                            ui->S_U_UC_Phase->setMinimum(0);
                            ui->S_U_UC_Phase->setValue(ctx->UC_Phase.dword);
                            ui->S_U_UC_Phase_V->display((double)STR3060_Get_U_Phase(ctx,STR3060_C));
                        }

                    }

                    {//滑块值更新
                        static uint32_t last_second_count=0;

                        if(Get_second_tick(last_second_count)>=4)//4秒一次
                        {
                            static int U_Last=ctx->UC.dword,U_Phase_Last=ctx->UC_Phase.dword;



                             if((ctx->UC.dword+STR3060_Get_KU(ctx,STR3060_C) <= (U_Last)) ||(ctx->UC.dword >= (U_Last+STR3060_Get_KU(ctx,STR3060_C))))
                             {
                                 U_Last=ctx->UC.dword;
                                 ui->S_U_UC->setValue(ctx->UC.dword);
                                 ui->S_U_UC_V->display((double)STR3060_Get_U(ctx,STR3060_C));
                             }


                             if((ctx->UC_Phase.dword+STR3060_Get_KPhase(ctx) <= (U_Phase_Last)) ||(ctx->UC_Phase.dword >= (U_Phase_Last+STR3060_Get_KPhase(ctx))))
                             {
                                 U_Phase_Last=ctx->UC_Phase.dword;
                                 ui->S_U_UC_Phase->setValue(ctx->UC_Phase.dword);
                                 ui->S_U_UC_Phase_V->display((double)STR3060_Get_U_Phase(ctx,STR3060_C));
                             }

                             last_second_count=Get_second_tick();
                        }

                    }
                }

         }





    {//电流界面
        {//A
            {//滑块最大值设定
                int I_MAX=STR3060_Get_KI(ctx,STR3060_A)*STR3060_Get_ILimit(ctx,STR3060_A)*1.2;
                int I_Phase_MAX=STR3060_Get_KPhase(ctx)*360.0;
                if(ui->S_I_IA->maximum() != I_MAX)
                {
                    ui->S_I_IA->setMaximum(I_MAX);
                    ui->S_I_IA->setSingleStep(1);
                    ui->S_I_IA->setMinimum(0);
                    ui->S_I_IA->setValue(ctx->IA.dword);
                    ui->S_I_IA_V->display((double)STR3060_Get_I(ctx,STR3060_A));
                }

                if(ui->S_I_IA_Phase->maximum() != I_Phase_MAX)
                {
                    ui->S_I_IA_Phase->setMaximum( I_Phase_MAX);
                    ui->S_I_IA->setSingleStep(1);
                    ui->S_I_IA_Phase->setMinimum(0);
                    ui->S_I_IA_Phase->setValue(ctx->IA_Phase.dword);
                    ui->S_I_IA_Phase_V->display((double)STR3060_Get_I_Phase(ctx,STR3060_A));
                }

            }

            {//滑块值更新
                static uint32_t last_second_count=0;

                if(Get_second_tick(last_second_count)>=4)//4秒一次
                {
                    static int I_Last=ctx->IA.dword,I_Phase_Last=ctx->IA_Phase.dword;



                     if((ctx->IA.dword+STR3060_Get_KI(ctx,STR3060_A) <= (I_Last)) ||(ctx->IA.dword >= (I_Last+STR3060_Get_KI(ctx,STR3060_A))))
                     {
                         I_Last=ctx->IA.dword;
                         ui->S_I_IA->setValue(ctx->IA.dword);
                         ui->S_I_IA_V->display((double)STR3060_Get_I(ctx,STR3060_A));
                     }


                     if((ctx->IA_Phase.dword+STR3060_Get_KPhase(ctx) <= (I_Phase_Last)) ||(ctx->IA_Phase.dword >= (I_Phase_Last+STR3060_Get_KPhase(ctx))))
                     {
                         I_Phase_Last=ctx->IA_Phase.dword;
                         ui->S_I_IA_Phase->setValue(ctx->IA_Phase.dword);
                         ui->S_I_IA_Phase_V->display((double)STR3060_Get_I_Phase(ctx,STR3060_A));
                     }

                     last_second_count=Get_second_tick();
                }

            }
        }


        {//B
            {//滑块最大值设定
                int I_MAX=STR3060_Get_KI(ctx,STR3060_B)*STR3060_Get_ILimit(ctx,STR3060_B)*1.2;
                int I_Phase_MAX=STR3060_Get_KPhase(ctx)*360.0;
                if(ui->S_I_IB->maximum() != I_MAX)
                {
                    ui->S_I_IB->setMaximum(I_MAX);
                    ui->S_I_IB->setSingleStep(1);
                    ui->S_I_IB->setMinimum(0);
                    ui->S_I_IB->setValue(ctx->IB.dword);
                    ui->S_I_IB_V->display((double)STR3060_Get_I(ctx,STR3060_B));
                }

                if(ui->S_I_IB_Phase->maximum() != I_Phase_MAX)
                {
                    ui->S_I_IB_Phase->setMaximum( I_Phase_MAX);
                    ui->S_I_IB->setSingleStep(1);
                    ui->S_I_IB_Phase->setMinimum(0);
                    ui->S_I_IB_Phase->setValue(ctx->IB_Phase.dword);
                    ui->S_I_IB_Phase_V->display((double)STR3060_Get_I_Phase(ctx,STR3060_B));
                }

            }

            {//滑块值更新
                static uint32_t last_second_count=0;

                if(Get_second_tick(last_second_count)>=4)//4秒一次
                {
                    static int I_Last=ctx->IB.dword,I_Phase_Last=ctx->IB_Phase.dword;



                     if((ctx->IB.dword+STR3060_Get_KI(ctx,STR3060_B) <= (I_Last)) ||(ctx->IB.dword >= (I_Last+STR3060_Get_KI(ctx,STR3060_B))))
                     {
                         I_Last=ctx->IB.dword;
                         ui->S_I_IB->setValue(ctx->IB.dword);
                         ui->S_I_IB_V->display((double)STR3060_Get_I(ctx,STR3060_B));
                     }


                     if((ctx->IB_Phase.dword+STR3060_Get_KPhase(ctx) <= (I_Phase_Last)) ||(ctx->IB_Phase.dword >= (I_Phase_Last+STR3060_Get_KPhase(ctx))))
                     {
                         I_Phase_Last=ctx->IB_Phase.dword;
                         ui->S_I_IB_Phase->setValue(ctx->IB_Phase.dword);
                         ui->S_I_IB_Phase_V->display((double)STR3060_Get_I_Phase(ctx,STR3060_B));
                     }

                     last_second_count=Get_second_tick();
                }
             }
           }

                {//C
                    {//滑块最大值设定
                        int I_MAX=STR3060_Get_KI(ctx,STR3060_C)*STR3060_Get_ILimit(ctx,STR3060_C)*1.2;
                        int I_Phase_MAX=STR3060_Get_KPhase(ctx)*360.0;
                        if(ui->S_I_IC->maximum() != I_MAX)
                        {
                            ui->S_I_IC->setMaximum(I_MAX);
                            ui->S_I_IC->setSingleStep(1);
                            ui->S_I_IC->setMinimum(0);
                            ui->S_I_IC->setValue(ctx->IC.dword);
                            ui->S_I_IC_V->display((double)STR3060_Get_I(ctx,STR3060_C));
                        }

                        if(ui->S_I_IC_Phase->maximum() != I_Phase_MAX)
                        {
                            ui->S_I_IC_Phase->setMaximum( I_Phase_MAX);
                            ui->S_I_IC->setSingleStep(1);
                            ui->S_I_IC_Phase->setMinimum(0);
                            ui->S_I_IC_Phase->setValue(ctx->IC_Phase.dword);
                            ui->S_I_IC_Phase_V->display((double)STR3060_Get_I_Phase(ctx,STR3060_C));
                        }

                    }

                    {//滑块值更新
                        static uint32_t last_second_count=0;

                        if(Get_second_tick(last_second_count)>=4)//4秒一次
                        {
                            static int I_Last=ctx->IC.dword,I_Phase_Last=ctx->IC_Phase.dword;



                             if((ctx->IC.dword+STR3060_Get_KI(ctx,STR3060_C) <= (I_Last)) ||(ctx->IC.dword >= (I_Last+STR3060_Get_KI(ctx,STR3060_C))))
                             {
                                 I_Last=ctx->IC.dword;
                                 ui->S_I_IC->setValue(ctx->IC.dword);
                                 ui->S_I_IC_V->display((double)STR3060_Get_I(ctx,STR3060_C));
                             }


                             if((ctx->IC_Phase.dword+STR3060_Get_KPhase(ctx) <= (I_Phase_Last)) ||(ctx->IC_Phase.dword >= (I_Phase_Last+STR3060_Get_KPhase(ctx))))
                             {
                                 I_Phase_Last=ctx->IC_Phase.dword;
                                 ui->S_I_IC_Phase->setValue(ctx->IC_Phase.dword);
                                 ui->S_I_IC_Phase_V->display((double)STR3060_Get_I_Phase(ctx,STR3060_C));
                             }

                             last_second_count=Get_second_tick();
                        }

                    }
                }




    }

}

void MainWindow::Request_Timer_timeout()
{
   if(Is_SerialPort_Open)
    {//请求数据
        STR3060_Request_Data(ctx);
        ui->statusbar->showMessage("正在请求数据");
    }
}

void MainWindow::Second_Tick_Timer_timeout()
{
    uint64_t overflow=((uint64_t)1<<32);
    second_tick_count++;
    if(second_tick_count>=overflow)
    {
        second_tick_count-=overflow;
    }

    //qDebug()<<"已启动时间:"<<second_tick_count<<"s";
}

uint32_t MainWindow::Get_second_tick(uint32_t start)
{
    uint64_t ret=0,overflow=((uint64_t)1<<32),tick=second_tick_count;
    if(start<=tick)
    {
        ret=tick-start;
    }
    else
    {
        tick+=overflow;
        ret=tick-start;
    }

    return (uint32_t)ret;

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_S_U_UA_sliderMoved(int position)
{
    ui->S_U_UA_V->display((double)position/STR3060_Get_KU(ctx,STR3060_A));
}

void MainWindow::on_S_U_UA_Phase_sliderMoved(int position)
{
     ui->S_U_UA_Phase_V->display((double)position/STR3060_Get_KPhase(ctx));
}

void MainWindow::on_S_U_UA_sliderReleased()
{
    STR3060_Set_Value(ctx,(long double)ui->S_U_UA->value()/STR3060_Get_KU(ctx,STR3060_A),-1,-1,-1,-1,-1);
    ui->statusbar->showMessage("A相电压设置命令已发出!\n\r");
}

void MainWindow::on_S_U_UA_Phase_sliderReleased()
{
    STR3060_Set_Phase(ctx,(long double)ui->S_U_UA_Phase->value()/STR3060_Get_KPhase(ctx),-1,-1,-1,-1,-1);
    ui->statusbar->showMessage("A相相位设置命令已发出!\n\r");
}

void MainWindow::on_S_U_UB_sliderMoved(int position)
{
    ui->S_U_UB_V->display((double)position/STR3060_Get_KU(ctx,STR3060_B));
}

void MainWindow::on_S_U_UB_Phase_sliderMoved(int position)
{
    ui->S_U_UB_Phase_V->display((double)position/STR3060_Get_KPhase(ctx));
}

void MainWindow::on_S_U_UB_sliderReleased()
{
    STR3060_Set_Value(ctx,-1,(long double)ui->S_U_UB->value()/STR3060_Get_KU(ctx,STR3060_B),-1,-1,-1,-1);
    ui->statusbar->showMessage("B相电压设置命令已发出!\n\r");
}

void MainWindow::on_S_U_UB_Phase_sliderReleased()
{
    STR3060_Set_Phase(ctx,-1,(long double)ui->S_U_UB_Phase->value()/STR3060_Get_KPhase(ctx),-1,-1,-1,-1);
    ui->statusbar->showMessage("B相相位设置命令已发出!\n\r");
}

void MainWindow::on_S_U_UC_sliderMoved(int position)
{
    ui->S_U_UC_V->display((double)position/STR3060_Get_KU(ctx,STR3060_C));
}

void MainWindow::on_S_U_UC_Phase_sliderMoved(int position)
{
    ui->S_U_UC_Phase_V->display((double)position/STR3060_Get_KPhase(ctx));
}

void MainWindow::on_S_U_UC_sliderReleased()
{
    STR3060_Set_Value(ctx,-1,-1,(long double)ui->S_U_UC->value()/STR3060_Get_KU(ctx,STR3060_C),-1,-1,-1);
    ui->statusbar->showMessage("C相电压设置命令已发出!\n\r");
}

void MainWindow::on_S_U_UC_Phase_sliderReleased()
{
    STR3060_Set_Phase(ctx,-1,-1,(long double)ui->S_U_UC_Phase->value()/STR3060_Get_KPhase(ctx),-1,-1,-1);
    ui->statusbar->showMessage("C相相位设置命令已发出!\n\r");
}

void MainWindow::on_S_I_IA_sliderMoved(int position)
{
    ui->S_I_IA_V->display((double)position/STR3060_Get_KI(ctx,STR3060_A));
}

void MainWindow::on_S_I_IA_Phase_sliderMoved(int position)
{
    ui->S_I_IA_Phase_V->display((double)position/STR3060_Get_KPhase(ctx));
}

void MainWindow::on_S_I_IA_sliderReleased()
{
    STR3060_Set_Value(ctx,-1,-1,-1,(long double)ui->S_I_IA->value()/STR3060_Get_KI(ctx,STR3060_A),-1,-1);
    ui->statusbar->showMessage("A相电流设置命令已发出!\n\r");
}

void MainWindow::on_S_I_IA_Phase_sliderReleased()
{
    STR3060_Set_Phase(ctx,-1,-1,-1,(long double)ui->S_I_IA_Phase->value()/STR3060_Get_KPhase(ctx),-1,-1);
    ui->statusbar->showMessage("A相相位设置命令已发出!\n\r");
}

void MainWindow::on_S_I_IB_sliderMoved(int position)
{
     ui->S_I_IB_V->display((double)position/STR3060_Get_KI(ctx,STR3060_B));
}

void MainWindow::on_S_I_IB_Phase_sliderMoved(int position)
{
    ui->S_I_IB_Phase_V->display((double)position/STR3060_Get_KPhase(ctx));
}

void MainWindow::on_S_I_IB_sliderReleased()
{
    STR3060_Set_Value(ctx,-1,-1,-1,-1,(long double)ui->S_I_IB->value()/STR3060_Get_KI(ctx,STR3060_B),-1);
    ui->statusbar->showMessage("B相电流设置命令已发出!\n\r");
}

void MainWindow::on_S_I_IB_Phase_sliderReleased()
{
    STR3060_Set_Phase(ctx,-1,-1,-1,-1,(long double)ui->S_I_IB_Phase->value()/STR3060_Get_KPhase(ctx),-1);
    ui->statusbar->showMessage("B相相位设置命令已发出!\n\r");
}

void MainWindow::on_S_I_IC_sliderMoved(int position)
{
    ui->S_I_IC_V->display((double)position/STR3060_Get_KI(ctx,STR3060_C));
}

void MainWindow::on_S_I_IC_Phase_sliderMoved(int position)
{
    ui->S_I_IC_Phase_V->display((double)position/STR3060_Get_KPhase(ctx));
}

void MainWindow::on_S_I_IC_sliderReleased()
{
    STR3060_Set_Value(ctx,-1,-1,-1,-1,-1,(long double)ui->S_I_IC->value()/STR3060_Get_KI(ctx,STR3060_C));
    ui->statusbar->showMessage("C相电流设置命令已发出!\n\r");
}

void MainWindow::on_S_I_IC_Phase_sliderReleased()
{
    STR3060_Set_Phase(ctx,-1,-1,-1,-1,-1,(long double)ui->S_I_IC_Phase->value()/STR3060_Get_KPhase(ctx));
    ui->statusbar->showMessage("C相相位设置命令已发出!\n\r");
}

void MainWindow::on_S_V_UA_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_UA->setText("220");
    }
}

void MainWindow::on_S_V_UB_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_UB->setText("220");
    }
}

void MainWindow::on_S_V_UC_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_UC->setText("220");
    }
}

void MainWindow::on_S_V_IA_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_IA->setText("5");
    }
}

void MainWindow::on_S_V_IB_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_IB->setText("5");
    }
}

void MainWindow::on_S_V_IC_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_IC->setText("5");
    }
}

void MainWindow::on_S_V_Set_Value_clicked()
{
    STR3060_Set_Value(ctx,(long double)ui->S_V_UA->text().toDouble(),
                          (long double)ui->S_V_UB->text().toDouble(),
                          (long double)ui->S_V_UC->text().toDouble(),
                          (long double)ui->S_V_IA->text().toDouble(),
                          (long double)ui->S_V_IB->text().toDouble(),
                          (long double)ui->S_V_IC->text().toDouble());
    ui->statusbar->showMessage("发送值设定命令成功\n\r");
}

void MainWindow::on_S_V_UA_phase_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_UA_phase->setText("0");
    }
}

void MainWindow::on_S_V_IA_phase_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_IA_phase->setText("0");
    }
}

void MainWindow::on_S_V_UB_phase_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_UB_phase->setText("120");
    }
}

void MainWindow::on_S_V_IB_phase_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_IB_phase->setText("120");
    }
}

void MainWindow::on_S_V_UC_phase_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_UC_phase->setText("240");
    }
}

void MainWindow::on_S_V_IC_phase_textChanged(const QString &arg1)
{
    bool is=false;
    arg1.toDouble(&is);
    if(!is)
    {
        ui->S_V_IC_phase->setText("240");
    }
}

void MainWindow::on_S_V_Set_Value_phase_clicked()
{
    STR3060_Set_Phase(ctx,(long double)ui->S_V_UA_phase->text().toDouble(),
                          (long double)ui->S_V_UB_phase->text().toDouble(),
                          (long double)ui->S_V_UC_phase->text().toDouble(),
                          (long double)ui->S_V_IA_phase->text().toDouble(),
                          (long double)ui->S_V_IB_phase->text().toDouble(),
                          (long double)ui->S_V_IC_phase->text().toDouble());
    ui->statusbar->showMessage("发送相位设定命令成功\n\r");
}

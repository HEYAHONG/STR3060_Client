#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <thread>
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <STR3060.h>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void STR3060_Check_Receive(STR3060_Context * ctx);//自动接收数据

private:
    Ui::MainWindow *ui;
    STR3060_Context *ctx;//STR3060设备上下文
    bool Is_SerialPort_Open;//是否已打开串口
    std::thread *STR3060_Receiver;
    QTimer *UI_Timer;//刷新UI的定时器
    QTimer *Request_Timer;//请求数据的定时器

    uint64_t second_tick_count;//某些控件需要的秒节拍
    QTimer   *Second_Tick_Timer;//实现秒节拍的定时器
    uint32_t Get_second_tick(uint32_t start=0);//获得秒节拍


public slots:

    void UI_Timer_timeout();
    void Request_Timer_timeout();
    void Second_Tick_Timer_timeout();

    void Update_Serialport();//更新串口
    void Open_SerialPort();//打开串口
    void Close_SerialPort();//关闭串口
    void ToTab1_clicked();//转入概览
    void S_Reset_clicked();//复位
    void S_Link_toggled(bool checked);//接线方式切换
    void S_U_Limits_Changed(const QString &text);
    void S_I_Limits_Changed(const QString &text);

    void S_Output_On_clicked();
    void S_Output_Off_clicked();

    void S_Phase_Check_double(const QString &text);
    void S_Phase_Clicked();

    void S_Init_3_Clicked();
    void S_Init_1_Clicked();

    void S_Freq_Changed(const QString &text);

private slots:
    void on_S_U_UA_sliderMoved(int position);
    void on_S_U_UA_Phase_sliderMoved(int position);
    void on_S_U_UA_sliderReleased();
    void on_S_U_UA_Phase_sliderReleased();
    void on_S_U_UB_sliderMoved(int position);
    void on_S_U_UB_Phase_sliderMoved(int position);
    void on_S_U_UB_sliderReleased();
    void on_S_U_UB_Phase_sliderReleased();
    void on_S_U_UC_sliderMoved(int position);
    void on_S_U_UC_Phase_sliderMoved(int position);
    void on_S_U_UC_sliderReleased();
    void on_S_U_UC_Phase_sliderReleased();
    void on_S_I_IA_sliderMoved(int position);
    void on_S_I_IA_Phase_sliderMoved(int position);
    void on_S_I_IA_sliderReleased();
    void on_S_I_IA_Phase_sliderReleased();
    void on_S_I_IB_sliderMoved(int position);
    void on_S_I_IB_Phase_sliderMoved(int position);
    void on_S_I_IB_sliderReleased();
    void on_S_I_IB_Phase_sliderReleased();
    void on_S_I_IC_sliderMoved(int position);
    void on_S_I_IC_Phase_sliderMoved(int position);
    void on_S_I_IC_sliderReleased();
    void on_S_I_IC_Phase_sliderReleased();
};
#endif // MAINWINDOW_H

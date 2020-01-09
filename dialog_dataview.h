#ifndef DIALOG_DATAVIEW_H
#define DIALOG_DATAVIEW_H

#include <QDialog>
#include <QIcon>
#include <STR3060.h>
#include <qcustomplot.h>
#include <QTimer>


namespace Ui {
class Dialog_DataView;
}

class Dialog_DataView : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_DataView(STR3060_Context *ctx_p,QWidget *parent = nullptr);
    ~Dialog_DataView();

    QTimer *UI_Timer;//刷新UI的定时器
    void Update_Axis();//更新坐标轴
    void Update_A();//更新A相数据
    void Update_B();//更新A相数据
    void Update_C();//更新A相数据

    //辅助线
    QCPItemEllipse * ellipse;//圆

    QCPItemLine * line_0,*line_1,*line_2,*line_3;//四条线
private:
    Ui::Dialog_DataView *ui;
    STR3060_Context *ctx;
public slots:
    void UI_Timer_timeout();

};

#endif // DIALOG_DATAVIEW_H

#include "dialog_dataview.h"
#include "ui_dialog_dataview.h"

Dialog_DataView::Dialog_DataView(STR3060_Context *ctx_p,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_DataView)
{
    {//定时器
        UI_Timer=new QTimer(this);
        UI_Timer->start(900);
        connect(UI_Timer,SIGNAL(timeout()),this,SLOT(UI_Timer_timeout()));
    }
    ui->setupUi(this);
    setFixedSize(size());
    setWindowTitle("电压电流数据");
    setWindowIcon(QIcon(":/window_resources.ico"));//设置窗口图标
    setWindowFlags(Qt::Tool);
    ctx=ctx_p;
    {//初始化坐标系
        ui->view->xAxis2->setVisible(true);
        ui->view->xAxis2->setLabel("电流实轴（A）");
        ui->view->yAxis2->setVisible(true);
        ui->view->yAxis2->setLabel("电流虚轴（A）");

        ui->view->xAxis->setLabel("电压实轴（V）");
        ui->view->yAxis->setLabel("电压虚轴（V）");

        {//做辅助线
            {//画圆
                ellipse=new QCPItemEllipse(ui->view);
                ellipse->setVisible(true);
                ellipse->setPen(QPen(Qt::black));


            }
            {//线
                line_0=new QCPItemLine(ui->view);
                line_0->setVisible(true);
                line_0->setPen(QPen(Qt::yellow));



                line_1=new QCPItemLine(ui->view);
                line_1->setVisible(true);
                line_1->setPen(QPen(Qt::yellow));

                line_2=new QCPItemLine(ui->view);
                line_2->setVisible(true);
                line_2->setPen(QPen(Qt::yellow));

                line_3=new QCPItemLine(ui->view);
                line_3->setVisible(true);
                line_3->setPen(QPen(Qt::yellow));



            }
        }



    }

}

#include "math.h"
#define PI (3.14159265)
void Dialog_DataView::Update_Axis()
{
    ui->view->xAxis->setRange(-STR3060_Get_ULimit(ctx,STR3060_A)*1.2,STR3060_Get_ULimit(ctx,STR3060_A)*1.2);//设置电压轴
    ui->view->yAxis->setRange(-STR3060_Get_ULimit(ctx,STR3060_A)*1.2,STR3060_Get_ULimit(ctx,STR3060_A)*1.2);//设置电压轴

    ui->view->xAxis2->setRange(-STR3060_Get_ILimit(ctx,STR3060_A)*1.2,STR3060_Get_ILimit(ctx,STR3060_A)*1.2);//设置电流轴
    ui->view->yAxis2->setRange(-STR3060_Get_ILimit(ctx,STR3060_A)*1.2,STR3060_Get_ILimit(ctx,STR3060_A)*1.2);//设置电流轴

    {//设置辅助线
        double r=STR3060_Get_ULimit(ctx,STR3060_A);
        QList< QCPItemPosition * > pos=ellipse->positions();
        ellipse->topLeft->setAxes(ui->view->xAxis,ui->view->yAxis);
        ellipse->topLeft->setCoords(-r,r);
        ellipse->bottomRight->setAxes(ui->view->xAxis,ui->view->yAxis);
        ellipse->bottomRight->setCoords(r,-r);

        {//线
            double pos0_x=r*cos(PI/3.0),pos0_y=r*sin(PI/3.0),pos1_x=r*cos(PI/6),pos1_y=r*sin(PI/6);

            line_0->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_0->start->setCoords(-pos0_x,-pos0_y);
            line_0->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_0->end->setCoords(pos0_x,pos0_y);

            line_1->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_1->start->setCoords(-pos1_x,-pos1_y);
            line_1->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_1->end->setCoords(pos1_x,pos1_y);

            line_2->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_2->start->setCoords(-pos0_x,pos0_y);
            line_2->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_2->end->setCoords(pos0_x,-pos0_y);

            line_3->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_3->start->setCoords(-pos1_x,pos1_y);
            line_3->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line_3->end->setCoords(pos1_x,-pos1_y);


        }
    }

}

void Dialog_DataView::UI_Timer_timeout()
{

    Update_Axis();
    Update_A();
    Update_B();
    Update_C();
    ui->view->replot();//更新界面
}

#include "math.h"
#define PI (3.14159265)
#include <QVector>
void Dialog_DataView::Update_A()
{
    {//设置电压
        static QCPItemLine * line=NULL;
        if(line ==NULL)
        {
            line=new QCPItemLine(ui->view);

            line->setPen(QPen(Qt::red));
            line->setVisible(true);
            line->setHead(QCPLineEnding::esSpikeArrow);
        }
        {
            double X_End=STR3060_Get_U(ctx,STR3060_A)*cos(PI*STR3060_Get_U_Phase(ctx,STR3060_A)/180);
            double Y_End=STR3060_Get_U(ctx,STR3060_A)*sin(PI*STR3060_Get_U_Phase(ctx,STR3060_A)/180);
            line->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line->start->setCoords(0,0);

            line->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line->end->setCoords(X_End,Y_End);

            {//写文字
                static QCPItemText * text=new  QCPItemText(ui->view);
                text->setPen(QPen(Qt::red));
                text->setColor(QColor(Qt::red));
                text->setFont(QFont(font().family(),8));//设置字体
                text->setText("U(A)");
                text->position->setAxes(ui->view->xAxis,ui->view->yAxis);
                text->position->setCoords(X_End*2/3,Y_End*2/3);
            }

        }

    }

    {//设置电流
        static QCPItemLine * line=NULL;
        if(line ==NULL)
        {
            line=new QCPItemLine(ui->view);
            line->setPen(QPen(Qt::red));
            line->setVisible(true);
            line->setHead(QCPLineEnding::esSpikeArrow);
        }
        {
            double X_End=STR3060_Get_I(ctx,STR3060_A)*cos(PI*STR3060_Get_I_Phase(ctx,STR3060_A)/180);
            double Y_End=STR3060_Get_I(ctx,STR3060_A)*sin(PI*STR3060_Get_I_Phase(ctx,STR3060_A)/180);
            line->start->setAxes(ui->view->xAxis2,ui->view->yAxis2);
            line->start->setCoords(0,0);

            line->end->setAxes(ui->view->xAxis2,ui->view->yAxis2);
            line->end->setCoords(X_End,Y_End);
            {//写文字
                static QCPItemText * text=new  QCPItemText(ui->view);
                text->setPen(QPen(Qt::red));
                text->setColor(QColor(Qt::red));
                text->setFont(QFont(font().family(),8));//设置字体
                text->setText("I(A)");
                text->position->setAxes(ui->view->xAxis2,ui->view->yAxis2);
                text->position->setCoords(X_End/3,Y_End/3);
            }
        }
    }
}

void Dialog_DataView::Update_B()
{
    {//设置电压
        static QCPItemLine * line=NULL;
        if(line ==NULL)
        {
            line=new QCPItemLine(ui->view);

            line->setPen(QPen(Qt::blue));
            line->setVisible(true);
            line->setHead(QCPLineEnding::esSpikeArrow);
        }
        {
            double X_End=STR3060_Get_U(ctx,STR3060_B)*cos(PI*STR3060_Get_U_Phase(ctx,STR3060_B)/180);
            double Y_End=STR3060_Get_U(ctx,STR3060_B)*sin(PI*STR3060_Get_U_Phase(ctx,STR3060_B)/180);
            line->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line->start->setCoords(0,0);

            line->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line->end->setCoords(X_End,Y_End);

            {//写文字
                static QCPItemText * text=new  QCPItemText(ui->view);
                text->setPen(QPen(Qt::blue));
                text->setColor(QColor(Qt::blue));
                text->setFont(QFont(font().family(),8));//设置字体
                text->setText("U(B)");
                text->position->setAxes(ui->view->xAxis,ui->view->yAxis);
                text->position->setCoords(X_End*2/3,Y_End*2/3);
            }

        }

    }

    {//设置电流
        static QCPItemLine * line=NULL;
        if(line ==NULL)
        {
            line=new QCPItemLine(ui->view);
            line->setPen(QPen(Qt::blue));
            line->setVisible(true);
            line->setHead(QCPLineEnding::esSpikeArrow);
        }
        {
            double X_End=STR3060_Get_I(ctx,STR3060_B)*cos(PI*STR3060_Get_I_Phase(ctx,STR3060_B)/180);
            double Y_End=STR3060_Get_I(ctx,STR3060_B)*sin(PI*STR3060_Get_I_Phase(ctx,STR3060_B)/180);
            line->start->setAxes(ui->view->xAxis2,ui->view->yAxis2);
            line->start->setCoords(0,0);

            line->end->setAxes(ui->view->xAxis2,ui->view->yAxis2);
            line->end->setCoords(X_End,Y_End);
            {//写文字
                static QCPItemText * text=new  QCPItemText(ui->view);
                text->setPen(QPen(Qt::blue));
                text->setColor(QColor(Qt::blue));
                text->setFont(QFont(font().family(),8));//设置字体
                text->setText("I(B)");
                text->position->setAxes(ui->view->xAxis2,ui->view->yAxis2);
                text->position->setCoords(X_End/3,Y_End/3);
            }
        }
    }
}

void Dialog_DataView::Update_C()
{
    {//设置电压
        static QCPItemLine * line=NULL;
        if(line ==NULL)
        {
            line=new QCPItemLine(ui->view);

            line->setPen(QPen(Qt::green));
            line->setVisible(true);
            line->setHead(QCPLineEnding::esSpikeArrow);
        }
        {
            double X_End=STR3060_Get_U(ctx,STR3060_C)*cos(PI*STR3060_Get_U_Phase(ctx,STR3060_C)/180);
            double Y_End=STR3060_Get_U(ctx,STR3060_C)*sin(PI*STR3060_Get_U_Phase(ctx,STR3060_C)/180);
            line->start->setAxes(ui->view->xAxis,ui->view->yAxis);
            line->start->setCoords(0,0);

            line->end->setAxes(ui->view->xAxis,ui->view->yAxis);
            line->end->setCoords(X_End,Y_End);

            {//写文字
                static QCPItemText * text=new  QCPItemText(ui->view);
                text->setPen(QPen(Qt::green));
                text->setColor(QColor(Qt::green));
                text->setFont(QFont(font().family(),8));//设置字体
                text->setText("U(C)");
                text->position->setAxes(ui->view->xAxis,ui->view->yAxis);
                text->position->setCoords(X_End*2/3,Y_End*2/3);
            }

        }

    }

    {//设置电流
        static QCPItemLine * line=NULL;
        if(line ==NULL)
        {
            line=new QCPItemLine(ui->view);
            line->setPen(QPen(Qt::green));
            line->setVisible(true);
            line->setHead(QCPLineEnding::esSpikeArrow);
        }
        {
            double X_End=STR3060_Get_I(ctx,STR3060_C)*cos(PI*STR3060_Get_I_Phase(ctx,STR3060_C)/180);
            double Y_End=STR3060_Get_I(ctx,STR3060_C)*sin(PI*STR3060_Get_I_Phase(ctx,STR3060_C)/180);
            line->start->setAxes(ui->view->xAxis2,ui->view->yAxis2);
            line->start->setCoords(0,0);

            line->end->setAxes(ui->view->xAxis2,ui->view->yAxis2);
            line->end->setCoords(X_End,Y_End);
            {//写文字
                static QCPItemText * text=new  QCPItemText(ui->view);
                text->setPen(QPen(Qt::green));
                text->setColor(QColor(Qt::green));
                text->setFont(QFont(font().family(),8));//设置字体
                text->setText("I(C)");
                text->position->setAxes(ui->view->xAxis2,ui->view->yAxis2);
                text->position->setCoords(X_End/3,Y_End/3);
            }
        }
    }
}

Dialog_DataView::~Dialog_DataView()
{
    delete ui;
}

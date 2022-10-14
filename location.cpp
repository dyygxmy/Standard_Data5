#include "location.h"
#include "ui_location.h"

Location::Location(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Location)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏

//    Qt::WindowFlags flags = Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;
//#ifndef WIN32
//    flags |= Qt::X11BypassWindowManagerHint;
//#endif

    ui->locationtitle_label->setText("无线定位系统");
    ui->locationtitle_label->setStyleSheet("font: 20pt;color: rgb(0, 0, 0);");
//    this->setWindowFlags( flags );
    QBrush backRole;
    backRole.setColor(QColor(248, 248, 255));
    backRole.setStyle(Qt::SolidPattern);
    ui->plot->setBackground(backRole);
    statistical = new QCPStatisticalBox(ui->plot->xAxis, ui->plot->yAxis);
    QBrush boxBrush(QColor(200, 60, 255, 200));
    statistical->setBrush(boxBrush);
}

Location::~Location()
{
    delete ui;
}

void Location::showPlot(QPointF P)
{
//    statistical->data().data()->clear();
    ui->label_2->setText(QString("(%1,%2)").arg(P.x()).arg(P.y()));
    statistical->addData(P.x(),0,0,0,0,0, QVector<double>() << P.y());
    ui->plot->replot();
}

void Location::setupPlot( QPointF a,QPointF b, QPointF c,QPointF d)
{  
    qint64 ab_length,bc_length,ad_length,dc_length;
    if(qAbs(b.x()-a.x())>qAbs(b.y()-a.y()))
        ab_length=qAbs(b.x()-a.x())+1;
    else
        ab_length=qAbs(b.y()-a.y())+1;

    if(qAbs(b.x()-c.x())>qAbs(b.y()-c.y()))
        bc_length=qAbs(b.x()-c.x())+1;
    else
        bc_length=qAbs(b.y()-c.y())+1;

    if(qAbs(d.x()-a.x())>qAbs(d.y()-a.y()))
        ad_length=qAbs(d.x()-a.x())+1;
    else
        ad_length=qAbs(d.y()-a.y())+1;

    if(qAbs(d.x()-c.x())>qAbs(d.y()-c.y()))
        dc_length=qAbs(d.x()-c.x())+1;
    else
        dc_length=qAbs(d.y()-c.y())+1;

    QVector<double> x0(ab_length),x1(bc_length),x2(ad_length),x3(dc_length), y0(ab_length), y1(bc_length),y2(ad_length),y3(dc_length);

    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::blue));
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(Qt::blue));
    ui->plot->addGraph();
    ui->plot->graph(2)->setPen(QPen(Qt::blue));
    ui->plot->addGraph();
    ui->plot->graph(3)->setPen(QPen(Qt::blue));

    for(int i=0;i<Temp_MAX+1;++i)//ab  x++,y
    {
        if((i>=a.x())&&(i<=b.x()))
        {
            x0[i-a.x()]=i;
            y0[i-a.x()]=a.y();
        }
        if((i>=b.y())&&(i<=c.y()))
        {
            x1[i-b.y()]=b.x();
            y1[i-b.y()]=i;
        }
        if((i>=a.y())&&(i<=d.y()))
        {
            x2[i-a.y()]=d.x();
            y2[i-a.y()]=i;
        }
        if((i>=d.x())&&(i<=c.x()))
        {
            x3[i-d.x()]=i;
            y3[i-d.x()]=c.y();
        }
        //        x0[i]=i;
        //        y0[i]=a.x();
        //        x1[i]=b.x();
        //        y1[i]=i;
        //        x2[i]=d.x();
        //        y2[i]=i;
        //        x3[i]=i;
        //        y3[i]=c.y();
    }
    ui->plot->graph(0)->setData(x0,y0);//ab
    ui->plot->graph(1)->setData(x1,y1);//bc
    ui->plot->graph(2)->setData(x2,y2);//ad
    ui->plot->graph(3)->setData(x3,y3);//dc
}


QPointF Location::Linearequation(QPointF a,QPointF b)
{
    double k,z;
    k=((b.y()-a.y())/(b.x()-a.x()));
    z=a.y()-k*a.x();
    //qDebug()<<k<<"ll"<<z;
    QPointF p(k,z);
    return p;
}

void Location::horzScrollBarChanged(int value)
{
    if (qAbs(ui->plot->xAxis->range().center()-value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
    {
        ui->plot->xAxis->setRange(value/100.0, ui->plot->xAxis->range().size(), Qt::AlignCenter);
        ui->plot->replot();
    }
}

void Location::vertScrollBarChanged(int value)
{
    if (qAbs(ui->plot->yAxis->range().center()+value/100.0) > 0.01) // if user is dragging plot, we don't want to replot twice
    {
        ui->plot->yAxis->setRange(-value/100.0, ui->plot->yAxis->range().size(), Qt::AlignCenter);
        ui->plot->replot();
    }
}

void Location::on_pushButton_close_clicked()
{
    emit closeDiglag();
}



double Location::max_value(double* arr,double size)
{
    double max=arr[0];
    for (int i=1;i<size;i++)
        if(max < arr[i]) max = arr[i];
    return max;
}

void Location::Paintrectangle(QPointF POINT_A,QPointF POINT_B,QPointF POINT_C,QPointF POINT_D)
{
    double tempBuf[8]={POINT_A.x(),POINT_A.y(),POINT_B.x(),POINT_B.y(),POINT_C.x(),POINT_C.y(),POINT_D.x(),POINT_D.y()};
    Temp_MAX = max_value(tempBuf,8);
    setupPlot(POINT_A,POINT_B,POINT_C,POINT_D);
    // initialize axis range (and scroll bar positions via signals we just connected):
    ui->plot->xAxis->setLabel("x(mm)");
    ui->plot->yAxis->setLabel("y(mm)");
    ui->plot->xAxis->setRange(POINT_A.x()-2000, Temp_MAX+5000,Qt::AlignLeft);
    ui->plot->yAxis->setRange(POINT_A.y()-2000, Temp_MAX+5000,Qt::AlignLeft);
    ui->plot->axisRect()->setupFullAxesBox(true);
    ui->plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}



void Location::on_PushButton_clearpoint_clicked()//add 20180116
{
    statistical->data().data()->clear();
}

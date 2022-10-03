#ifndef LOCATION_H
#define LOCATION_H

#include "qcustomplot.h"
#include <QGraphicsDropShadowEffect>
#include <QDialog>
#include "GlobalVarible.h"

namespace Ui {
class Location;
}

class Location : public QDialog
{
    Q_OBJECT

public:
    explicit Location(QWidget *parent = 0);
    ~Location();
    void setupPlot(QPointF,QPointF,QPointF,QPointF);
    QPointF Linearequation(QPointF,QPointF);
    double max_value(double *arr, double);
    void Paintrectangle(QPointF POINT_A,QPointF POINT_B,QPointF POINT_C,QPointF POINT_D);
signals:
    void closeDiglag();

public slots:
    void  showPlot(QPointF);

private slots:
    void horzScrollBarChanged(int value);
    void vertScrollBarChanged(int value);
    //    void xAxisChanged(QCPRange range);
    //    void yAxisChanged(QCPRange range);
    void on_pushButton_close_clicked();

    void on_PushButton_clearpoint_clicked();

private:
    Ui::Location *ui;
    QCPStatisticalBox *statistical;
    double Temp_MAX;
};

#endif // LOCATION_H

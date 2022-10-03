#ifndef REPLACEBARCODE_H
#define REPLACEBARCODE_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QList>
#include <GlobalVarible.h>
//#include "serialcom.h"
#include "qextserial/qextserialport.h"

#include <QApplication>
#include "GlobalVarible.h"

//延时，TIME_OUT是串口读写的延时
#define TIME_OUT 10

class ReplaceBarcode : public QObject
{
    Q_OBJECT
public:
    explicit ReplaceBarcode(QObject *parent = 0);
    
signals:
    void signalReplaceStatus( QVariantMap ) ;
    void signalQRCodeFromSerial(QString QRCode) ;
    
public slots:
    void myreplace();
    void mysqlopen();
    void sqlclose();
    void repalace(QString VIN, QString code);
//    void slot_read_com();
    void readMyCom();
    void on_actionOpen_triggered();

private:
    QThread replace_thread;
    QSqlDatabase db2;
    QSqlQuery query2;
//    SerialCom serial_Com;      //串口类
    QTimer comsTimer;
    QString serialNums;
    char buff[20];
//    bool flag_l;
    int Ser_Fd;               //串口文件描述符
    bool isequeal;
    QList<QString> list;
    QextSerialPort *myCom;
    QString tempbuf;
};

#endif // REPLACEBARCODE_H

#ifndef GWKTHREAD_H
#define GWKTHREAD_H

#include <QObject>
#include <qextserial/qextserialport.h>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "GlobalVarible.h"
#include "qsettings.h"
class GwkThread : public QObject
{
    Q_OBJECT
public:
    explicit GwkThread(QObject *parent = 0);
    
signals:    
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void IsTigntenReady(bool);
public slots:

    void gwkstart();
    void slot_read_com();
    void gwkSendCmdFun(char);
    void sendReadOperate(bool,int);
    void mysql_open();

private:
    QThread m_thread;
    QextSerialPort *myCom3;
    
    QByteArray torqueValue;
    QByteArray angleValue;
    QByteArray gwkStatus;
    QByteArray revRsDataBuf;
    QByteArray sendRsDataBuf;
    QString  channel;
    QString  vinBuf;
    QString  boltNumberBuf;
    QString  programNO;
    int  boltNum;
    bool gwkStartFlag;
    bool gwkCloseFlag;
    bool gwkOpenStatusFlag;
    short TorqueMin;
    short TorqueMax;
    QSqlDatabase db2;
    QSqlQuery query2;
    int Cycle;
};

#endif // GWKTHREAD_H

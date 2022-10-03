#ifndef TIGHTEN_OP_RS232_H
#define TIGHTEN_OP_RS232_H

#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
#include <qextserial/qextserialport.h>

class Tighten_OP_Rs232 : public QObject
{
    Q_OBJECT
public:
    explicit Tighten_OP_Rs232(QObject *parent = 0);
    QString vari1;    
    void setRepair(bool temp);
signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void IsTigntenReady(bool);
private:
    QThread m_thread;
    QextSerialPort *myCom;
    QTimer *timer5000ms;

    int  nexoHandleFlag;             // NC/Time set/set time/system error system/system error system ACK/old data upload/data ACK/data subscribe
                                    // Alive/NC/NC/NC/NC/NC/NC/vin download
                                    // curve Angle subscribe/curve Torque subscribe/curve ACK/71 alarm ack/NC/enable tool/disable tool/tool data
                                    // Tightening program numbers/NC/NC/commucation stop/reset batch counter/set batch size/selected program/commucaiton start
    short programNO;
    short boltCount;
    short boltNokCount;
    int   intReadTighteningID;
    bool  controlLinkOk;
    int   errCount;
    int nexoAliveCount;
    QByteArray vinBuf;
    QByteArray boltNumberBuf;

    QByteArray nexoProNO;
    QString nexoResult;
    QByteArray torqueValue;
    QByteArray angleValue;
    QByteArray tighenTime;
    QByteArray tighteningID;
    QByteArray Channel;

    QByteArray opStart;
    QByteArray opEnd;
    QByteArray revRsDataBuf;
    int cmdLen;
    int cmdMID;
    int groupNum;
    bool isRepair;

private slots:
    void tightenStart();
    void ctlNexoHandle();
    void slot_read_com3();
    void timerFunc5000ms();
    void sendReadOperate(bool,int);
    bool comInit();
};

#endif // TIGHTENTHREAD_H


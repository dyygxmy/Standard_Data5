#ifndef MASTERTCP_H
#define MASTERTCP_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QtNetwork>
#include <QDebug>

#include "GlobalVarible.h"

class MasterTCP : public QObject
{
    Q_OBJECT
public:
    explicit MasterTCP(QObject *parent = 0);

signals:
    void sendCmdClearPinCode();
public slots:
    void masterServerStart();
    void DataReceived();
    void heartTimerFunc();
    void closeTcpServer();
    void recMessage();
    void revRFID_PIN(QString,bool,QString);
    void sendCmdToSalve(QString,QString,int);
private:
    QTcpServer *tcpServer1;
    QTcpSocket *tcpSocket1;
    QThread tcp_thread;

    QByteArray revBuf;
    QString pin_Code,G9_Code;
    bool socketConnectFlag;
    bool connectFlag;
    int timerCount;
    int timerCount1;
    bool sendPinFlag;
    bool systemStatueFlag;
    bool controlStatueFlag;

};

#endif // MASTERTCP_H

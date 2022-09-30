#ifndef QUICKREQUEST_H
#define QUICKREQUEST_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QtNetwork>
#include <QTimer>
#include "GlobalVarible.h"

class QuickRequest : public QObject
{
    Q_OBJECT
public:
    explicit QuickRequest(QObject *parent = 0);
//    QByteArray addzero(int m, QByteArray s);
    QByteArray SumChk(QByteArray);

signals:
    void sendJob(QString);
    void JobError(int);

public slots:
    void requestStart();
    void request(QString);
    void sendRequest();
    void readPendingDatagrams();
    void sendHeart();
    void receiveErrorCode(bool,bool,bool);

private:
    QThread m_thread;
    QUdpSocket * sender;
    QHostAddress dataserver;
    QTimer HeartTimer;
    int OutTime_times;
    int IDCode_OutTime_times;
    bool isRFIDConnected;
    bool isController1;
    bool isController2;
    unsigned char errorcode;
    QByteArray station;
    QByteArray idcode;
};

#endif // QUICKREQUEST_H

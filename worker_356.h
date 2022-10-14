#ifndef WORKER356_H
#define WORKER356_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QtNetwork>
#include <QDomDocument>
#include <QtCore/QCoreApplication>
#include <QSqlDatabase>
#include <QDebug>
#include <QStringList>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include "GlobalVarible.h"
#include <QSettings>

class Worker356 : public QObject
{
    Q_OBJECT
public:
    explicit Worker356(QObject *parent = 0);
    void dbconnect();

    double dXVA ;
    double dY1V ;  //double 类型的 Y1V

signals:
//    void haverec(int);
//    void clo2();
    void sendfromworkthread(QVariant);
    void havedconnect(bool);
    void disconnectTellPortA();
    void send_mainwindow(QString,QString,QString,int,int,int);
    void coming_VIN(QString) ;
    void VIN_Match_Wrong(QString,QString);
    void sendfromjsonthread(QVariant);

public slots:
    void recMessage();
    void DataReceived();
    void fun1();
    void closePortBTcpServer();
    void xml_parse(QByteArray);
//    void heartTimerFunc();
//    void portAsendPortB();
//    void timeoutHeartBeat() ;
//    void slot_tightenConnectStatus(bool);

private:
    QTcpServer *tcpServer;
    QTcpSocket *m_pTcpSocket;
//    QTimer *heartTimer;
    QThread m_thread;
    QByteArray receiveResult;
    QByteArray receiveResultDo;
    QDomNode node;
//    QString data_model[11];
    QString CurveStr;
    int timerCount;
    QString CyclePrevious;
    QString channelPrevious ;
//    QString data_model[15];
    QString Factory;
    bool socket4710Flag;
    QString vinCode;

    QString screwid[4];
//    QString CurveStr;
//    QString CyclePrevious;
//    QString vinCode;
//    QByteArray receiveResult;
//    QByteArray receiveResultDo;
//    int timerCount;
    int whichar;  //匹配出来的是哪个车型
    bool pro_is_true;
    bool ppFlag;
    int screwid_enable[20];

    bool matchVin();
    int testChannel ;
//    QTimer *timerHeartBeat ;
//    int timeoutHeartBeatCount ;
    QString configParse(QString chanel_tmp, QString program_tmp,QString state_ok);
//    bool pingStatus;//ping的状态

};

#endif // WORKER356_H

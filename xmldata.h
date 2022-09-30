#ifndef XMLDATA_H
#define XMLDATA_H

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

class xmlData : public QObject
{
    Q_OBJECT
public:
    explicit xmlData(QObject *parent = 0);
    QString vari1;
    double dXVA ;
    double dY1V ;

signals:
    void send_mainwindow(QString,QString,QString,int,int,int);
    void sendfromjsonthread(QVariant);
    void coming_VIN(QString);
    void VIN_Match_Wrong(QString,QString);
    void havedconnect(bool);

public slots:
    void xmlInit();
    void DataReceived();
    void heartTimerFunc();
    void closePortBTcpServer();
    void recMessage();
    void xml_parse(QByteArray);
    QString configParse(QString,QString,QString);
    bool matchVin();

private:
    QThread xml_thread;
    QTimer *xmlTimer;
    QTcpServer *tcpServer;
    QTcpSocket *m_pTcpSocket;
    QDomNode node;
    QString screwid[3];
    QString CurveStr;
    QString CyclePrevious;
    QString vinCode;
    QByteArray receiveResult;
    QByteArray receiveResultDo;
    int timerCount;
    int whichar;  //匹配出来的是哪个车型
    int screwid_enable[20];
    bool socket4710Flag;
    bool pro_is_true;
    bool ppFlag;

};

#endif // XMLDATA_H

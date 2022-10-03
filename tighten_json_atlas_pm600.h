#ifndef TIGHTEN_JSON_ATLAS_PM600_H
#define TIGHTEN_JSON_ATLAS_PM600_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include "GlobalVarible.h"

#include "./json/serializer.h"
#include "./json/parser.h"

class TightenJsonAtlasPM600 : public QObject
{
    Q_OBJECT
public:
    explicit TightenJsonAtlasPM600(QObject *parent = 0);
//    void mysql_open();

    void setRepair(bool temp);
signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void IsTigntenReady(bool);
public slots:
    void init();
    void sendReadOperate(bool,int);

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req;
    QTimer PostTimer;
    QSqlDatabase db2;
    QSqlQuery query2;
    bool ErrorCode;
    int sessionId;
    QString torqueValue;
    QString angleValue;
    QString resultStatus;
    QString programNO;
    QString tightenTime;
    int tighteningID;
    int tighteningID_Max;
    int aliveCount;
    bool PF_Connect_Flag;
    bool powerONFlag;

    QString boltNumberBuf;
    QString vinBuf;
    int boltCount ;
    int proNum;
    int Channel ;
    bool isRepair;
private slots:
    void finishedSlot(QNetworkReply *);
    void dataPost();

};

#endif // TIGHTEN_JSON_ATLAS_PM600_H

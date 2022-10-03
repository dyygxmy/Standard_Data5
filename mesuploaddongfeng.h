#ifndef MESUPLOADDONGFENG_H
#define MESUPLOADDONGFENG_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include <QTime>
#include <QElapsedTimer>
#include <QApplication>
#include "GlobalVarible.h"

#include "./json/serializer.h"
#include "./json/parser.h"

class MESUploadDongfeng : public QObject
{
    Q_OBJECT
public:
    explicit MESUploadDongfeng(QObject *parent = 0);
    void mysql_open();

signals:
    void sendVinToUi(QString,bool,QString); //发送条码给UI
    void sendGetCar();

public slots:
    void init();

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req_Getcardata;
    QNetworkRequest req_Operate;
    QNetworkRequest req_Submit;
    QTimer InsertTimer;
    QSqlDatabase db2;
    QSqlQuery query2;
    int GetTimes;
    bool isFirst;
    int wrongRecordID;
    int wrongTimes;
    int TIMEOUT;
    int enablePercentage;

private slots:
    void finishedSlot(QNetworkReply *);
    void dataInsert();

};

#endif // MESUPLOADDONGFENG_H

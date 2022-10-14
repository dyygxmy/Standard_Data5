#ifndef QUEUEINTERFACE_H
#define QUEUEINTERFACE_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include <QTime>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "GlobalVarible.h"

#include "./json/serializer.h"
#include "./json/parser.h"

class QueueInterface : public QObject
{
    Q_OBJECT
public:
    explicit QueueInterface(QObject *parent = 0);
    void mysqlopen();

signals:
    void sendVinToUi(QString,bool,QString); //发送条码给UI
    void sendGetCar();
    void sendQueueError(int);
    void sendAlign(QString,QString);

public slots:
    void init();
    void delete_car(QString);
    void IsTightenReady(int,bool);
    void IsTightenReady(bool);

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req_Getcardata;
    QNetworkRequest req_Operate;
    QTimer GetTimer;
    int TIMEOUT;
    int enablePercentage;
    bool isReady;
    bool isFirst;
    QSqlDatabase db2;
    QSqlQuery query2;

private slots:
    void getcardata();

};

#endif // QUEUEINTERFACE_H

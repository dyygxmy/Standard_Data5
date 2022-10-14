#ifndef QUEUETHREAD_H
#define QUEUETHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "./json/serializer.h"
#include "./json/parser.h"

#include "GlobalVarible.h"

class queueThread : public QObject
{
    Q_OBJECT
public:
    explicit queueThread(QObject *parent = 0);

signals:
    void time_error(bool);
    void sendTime(QString);
    void FisSendSerial(QString,QString); //

public slots:
    void getData(int,QString);
    void init();
    void firstGetData();
    void getCarInfoAgain();

private:
    QThread m_thread;

    QSqlDatabase        mDataBase;
    QSqlQuery           query2;

    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest getVIN;
    QNetworkRequest errorMessage;
    int HTTPTIMEOUT;
    int _type;
    QString _strVin ;

};

#endif // QUEUETHREAD_H

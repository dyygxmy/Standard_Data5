#ifndef GETCARINFORMATION_H
#define GETCARINFORMATION_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QApplication>
#include <QSqlDatabase>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <QDebug>

#include "GlobalVarible.h"
#include "serializer.h"
#include "parser.h"

class getCarInformation : public QObject
{
    Q_OBJECT
public:
    explicit getCarInformation(QObject *parent = 0);

signals:
    void time_error(bool);
    void sendTime(QString);
    void requst(bool,int); //


public slots:
    void getData(QString,bool,int);
    void mysql_open();
    void init();

private:
    QThread m_thread;
    QSqlDatabase db2;
    QSqlQuery query2;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest getVIN;
    QNetworkRequest errorMessage;
    int HTTPTIMEOUT;
};

#endif // GETCARINFORMATION_H





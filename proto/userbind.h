#ifndef USERBIND_H
#define USERBIND_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QTime>
#include <QMap>
#include "./json/serializer.h"
#include "./json/parser.h"
#include "GlobalVarible.h"

class userBind : public QObject
{
    Q_OBJECT
public:
    explicit userBind(QObject *parent = 0);

    enum requestType
    {
        login = 1,
        logout ,
        defult
    };

    bool setUrlRequest( userBind::requestType, QVariantMap ) ;
    bool ErrorMessage( QString );
    bool bindStatus;

public:


private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req_Getproduction;
    QNetworkRequest req_ErrorMessage;
    QString Factory;
    int TIMEOUT;

signals:

public slots:

};

#endif // USERBIND_H



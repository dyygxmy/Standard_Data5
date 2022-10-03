#ifndef BAICINFOINTERFACE_H
#define BAICINFOINTERFACE_H

#include <QObject>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMap>
#include <QDebug>
#include <QTime>
#include <QApplication>
#include "GlobalVarible.h"

#include "./json/serializer.h"
#include "./json/parser.h"

class BaicInfoInterface : public QObject
{
    Q_OBJECT
public:
    explicit BaicInfoInterface(QObject *parent = 0);
    QString getProduction(QString);
    bool ErrorMessage(QString);

signals:

public slots:

private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req_Getproduction;
    QNetworkRequest req_ErrorMessage;
    QString Factory;
    int TIMEOUT;

};

#endif // BAICINFOINTERFACE_H

#ifndef MESUPLOADBAIC_H
#define MESUPLOADBAIC_H

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
#include <QTime>
#include <QApplication>
#include "GlobalVarible.h"

#include "./json/serializer.h"
#include "./json/parser.h"

class MESUploadBAIC : public QObject
{
    Q_OBJECT
public:
    explicit MESUploadBAIC(QObject *parent = 0);
    void mysql_open();

public slots:
    void init();

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req_uploaddata;
    QNetworkRequest req_gettime;
    QNetworkRequest reqFis;
    QSqlDatabase db2;
    QSqlQuery query2;
    int GetTimes;
    bool isFirst;
    int wrongRecordID;
    int wrongTimes;
    QString Factory;
    int TIMEOUT;

private slots:
    void dataInsert();

};

#endif // MESUPLOADBAIC_H

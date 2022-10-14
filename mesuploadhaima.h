#ifndef MESUPLOADHAIMA_H
#define MESUPLOADHAIMA_H

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

class MESUploadHaima : public QObject
{
    Q_OBJECT

signals:
    void replyRepair(int,QVariantMap);

public:
    explicit MESUploadHaima(QObject *parent = 0);
//    ~MESUploadHaima();
    void mysql_open();
    void setRepair(bool temp);

public slots:
    void init();

    void receiveRepairVIN(QString repairVIN);
private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkRequest req_Submit;
    QNetworkRequest req_RepairCheck;
    QTimer InsertTimer;
    QSqlDatabase db2;
    QSqlQuery query2;
    int GetTimes;
    int wrongRecordID;
    int wrongTimes;
    int TIMEOUT;
    bool isRepair;
    QString strIP ;

private slots:
//    void finishedSlot(QNetworkReply *);
    void dataInsert();
};

#endif // MESUPLOADHAIMA_H

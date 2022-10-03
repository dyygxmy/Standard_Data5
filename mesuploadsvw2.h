#ifndef MESUPLOADSVW2_H
#define MESUPLOADSVW2_H

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
#include <QtCore>

#include "./json/serializer.h"
#include "./json/parser.h"

class MESUploadSVW2 : public QObject
{
    Q_OBJECT
public:
    explicit MESUploadSVW2(QObject *parent = 0);

    QString GetVIN(QString pWerk, QString pSPJ, QString pKNR);
    QStringList GetVINs();

    void UpdateFisList();

public slots:
    void init();

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QNetworkRequest req;
//    QTimer InsertTimer;

    QSqlDatabase mDataBase;
    QSqlQuery   *query2;

    int GetTimes;
    bool isFirst;
    int wrongRecordID;
    int wrongTimes;
    QString Factory;
    int TIMEOUT;

    int  curRecordID;

    bool        mGetFisOk;
    QString     mDataServerIp;
    QString     mServerPort;
    int     dataInsertCount;

private slots:
//    void finishedSlot(QNetworkReply *);
    void dataInsert();
};

#endif // MESUPLOADSVW2_H

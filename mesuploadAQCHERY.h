#ifndef MESUPLOADAQCHERY_H
#define MESUPLOADAQCHERY_H

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

typedef enum{
    E_HttpUnkonw = 0,
    E_GetVinInfo,
    E_SendPLCHeartbeat,
    E_SendReady,
    E_SendVinVerifyOK,
    E_SendVinVerifyFail,
    E_SendBeginTightening,
    E_SendEndTightening,
    E_SendSetPassby,
    E_SendClearPassby,
    E_GetUserRoleLevel
} T_HttpType;

class MESUploadAQCHERY : public QObject
{
    Q_OBJECT
public:
    explicit MESUploadAQCHERY(QObject *parent = 0);

    void PLCDisconnected();

public slots:
    void init();
    void dataInsert();

    void slot_SendHttp();
    void SendHttp(int pType);

    void slot_DetectCard(QString);

    void slot_GetVinInfo();

signals:
    void serialCom(QString,bool,QString);//发给UI线程
    void signalPLCHeartbeat(bool);
    void sendResult(bool);

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;

    QNetworkRequest req_uploaddata;
    QNetworkRequest req_gettime;
    QNetworkRequest reqFis;

    QNetworkRequest reqVinInfo;
    QNetworkRequest reqUploadStationStatus;
    QNetworkRequest reqGetUserRoleLevel;

    QList<QString> seriallist;//条码链表 存储10个条码以内不能重复
    bool isEqual;            //条码是否重复
    int whichindex;           //当前条码索引

    QSqlDatabase        mDataBase;
    QSqlQuery           query2;

    int GetTimes;
    bool isFirst;
    int wrongRecordID;
    int wrongTimes;
    QString Factory;
    int TIMEOUT;

    QList<int>      mHttpTaskList;
    T_HttpType      mCurHttpTask;

    QString         mDeviceNo;

    QString         mCurVin;
    QString         mCurJobID;
    bool            mForcePassby;

    QString         mUserCardNo;
    QString         mStation;

    int             mCardFrom;  //
};

#endif // MESUPLOADBAIC_H

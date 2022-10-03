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
    E_CMD_None = 0,
    E_CMD_Ready,
    E_CMD_PLCHeartbeat,
    E_CMD_GetVinInfo,
    E_CMD_UploadVinVerify,
    E_CMD_UploadBeginTightening,
    E_CMD_UploadStopPassby,
    E_CMD_UploadStopTightening
} T_CMDTYPE;

class MESUploadAQCHERY : public QObject
{
    Q_OBJECT
public:
    explicit MESUploadAQCHERY(QObject *parent = 0);
    void mysql_open();

    void PLCDisconnected();

public slots:
    void init();
    void slot_SendHttp();

    void slot_SendPLCHeartbeat();
    void slot_SendReady();
    void slot_GetVinInfo();
    void slot_UploadVinVerify(int pResult);  //0:ok,1:err
    void slot_UploadBeginTightening();
    void slot_UploadStopTightening(int pResult);  //0:group ok, 1:passby
    void slot_UploadStopPassby();

private:
    QThread m_thread;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;

    QNetworkRequest req_uploaddata;
    QNetworkRequest req_gettime;
    QNetworkRequest reqFis;
    QNetworkRequest reqVinInfo;
    QNetworkRequest reqUploadStationStatus;

    QList<QString> seriallist;//条码链表 存储10个条码以内不能重复
    bool isEqual;            //条码是否重复
    int whichindex;           //当前条码索引

    QSqlDatabase db2;
    QSqlQuery query2;
    int GetTimes;
    bool isFirst;
    int wrongRecordID;
    int wrongTimes;
    QString Factory;
    int TIMEOUT;

    QString mDataServerIp;
    QString mServerPort;

    QString mDeviceNo;
    bool mResultState;
    bool mNeedSetErrFalse;

    QTimer          mSendHttpTimer;
    T_CMDTYPE       mHttpCmd;

private slots:
    void dataInsert();
    void slotSetErrFalse();
    void slotUploadResult0Timer();

signals:
    void serialCom(QString,bool,QString);//发给UI线程
    void signalSetStation(QString);
    void signalPLCHeartbeat(bool);

};

#endif // MESUPLOADBAIC_H

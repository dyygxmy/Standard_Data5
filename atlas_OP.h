#ifndef ATLAS_OP_H
#define ATLAS_OP_H


#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
class AtlasOP : public QObject
{
    Q_OBJECT
public:
    explicit AtlasOP(QObject *parent = 0);
    QString vari1;

    void setRepair(bool temp);
signals:
    void IsTigntenReady(bool);    
    void send_mainwindow(QString,QString,QString,int,int,int);
    void sendfromOpThread(QVariant);
    void coming_VIN(QString);
    void VIN_Match_Wrong(QString,QString);

private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
//    QTimer *timer5000ms;

    int  nexoHandleFlag;             // NC/Time set/set time/system error system/system error system ACK/old data upload/data ACK/data subscribe
                                    // Alive/NC/NC/NC/NC/NC/NC/vin download
                                    // curve Angle subscribe/curve Torque subscribe/curve ACK/71 alarm ack/NC/enable tool/disable tool/tool data
                                    // Tightening program numbers/NC/NC/commucation stop/reset batch counter/set batch size/selected program/commucaiton start
    short programNO;
    short boltCount;
    short boltNokCount;
    int   intReadTighteningID;
    //    int   intNexoTighteningID;

    int   nexoAliveCount;
    bool  nexoLinkOk;
    bool  nexoStartFlag;
    short nexoReadyFlag;
    bool  powerOnFlag;
    int   errCount;
    int   errType;
    int   angleSubFlag;
    int   curveDataFlag;
    int   sendVinFlag;
    int   sendNextEnableFlag;
    int   sendDisableFlag;
    QByteArray vinBuf;
    QByteArray oldVinCode;
    QByteArray boltNumberBuf;

    QByteArray nexoProNO;
    QByteArray nexoResult;
    QByteArray torqueValue;
    QByteArray angleValue;
    QByteArray tighenTime;
    QByteArray tighteningID;
    QByteArray nexoCycleID;
    QByteArray nexoVin;
    QByteArray Channel;

    QByteArray nexoOldTigheningID;
    bool isRepair;

    QString screwid[3];
    int whichar;  //匹配出来的是哪个车型
    int screwid_enable[20];
    bool ppFlag;
    int timerNum;
    QString strNexoResult ;
    QString strG9 ;
    bool isVin ;

private slots:
    void tightenStart();
    void disConnectDO();
    void newConnects();
    void ctlNexoHandle();
    void revNexoData();
    void timerFunc5000ms();
    bool matchVin();
    bool matchG9() ;
    QString configParse(QString,QString);

};
#endif

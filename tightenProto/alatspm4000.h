#ifndef ALATSPM4000_H
#define ALATSPM4000_H

#include <QObject>
#include <QThread>
#include <QtNetwork>
#include <QDateTime>
#include <QThread>
#include "GlobalVarible.h"

class alatsPM4000 : public QObject
{
    Q_OBJECT
public:
    explicit alatsPM4000(QObject *parent = 0);



private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
    int timerNum;
private:
    void splitNexoData( QByteArray ) ;
    void ctlNexoHandle() ;
    bool matchVin();
    QString configParse(QString ,QString );

private:
    QString screwid[3];
    int screwid_enable[20];
    int boltAmount ;
    int currentBolt ;
    bool chooseCarType ;
    bool ppFlag;

    int  nexoHandleFlag;             // NC/Time set/set time/system error system/system error system ACK/old data upload/data ACK/data subscribe
                                    // Alive/Job info subscribe/Job ACK/NC/NC/NC/NC/vin download
                                    // curve Angle subscribe/curve Torque subscribe/curve ACK/71 alarm ack/NC/enable tool/disable tool/tool data
                                    // Tightening program numbers/NC/NC/commucation stop/reset batch counter/set batch size/selected program/commucaiton start
    short programNO;
    short boltCount;
    short boltNokCount;
    int   intReadTighteningID;
    //    int   intNexoTighteningID;
    int   intNexoMaxTighteningID;

    int   nexoAliveCount;
    bool  nexoLinkOk;
    bool  readOldDataFlag;
    bool  nexoStartFlag;
    bool  readMaxTightenIDFlag;
    short nexoReadyFlag;
    bool  powerOnFlag;
    int   revDataLen;
    int   revFinishLen;
    int   curveFirstFlag;
    int   errCount;
    int   errType;
    int   angleSubFlag;
    int   curveDataFlag;
    int   sendVinFlag;
    int   sendNextEnableFlag;
    int   sendDisableFlag;
    QByteArray curveBuf;
    QString curveType;
    QString curveBufTemp;
    QString idCode;
    QByteArray vinBuf;
    QByteArray boltNumberBuf;

    QByteArray nexoOldTigheningID;
    QString Factory;
    QString curveNullBuf;

    QByteArray nexoProNO;
    QByteArray nexoResult;
    QByteArray torqueValue;
    QByteArray angleValue;
    QByteArray tighenTime;
    QByteArray tighteningID;
    QByteArray nexoCycleID;
    QByteArray nexoVin;
    QByteArray Channel;

signals:
    void IsTigntenReady(bool);
    void send_mainwindow(QByteArray,QString,QString,QString,int);
    void sendfromOpThread(QVariant);
    void coming_VIN(QString);
    void VIN_Match_Wrong(QString,QString);

public slots:
    void slotBoltAmount(int);
    void timerFunc5000ms() ;
    void slotSkipBolt() ;

private slots:
    void tightenStart();
    void disConnectDO();
    void newConnects();
    void revNexoData();


};

#endif // ALATSPM4000_H

#if 0
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



    QByteArray nexoOldTigheningID;


    int timerNum;
    QString strNexoResult ;
    QString strG9 ;
    bool isVin ;

private slots:


    void ctlNexoHandle();

    void timerFunc5000ms();
    bool matchVin();
    bool matchG9() ;
    QString configParse(QString,QString);

};
#endif

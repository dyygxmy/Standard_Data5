#ifndef TIGHTEN_OP_ATLAS_PMM_H
#define TIGHTEN_OP_ATLAS_PMM_H


#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
class TightenOpAtalsPMM : public QObject
{
    Q_OBJECT
public:
    explicit TightenOpAtalsPMM(QObject *parent = 0);

signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int);
    void IsTigntenReady(bool);

private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
    QTimer *timer5000ms;

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
    QByteArray curveBuf;
    QString curveType;
    QString curveBufTemp;
    QString idCode;
    QByteArray vinBuf;
    QByteArray boltNumberBuf;

    QByteArray nexoProNO;
    QByteArray TightenResult;
    QByteArray torqueValue;
    QByteArray angleValue;
    QByteArray tighenTime;
    QByteArray tighteningID;
    QByteArray nexoCycleID;
    QByteArray nexoVin;
    QByteArray Channel;

    QByteArray nexoOldTigheningID;
//    QString data_model[16];
    QString Factory;
    int StartBolt;
    int stationNo;
    QString curveNullBuf;

private slots:
    void tightenStart();
    void disConnectDO();
    void newConnects();
    void ctlNexoHandle();
    void revNexoData();
    void timerFunc5000ms();
    void sendReadOperate(bool,int);


};
#endif

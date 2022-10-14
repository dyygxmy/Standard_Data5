#ifndef TIGHTEN_OP_ATLAS_IRMM_H
#define TIGHTEN_OP_ATLAS_IRMM_H


#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
class TightenOpAtalsIRMM : public QObject
{
    Q_OBJECT
public:
    explicit TightenOpAtalsIRMM(QObject *parent = 0);
    QString controllerIP;
    int ch;

signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int);
    void IsTigntenReady(int,bool);

private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
//    QTimer *timer5000ms;

    int  nexoHandleFlag;             // NC/Time set/set time/system error system/system error system ACK/old data upload/data ACK/data subscribe
                                    // Alive/Job info subscribe/Job ACK/NC/NC/NC/NC/vin download
                                    // curve Angle subscribe/curve Torque subscribe/curve ACK/71 alarm ack/NC/enable tool/disable tool/tool data
                                    // Tightening program numbers/NC/NC/commucation stop/reset batch counter/set batch size/selected program/commucaiton start
    int StartBolt;
    int programNO;
    int boltCount;
    int boltNokCount;
    int   intReadTighteningID;
    int   intNexoMaxTighteningID;

    int   controlAliveCount;
    bool  controlLinkOk;
    bool  readOldDataFlag;
    bool  nexoStartFlag;
    bool  readMaxTightenIDFlag;
    bool  powerOnFlag;
    int   revDataLen;
    int   revFinishLen;
    int   curveFirstFlag;
    int   errCount;
    int   errType;
    int   angleSubFlag;
    int   curveDataFlag;
    int   sendNextEnableFlag;
    int   sendDisableFlag;
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
    int groupNum;

    QByteArray nexoOldTigheningID;
    QByteArray strJobID;

    QString controlType;
    QString controlType2;
    bool dataSubFlag;
    QByteArray MID;
    int IR_AliveCount;
    int curve_AngleTorque;
    QString curveNullBuf;



private slots:
    void tightenStart();
    void newConnects();
    void disConnectDO();
    void sendCMD_IR(int,QByteArray);
    void revIRData();
    void timerFunc1000ms();
    void sendReadOperate(bool,int);

};
#endif

#ifndef TIGHTEN_OP_ATLAS_PFC_H
#define TIGHTEN_OP_ATLAS_PFC_H


#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
class TightenOpAtlasPFC : public QObject
{
    Q_OBJECT
public:
    explicit TightenOpAtlasPFC(QObject *parent = 0);
    QString vari_2;
    QString vari_1;

signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void IsTigntenReady(bool);
    void QueueAlign(QString,bool,QString);//发给UI线程

private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
    QTimer *timer5000ms;

    int  nexoHandleFlag;             // NC/Time set/set time/system error system/system error system ACK/old data upload/data ACK/data subscribe
                                    // Alive/abort Job/Job off and reset/select Job/Job batch increment/vin ACK/vin subscribe4/vin download
                                    // curve Angle subscribe/curve Torque subscribe/curve ACK/71 alarm ack/curve unsubscribe/enable tool/disable tool/tool data
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
    //int   SenddataFlag;
    QByteArray curveBuf;
    QString curveType;
    QString curveBufTemp;
    QString curveBufTemp1;
    QString idCode;
    QByteArray vinBuf;
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
//    QString data_model[16];
    QByteArray strJobID;
    QString Factory;
    int curve_AngleTorque;
    QString curveNullBuf;
    bool resetJobOffFlag;
    int Detecttimes;
    int LastCycle;
    bool reconnectflag;
    bool Disconnectflag;

    void requestCycle();
    void receivenormaldata(int revLen, int cmdLen, int nexoMID, QByteArray recBuf);
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

﻿#ifndef TIGHTEN_OP_ATLAS_PF_H
#define TIGHTEN_OP_ATLAS_PF_H


#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
class TightenOpAtalsPF : public QObject
{
    Q_OBJECT
public:
    explicit TightenOpAtalsPF(QObject *parent = 0);
    QString vari1;

    void setRepair(bool temp);
signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void send_mainwindow(QString,QString,QString,int);
    void IsTigntenReady(bool);
    void sendVinToMain(QString);

private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
    QTimer *timer5000ms;

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
    QString Factory;
	QByteArray oldVinCode;
    bool isRepair;

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

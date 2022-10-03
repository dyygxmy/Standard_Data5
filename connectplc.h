#ifndef CONNECTPLC_H
#define CONNECTPLC_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QtNetwork>

class ConnectPLC : public QObject
{
    Q_OBJECT
public:
    explicit ConnectPLC(QObject *parent = 0);
    

signals:
    void nokBolt(int);
    void PLCStatus(int);
private:
    QThread plc_thread;
    QTcpSocket *tcpSocket;
    QTimer *timer5000ms;

    QByteArray startCmdBuf1;
    QByteArray startCmdBuf2;
    QByteArray readCmdBuf;
    QByteArray writeCmdBuf;
    int setUpCommFlag;
    int sendCmd;
    int aliveCount;
    bool plcLinkFlag;
    bool clearPLCFlag;
    bool sendVinFlag;
    QString VinBuf;
    int vinAddr;
    int vinLen;
    int sendCount;

public slots:
    void disConnectDO();
    void plcConnects();
    void ctlPLCHandle(QByteArray,bool);
    void revPLCData();
    void timerFuncXS();
    void writePLCFunc(int,int,QString);
    void revVinFunc(int,int,QString);
    void readPLCFunc(int,int);
    void PLCStart();
};

#endif // CONNECTPLC_H

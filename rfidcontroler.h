#ifndef RFIDCONTROLER_H
#define RFIDCONTROLER_H

#include <QObject>
#include <QThread>
#include <QtNetwork>
#include "GlobalVarible.h"
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class RfidControler : public QObject
{
    Q_OBJECT
public:
    explicit RfidControler(QObject *parent = 0);
    void mysqlopen();
    QString rfidip;
    int rfidport;
    QString vari_1;
    bool RFID_Enable_Flag;

signals:
    void sendPinToMasterTcp(QString,bool,QString);
    void sendPinToUi(QString,bool,QString); //发送条码给UI
    void sendGetCar();
    void rfidConnected(bool);
public slots:
    void rfidInit();
    void pingTimers();
    void receivemss();
    void newConnect();
    void displayError(QAbstractSocket::SocketError);
    void connectedDo();
    void disconnectedDo();
    void rfidDisconnectDo(bool);
    void receiveLogin(bool);
    void receivePlusFlag(bool);
    void receiveSystemStatus(QString);
    void sendSystemStaus(int,bool,bool);
    bool pinCodeRepeatHandle(QString);
    void pinCodeRepeatRemove(QString code);
    void revControllerStatus(bool);
    void clearCodeBuf();
    void slotDeletArray() ;

private:
    QThread m_thread;
    QTcpSocket *m_pTcpSocket;
    //QList<QString> seriallist;//条码链表 存储10个条码以内不能重复
    QString seriallist[10];
    int whichindex;
    bool isEqual;
    QTimer pingTimer;
    QString tempPin;
    QString tempG9;
    bool RFIDIsConnect;
    QString Factory;
    bool isLogin;
	bool plusFlag;
    bool isFirst;
    QSqlDatabase db2;
    QSqlQuery query2;
    QByteArray revBuf;
    QByteArray sendBuf;
    bool systemRunStatusFlag;
    bool controllerStatusFlag;
    bool RFIDIngNotIn;
};

#endif // RFIDCONTROLER_H

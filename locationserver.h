#ifndef LOCATIONSERVER_H
#define LOCATIONSERVER_H

#include <QObject>
#include <QtNetwork>
#include <QTimer>
#include <QThread>
#include "CurrentStatus.h"

class LocationServer : public QObject
{
    Q_OBJECT
public:
    explicit LocationServer(QObject *parent = 0);
    QTcpServer *tcpServer;
    QTcpSocket *m_pTcpSocket;
    bool isConnected;
    QTimer heartTimer;

signals:
    void sendTagStatus(bool,int,QPointF,QPointF,QPointF,QPointF,QPointF,bool);

public slots:
    void init();
    void connectedDo();
    void revMessage();
//    void sendMessage();
    void disconnectedDo();
    void displayError(QAbstractSocket::SocketError);
    void heartTimeout();
private:
    QThread m_thread;
    bool currentIsInside;
    uint8_t currentPowerStatus;

};

#endif // LOCATIONSERVER_H

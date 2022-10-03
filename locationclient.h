#ifndef LOCATIONCLIENT_H
#define LOCATIONCLIENT_H

#include <QObject>
#include <QtNetwork>
#include "CurrentStatus.h"

class LocationClient : public QObject
{
    Q_OBJECT
public:
    explicit LocationClient(QObject *parent = 0);
    void setIP(QString);
    void setCurrentStatus(CurrentStatus);

signals:
    void rfidConnected(bool);

public slots:
    void Init();
    void pingTimers();
    void newConnect();
    void displayError(QAbstractSocket::SocketError);
    void connectedDo();
    void disconnectedDo();
    void sendHeart();

private:
    QThread m_thread;
    QTcpSocket *m_pTcpSocket;
    QTimer pingTimer;
    QString IP;
    bool IsConnect;
    QTimer heartTimer;
    CurrentStatus currentStatus;
};

#endif // LOCATIONCLIENT_H

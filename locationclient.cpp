#include "locationclient.h"

LocationClient::LocationClient(QObject *parent) :
    QObject(parent)
{
    IsConnect = false;
}

void LocationClient::setIP(QString tmpIp)
{
    IP = tmpIp;
}

void LocationClient::setCurrentStatus(CurrentStatus tmpCurrentStatus)
{
    currentStatus = tmpCurrentStatus;
}

void LocationClient::Init()
{
    qDebug()<< "LocationClient start";
    m_pTcpSocket = new QTcpSocket;
    connect(m_pTcpSocket,SIGNAL(connected()),this,SLOT(connectedDo()));
    connect(m_pTcpSocket,SIGNAL(disconnected()),this,SLOT(disconnectedDo()));
    connect(&pingTimer,SIGNAL(timeout()),this,SLOT(pingTimers()));
    connect(&heartTimer,SIGNAL(timeout()),this,SLOT(sendHeart()));
    connect(m_pTcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    pingTimer.start(5000);
    heartTimer.start(1000);
}

void LocationClient::newConnect()
{
    m_pTcpSocket->abort();                        //取消已有的连接
    m_pTcpSocket->connectToHost(IP,5001); //连接到主机
}

void LocationClient::displayError(QAbstractSocket::SocketError)
{
    qDebug() << "LocationClient displayError" << m_pTcpSocket->errorString(); //输出错误信息
    disconnectedDo();
}

void LocationClient::connectedDo()
{
    qDebug() << "LocationClient connect success";
    pingTimer.stop();
    IsConnect = true;
}

void LocationClient::disconnectedDo()
{
    qDebug() << "LocationClient disconnectedDo";
    IsConnect = false;
    pingTimer.start(5000);
}

void LocationClient::pingTimers()
{
    if(!IsConnect)
    {
        newConnect();
    }
}

void LocationClient::sendHeart()   //发送标签的状态信息给服务器端
{
    if(IsConnect)
    {
        QByteArray dataArray = QByteArray::fromRawData((char*)&currentStatus,sizeof(CurrentStatus));
        m_pTcpSocket->write(dataArray,dataArray.size());
    }
}

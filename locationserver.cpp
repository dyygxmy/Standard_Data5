#include "locationserver.h"

LocationServer::LocationServer(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void LocationServer::init()
{
    qDebug()  << "LocationServer Thread start...";
    currentIsInside = false;
    currentPowerStatus = 0;
    isConnected = false;
    tcpServer = new QTcpServer;
    if(!tcpServer->listen(QHostAddress::Any,5001))
    {  //**本地主机的4710端口，如果出错就输出错误信息，并关闭
        qDebug() << tcpServer->errorString();
    }
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(connectedDo()));
    connect(&heartTimer,SIGNAL(timeout()),this,SLOT(heartTimeout()));
}

void LocationServer::connectedDo()
{
    if(heartTimer.isActive())
        heartTimer.stop();
    qDebug() << "LocationServer  ************ connect success";
    isConnected = true;
//    emit sendStates(id,true);
    m_pTcpSocket = tcpServer->nextPendingConnection();
    connect(m_pTcpSocket,SIGNAL(disconnected()),this,SLOT(disconnectedDo()));
    connect(m_pTcpSocket, SIGNAL(readyRead()),this, SLOT(revMessage()));
    connect(m_pTcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    heartTimer.start(4000);
}

/***************************************************/
//receive data and Ack
/***************************************************/
void LocationServer::revMessage()
{
    if(heartTimer.isActive())
        heartTimer.stop();
    QByteArray receiveMessage = m_pTcpSocket->readAll();
    if((uint)receiveMessage.size() == sizeof(CurrentStatus))
    {
        CurrentStatus currentStatus ;
        memcpy(&currentStatus,receiveMessage.data(),sizeof(CurrentStatus));
//        if((currentStatus.isInside != currentIsInside )|| (currentStatus.powerStatus != currentPowerStatus))
//        {
//            if(currentStatus.powerStatus == 0)  //add lw 2017/9/19
//            {
//                currentStatus.Label_Time_out = false;
//            }
            emit sendTagStatus(currentStatus.isInside,currentStatus.powerStatus,currentStatus.A,currentStatus.B,currentStatus.C,currentStatus.D,currentStatus.P,currentStatus.Label_Time_out);  //接收到标签的状态信息若跟之前不同发送给界面
//            qDebug()<<"run here,emit sendTagStatus"<<currentStatus.Time_out<<currentStatus.isInside<<currentIsInside<<currentStatus.powerStatus<<currentPowerStatus;
//        }
    }
    else
    {
//        qDebug()<<"run here,Dont emit sendTagStatus";
        return;
    }
    heartTimer.start(4000);
}

void LocationServer::disconnectedDo()
{
    qDebug()<<"LocationServer comming in disconnectedDo";
    if(heartTimer.isActive())
        heartTimer.stop();
    if(isConnected)
    {
        isConnected = false;
//        emit sendStates(id,false);
        m_pTcpSocket->deleteLater();
        qDebug()<<"LocationServer disconnectedDo";
    }
}

void LocationServer::displayError(QAbstractSocket::SocketError)
{
    qDebug()<<"LocationServer displayError"<<m_pTcpSocket->errorString();
    if(heartTimer.isActive())
        heartTimer.stop();
    if(isConnected)
    {
        isConnected = false;
//        emit sendStates(id,false);
        m_pTcpSocket->deleteLater();
        qDebug()<<"LocationServer disconnect";
    }
}

void LocationServer::heartTimeout()
{
    qDebug()<<"LocationServer heart time out ! ! !";
    disconnectedDo();
}

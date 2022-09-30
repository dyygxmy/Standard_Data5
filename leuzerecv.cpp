#include "leuzerecv.h"
#include <QtNetwork>
#include <QDebug>

LeuzeRecv::LeuzeRecv(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void LeuzeRecv::init()
{
    qDebug() <<"LeuzeRecv thread start!!";
    receiver = new QUdpSocket(this);
    receiver->bind(10001, QUdpSocket::ShareAddress);
    connect(receiver, SIGNAL(readyRead()), this, SLOT(processPendingDatagram()));
}

// 处理等待的数据报
void LeuzeRecv::processPendingDatagram()
{
    // 拥有等待的数据报
    while(receiver->hasPendingDatagrams())
    {
        QByteArray datagram;
        // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        datagram.resize(receiver->pendingDatagramSize());
        // 接收数据报，将其存放到datagram中
        receiver->readDatagram(datagram.data(), datagram.size());
        if(datagram != currentCode)
        {
            qDebug()<< "receive datagram" << datagram;
            currentCode = datagram;
            emit sendNewCode(currentCode);
        }
    }
}
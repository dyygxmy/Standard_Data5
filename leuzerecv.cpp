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

// ����ȴ������ݱ�
void LeuzeRecv::processPendingDatagram()
{
    // ӵ�еȴ������ݱ�
    while(receiver->hasPendingDatagrams())
    {
        QByteArray datagram;
        // ��datagram�Ĵ�СΪ�ȴ���������ݱ��Ĵ�С���������ܽ��յ�����������
        datagram.resize(receiver->pendingDatagramSize());
        // �������ݱ��������ŵ�datagram��
        receiver->readDatagram(datagram.data(), datagram.size());
        if(datagram != currentCode)
        {
            qDebug()<< "receive datagram" << datagram;
            currentCode = datagram;
            emit sendNewCode(currentCode);
        }
    }
}
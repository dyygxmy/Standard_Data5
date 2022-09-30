#include "quickrequest.h"

QuickRequest::QuickRequest(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void QuickRequest::requestStart()
{
    qDebug() << "quick request thread start !!!";
    station = Station.toAscii();
    idcode = "";
    OutTime_times=0;
    IDCode_OutTime_times=0;
    sender = new QUdpSocket(this);
    sender->bind();
    connect(sender, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    dataserver = configIniRead->value("baseinfo/DataServerIp").toString() ;
//    dataserver = "192.168.31.110";
    delete configIniRead;
//    connect (&HeartTimer,SIGNAL(timeout()),this,SLOT(sendHeart()));
//    sendHeart();
}

QByteArray QuickRequest::SumChk(QByteArray data)
{
    int checkSum = 0;
    for(int i=0;i<data.length()-2;i++)
    {
        checkSum += data[i];
    }
    QByteArray result;
    result.resize(1);
    result[0] = (uchar)(checkSum%256);
    return result;
}

void QuickRequest::sendHeart()
{
//    if(HeartTimer.isActive())
//        HeartTimer.stop();
    OutTime_times++;
    if(OutTime_times == 4)
        qDebug()<<"connect data change software time out";  //断线报错
    if(OutTime_times == 300)
        OutTime_times = 0;
    QByteArray T_buff;//待发送数组
    T_buff.resize(19);
    T_buff[0] = 0x02;
    T_buff[1] = 0x11;
    T_buff[2] = 0x00;
    T_buff[3] = 0x11;

    T_buff[4] = 0x00;   //0:在线 1：离线

    int sta_len = station.length();
    for(int i=0;i<sta_len;i++)
    {
        T_buff[5+i] = station[i];
    }
    for(int i=0;i<10-sta_len;i++)
    {
        T_buff[5+sta_len+i] = 0x00;
    }

    T_buff[15]= 0x00;
    errorcode = 0;
    errorcode |= isRFIDConnected;
    errorcode  = errorcode<<1;
    errorcode |= isController2;
    errorcode  = errorcode<<1;
    errorcode |= isController1;
    T_buff[16]= errorcode;

    T_buff[17]= SumChk(T_buff)[0];
    T_buff[18]= 0x03;

//    qDebug() << "udp send heart%%%%%%%%%%"<<T_buff.toHex();

//    sender->writeDatagram(T_buff.data(),T_buff.size(),dataserver, 9001);
//    HeartTimer.start(100);
}

void QuickRequest::receiveErrorCode(bool a, bool b, bool c)
{
//    qDebug()<<"###################receiveErrorCode&&&&&&&&&&&&&&&&&&&&&&&";
    isController1 = !a; //拧紧机1
    isController2 = !b;  //拧紧机2
    isRFIDConnected = !c;       //操作剩余时间
    if(idcode != "")
        sendRequest();
    else
        sendHeart();
}

void QuickRequest::request(QString IDCode)
{
    idcode = IDCode.toAscii();
}

void QuickRequest::sendRequest()
{
    IDCode_OutTime_times++;
    if(IDCode_OutTime_times == 5)
    {
        qDebug()<<"Request Job time out";  //断线报错
        idcode = "";
        IDCode_OutTime_times = 0;
    }
    QByteArray T_buff;//待发送数组
    T_buff.resize(44);
    T_buff[0] = 0x02;
    T_buff[1] = 0x10;
    T_buff[2] = 0x00;
    T_buff[3] = 0x2C;

    int sta_len = station.length();
    for(int i=0;i<sta_len;i++)
    {
        T_buff[4+i] = station[i];
    }
    for(int i=0;i<10-sta_len;i++)
    {
        T_buff[4+sta_len+i] = 0x00;
    }

    int code_len = idcode.length();
    for(int i=0;i<code_len;i++)
    {
        T_buff[14+i] = idcode[i];
    }
    for(int i=0;i<28-code_len;i++)
    {
        T_buff[14+code_len+i] = 0x00;
    }

    T_buff[42]= SumChk(T_buff)[0];
    T_buff[43]= 0x03;

    qDebug()<< "send request "<<T_buff.toHex();
    sender->writeDatagram(T_buff.data(),T_buff.size(), dataserver, 9001);
}

void QuickRequest::readPendingDatagrams()
{
    QByteArray datagram;
    do {
        datagram.resize(sender->pendingDatagramSize());
        sender->readDatagram(datagram.data(), datagram.size());
    } while (sender->hasPendingDatagrams());

//    QString rev_buff = datagram.toHex()     ;// 转成16进制方便后面各种判断
//    qDebug()<<"rev_buff"<<rev_buff;

    if(datagram[0]== (char)0x02 && datagram[datagram.length()-1] == (char)0x03)
    {
        OutTime_times=0;
//        if(HeartTimer.isActive())
//            HeartTimer.stop();
//        HeartTimer.start(100);

        if(datagram[1] == (char)0x10)
        {
            if( datagram.length()-6 == ( datagram[3]+(datagram[2]<<8) ) )
            {
                if(datagram[4] == (char)0)
                {
                    if(idcode !="")
                    {
                        QString Job = QString(datagram.mid(5,8));
                        //                    while(Job.endsWith("0"))
                        //                        Job = Job.left(Job.length()-1);
                        emit sendJob(Job);
                    }
                }
                else
                {
                    int jobError = (char)datagram[4];
                    emit JobError(jobError);
                }
                idcode = "";
                IDCode_OutTime_times = 0;
            }
            else
                qDebug()<<"datagram length wrong";
        }
        else if(datagram[1]== (char)0x11)
        {
            if( datagram.length()-6 == ( datagram[3]+(datagram[2]<<8) ) )
            {

            }
        }
        else
            qDebug()<<"The command number is wrong";
    }
    else
        qDebug()<<"datagram[0]!= 0x02 || datagram[datagram.length()-1] != 0x03";
}


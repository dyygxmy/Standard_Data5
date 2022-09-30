#include "mastertcp.h"

MasterTCP::MasterTCP(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&tcp_thread);
    tcp_thread.start();
}

void MasterTCP::masterServerStart()
{
    systemStatueFlag = false;
    controlStatueFlag = false;
    socketConnectFlag = false;
    connectFlag = false;
    sendPinFlag = false;
    timerCount = 0;
    revBuf = "";
    pin_Code = "";
    G9_Code = "";
    DTdebug()  << "MasterTCP Thread start...";
    tcpServer1 = new QTcpServer(this);
    if(!tcpServer1->listen(QHostAddress::Any,4545))
    {  //**本地主机的4710端口，如果出错就输出错误信息，并关闭
        DTdebug() << tcpServer1->errorString();
    }
    connect(tcpServer1,SIGNAL(newConnection()),this,SLOT(DataReceived()));
}
/***************************************************/
//link success
/***************************************************/
void MasterTCP::DataReceived()
{
    DTdebug() << "****** Listening ****** connect success";
    tcpSocket1 = tcpServer1->nextPendingConnection();
    connect(tcpSocket1, SIGNAL(readyRead()),this, SLOT(recMessage()));
    QTimer::singleShot(1000,this,SLOT(heartTimerFunc()));
    socketConnectFlag = true;
    connectFlag = true;
}
/***************************************************/
//5000ms timer function
/***************************************************/
void MasterTCP::heartTimerFunc()
{
    timerCount++;

    if(connectFlag)
    {
        timerCount1++ ;
        if(sendPinFlag) //send pin and G9
        {
            sendCmdToSalve(pin_Code,G9_Code,1);
        }
        else if(timerCount1>5) //alive
        {
            timerCount1 = 0;
            sendCmdToSalve("","",0);
        }
    }

    if(timerCount>16)
    {
        DTdebug() << "******heart time out******";
        timerCount=0;
        closeTcpServer();
        return ;
    }

    QTimer::singleShot(1000,this,SLOT(heartTimerFunc()));
}
/***************************************************/
//close link
/***************************************************/
void MasterTCP::closeTcpServer()
{
    DTdebug()<<"****closeTcpServer******";
//    if(tcpServer->hasPendingConnections())
    if(socketConnectFlag)
    {
        DTdebug()<<"**** tcpSocket1 delete******";
        socketConnectFlag = false;
        tcpSocket1->abort();
        delete tcpSocket1;
    }
    connectFlag = false;
}

/***************************************************/
//receive data and Ack
//ACK : 0x02   0x01   0x04       0x00 0x00                 0x 0x03
//          receive ACK cmd      error code
//ACK : 0x02   0x81   0x04       0x31 0x31                 0x 0x03
//                   system status and controller status
/***************************************************/
void MasterTCP::recMessage()
{
    timerCount = 0;
    revBuf  = tcpSocket1->readAll();
    DTdebug()<<"*****master receive****"<<revBuf.toHex();
    if(revBuf.size()>=7)
    {
        if( ((unsigned char)revBuf[0] == 0x02)&&((unsigned char)revBuf[6] == 0x03) )
        {
            if((unsigned char)revBuf[1] == 0x81)//alive
            {
                if(revBuf.mid(3,1) == "1")
                    systemStatueFlag = true;
                else
                    systemStatueFlag = false;
                if(revBuf.mid(4,1) == "1")
                    controlStatueFlag = true;
                else
                    controlStatueFlag = false;
            }
            else//send pin code
            {
                sendPinFlag = false;
            }
        }
    }
    revBuf.clear();
}
/***************************************************/
//receive ping and G9 code
/***************************************************/
void MasterTCP::revRFID_PIN(QString pinCode, bool validFlag, QString G9Code)
{
    Q_UNUSED(validFlag);
    if( (!systemStatueFlag)&&(controlStatueFlag) )//salve system not ING and controller ready
    {
        sendPinFlag = true;
        pin_Code = pinCode;
        G9_Code = G9Code;
    }
    else
    {
        emit sendCmdClearPinCode();
    }
}
/***************************************************/
//send cmd to salve
//cmd : 0x02   0x01   0x1c  0x31 xxxxxxxxxxxxxxxxxxxxxxxxx 0x 0x03
//      start  cmd    len  pin code enable    data 25bit      end
/***************************************************/
void MasterTCP::sendCmdToSalve(QString pinCode,QString G9Code,int cmd)
{
    QByteArray sendBuf = "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR";
    unsigned char sum = 0;
    sendBuf[0] = 0x02;
    sendBuf[1] = 0x01;
    sendBuf[2] = 0x1c;           //len
    sendBuf[3] = 0x30 + cmd;
    if(cmd == 1) //pin and G9
    {
        sendBuf.replace(4,14,pinCode.toAscii().data());
        sendBuf.replace(18,2,G9Code.toAscii().data());
    }
    for(int i=2;i<29;i++)
    {
        sum += sendBuf[i];
    }
    sendBuf[29] = sum;
    sendBuf[30] = 0x03;
    tcpSocket1->write(sendBuf,31);
    DTdebug()<<"*****master send*****"<<sendBuf.mid(3,26);
}

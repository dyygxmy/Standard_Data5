#include "connectplc.h"
#include "GlobalVarible.h"

ConnectPLC::ConnectPLC(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&plc_thread);
    plc_thread.start();
}

void ConnectPLC::PLCStart()
{
    qDebug() << "connect plc thread start";
    setUpCommFlag = 2;
    sendCmd = 0;
    aliveCount = 1;
    plcLinkFlag = false;
    startCmdBuf1 = "\x03**\x16\x11\xE0***\x01*\xC1\x02\x10*\xC2\x02\x03\x01\xC0\x01\x0A";     //set up communication
    startCmdBuf2 = "\x03**\x19\x02\xF0\x80\x32\x01**\xCC\xC1*\x08**\xF0**\x01*\x01\x03\xC0";  //set up communication
    readCmdBuf   = "";
    writeCmdBuf  = "";
    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFuncXS()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revPLCData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    plcConnects();
    timer5000ms->start(1000);
    clearPLCFlag = false;
    VinBuf = "";
    sendVinFlag = false;
    vinAddr = 0;
    vinLen = 0;
    sendCount = 0;
}


/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void ConnectPLC::disConnectDO()
{
    plcLinkFlag = false;
    plcConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void ConnectPLC::plcConnects()
{
    setUpCommFlag = 2;
    tcpSocket->abort(); //取消已有的连接
    tcpSocket->connectToHost("192.168.1.10",102); //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    qDebug() << "plc connect waiting>>>>>>>>"<< tcpSocket->state();
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "plc connect success!!!!!!!!!!!!!";
        ctlPLCHandle(startCmdBuf1,true);
    }
    else
    {
        qDebug() << "plc connect fail?????????????????" ;
    }
}
/*******************************************************/
//1000ms timer function
/*******************************************************/
void ConnectPLC::timerFuncXS()
{
//    qDebug() << "plc 1000ms connects <<<<<";
    if(plcLinkFlag)
    {
        if(sendVinFlag)
        {
//            sendVinFlag = false;
            sendCount = 0;
            writePLCFunc(vinAddr,vinLen,VinBuf);//upload VIN
        }
        else
        {
            if(clearPLCFlag)
            {
                clearPLCFlag = false;
                writePLCFunc(50,7,"0000000");//clear NOK bolt status
            }
            else
            {
                readPLCFunc(50,7);//read NOK one and all bolt
            }
        }
    }
    if(aliveCount > 2)
    {
        if(PLCIsConnect)
        {
            emit PLCStatus(2);
            PLCIsConnect = false;   //PLC disconnect
        }
        aliveCount = 0;
        plcLinkFlag = false;
        plcConnects();
    }
    aliveCount ++ ;
}
/*******************************************************/
//read plc cmd
/*******************************************************/
void ConnectPLC::readPLCFunc(int addr,int len)
{
    QByteArray tempBuf ="\x03**\x1F\x02\xF0\x80\x32\x01***C*\x0E**\x04\x01\x12\x0A\x10\x02*\x0A*\xC6\x84**\x50";
    if(plcLinkFlag)
    {
        tempBuf[12] = sendCmd;//SN
        tempBuf[24] = len;    //read data length
        tempBuf[25] = 0;      //DB station high byte
        tempBuf[26] = 198;    //DB station low byte
        tempBuf[27] = 132;     //wr area  DB(132) M(133) I(129) Q(130)
        tempBuf[29] = (addr<<3)>>8;//read high address
        tempBuf[30] = addr<<3;//read low address
        readCmdBuf = tempBuf;
        ctlPLCHandle(readCmdBuf,true);
        sendCmd ++;
    }
}

/*******************************************************/
//write plc cmd
/*******************************************************/
void ConnectPLC::writePLCFunc(int addr,int len,QString buf)
{
    QByteArray tempBuf ="\x03**\x24\x02\xF0\x80\x32\x01***C*\x0E*\x08\x05\x01\x12\x0A\x10\x06*\x01*\xC6\x84**\x50*\x04*\x20";//writeCmdBuf;
    for(int i=0;i<tempBuf.size();i++)
    {
        if(tempBuf[i]=='*')
        {
            tempBuf[i] = 0;
        }
    }
    if(plcLinkFlag)
    {
        tempBuf[12] = sendCmd; //SN
        tempBuf[16] = len + 4; //write data length
        tempBuf[22] = 2;       //data type
        tempBuf[23] = 0;       //data len high byte
        tempBuf[24] = len;     //data len low byte
        tempBuf[25] = 0;       //DB station high byte
        tempBuf[26] = 198;     //DB station low byte
        tempBuf[27] = 132;     //wr area  DB(132) M(133) I(129) Q(130)
        tempBuf[29] = (addr<<3)>>8;//write high address(addr*8)
        tempBuf[30] = addr<<3; //write low address(addr*8)
        tempBuf[33] = (len<<3)>>8;//data len high byte(len*8)
        tempBuf[34] = len<<3;  //data len low byte(len*8)
        tempBuf.append(buf.toAscii());//data
        tempBuf[3]  = tempBuf.size(); //cmd length
        writeCmdBuf = tempBuf;
        qDebug() << "**** write ******" <<tempBuf.mid(3,1).toHex()<<tempBuf.toHex();
        ctlPLCHandle(writeCmdBuf,false);
        sendCmd ++;
    }
}
/*******************************************/
//upload VIN to PLC
/*******************************************/
void ConnectPLC::revVinFunc(int addr, int len, QString vin)
{
    VinBuf = vin;
    sendVinFlag = true;
    vinAddr = addr;
    vinLen = len;
}

/***********************************/
//send data to PLC  type = true(* invalue)
/*************************************/
void ConnectPLC::ctlPLCHandle(QByteArray sendBuf,bool type)
{
    int Tx_len=0,i;
    Tx_len = sendBuf.size();
    if(type)
    {
        for(i=0;i<Tx_len;i++)
        {
            if(sendBuf[i]=='*')
            {
                sendBuf[i] = 0;
            }
        }
    }
    tcpSocket->write(sendBuf,Tx_len);
    if(tcpSocket->waitForBytesWritten() == false)
    {}
//    qDebug() << "plc send    len:" << Tx_len << "data:" << sendBuf.toHex();
}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void ConnectPLC::revPLCData()
{
    int revLen;
    QByteArray recBuf;
    recBuf = tcpSocket->readAll();
    revLen = recBuf.size();
//    qDebug() << "plc receive len:" << revLen << "data:" << recBuf.toHex();
    aliveCount = 0;
    if(setUpCommFlag == 2)
    {
        setUpCommFlag = 1;
        ctlPLCHandle(startCmdBuf2,true);
    }
    else if(setUpCommFlag == 1)
    {
        setUpCommFlag = 0;
        plcLinkFlag = true;
        if(!PLCIsConnect)
        {
            emit PLCStatus(1);
            PLCIsConnect = true;   //PLC read valid
        }
    }
    else
    {
        if(revLen == 22)//write ack
        {
            if(sendVinFlag) //send VIN OK
            {
                sendVinFlag = false;
            }
        }
        else if(revLen > 22)//read data
        {
//            qDebug() << "********plc read ok**********";
            emit PLCStatus(1);
//            qDebug() << "result:"<<recBuf.mid(21,1).toHex()<<"lenght:"<<recBuf.mid(24,1).toHex();
            if((recBuf.mid(21,1).toHex()=="ff")&&(recBuf.mid(24,1).toHex()=="38"))
            {
                if(recBuf.mid(31,1) == "1") //NOK all bolt
                {
                    emit nokBolt(10);
                    clearPLCFlag = true;
                }
                else if(recBuf.mid(25,1) == "1")//NOK one group bolt
                {
                    emit nokBolt(0);
                    clearPLCFlag = true;
                }
                else if(recBuf.mid(26,1) == "1")//NOK two group bolt
                {
                    emit nokBolt(1);
                    clearPLCFlag = true;
                }
                else if(recBuf.mid(27,1) == "1")//NOK three group bolt
                {
                    emit nokBolt(2);
                    clearPLCFlag = true;
                }
                else if(recBuf.mid(28,1) == "1")//NOK four group bolt
                {
                    emit nokBolt(3);
                    clearPLCFlag = true;
                }
                else if(recBuf.mid(29,1) == "1")//NOK five group bolt
                {
                    emit nokBolt(4);
                    clearPLCFlag = true;
                }
            }
        }
        else
        {
            qDebug() << "********plc read error**********";
            emit PLCStatus(3);
            if(sendVinFlag) //send VIN fail
            {
                sendCount ++;
            }
            if(sendCount>3)
            {
                sendCount = 0 ;
                sendVinFlag = false;
            }
        }
    }
}


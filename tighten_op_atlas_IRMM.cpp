/**********************************************************************
 * file name  tighten_op_atlas_IRMM.c
  describe   ：control enable / program / vin
 *             Last tightening result data subscribe
 *             Last tightening result curve subscribe
 *             Old tightening result upload request
 *
***********************************************************************/
#include "tighten_op_atlas_IRMM.h"


TightenOpAtalsIRMM::TightenOpAtalsIRMM(QObject *parent) :
    QObject(parent)
{
    powerOnFlag = true;                //data2 restart
    errCount = 0;
    errType = 0;
    programNO = 0;
    boltCount = 0;
    boltNokCount = 0;
    intReadTighteningID = 0;     //current read Tightening form nexo
    nexoHandleFlag = 0;
    readMaxTightenIDFlag = false;
    tighteningID = "";
    StartBolt = 0;
    curveNullBuf = "Curve is null";
    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpAtalsIRMM::tightenStart()
{
    qDebug() <<"ch:"<< ch << "tighten_op_atlas_IRMM thread start !!!"<<controllerIP;
//    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
//    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc1000ms()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revIRData()));
//    timer5000ms->start(1000);
    QTimer::singleShot(1000,this,SLOT(timerFunc1000ms()));
    newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpAtalsIRMM::newConnects()
{
    dataSubFlag = false;
    tcpSocket->abort(); //取消已有的连接
    tcpSocket->connectToHost(controllerIP,1069);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    qDebug() << ch << "ch111111 the socket state is" << controllerIP ;
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << ch <<"ch111111 IR link success  >>>>>>>>";
        controlAliveCount = 0;
        sendCMD_IR(0x00000001,"SES01Q01");//start communication
    }
    else
    {
        emit IsTigntenReady(ch,false);
        controlLinkOk = false;
        qDebug() << ch << "ch111111 IR link fail XXXXXXXXXXXX" ;
    }
}
/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void TightenOpAtalsIRMM::disConnectDO()
{
    controlLinkOk = false;
    qDebug() << ch << "ch111111 OP disconnect 22222222222";
    newConnects();
}
/**************************************/
//send CMD IR controller
/**************************************/
void TightenOpAtalsIRMM::sendCMD_IR(int IRHandleFlag,QByteArray mid)
{
    QByteArray sendBuf;
    int Tx_len;
    /****************************************/
    /**********communication stop***********/
    /****************************************/
    if(IRHandleFlag&0x00000010)
    {
        Tx_len = 21;
        sendBuf = "00200003001         ";
    }
    /****************************************/
    /**********communication start***********/
    /****************************************/
    else if(IRHandleFlag&0x00000001)
    {
        sendBuf = "SES01Qss00000027ICS,ICS USER,INSIGHTREAD,1";
        Tx_len = sendBuf.size();
    }
    /****************************************/
    /**********data  subscribe***********/
    /****************************************/
    else if(IRHandleFlag&0x00000100)
    {
        sendBuf = "EOR01Qss000000011";
        Tx_len = sendBuf.size();
    }
    /****************************************/
    /**********       ACK       ***********/
    /****************************************/
    else if(IRHandleFlag&0x00800000)
    {
        IRHandleFlag &= ~0x00800000;
        sendBuf = "SES01Ass00000000";
        Tx_len  = sendBuf.size();
    }
    /****************************************/
    /**********       Alive       ***********/
    /****************************************/
    else if(IRHandleFlag&0x00400000)
    {
        IRHandleFlag &= ~0x00400000;
        sendBuf = "SES02Ass00000000";
        Tx_len  = sendBuf.size();
    }
    else
    {
        return;
    }
    sendBuf.replace(0,8,mid);
    qDebug() << "chXXXXXXX send    :"<< ch <<Tx_len<< sendBuf;
    tcpSocket->write(sendBuf,Tx_len);
    tcpSocket->waitForBytesWritten(300);
}


/***********************************/
//处理接受IR数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/

void TightenOpAtalsIRMM::revIRData()
{
    QByteArray torqueMax;
    QByteArray torqueMin;
    int revLen,dataLen;
    QByteArray recBuf;//,hexBufTemp;
    recBuf.clear();
    recBuf = tcpSocket->readAll();
    revLen = recBuf.size();
    qDebug() << "chXXXXXXX recevice:" <<ch<< revLen  << recBuf;
    if(revLen>=16)
    {
        IR_AliveCount = 0;
        MID = recBuf.mid(6,2);
        if(recBuf.mid(0,6) == "SES01R")      //session start success
        {
            qDebug()<<"##############################"<<ch;
            emit IsTigntenReady(ch,true);
            controlLinkOk = true;
            sendCMD_IR(0x00800000,"SES01A"+MID);//ACK
            dataSubFlag = true;
        }
        else if(recBuf.mid(0,6) == "EOR01A")  //subscribe data success
        {
            sendCMD_IR(0x00800000,"SES01A"+MID);//ACK
            dataSubFlag = false;
        }
        else if(recBuf.mid(0,6) == "EOR03U")  //data ok
        {
            dataLen = recBuf.mid(13,3).toInt();
            if(revLen == (dataLen + 16))
            {
                boltNumberBuf = carInfor[0].boltSN[StartBolt+ch].toLatin1();
                tighteningID = recBuf.mid(16,6).trimmed();                 //Tightening ID
                nexoProNO    = recBuf.mid(32,3).trimmed();                 //tighten program
                tighenTime   = recBuf.mid(37,16);                          //tighten time
                TightenResult   = recBuf.mid(54,1).trimmed();                 //tighten result
                torqueValue  = recBuf.mid(56,6).trimmed();                 //torque value
                angleValue   = recBuf.mid(68,6).trimmed();                 //angle value
                torqueMin    = recBuf.mid(99,6).trimmed();
                torqueMax    = recBuf.mid(92,6).trimmed();
                intReadTighteningID = tighteningID.toInt();
                if(intReadTighteningID == 0)
                {
                    qDebug() << "ch111111 data error ***********intReadTighteningID = 0****************";
                    return ;
                }
                if(TightenResult == "F")
                {
                    TightenResult = "NOK";   //0=tightening NOK ,1=OK
                }
                else
                {
                    TightenResult = "OK" ;
                }
                if(TightenResult == "NOK")
                {
                    if(boltNokCount > 0)
                        boltNokCount --;
                }
                else
                {
                    if(boltCount > 0)
                        boltCount --;
                }
                qDebug()  <<"ch:"<< ch << tighteningID << nexoProNO << torqueValue << angleValue << TightenResult << tighenTime;

                if(SYSS == "ING")
                {
                    DATA_STRUCT demo;
                    demo.data_model[0] = QString(tighenTime.mid(0,10));
                    demo.data_model[1] = QString(tighenTime.mid(11,8));
                    demo.data_model[2] = QString(TightenResult);
                    demo.data_model[3] = QString(torqueValue);
                    demo.data_model[4] = QString(angleValue);
                    demo.data_model[5] = QString(boltNumberBuf);
                    demo.data_model[6] = QString(vinBuf);
                    demo.data_model[7] = QString(tighteningID);
                    demo.data_model[8] = curveNullBuf;
                    demo.data_model[9] = QString(nexoProNO);
                    demo.data_model[10] = QString::number(ch+1);;
                    if(manualMode)
                        demo.data_model[11] = "manual";
                    else
                        demo.data_model[11] = Type;
                    if(TightenResult == "OK")
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]++);
                    }
                    else
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]);
                    }
                    demo.data_model[13] = QString(torqueMax);
                    demo.data_model[14] = QString(torqueMin);
                    demo.data_model[15] = "1";

                    QVariant DataVar;
                    DataVar.setValue(demo);
                    emit sendfromworkthread(DataVar);
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(TightenResult),ch);
                }
                sendCMD_IR(0x00800000,"EOR03A"+MID);//ACK
            }
        }
        else if(recBuf.mid(0,6) == "SES02U")  //Alive
        {
            sendCMD_IR(0x00400000,"SES02A"+MID);//ACK
        }
    }
}

/**************************************/
//IR alive
/**************************************/
void TightenOpAtalsIRMM::timerFunc1000ms()
{
    //    qDebug() << "ch111111 IR 1000ms tiemr" << controlLinkOk << nexoHandleFlag << controlAliveCount << nexoHandleFlag<< DebugMode;
    if(controlLinkOk)                   //nexo link
    {
        if(dataSubFlag)
        {
            sendCMD_IR(0x00000100,"EOR01Q02");  //data subscrtorqueValueibe
        }
        IR_AliveCount++;
        if(IR_AliveCount>60) //45s alive
        {
            IR_AliveCount = 0;
            controlLinkOk = false;
            newConnects();
        }
    }
    else //
    {
        IR_AliveCount++;
        if(IR_AliveCount>20)//15s relink
        {
            IR_AliveCount = 0;
            newConnects();
        }
    }
    QTimer::singleShot(1000,this,SLOT(timerFunc1000ms()));

}
//使能
void TightenOpAtalsIRMM::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {}
    else
    {
        programNO = carInfor[0].proNo[n].toShort();// proNum.toShort();
        //        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        //        boltNokCount = 3;
        StartBolt = n;
        //        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
        vinBuf = VIN_PIN_SQL.toLatin1();
        qDebug()<<"ch:::::"<<ch<<StartBolt;
    }
}


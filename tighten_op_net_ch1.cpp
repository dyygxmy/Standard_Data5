/**********************************************************************
 * file name  tighten_op_net.c
  describe   ：control enable / program / vin
 *             Last tightening result data subscribe
 *             Old tightening result upload request
 *
***********************************************************************/
#include "tighten_op_net_ch1.h"
TightenOpNetCh1::TightenOpNetCh1(QObject *parent) :
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
    readOldDataFlag = false;     //read old data
    readMaxTightenIDFlag = false;
    controlLinkOk = false;
    controlAliveCount = 0;
    revDataLen =0;
    revFinishLen = 0;
    curveFirstFlag = 0;
    curveDataFlag = 0;
    curveBuf.clear();
    sendNextEnableFlag = false;
    sendDisableFlag = false;
    strJobID = "01";
    groupNum = 0;
    Channel = "1";
    dataSubFlag = false;
    MID = "";
    IR_AliveCount = 0;
    curve_AngleTorque = 0;
    vinBuf = "";
    curveNullBuf = "Curve is null";


    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpNetCh1::tightenStartCh1()
{
    if(ControlType_1 == "IR")
    {
        controlType = "IR";
    }
    else
    {
        controlType = "PF";
    }
    controlType2 = ControlType_2;
    controllerIp = ControllerIp_1;
    qDebug() << "tighten_op_net_ch111111 thread start!!!"<<controlType;
    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    if(controlType == "PF")
    {
        connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
        connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
        connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
        timer5000ms->start(5000);
    }
    else
    {
        connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc1000ms()));
        connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revIRData()));
        timer5000ms->start(1000);
        system("echo 0 > /sys/class/leds/OUTC1/brightness");       //IR enable off
        system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
        system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
        system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
    }
    newConnects();
}
//使能
void TightenOpNetCh1::sendReadOperateCh1(bool enable,int n)
{
    if(controlType == "PF")
    {
        if(!enable) //
        {
            if(ISRESET)
            {
                ISRESET = false;
                timer5000ms->stop();
                nexoHandleFlag = 0x00400000;                  //abort job
                ctlNexoHandle();
                timer5000ms->start(5000);
                qDebug() << "********** abort job***********";
            }
            vinBuf = "";
            qDebug()<<"************ reset #############";
        }
        else
        {
            groupNum = n;
            Channel = "1";
            programNO = carInfor[0].proNo[n].toInt();// proNum.toShort();
            boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
            boltNokCount = 3;
            boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
            if(VIN_PIN_SQL.toLatin1() == vinBuf)
            {}
            else
            {
                strJobID = QByteArray::number(100+programNO).mid(1,2);
                vinBuf = VIN_PIN_SQL.toLatin1();
                timer5000ms->stop();
                nexoHandleFlag = 0x00100000;           //select JOB
                ctlNexoHandle();
                timer5000ms->start(5000);
            }
        }
    }
    else
    {
        if(!enable) //
        {
            system("echo 0 > /sys/class/leds/OUTC1/brightness");       //IR enable off
            system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
            system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
            system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
        }
        else
        {
            groupNum = n;
            Channel = "1";
            boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
            programNO = carInfor[0].proNo[n].toInt();// proNum.toShort();
            qDebug() << "*****ch1 send enable******" << programNO;
            if(programNO == 1)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 2)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 1 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 3)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable
                system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 1 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 4)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 1 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 1 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 5)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 1 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 6)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 1 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 1 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 7)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 1 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 1 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else if(programNO == 8)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness");       //IR enable on
                system("echo 1 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 1 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 1 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            else
            {
                system("echo 0 > /sys/class/leds/OUTC1/brightness");       //IR enable off
                system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
                system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
                system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
            }
            vinBuf = VIN_PIN_SQL.toLatin1();
        }
    }
}


/**************************************/
//nexo alive
/**************************************/
void TightenOpNetCh1::timerFunc5000ms()
{
//    qDebug() << "ch111111 op 5000ms tiemr" << controlLinkOk << nexoHandleFlag << controlAliveCount << nexoHandleFlag;
    if(controlLinkOk)                   //nexo link
    {
        nexoHandleFlag |= 0x00800000;//Alive
        ctlNexoHandle();             //send CMD to nexo
    }
    else
    {}
    if(controlAliveCount > 2)             //3 times alive not ack
    {
        controlAliveCount = 0;
        if(controlLinkOk )
        {
            emit IsTigntenReady(1,false);
            controlLinkOk = false;
        }
        else
        {
            qDebug() << "ch111111 3 times alive no ack disconnect  ";
            newConnects();
        }
    }
    controlAliveCount++;
}

/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void TightenOpNetCh1::disConnectDO()
{
    controlLinkOk = false;
    qDebug() << "ch111111 OP disconnect 22222222222";
    newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpNetCh1::newConnects()
{
    if(controlType == "PF")
    {
        tcpSocket->abort(); //取消已有的连接
        //连接到主机，这里从界面获取主机地址和端口号
        tcpSocket->connectToHost(controllerIp,4545);
        tcpSocket->waitForConnected(2000); //waitting 2000ms
        qDebug() << "OP ch1 the socket state is" << controllerIp ;
        if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "ch111111 OP link success  >>>>>>>>";
            controlAliveCount = 0;
            timer5000ms->stop();
            nexoHandleFlag  = 0x00000001;  //start communication
            ctlNexoHandle();               //send CMD to nexo
            timer5000ms->start(5000);
        }
        else
        {
            emit IsTigntenReady(1,false);
            controlLinkOk = false;
            qDebug() << "ch111111 OP link fail XXXXXXXXXXXX" ;
        }
    }
    else //IR
    {
        dataSubFlag = false;
        tcpSocket->abort(); //取消已有的连接
        tcpSocket->connectToHost(controllerIp,1069);
        tcpSocket->waitForConnected(2000); //waitting 2000ms
        qDebug() << "OP ch1 the socket state is" << controllerIp ;
        if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "ch111111 IR link success  >>>>>>>>";
            controlAliveCount = 0;
            sendCMD_IR(0x00000001,"SES01Q01");//start communication
        }
        else
        {
            emit IsTigntenReady(1,false);
            controlLinkOk = false;
            qDebug() << "ch111111 IR link fail XXXXXXXXXXXX" ;
        }
    }
}

/***********************************/
//处理控制Nexo函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void TightenOpNetCh1::ctlNexoHandle()
{
    short Tx_len = 0;
    QByteArray sendBuf;
    /****************************************/
    /*******system error system ACK**********/
    /****************************************/
    if(nexoHandleFlag&0x10000000)
    {
        nexoHandleFlag &= ~0x10000000;
        Tx_len = 21;
        sendBuf = "00200077001         ";
    }
    /****************************************/
    /*******results data ACK**********/
    /****************************************/
    else if(nexoHandleFlag&0x02000000)
    {
        nexoHandleFlag &= ~0x02000000;
        Tx_len = 21;
        sendBuf = "00200062001         ";
    }
    /****************************************/
    /*******Alarm  ACK**********/
    /****************************************/
    else if(nexoHandleFlag&0x00001000)
    {
        nexoHandleFlag &= ~0x00001000;
        Tx_len = 21;
        sendBuf = "00200072001         ";
    }
    /****************************************/
    /*******results curve ACK**********/
    /****************************************/
    else if(nexoHandleFlag&0x00002000)
    {
        nexoHandleFlag &= ~0x00002000;
        Tx_len = 25;
        sendBuf = "00240005001         0900";
    }
    /****************************************/
    /**********communication stop***********/
    /****************************************/
    else if(nexoHandleFlag&0x00000010)
    {
        Tx_len = 21;
        sendBuf = "00200003001         ";
    }
    /****************************************/
    /**********communication start***********/
    /****************************************/
    else if(nexoHandleFlag&0x00000001)
    {
        Tx_len = 21;
        sendBuf = "00200001001         ";
    }
    /****************************************/
    /*********system error messages**********/
    /****************************************/
    else if(nexoHandleFlag&0x20000000)
    {
        Tx_len = 21;
        sendBuf = "00200070001         ";
    }
    /****************************************/
    /*Last tightening results data subscribe*/
    /****************************************/
    else if(nexoHandleFlag&0x01000000)
    {
        Tx_len = 21;
        sendBuf = "002000600011        ";
    }
    /****************************************/
    /*New curve Angle subscribe*/
    /****************************************/
    else if(nexoHandleFlag&0x00008000)
    {
        Tx_len = 65;
        sendBuf = "006400080011        0900001350                             01001";
        //         1234567890123456789012345678901234567890123456789012345678901234
    }
    /****************************************/
    /*New curve Torque subscribe*/
    /****************************************/
    else if(nexoHandleFlag&0x00004000)
    {
        Tx_len = 65;
        sendBuf = "006400080011        0900001350                             01002";
    }
    /****************************************/
    /*********      time set       **********/
    /****************************************/
    else if(nexoHandleFlag&0x40000000)
    {
        Tx_len = 40;
        sendBuf = "00390082001         2015-05-01:12:01:01";
        QDateTime time = QDateTime::currentDateTime();
        QString strTime = time.toString("yyyy-MM-dd:hh:mm:ss");
        sendBuf.replace(20,19,strTime.toLatin1());
    }
    /****************************************/
    /*********history results upload request**********/
    /****************************************/
    else if(nexoHandleFlag&0x04000000)
    {
        Tx_len = 31;
        sendBuf = "00300064001         0000000000";
        if(!readMaxTightenIDFlag)
        {
            QByteArray tempBuf = QByteArray::number(intReadTighteningID,10);
            sendBuf.replace(30-tempBuf.size(),tempBuf.size(),tempBuf);
        }
    }
    /****************************************/
    /*********selected program**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000002)
    {
        Tx_len = 24;
        sendBuf = "00230018001         001";
        sendBuf[21] = programNO/10 + 0x30;
        sendBuf[22] = programNO%10 + 0x30;
    }
    /****************************************/
    /*********set bolt number**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000004)
    {
        Tx_len = 26;
        sendBuf = "00250019001         00105";
        sendBuf[21] = programNO/10 + 0x30;
        sendBuf[22] = programNO%10 + 0x30;
        sendBuf[23] = boltCount/10 + 0x30;
        sendBuf[24] = boltCount%10 + 0x30;
    }
    /****************************************/
    /*********      ID code        **********/
    /****************************************/
    else if(nexoHandleFlag&0x00010000)
    {
        Tx_len = 38;
        sendBuf = "00370050            LSVAE45E7EN123456";
        sendBuf.replace(20,17,vinBuf); //replace vincode
    }
    /****************************************/
    /******** tool disable**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000200)
    {
        Tx_len = 21;
        sendBuf = "00200042001         ";
    }
    /****************************************/
    /******** tool enable**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000400)
    {
        Tx_len = 21;
        sendBuf = "00200043001         ";
    }
    /****************************************/
    /********** abort job//  Job info subscribe***********/
    /****************************************/
    else if(nexoHandleFlag&0x00400000)
    {
        Tx_len = 21;
//        sendBuf = "00200034001         ";
        sendBuf = "00200127001         ";
    }

    /****************************************/
    /**********   Job select ***********/
    /****************************************/
    else if(nexoHandleFlag&0x00100000)
    {
        Tx_len = 23;
        sendBuf = "00220038001         "; //Job ID
        sendBuf.append(strJobID);
    }
    /****************************************/
    /**********       Alive       ***********/
    /****************************************/
    else if(nexoHandleFlag&0x00800000)//5S send Alive
    {
        nexoHandleFlag &= ~0x00800000;
        Tx_len = 21;
        sendBuf = "00209999001         ";
    }

    if(sendBuf.mid(4,4) != "9999")
    {
        qDebug() << "ch111111 send      :" << sendBuf;// << "nexoHandleFlag :" << nexoHandleFlag ;
    }
    tcpSocket->write(sendBuf,Tx_len);
    tcpSocket->waitForBytesWritten(300);

}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void TightenOpNetCh1::revNexoData()
{
    int nexoMID;
    int cmdType;
    int errCode;
    int nullCount=0,i=0,j=0,k=0;
    int hByte,lByte;
    int revLen,cmdLen;
    int torqueMax = 0;
    int torqueMin = 0;
    QByteArray recBuf,hexBuf;//,hexBufTemp;

    recBuf.clear();
    recBuf = tcpSocket->readAll();
    nexoMID = recBuf.mid(4,4).toInt();
    revLen = recBuf.size();
    cmdLen = recBuf.mid(0,4).toInt();
    if(nexoMID == 900)
    {
        revDataLen   = revLen;
        revFinishLen = cmdLen;
        curveBuf = recBuf;
        curveFirstFlag = 1;
        curveDataFlag = 1;   //receive curve
        idCode = "";
    }
    else
    {}
    if(revLen == (cmdLen+1))              //length of the same
    {
        if(nexoMID != 9999)
        {
            qDebug() << "ch111111 recevice len:" << revLen << cmdLen << "note:" << recBuf;
        }
        controlAliveCount = 0;           //recvice nexo data
        switch(nexoMID)
        {
        case 4:   //cmd send error         //send CMD to n
            cmdType = recBuf.mid(20,4).toInt();
            errCode = recBuf.mid(24,2).toInt();
            errCount ++;
            if(errCount > 2)   //over 3 timer no send cmdType
            {
                errCount = 0;
                qDebug() << "ch111111 nexo cmd error:" << cmdType << errCode;
                nexoHandleFlag = 0;
            }
            else
            {
                timer5000ms->stop();
                ctlNexoHandle();             //send CMD to nexo
                timer5000ms->start(5000);
            }
            break;
        case 5:  //cmd send ok
            cmdType = recBuf.mid(20,4).toInt();
            errCount = 0;
            switch(cmdType)
            {
            case 900:
                curveNullBuf = "null";
                nexoHandleFlag &= ~0x00008000;  //angle subscribe
                nexoHandleFlag &= ~0x00004000; //torque subscribe
                break;
            case 18:     //select program
                timer5000ms->stop();
                nexoHandleFlag = 0x00000004;//set bolt number
                ctlNexoHandle();             //send CMD to nexo
                timer5000ms->start(5000);
                break;
            case 19:     //set bolt number
                timer5000ms->stop();
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
                timer5000ms->start(5000);
                break;
            case 42:    //tool disable
                nexoHandleFlag &= ~0x00000200;
                if(sendNextEnableFlag)
                {
                    sendNextEnableFlag = false;
                    qDebug() << "22222222222222 send next program";
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNum);
                }
                break;
            case 43:    //tool enable
                nexoHandleFlag &= ~0x00000400;
                break;
            case 50:    //vin download
                nexoHandleFlag &= ~0x00010000;
//                timer5000ms->stop();
//                nexoHandleFlag = 0x00000002;//select program
//                ctlNexoHandle();             //send CMD to nexo
//                timer5000ms->start(5000);
                break;
            case 60:    //result subscribe
                timer5000ms->stop();
                nexoHandleFlag = 0x40000000;//time set
                ctlNexoHandle();             //send CMD to nexo
                timer5000ms->start(5000);
                break;
            case 64:    //read old result
                nexoHandleFlag &= ~0x04000000;
                break;
            case 70:    //alarm subscribe
                break;
            case 127:    //abort job
                nexoHandleFlag &= ~0x00400000;
                break;
            case 34:
                nexoHandleFlag &= ~0x00400000;//Job info subcrible
                break;
            case 38:
                nexoHandleFlag &= ~0x00100000; //select Job
                break;
            case 82:    //set time
                timer5000ms->stop();
                nexoHandleFlag = 0x00008000;               //angle subscribe
                ctlNexoHandle();
                timer5000ms->start(5000);
                curve_AngleTorque = 3;
//                nexoHandleFlag &= ~0x40000000;

                break;
            default:
                break;
            }
            break;
        case 2:
            controlLinkOk = true;                //communication start OK
            if(controlType2 == "OFF")
            {
                emit IsTigntenReady(3,true);
            }
            else
            {
                emit IsTigntenReady(1,true);
            }
            timer5000ms->stop();
            nexoHandleFlag = 0x01000000;//tightening results data subscription
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
            break;
        case 9999://应答不显示在PC 测试使用
            break;
        case 71:    //alarm
            timer5000ms->stop();
            nexoHandleFlag |= 0x00001000;
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);
            break;
        case 76://system error status
            timer5000ms->stop();
            nexoHandleFlag = 0x10000000;//Status ACK ???????? not nexo ack
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
            break;
        case 61://Rev result data
            timer5000ms->stop();
            nexoHandleFlag = 0x02000000;//result ACK
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);
            if((SYSS == "ING")&&(vinBuf != ""))
            {
                revDataLen   = 0;
                revFinishLen = 0;
                curveBuf = "";
                curveBufTemp ="";
                curveFirstFlag = 0;
                curveDataFlag = 0;   //receive curve
                idCode = "";
                curve_AngleTorque = 0;

                nexoProNO = recBuf.mid(90,3);
                nexoResult = recBuf.mid(107,1);
                torqueMin = recBuf.mid(116,6).toDouble()/100;
                torqueMax = recBuf.mid(124,6).toDouble()/100;
                torqueValue = recBuf.mid(140,6);                           //torque value
                torqueValue = QByteArray::number(torqueValue.toDouble()/100);
                angleValue  = recBuf.mid(169,5);                           //angle value
                angleValue = QByteArray::number(angleValue.toDouble());
                tighenTime   = recBuf.mid(176,19);                          //Torque time
//                tighteningID = recBuf.mid(60,9);                           //nexo Tightening ID
                tighteningID = recBuf.mid(221,10);                           //Tightening ID
                intReadTighteningID = tighteningID.toInt();
                tighteningID.replace(0,1,Channel);
                if(intReadTighteningID == 0)
                {
                    qDebug() << "ch111111 data error ***********intReadTighteningID = 0****************";
                    return ;
                }
                if(nexoProNO != "099")       //99 program not count
                {
                    if(nexoResult == "0")
                    {
                        nexoResult = "NOK";   //0=tightening NOK ,1=OK
                    }
                    else
                    {
                        nexoResult = "OK" ;
                        if(bound_enabled)
                        {
                            QSettings *config = new QSettings("/config_bound.ini", QSettings::IniFormat);
                            double torque_max = config->value("/ProNumber"+QString::number(nexoProNO.toInt())+"/Torque_max").toDouble();
                            double torque_min = config->value("/ProNumber"+QString::number(nexoProNO.toInt())+"/Torque_min").toDouble();
                            double Angle_max = config->value("/ProNumber"+QString::number(nexoProNO.toInt())+"/Angle_max").toDouble();
                            double Angle_min = config->value("/ProNumber"+QString::number(nexoProNO.toInt())+"/Angle_min").toDouble();
                            delete config;
                            if(torque_max == 0)
                            {
                                torque_max = torqueValue.toDouble();
                                torque_min = torqueValue.toDouble();
                            }
                            if(torque_min == 0)
                                torque_min = torqueValue.toDouble();
                            if(Angle_max == 0)
                            {
                                Angle_max = angleValue.toDouble();
                                Angle_min = angleValue.toDouble();
                            }
                            if(Angle_min == 0)
                                Angle_min = angleValue.toDouble();
                            qDebug() << "*******" << nexoProNO<<torque_max<<torque_min<<torqueValue<< angleValue;
                            if((torqueValue.toDouble()<torque_min) || (torqueValue.toDouble()>torque_max) ||
                                    (angleValue.toDouble()<Angle_min) || (angleValue.toDouble() >Angle_max) )
                            {
                                nexoResult = "NOK";
                            }
                        }
                    }
                    if(nexoResult == "NOK")
                    {
                        if(boltNokCount > 0)
                            boltNokCount --;
                    }
                    else
                    {
                        if(boltCount > 0)
                            boltCount --;
                    }
                }
                else
                {
                    nexoResult = "NOK";                                  //99 program nok
                }
                qDebug()  <<"ch111111"<< tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime<<torqueMin<<torqueMax;
                if((torqueValue == "0")&&(angleValue == "0"))
                {
                    qDebug()<<"***curve is null ***";
                    curveNullBuf = "Curve is null";
                }
                else
                {
                    curveNullBuf = "null";
                }
                DATA_STRUCT demo;
                demo.data_model[0] = QString(tighenTime.mid(0,10));
                demo.data_model[1] = QString(tighenTime.mid(11,8));
                demo.data_model[2] = QString(nexoResult);
                demo.data_model[3] = QString(torqueValue);
                demo.data_model[4] = QString(angleValue);
                demo.data_model[5] = QString(boltNumberBuf);
                demo.data_model[6] = QString(vinBuf);
                demo.data_model[7] = QString(tighteningID);
                demo.data_model[8] = curveNullBuf;
                demo.data_model[9] = QString(nexoProNO);
                demo.data_model[10] = "0";
                demo.data_model[11] = Type;
                if(nexoResult == "OK")
                {
                    demo.data_model[12] = QString::number(BoltOrder[0]++);
                }
                else
                {
                    demo.data_model[12] = QString::number(BoltOrder[0]);
                }
                demo.data_model[13] = QString::number(torqueMax);
                demo.data_model[14] = QString::number(torqueMin);
                demo.data_model[15] = QString(Channel);
                demo.data_model[16] = QString(controllerIp);
                QVariant DataVar;
                DataVar.setValue(demo);
                emit sendfromworkthread(DataVar);
                if(sendDisableFlag)
                {
                    sendDisableFlag = false;
                    sendNextEnableFlag = true;           //receive disable cmd ACK
                }
                else
                {
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNum);
                }
            }
            break;
        default:
            break;
        }
    }
    else if(curveDataFlag == 1)
    {
        qDebug() <<curveFirstFlag << curveDataFlag << "recevice len:" << revLen << cmdLen ;
        if(curveFirstFlag == 0)
        {
            curveBuf = curveBuf + recBuf;
            revDataLen = revDataLen + revLen;
            qDebug() << "more data:" << curveBuf.size() << revDataLen << revFinishLen;
        }
        if(revDataLen >= (revFinishLen<<1))//curve receive finished
        {
            for(k=0;k<2;k++)
            {
                curveType = curveBuf.mid(56,3).data();
                idCode = curveBuf.mid(20,10); //ID code
                qDebug()<< "******curveType*********" <<curveType << idCode;
                for(i=0;i<revFinishLen;i++)
                {
                    if(curveBuf[i] == '\0')
                    {
                        nullCount = i + 1;
                        hexBuf[0] = '#';
                        j = 1;
                        break;
                    }
                }
                for(i=nullCount;i<revFinishLen;i++)
                {
                    hByte = (curveBuf[i]&0xf0)>>4;
                    lByte = curveBuf[i]&0x0f;
                    if(hByte<10)
                    {
                        hexBuf[j++] = hByte + '0';
                    }
                    else
                    {
                        hexBuf[j++] = hByte - 10 + 'a';
                    }
                    if(lByte<10)
                    {
                        hexBuf[j++] = lByte + '0';
                    }
                    else
                    {
                        hexBuf[j++] = lByte - 10 + 'a';
                    }
                }
                if(curveType == "050")//1 times angle
                {
                    qDebug()<< "**curve angle** curveType:" <<curveType << idCode;
                    if(curve_AngleTorque == 0)//curve is null
                    {
                        curve_AngleTorque = 1;//curve angle
                        curveBufTemp = "*";
                        curveBufTemp.append(curveBuf.mid(0,139));
                        curveBufTemp.append(hexBuf);
                        curveBufTemp.append('|');
                    }
                    else if(curve_AngleTorque == 2)//curve is torque
                    {
                        recBuf = "*";
                        recBuf.append(curveBuf.mid(0,139));
                        recBuf.append(hexBuf);
                        recBuf.append('|');
                        curveBufTemp.replace(0,1,recBuf);
                        if(tighteningID != "")
                        {
                            QString fileName = "/curveFile/" + tighteningID + ".txt";
                            QFile file(fileName);
                            if(!file.open(QIODevice::ReadWrite))
                            {
                                qDebug()   << "Cannot open wifi file2 for Writing";
                            }
                            else
                            {
                                file.write(curveBufTemp.toUtf8());
                                file.close();
                            }
                            curveDataFlag = 0;
                            tighteningID = "";
                            qDebug() << "torque_angle curve file path:" <<fileName;
                        }
                        else
                        {
                            qDebug() << "tighteningID == null:";
                        }
                    }
                    else
                    {
                        curve_AngleTorque = 0;
                    }
                }
                else if(curveType == "001") //2 times torque
                {
                    qDebug()<< "**curve torque** curveType:" <<curveType << idCode;
                    if(curve_AngleTorque == 0)//curve is null
                    {
                        curve_AngleTorque = 2;//curve torque
                        curveBufTemp = "*";
                        curveBufTemp.append(curveBuf.mid(0,139));
                        curveBufTemp.append(hexBuf);
                        curveBufTemp.append('}');
                    }
                    else if(curve_AngleTorque == 1)//curve is angle
                    {
                        curveBufTemp.append(curveBuf.mid(0,139));
                        curveBufTemp.append(hexBuf);
                        curveBufTemp.append('}');
                        if(tighteningID != "")
                        {
                            QString fileName = "/curveFile/" + tighteningID + ".txt";
                            QFile file(fileName);
                            if(!file.open(QIODevice::ReadWrite))
                            {
                                qDebug()   << "Cannot open wifi file2 for Writing";
                            }
                            else
                            {
                                file.write(curveBufTemp.toUtf8());
                                file.close();
                            }
                            curveDataFlag = 0;
                            tighteningID = "";
                            curve_AngleTorque = 0;
                            qDebug() << "angle_torque curve file path:" <<fileName;
                        }
                        else
                        {
                            qDebug() << "tighteningID == null:";
                        }
                    }
                    else
                    {
                        curve_AngleTorque = 0;
                    }
                }
                else
                {}
                if(k==1)
                {
                    curveBuf = curveBuf.mid(revFinishLen,revFinishLen);
                }
            }
            revDataLen   = 0;
            revFinishLen = 0;
            curveFirstFlag = 0;
        }
        else if(revDataLen >= revFinishLen) //curve receive finished
        {
            curveType = curveBuf.mid(56,3).data();
            idCode = curveBuf.mid(20,10); //ID code
            for(i=0;i<revFinishLen;i++)
            {
                if(curveBuf[i] == '\0')
                {
                    nullCount = i + 1;
                    hexBuf[0] = '#';
                    j = 1;
                    break;
                }
            }
            for(i=nullCount;i<revFinishLen;i++)
            {
                hByte = (curveBuf[i]&0xf0)>>4;
                lByte = curveBuf[i]&0x0f;
                if(hByte<10)
                {
                    hexBuf[j++] = hByte + '0';
                }
                else
                {
                    hexBuf[j++] = hByte - 10 + 'a';
                }
                if(lByte<10)
                {
                    hexBuf[j++] = lByte + '0';
                }
                else
                {
                    hexBuf[j++] = lByte - 10 + 'a';
                }
            }
            if(curveType == "050")//1 times angle
            {
                qDebug()<< "**curve angle** curveType:" <<curveType << idCode;
                if(curve_AngleTorque == 0)//curve is null
                {
                    curve_AngleTorque = 1;//curve angle
                    curveBufTemp = "*";
                    curveBufTemp.append(curveBuf.mid(0,139));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('|');

                    timer5000ms->stop();
                    nexoHandleFlag = 0x00004000;                  //torque subscribe
                    ctlNexoHandle();                              //send CMD to nexo
                    timer5000ms->start(5000);
                }
                else if(curve_AngleTorque == 2)//curve is torque
                {
                    recBuf = "*";
                    recBuf.append(curveBuf.mid(0,139));
                    recBuf.append(hexBuf);
                    recBuf.append('|');
                    curveBufTemp.replace(0,1,recBuf);
                    if(tighteningID != "")
                    {
                        QString fileName = "/curveFile/" + tighteningID + ".txt";
                        QFile file(fileName);
                        if(!file.open(QIODevice::ReadWrite))
                        {
                            qDebug()   << "Cannot open wifi file2 for Writing";
                        }
                        else
                        {
                            file.write(curveBufTemp.toUtf8());
                            file.close();
                        }
                        curveDataFlag = 0;
                        tighteningID = "";
                        curve_AngleTorque = 0;
                        qDebug() << "torque_angle curve file path:" <<fileName;
                    }
                    else
                    {
                        qDebug() << "tighteningID == null:";
                    }
                }
                else
                {
                    curve_AngleTorque = 0;
                }
            }
            else if(curveType == "001") //2 times Torque
            {
                qDebug()<< "**curve torque** curveType:" <<curveType << idCode;
                if(curve_AngleTorque == 0)//curve is null
                {
                    curve_AngleTorque = 2;//curve torque
                    curveBufTemp = "*";
                    curveBufTemp.append(curveBuf.mid(0,139));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('}');

                    timer5000ms->stop();
                    nexoHandleFlag = 0x00008000;                  //angle subscribe
                    ctlNexoHandle();                              //send CMD to nexo
                    timer5000ms->start(5000);
                }
                else if(curve_AngleTorque == 1)//curve is angle
                {
                    curveBufTemp.append(curveBuf.mid(0,139));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('}');
                    if(tighteningID != "")
                    {
                        QString fileName = "/curveFile/" + tighteningID + ".txt";
                        QFile file(fileName);
                        if(!file.open(QIODevice::ReadWrite))
                        {
                            qDebug()   << "Cannot open wifi file2 for Writing";
                        }
                        else
                        {
                            file.write(curveBufTemp.toUtf8());
                            file.close();
                        }
                        curveDataFlag = 0;
                        tighteningID = "";
                        qDebug() << "angle_torque curve file path:" <<fileName;
                    }
                    else
                    {
                        qDebug() << "tighteningID == null:";
                    }
                }
                else
                {
                    curve_AngleTorque = 0;
                }
            }
            else
            {}
            if(revDataLen > revFinishLen + 21)//more then 1460
            {
                revDataLen = revDataLen - revFinishLen;
                curveBuf   = curveBuf.mid(revFinishLen,revDataLen);
                qDebug() << "&&&& more then 1460&&&&" << revDataLen;
                return;
            }
            else if(revDataLen > revFinishLen)
            {
                qDebug() << "&&&& ++++++ &&&&&&" << curveBuf.mid(revFinishLen , revDataLen-revFinishLen);
            }
            else
            {
                qDebug() << "&& curve receive OK&&&&&";
            }
            revDataLen   = 0;
            revFinishLen = 0;
            curveFirstFlag = 0;
        }
        else
        {
            qDebug() << "88888888888 data length" << revDataLen << revFinishLen;
            curveFirstFlag = 0;
        }
    }
    else
    {
        qDebug() << "ch111111 9999999999 data error";
    }

}
/**************************************/
//send CMD IR controller
/**************************************/
void TightenOpNetCh1::sendCMD_IR(int IRHandleFlag,QByteArray mid)
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
    qDebug() << "ch111111 send    :" <<Tx_len<< sendBuf;
    tcpSocket->write(sendBuf,Tx_len);
    tcpSocket->waitForBytesWritten(300);
}


/***********************************/
//处理接受IR数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/

void TightenOpNetCh1::revIRData()
{
    QByteArray torqueMax;
    QByteArray torqueMin;
    int revLen,dataLen;
    QByteArray recBuf;//,hexBufTemp;
    recBuf.clear();
    recBuf = tcpSocket->readAll();
    revLen = recBuf.size();
    qDebug() << "ch111111 recevice:" << revLen  << recBuf;
    if(revLen>=16)
    {
        IR_AliveCount = 0;
        MID = recBuf.mid(6,2);
        if(recBuf.mid(0,6) == "SES01R")      //session start success
        {
            if(controlType2 == "OFF")
            {
                emit IsTigntenReady(3,true);
            }
            else
            {
                emit IsTigntenReady(1,true);
            }
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
                tighteningID = recBuf.mid(16,6).trimmed();                 //Tightening ID
                nexoProNO    = recBuf.mid(32,3).trimmed();                 //tighten program
                tighenTime   = recBuf.mid(37,16);                          //tighten time
                nexoResult   = recBuf.mid(54,1).trimmed();                 //tighten result
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
                if(nexoResult == "F")
                {
                    nexoResult = "NOK";   //0=tightening NOK ,1=OK
                }
                else
                {
                    nexoResult = "OK" ;
                }
                if(nexoResult == "NOK")
                {
                    if(boltNokCount > 0)
                        boltNokCount --;
                }
                else
                {
                    if(boltCount > 0)
                        boltCount --;
                }
                qDebug()  <<"ch111111"<<boltNumberBuf<< tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime<<torqueMin<<torqueMax;
                if(SYSS == "ING")
                {
                    DATA_STRUCT demo;
                    demo.data_model[0] = QString(tighenTime.mid(6,2) + tighenTime.mid(2,4)+ tighenTime.mid(0,2));
                    demo.data_model[1] = QString(tighenTime.mid(8,8));
                    demo.data_model[2] = QString(nexoResult);
                    demo.data_model[3] = QString(torqueValue);
                    demo.data_model[4] = QString(angleValue);
                    demo.data_model[5] = QString(boltNumberBuf);
                    demo.data_model[6] = QString(vinBuf);
                    demo.data_model[7] = QString(tighteningID);
                    demo.data_model[8] = "Curve is null";
                    demo.data_model[9] = QString(nexoProNO);
                    demo.data_model[10] = "0";
                    demo.data_model[11] = Type;
                    if(nexoResult == "OK")
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]++);
                    }
                    else
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]);
                    }
                    demo.data_model[13] = QString(torqueMax);
                    demo.data_model[14] = QString(torqueMin);
                    demo.data_model[15] = QString(Channel);
                    demo.data_model[16] = controllerIp;
                    QVariant DataVar;
                    DataVar.setValue(demo);
                    emit sendfromworkthread(DataVar);
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNum);
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
void TightenOpNetCh1::timerFunc1000ms()
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
}





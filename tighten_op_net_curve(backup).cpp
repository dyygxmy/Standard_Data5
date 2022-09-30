/**********************************************************************
 * file name  tighten_op_net.c
  describe   ：control enable / program / vin
 *             Last tightening result data subscribe
 *             Old tightening result upload request
 *
***********************************************************************/
#include "tighten_op_net_curve.h"
#define  DEBUG_OUT

TightenOpNetCurve::TightenOpNetCurve(QObject *parent) :
    QObject(parent)
{
    powerOnFlag = true;                //data2 restart
    errCount = 0;
    errType = 0;
    programNO = 0;
    boltCount = 0;
    boltNokCount = 0;
    intReadTighteningID = 0;     //current read Tightening form nexo
    intNexoMaxTighteningID = 0;    //nexo Tightening ID
    nexoHandleFlag = 0;
    readOldDataFlag = false;     //read old data
    readMaxTightenIDFlag = false;
    nexoLinkOk = false;
    nexoAliveCount = 0;
    revDataLen =0;
    revFinishLen = 0;
    curveFirstFlag = 0;
    curveDataFlag = 0;
    sendVinFlag = 0;
    curveBuf.clear();
    sendNextEnableFlag = false;
    sendDisableFlag = false;
    strJobID = "01";

    testNum = 0;
    Factory = factory;

    curveBuf = "null";

    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpNetCurve::tightenStart()
{
    qDebug() << "tighten_op_net thread start!!!";

    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    newConnects();
    timer5000ms->start(1000);
}
//使能
void TightenOpNetCurve::sendReadOperate(bool enable,int n)
{
    DTdebug() << enable << n ;
    if(!enable) //
    {
        timer5000ms->stop();
        nexoHandleFlag = 0x00000200;//nexo disable
        ctlNexoHandle();             //send CMD to nexo
        timer5000ms->start(5000);
    }
    else
    {
        Channel = "0";
        programNO = carInfor[0].proNo[n].toShort();// proNum.toShort();
        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        boltNokCount = 3;
        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
        if(VIN_PIN_SQL.toLatin1() == vinBuf)
        {
            sendVinFlag = 0;
        }
        else
        {
            vinBuf = VIN_PIN_SQL.toLatin1();
            sendVinFlag = 1;
        }
        if(nexoLinkOk)
        {
            timer5000ms->stop();
            { //add by wxm 2020-11-25
                nexoHandleFlag = 0x00010000;
                ctlNexoHandle();
            }
            { //add by wxm 2020-11-25
                nexoHandleFlag = 0x00000004;//set bolt number
                ctlNexoHandle();             //send CMD to nexo
            }
            nexoHandleFlag = 0x00000002;//select program
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);
        }
        else
        {}
    }
}

/**************************************/
//nexo alive
/**************************************/
void TightenOpNetCurve::timerFunc5000ms()
{
    static int count = 0;
    count++;

    if(tcpSocket && (tcpSocket->state() != QAbstractSocket::ConnectedState))
    {
        newConnects();
    }

    if(count >= 5)
    {
        count = 0;

        qDebug() << "op 5000ms tiemr" << nexoLinkOk << nexoHandleFlag << nexoAliveCount << nexoHandleFlag<< SYSS;
        if(nexoLinkOk)                   //
        {
            nexoHandleFlag |= 0x00800000;//Alive
            ctlNexoHandle();            //send CMD to nexo
        }
        if(nexoAliveCount > 2)             //3 times alive not ack
        {
            nexoAliveCount = 0;
            if(nexoLinkOk )
            {
                emit IsTigntenReady(false);
                nexoLinkOk = false;
            }
            else
            {
                qDebug() << "3 times alive no ack disconnect  ";
                newConnects();
            }
        }
        nexoAliveCount++;
    }
}

/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void TightenOpNetCurve::disConnectDO()
{
    nexoLinkOk = false;
    qDebug() << "OP disconnect 22222222222";
    //newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpNetCurve::newConnects()
{
    tcpSocket->abort(); //取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(ControllerIp_1,4545);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    DTdebug() << "OP the socket state is" << ControllerIp_1 << tcpSocket->state();
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "OP link success  >>>>>>>>";
        nexoAliveCount = 0;
        timer5000ms->stop();
        nexoHandleFlag  = 0x00000001;  //start communication
        ctlNexoHandle();               //send CMD to nexo
        timer5000ms->start(5000);
    }
    else
    {
        nexoLinkOk = false;
        qDebug() << "OP link fail XXXXXXXXXXXX" ;
    }
}

/***********************************/
//处理控制Nexo函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void TightenOpNetCurve::ctlNexoHandle()
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
        sendBuf = "002000605001        ";    //REV=500
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
        return;
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
    /**********   Job info subscribe***********/
    /****************************************/
    else if(nexoHandleFlag&0x00400000)
    {
        nexoHandleFlag &= ~0x00400000;
        Tx_len = 21;
        sendBuf = "00200034001         ";
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

    //   if(sendBuf.mid(4,4) != "9999")

    quint64 tmpLen = tcpSocket->write(sendBuf,Tx_len);
    bool tmpSendOK = tcpSocket->waitForBytesWritten(300);

    DTdebug() << "send " << tmpLen << tmpSendOK << " :" << sendBuf;   // << "nexoHandleFlag
}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/

void TightenOpNetCurve::revNexoData()
{
    bool okFlag;
    int nexoMID;
    int cmdType;
    int errCode;
    int revLen,cmdLen;
    double torqueMax = 0.0;
    double torqueMin = 0.0;

    recBuf.append(tcpSocket->readAll());

    nexoMID = recBuf.mid(4,4).toInt();
    revLen = recBuf.size();
    cmdLen = recBuf.mid(0,4).toInt();
    qDebug() << "recevice len:" << revLen << cmdLen << "note:" << recBuf;

    if(revLen < (cmdLen+1))
    {
        qDebug() << "continue.......";
        return;
    }

    if(revLen == (cmdLen+1))              //length of the same
    {
        nexoAliveCount = 0;           //recvice nexo data
        switch(nexoMID)
        {
        case 4:   //cmd send error         //send CMD to n
            cmdType = recBuf.mid(20,4).toInt();
            errCode = recBuf.mid(24,2).toInt();
            errCount ++;
            if(errCount > 2)   //over 3 timer no send cmdType
            {
                errCount = 0;
                qDebug() << "nexo cmd error:" << cmdType << errCode;
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
                        emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNumCh[0]);
                    }
                    break;
                case 43:    //tool enable
                    nexoHandleFlag &= ~0x00000400;
                    break;
                case 50:    //vin download
                    timer5000ms->stop();
                    nexoHandleFlag = 0x00000002;//select program
                    ctlNexoHandle();             //send CMD to nexo
                    timer5000ms->start(5000);
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
                case 34:
                    nexoHandleFlag &= ~0x00400000;//Job info subcrible
                    break;
                case 38:
                    nexoHandleFlag &= ~0x00100000; //select Job
                    break;
                case 82:    //set time
                    timer5000ms->stop();
                    if(SYSS == "ING")
                    {
                        readOldDataFlag = true;
                    }
                    else
                    {
                        readOldDataFlag = false;
                    }
                    readMaxTightenIDFlag = true;
                    nexoHandleFlag = 0x04000000;//read old data max tightenID
                    ctlNexoHandle();                 //send CMD to nexo
                    timer5000ms->start(5000);
                    break;
                default:
                    break;
            }
            break;
        case 2:
            nexoLinkOk = true;                //communication start OK
            if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
            {
                SYSS = "Ready";
            }
            emit IsTigntenReady(true);
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
            if(SYSS == "ING")
            {
                testNum ++;

                //add by wxm 2021-1-7
                {
                    T_MID61_DATA_V2 tmpData;
                    //memcpy((char*)&tmpData, recBuf.data(), recBuf);
                    memcpy((char*)&tmpData, recBuf.data(), qMin((int)sizeof(T_MID61_DATA_V2), recBuf.size())); //add by wxm 2021-3-1
                    QStringList tmpTorqueList, tmpAngleList;
                    int tmpTraceLength = QString::fromLatin1((char*)(tmpData.mTraceLength+2), 4).toInt();
                    int tmpAngleOffset = QString::fromLatin1((char*)(tmpData.mAngleOffset+2), 6).toInt();
                    int tmpBinaryOffset = QString::fromLatin1((char*)(tmpData.mBinaryOffset+2), 10).toInt();
                    double tmpTorqueScaleFactor = 1000000.0 / QString::fromLatin1((char*)(tmpData.mTorqueScaleFactor+2), 10).toDouble();
                    if(tmpTorqueScaleFactor == 0)
                        tmpTorqueScaleFactor = 1;
                    DTdebug() << "tmpTraceLength" << tmpTraceLength;
                    DTdebug() << "tmpAngleOffset" << tmpAngleOffset;
                    DTdebug() << "tmpBinaryOffset" << tmpBinaryOffset;
                    DTdebug() << "tmpTorqueScaleFactor" << tmpTorqueScaleFactor;
                    for(int i=0;i<tmpTraceLength;i++)
                    {
                        int tmpIndex = 2+i*2;
                        if(tmpIndex > 9000)  //add by wxm 2021-3-1
                            break;
                        int tmpDataBlobValue = tmpData.mDataBlob[tmpIndex];
                        tmpDataBlobValue = (tmpDataBlobValue<<8) + tmpData.mDataBlob[tmpIndex+1];
                        DTdebug() << tmpDataBlobValue;
                        double tmpTorqueValue = (double)(tmpDataBlobValue-tmpBinaryOffset)/tmpTorqueScaleFactor;
                        tmpTorqueList.append(QString::number(tmpTorqueValue));
                        tmpAngleList.append(QString::number((i+1)*tmpAngleOffset));
                    }
                    curveBuf = QString(tmpTorqueList.join(",") + QString("|") + tmpAngleList.join(",")).toLatin1();

                    nexoProNO = QByteArray::fromRawData((char*)(tmpData.mApplicationID+2), 3);
                    nexoResult = QByteArray::fromRawData((char*)(tmpData.mTighteningStatus+2), 1);
                    torqueValue = QByteArray::fromRawData((char*)(tmpData.mTorque+2), 6);                           //torque value
                    torqueValue = QByteArray::number(torqueValue.toDouble()/100);
                    angleValue  = QByteArray::fromRawData((char*)(tmpData.mAngle+2), 5);                             //angle value
                    angleValue = QByteArray::number(angleValue.toDouble());
                    tighenTime   = QByteArray::fromRawData((char*)(tmpData.mTimeStamp+2), 19);                          //Torque time
                    tighteningID = QByteArray::fromRawData((char*)(tmpData.mTightenID+2), 10);                          //Tightening ID
                }

#if 0
                nexoProNO = recBuf.mid(90,3);
                nexoResult = recBuf.mid(107,1);
                torqueValue = recBuf.mid(140,6);                           //torque value
                torqueValue = QByteArray::number(torqueValue.toDouble()/100);
                angleValue  = recBuf.mid(169,5);                           //angle value
                angleValue = QByteArray::number(angleValue.toDouble());
                tighenTime   = recBuf.mid(176,19);                          //Torque time
//                tighteningID = recBuf.mid(60,9);                           //nexo Tightening ID
                tighteningID = recBuf.mid(221,10);                           //Tightening ID
#endif

                qDebug() << "****************tighteningID" << tighteningID;
                intReadTighteningID = tighteningID.toInt();
                intNexoMaxTighteningID = intReadTighteningID;
                if(intReadTighteningID == 0)
                {
                    qDebug() << "data error ***********intReadTighteningID = 0****************";
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
                            qDebug() << "*******" << nexoProNO << programNO << nexoProNO.toInt() ;
                            if((torqueValue.toDouble()<torque_min) || (torqueValue.toDouble()>torque_max) ||
                                    (angleValue.toDouble()<Angle_min) || (angleValue.toDouble() >Angle_max)||
                                    (nexoProNO.toInt() != programNO) )
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
                DTdebug()  << tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime;

                demo.data_model[9] = QString(nexoProNO);
                demo.data_model[0] = QString(tighenTime.mid(0,10));
                demo.data_model[1] = QString(tighenTime.mid(11,8));
                demo.data_model[2] = QString(nexoResult);
                demo.data_model[3] = QString(torqueValue);
                demo.data_model[4] = QString(angleValue);
                demo.data_model[5] = QString(boltNumberBuf);
                demo.data_model[6] = QString(vinBuf);
                demo.data_model[7] = QString(tighteningID);
                demo.data_model[8] = curveBuf;
                demo.data_model[10] = "0";
                if(Factory =="BYDSZ" ||Factory=="BYDXA" ||Factory=="NOVAT"||Factory == "AQCHERY")
                {
                    demo.data_model[11] = AutoNO;
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
                    demo.data_model[15] = "0";
                }
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
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNumCh[0]);
                    if(testNum == 5)
                    {
                        testNum = 0;
                    }
                    usleep(10000);
                }
            }
            break;
        case 65://old result data
            nexoHandleFlag &= ~0x04000000;                    //history results upload request
            nexoProNO = recBuf.mid(61,3);
            torqueValue = recBuf.mid(81,6);
            angleValue  = recBuf.mid(89,5);
            tighenTime   = recBuf.mid(96,19);
            nexoOldTigheningID = recBuf.mid(22,10);
            if(readMaxTightenIDFlag)                                   //send 000000000 Tightening ID
            {
                readMaxTightenIDFlag = false;
                intNexoMaxTighteningID = nexoOldTigheningID.toInt(&okFlag,10);
                timer5000ms->stop();
                if(!readOldDataFlag) //not read old data
                {
                    intReadTighteningID = intNexoMaxTighteningID;
                    nexoHandleFlag = 0x00000200;//nexo disable
                }
                else                 //need read old data
                {
                    if(intNexoMaxTighteningID > intReadTighteningID)
                    {
                        intReadTighteningID ++;
                        nexoHandleFlag = 0x04000000;            //history results upload request
                    }
                    else
                    {
//                                nexoHandleFlag = 0x00010000;           //ID code download
                        nexoHandleFlag = 0x00000002;//select program
//                        nexoHandleFlag = 0x00000400;//tool enable
                    }
                }
                ctlNexoHandle();                              //send CMD to nexo
                timer5000ms->start(5000);
            }
            else//
            {
                if(nexoProNO != "099")       //99 program not count
                {
                    if(recBuf[72] == '0')
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
                            if((torqueValue.toDouble()<torque_min) || (torqueValue.toDouble()>torque_max) ||
                                    (angleValue.toDouble()<Angle_min) || (angleValue.toDouble() >Angle_max))
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
                    if(boltNokCount == 0)                                   //nok count finished
                    {
                        timer5000ms->stop();
                        nexoHandleFlag = 0x00000200;                        //tool disable
                        ctlNexoHandle();                                     //send CMD to nexo
                        timer5000ms->start(5000);
                    }
                    else if(boltCount == 0)                                 //ok count finished
                    {
                        timer5000ms->stop();
                        nexoHandleFlag = 0x00000200;                        //tool disable
                        ctlNexoHandle();                                     //send CMD to nexo
                        timer5000ms->start(5000);
                    }
                    else
                    {
                        timer5000ms->stop();
                        if(intNexoMaxTighteningID > intReadTighteningID)
                        {
                            intReadTighteningID ++;
                            nexoHandleFlag = 0x04000000;                    //history results upload request
                        }
                        else
                        {
                            //                                    nexoHandleFlag = 0x00010000;                     //ID code download
                            //                                    nexoHandleFlag = 0x00000002;//select program
                            nexoHandleFlag = 0x00000400;//tool enable
                        }
                        ctlNexoHandle();                                     //send CMD to nexo
                        timer5000ms->start(5000);
                    }
                }
                else
                {
                    timer5000ms->stop();
                    if(intNexoMaxTighteningID > intReadTighteningID)
                    {
                        intReadTighteningID ++;
                        nexoHandleFlag = 0x04000000;                    //history results upload request
                    }
                    else
                    {
                        //                                nexoHandleFlag = 0x00000002;//select program
                        //                                nexoHandleFlag = 0x00010000;                     //ID code download
                        nexoHandleFlag = 0x00000400;//tool enable
                    }
                    ctlNexoHandle();                                     //send CMD to nexo
                    timer5000ms->start(5000);
                    nexoResult = "NOK";                                  //99 program nok
                }
                qDebug()  << nexoOldTigheningID  << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime << intNexoMaxTighteningID << intReadTighteningID;
                DATA_STRUCT demo;
                demo.data_model[0] = QString(tighenTime.mid(0,10));
                demo.data_model[1] = QString(tighenTime.mid(11,8));
                demo.data_model[2] = QString(nexoResult);
                demo.data_model[3] = QString(torqueValue);
                demo.data_model[4] = QString(angleValue);
                demo.data_model[5] = QString(boltNumberBuf);
                demo.data_model[6] = QString(vinBuf);
                demo.data_model[7] = QString(nexoOldTigheningID);
                demo.data_model[8] = "null";
                demo.data_model[9] = QString(nexoProNO);
                demo.data_model[10] = "0";
                if(Factory =="BYDSZ" ||Factory=="BYDXA")
                {
                    demo.data_model[11] = AutoNO;
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
                    demo.data_model[15] = "0";
                }
                QVariant DataVar;
                DataVar.setValue(demo);
                emit sendfromworkthread(DataVar);
                emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNumCh[0]);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        qDebug() << "9999999999 data error"<< revLen << cmdLen << "note:" << recBuf;
    }
    recBuf.clear();
}


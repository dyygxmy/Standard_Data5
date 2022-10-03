/**********************************************************************
 * file name  tighten_op_atlas_PM.c
 * describe   ：control enable
 *              Last tightening result data subscribe
 *
***********************************************************************/
#include "alatspm4000.h"

alatsPM4000::alatsPM4000(QObject *parent) :
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
    nexoLinkOk = false;
    nexoAliveCount = 0;
    curveFirstFlag = 0;
    curveDataFlag = 0;
    tighteningID = "";
    Factory = factory;
    angleSubFlag = 0;
    boltAmount = 0 ;
    currentBolt = 0 ;
    chooseCarType = false ;
    curveNullBuf = "Curve is null";
    timerNum = 5000;

    m_thread.start();
    this->moveToThread(&m_thread);
}

//线程开始
void alatsPM4000::tightenStart()
{
    FUNC() ;    
    //timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    newConnects();
    QTimer::singleShot(timerNum,this,SLOT(timerFunc5000ms()));
}

void alatsPM4000::slotBoltAmount(int num)
{
    FUNC() ;
    chooseCarType =true ;
    currentBolt = 0 ;
    boltAmount = num /*+1*/ ; //应该是num的，+1为了后面走for循环
    DTdebug() << chooseCarType << num << boltCount ;
}

void alatsPM4000::slotSkipBolt()
{
    currentBolt++ ;
}

/**************************************/
//nexo alive
/**************************************/
void alatsPM4000::timerFunc5000ms()
{
    DTdebug() << "op 5000ms tiemr" << nexoLinkOk << nexoHandleFlag << nexoAliveCount << SYSS;
    if(nexoLinkOk)                   //nexo link
    {
        nexoHandleFlag |= 0x00800000;//Alive
        ctlNexoHandle();             //send CMD to nexo
    }
    else
    {}
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
			  nexoAliveCount = 0;
            qDebug() << "3 times alive no ack disconnect  ";
            newConnects();
        }
    }
    nexoAliveCount++;
    QTimer::singleShot(timerNum,this,SLOT(timerFunc5000ms()));
}

/*******************************************************
 * nexo disconnect handle
*******************************************************/
void alatsPM4000::disConnectDO()
{
    nexoLinkOk = false;
    DTdebug() << "OP disconnect";
    newConnects();
}

/*******************************************************
 * nexo bulid connect
*******************************************************/
void alatsPM4000::newConnects()
{
    tcpSocket->abort(); //取消已有的连接

    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(ControllerIp_1,4545);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    DTdebug() << "PF the socket state is" << ControllerIp_1 << tcpSocket->state();
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DTdebug() << "OP link success";
        nexoAliveCount = 0;
        nexoHandleFlag  = 0x00000001;  //start communication
        ctlNexoHandle();               //send CMD to nexo
    }
    else
    {
        nexoLinkOk = false;
        DTdebug() << "OP link fail" ;
    }
}

/***********************************
 * 处理控制Nexo函数
 * len:收到的网络包长度
 * Rx_Buffer[]:收到的网络包数据
 * Tx_len:发送网络包长度
 * Tx_Buffer[]:发送网络包数据
 *************************************/
void alatsPM4000::ctlNexoHandle()
{
    FUNC() ;
    short Tx_len = 0;
    QByteArray sendBuf;
    /****************************************
     * system error system ACK
     ****************************************/
    if(nexoHandleFlag&0x10000000)
    {
        nexoHandleFlag &= ~0x10000000;
        Tx_len = 21;
        sendBuf = "00200077001         ";
    }

    /****************************************
     * Atlas PM results data ACK
     * MID 0108
     ****************************************/
    else if(nexoHandleFlag&0x02000000)
    {
        nexoHandleFlag &= ~0x02000000;
        Tx_len = 22;
        sendBuf = "00210108            0";
    }
    /****************************************
     * Alarm  ACK
     ****************************************/
    else if(nexoHandleFlag&0x00001000)
    {
        nexoHandleFlag &= ~0x00001000;
        Tx_len = 21;
        sendBuf = "00200072001         ";
    }

    /****************************************
     * results curve ACK
     ****************************************/
    else if(nexoHandleFlag&0x00002000)
    {
        nexoHandleFlag &= ~0x00002000;
        Tx_len = 25;
        sendBuf = "00240005001         0900";
    }
    /****************************************
     * communication stop
     ****************************************/
    else if(nexoHandleFlag&0x00000010)
    {
        Tx_len = 21;
        sendBuf = "00200003001         ";
    }
    /****************************************
     * communication start
     ****************************************/
    else if(nexoHandleFlag&0x00000001)
    {
        Tx_len = 21;
        sendBuf = "00200001001         ";
    }

    /****************************************
     * system error messages
     ****************************************/
    else if(nexoHandleFlag&0x20000000)
    {
        Tx_len = 21;
        sendBuf = "00200070001         ";
    }

    /****************************************
     * Atlas PM Last tightening results data subscribe
     * MID 0105
     ****************************************/
    else if(nexoHandleFlag&0x01000000)
    {
        Tx_len = 21;
        sendBuf = "00200105            ";
    }

    /****************************************
     * New curve Angle subscribe
     ****************************************/
    else if(nexoHandleFlag&0x00008000)
    {
        Tx_len = 65;
        sendBuf = "006400080011        0900001350                             01001";
    }
    /****************************************
     * New curve Torque subscribe
     ****************************************/
    else if(nexoHandleFlag&0x00004000)
    {
        Tx_len = 65;
        sendBuf = "006400080011        0900001350                             01002";
    }
    /****************************************
     * time set
     ****************************************/
    else if(nexoHandleFlag&0x40000000)
    {
        Tx_len = 40;
        sendBuf = "00390082001         2015-05-01:12:01:01";
        QDateTime time = QDateTime::currentDateTime();
        QString strTime = time.toString("yyyy-MM-dd:hh:mm:ss");
        sendBuf.replace(20,19,strTime.toLatin1());
    }
    /****************************************
     * history results upload request
     ****************************************/
    else if(nexoHandleFlag&0x04000000)
    {
        Tx_len = 31;
        sendBuf = "00300064001         0000000000";

    }
    /****************************************
     * selected program
     ****************************************/
    else if(nexoHandleFlag&0x00000002)
    {
        Tx_len = 24;
        sendBuf = "00230018001         001";
        sendBuf[21] = programNO/10 + 0x30;
        sendBuf[22] = programNO%10 + 0x30;
    }

    /***************************************
     * set bolt number
     ***************************************/
    else if(nexoHandleFlag&0x00000004)
    {
        Tx_len = 26;
        sendBuf = "00250019001         00105";
        sendBuf[21] = programNO/10 + 0x30;
        sendBuf[22] = programNO%10 + 0x30;
        sendBuf[23] = boltCount/10 + 0x30;
        sendBuf[24] = boltCount%10 + 0x30;
    }
    /****************************************
     * ID code
     ****************************************/
    else if(nexoHandleFlag&0x00010000)
    {
        Tx_len = 38;
        sendBuf = "00370050            LSVAE45E7EN123456";
        sendBuf.replace(20,17,vinBuf); //replace vincode
    }
    /****************************************
     * tool disable
     ****************************************/
    else if(nexoHandleFlag&0x00000200)
    {
        Tx_len = 21;
        sendBuf = "00200042001         ";
    }
    /****************************************
     * tool enable
     ****************************************/
    else if(nexoHandleFlag&0x00000400)
    {
        Tx_len = 21;
        sendBuf = "00200043001         ";
    }
    /****************************************
     * Job info subscribe
    /****************************************/
    else if(nexoHandleFlag&0x00400000)
    {
        nexoHandleFlag &= ~0x00400000;
        Tx_len = 21;
        sendBuf = "00200034001         ";
    }

    DTdebug() << "send" << sendBuf << "nexoHandleFlag :" << nexoHandleFlag ;
    tcpSocket->write(sendBuf,Tx_len);
    tcpSocket->waitForBytesWritten(300);

}

void alatsPM4000::revNexoData()
{
    FUNC() ;
    QByteArray recBuf = "";
    recBuf.clear();
    recBuf = tcpSocket->readAll();
    DTdebug()<<recBuf;

    splitNexoData(recBuf);
}

void alatsPM4000::splitNexoData(QByteArray buf)
{
    FUNC() ;

    DATA_STRUCT demo;
    QVariant DataVar;
    bool okFlag;
    int nexoMID;
    int cmdType;
    int errCode;
    int nullCount=0,i=0,j=0,k=0;
    int hByte,lByte;
    int revLen,cmdLen;
    int PID_Len = 0;
    int TempLen = 0;
    QByteArray torqueMax = "";
    QByteArray torqueMin = "";
    double doubleAngle = 0.0;
    int intAngle = 0;
    QByteArray hexBuf;//,hexBufTemp;

    QByteArray byBuf = buf ;
    nexoMID = byBuf.mid(4,4).toInt();
    revLen = byBuf.size();
    cmdLen = byBuf.mid(0,4).toInt();

    DTdebug() << "recevice len:" << revLen
              << cmdLen /*<< "note:" << byBuf*/;

    if(nexoMID == 900)
    {
        revDataLen   = revLen;
        revFinishLen = cmdLen;
        curveBuf = byBuf;
        curveFirstFlag = 1;
        curveDataFlag = 1;   //receive curve
        idCode = "";
    }
    else
    {
        DTdebug() << "nexoMID" << nexoMID;
    }

    if(revLen == (cmdLen+1))              //length of the same
    {
        DTdebug() <<curveFirstFlag << curveDataFlag ;
        nexoAliveCount = 0;           //recvice nexo data
        switch(nexoMID)
        {
        case 4:   //cmd send error         //send CMD to n
            cmdType = byBuf.mid(20,4).toInt();
            errCode = byBuf.mid(24,2).toInt();
            errCount ++;
            if(errCount > 2)   //over 3 timer no send cmdType
            {
                errCount = 0;
                DTdebug() << "nexo cmd error:" << cmdType << errCode;
                nexoHandleFlag = 0;
            }
            else
            {
                ctlNexoHandle();             //send CMD to nexo
            }
            break;
        case 5:  //cmd send ok
            cmdType = byBuf.mid(20,4).toInt();
            errCount = 0;
            switch(cmdType)
            {
            case 8:
                curveNullBuf = "null";
                if(angleSubFlag)
                {
                    angleSubFlag = 0;
                    nexoHandleFlag = 0x00004000;   //torque subscribe
                    ctlNexoHandle();               //send CMD to nexo
                }
                else
                {
                    nexoHandleFlag &= ~0x00004000;   //torque subscribe
                }
                break;
            case 18:     //select program
                nexoHandleFlag = 0x00000004;//set bolt number
                ctlNexoHandle();             //send CMD to nexo
                break;
            case 19:     //set bolt number
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
                break;
            case 42:    //tool disable
                nexoHandleFlag &= ~0x00000200;
                if(sendNextEnableFlag)
                {
                    sendNextEnableFlag = false;
                    DTdebug() << "send next program";
                }
                break;
            case 43:    //tool enable
                nexoHandleFlag &= ~0x00000400;
                break;
            case 50:    //vin download
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
                break;
            case 60:    //result subscribe
                nexoHandleFlag = 0x40000000;//time set
                ctlNexoHandle();             //send CMD to nexo
                break;
            case 64:    //read old result
                nexoHandleFlag &= ~0x04000000;
                break;
            case 70:    //alarm subscribe
                break;
            case 82:    //set time
                nexoHandleFlag = 0x00008000;     //angle subscribe
                ctlNexoHandle();                 //send CMD to nexo
                break;
            case 105:
                nexoHandleFlag = 0x00008000;     //angle subscribe
                ctlNexoHandle();                 //send CMD to nexo
                angleSubFlag = 1;
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
            nexoHandleFlag = 0x01000000;//tightening results data subscription
            ctlNexoHandle();             //send CMD to nexo
            break;
        case 9999://应答不显示在PC 测试使用
            break;
        case 106://PM Rev result data
            nexoHandleFlag = 0x02000000;//result ACK
            ctlNexoHandle();            //send CMD to nexo

            vinBuf      = byBuf.mid(121,17);
            nexoProNO   = byBuf.mid(89,2);
            torqueValue = byBuf.mid(180,7);                           //torque value
            torqueValue = QByteArray::number(torqueValue.toDouble());
            doubleAngle = byBuf.mid(189,7).toDouble() + 0.5;
            intAngle    = doubleAngle;
            angleValue  = QByteArray::number(intAngle,10);
            torqueMax   = byBuf.mid(198,7);                             //torque high limit
            torqueMin   = byBuf.mid(207,7);                             //torque low limit
            tighenTime  = byBuf.mid(68,19);                           //Torque time
            tighteningID  = byBuf.mid(30,10);                           //Tightening ID
            intReadTighteningID = tighteningID.toInt(&okFlag,10);

//            emit coming_VIN(vinBuf);
            if(vinBuf == "                 " || vinBuf.contains(" "))
            {
                DTdebug() << "vin is empty" << vinBuf ;
            }
            else {
                VIN_PIN_SQL = vinBuf ;
            }


            if(intReadTighteningID == 0)
            {
                DTdebug() << "data error:intReadTighteningID = 0";
                return ;
            }

            DTdebug() << currentBolt  ;


            if(nexoResult == "NOK")
            {
                DTdebug() << "last is NOK" ;
            }
            else if(nexoResult == "OK")
            {
                if(chooseCarType && (currentBolt < boltAmount))
                {
                    currentBolt++ ;
                    DTdebug() << currentBolt ;
                }
                else if (chooseCarType && (currentBolt == boltAmount) )
                {
                    currentBolt = 0 ;
                    chooseCarType = false ;
                    DTdebug() << currentBolt ;
                }
                else if (!chooseCarType)
                {
                    currentBolt ++ ;
                    DTdebug() << currentBolt ;
                }
            }
            else {
                DTdebug() << nexoResult ;
            }

            if (currentBolt==0)
            {
                currentBolt++ ;

            }

            if(byBuf[171] == '0')
            {
                nexoResult = "NOK";   //0=tightening NOK ,1=OK
            }
            else
            {
                nexoResult = "OK" ; 
            }

            DTdebug()  <<currentBolt <<tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime<<torqueMax<<torqueMin;
#if 1
//            DATA_STRUCT demo;
            demo.data_model[0] = QString(tighenTime.mid(0,10));
            demo.data_model[1] = QString(tighenTime.mid(11,8));
            demo.data_model[2] = QString(nexoResult);
            demo.data_model[3] = QString(torqueValue);
            demo.data_model[4] = QString(angleValue);
#if 0
            demo.data_model[5] = QString(boltNumberBuf);
#else
            demo.data_model[5] = QString::number(currentBolt);
#endif
            demo.data_model[6] = QString(VIN_PIN_SQL);
            demo.data_model[7] = QString(tighteningID);
            demo.data_model[8] = "null";
            demo.data_model[9] = QString(nexoProNO);

//            QVariant DataVar;
            DataVar.setValue(demo);
#endif
            emit sendfromOpThread(DataVar);
            if(currentBolt==boltAmount)
            {
                VIN_PIN_SQL =="" ;
            }
#if 0
            if(sendDisableFlag)
            {
                sendDisableFlag = false;
                sendNextEnableFlag = true;           //receive disable cmd ACK
            }
            else
            {
#endif
                DTdebug() << "send next program";
                emit send_mainwindow(VIN_PIN_SQL.toUtf8(),QString(torqueValue),QString(angleValue),QString(nexoResult),currentBolt);
//            }
            break;

        default:
            break;

        }
    }
    else if(curveDataFlag == 1)
    {
        DTdebug() <<curveFirstFlag << curveDataFlag << "recevice len:" << revLen << cmdLen ;
        if(curveFirstFlag == 0)
        {
            curveBuf = curveBuf + byBuf;
            revDataLen = revDataLen + revLen;
            DTdebug() << "++++++++:" << curveBuf.size() << revDataLen << revFinishLen;
        }
        if(revDataLen >= (revFinishLen<<1))//curve receive finished
        {
            for(k=0;k<2;k++)
            {
                TempLen = 57;
                PID_Len = curveBuf.mid(TempLen,3).toInt();
                TempLen = TempLen + 12 + PID_Len + 5;
                PID_Len = curveBuf.mid(TempLen,3).toInt();
                TempLen = TempLen + 12 + PID_Len + 17;
                curveType = curveBuf.mid(TempLen+1,2).data();
                DTdebug()<< "******curveType*********" <<curveType << tighteningID; //01=angle 02=torque
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
                if(curveType == "01")//1 times angle
                {
                    DTdebug()<< "**curve angle**";
                    curveBufTemp ="*PM";
                    curveBufTemp.append(curveBuf.mid(0,nullCount));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('|');
                }
                else if(curveType == "02") //2 times torque
                {
                    DTdebug()<< "**curve angle**";
                    curveBufTemp.append(curveBuf.mid(0,nullCount));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('}');
                    if(tighteningID != "")
                    {
                        QString fileName = "/curveFile/" + tighteningID + ".txt";
                        QFile file(fileName);
                        if(!file.open(QIODevice::ReadWrite))
                        {
                            DTdebug()   << "Cannot open wifi file2 for Writing";
                        }
                        else
                        {
                            file.write(curveBufTemp.toUtf8());
                            file.close();
                        }
                        curveDataFlag = 0;
                        DTdebug() << "22222 curve file path:" <<fileName;
                        tighteningID = "";
                    }
                    else
                    {
                        DTdebug() << "tighteningID == null:";
                    }
                }
                else
                {}
                if(k==1)
                {
                    curveBuf = curveBuf.mid(revFinishLen,revFinishLen);
                }
            }
            nexoHandleFlag |= 0x00002000;                 //results curve ACK
            ctlNexoHandle();                              //send CMD to nexo
            revDataLen   = 0;
            revFinishLen = 0;
            curveFirstFlag = 0;
        }
        else if(revDataLen >= revFinishLen) //curve receive finished
        {
            TempLen = 57;
            PID_Len = curveBuf.mid(TempLen,3).toInt();
            TempLen = TempLen + 12 + PID_Len + 5;
            PID_Len = curveBuf.mid(TempLen,3).toInt();
            TempLen = TempLen + 12 + PID_Len + 17;
            curveType = curveBuf.mid(TempLen+1,2).data();
            DTdebug()<< "******curveType*********" <<curveType << tighteningID; //01=angle 02=torque
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
            if(curveType == "01")//1 times angle
            {
                DTdebug()<< "**curve angle**";
                curveBufTemp ="*PM";
                curveBufTemp.append(curveBuf.mid(0,nullCount));
                curveBufTemp.append(hexBuf);
                curveBufTemp.append('|');
            }
            else if(curveType == "02") //2 times torque
            {
                DTdebug()<< "**curve torque**";
                curveBufTemp.append(curveBuf.mid(0,nullCount));
                curveBufTemp.append(hexBuf);
                curveBufTemp.append('}');
                if(tighteningID != "")
                {
                    QString fileName = "/curveFile/" + tighteningID + ".txt";
                    QFile file(fileName);
                    if(!file.open(QIODevice::ReadWrite))
                    {
                        DTdebug()   << "Cannot open wifi file2 for Writing";
                    }
                    else
                    {
                        file.write(curveBufTemp.toUtf8());
                        file.close();
                    }
                    curveDataFlag = 0;
                    DTdebug() << "curve file path:" <<fileName;
                    tighteningID = "";
                }
                else
                {
                    DTdebug() << "tighteningID == null:";
                }
            }
            else
            {}
            if(revDataLen > revFinishLen + 21)//more then 1460
            {
                revDataLen = revDataLen - revFinishLen;
                curveBuf   = curveBuf.mid(revFinishLen,revDataLen);
                DTdebug() << "&&&& more then 1460&&&&" << revDataLen;
                return;
            }
            else if(revDataLen > revFinishLen)
            {
                DTdebug() << "&&&& ++++++ &&&&&&" << curveBuf.mid(revFinishLen , revDataLen-revFinishLen);
            }
            else
            {
                DTdebug() << "&& curve receive OK&&&&&";
            }
            nexoHandleFlag |= 0x00002000;                 //results curve ACK
            ctlNexoHandle();                              //send CMD to nexo
            revDataLen   = 0;
            revFinishLen = 0;
            curveFirstFlag = 0;
        }
        else
        {
            DTdebug() << "data length" << revDataLen << revFinishLen;
            curveFirstFlag = 0;
        }
    }
    else
    {
        DTdebug() << "data error";
    }

}

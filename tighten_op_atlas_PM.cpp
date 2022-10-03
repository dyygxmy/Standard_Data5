/**********************************************************************
 * file name  tighten_op_atlas_PM.c
 * describe   ：control enable
 *              Last tightening result data subscribe
 *
***********************************************************************/
#include "tighten_op_atlas_PM.h"
//#define  ATLAS_PF  //select atlas PF control

TightenOpAtalsPM::TightenOpAtalsPM(QObject *parent) :
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
    sendNextEnableFlag = false;
    sendDisableFlag = false;
    tighteningID = "";
    Factory = factory;
    curveNullBuf = "Curve is null";
    angleSubFlag = 0;

    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpAtalsPM::tightenStart()
{
    system("echo 1 > /sys/class/leds/OUTC1/brightness"); //PM cycle stop enable
    DTdebug() << "tighten_op_atlas_PM thread start Tool Tightening stop = 1";
    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    newConnects();
    timer5000ms->start(5000);
}
//使能
void TightenOpAtalsPM::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
        if(n == 99)
        {
            system("echo 0 > /sys/class/leds/OUTC1/brightness"); //PM cycle stop disable
        }
        else
        {
            system("echo 1 > /sys/class/leds/OUTC1/brightness"); //PM cycle stop enable
        }
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
            sendVinFlag = 1;
        }
        if(nexoLinkOk)
        {
            system("echo 0 > /sys/class/leds/OUTC1/brightness"); //PM cycle stop disable
            revDataLen   = 0;
            revFinishLen = 0;
            curveBuf = "";
            curveFirstFlag = 0;
            curveDataFlag = 0;   //receive curve
            idCode = "";
            curveBufTemp ="*PM";
        }
        else
        {}
        vinBuf = VIN_PIN_SQL.toLatin1();
    }
}

/**************************************/
//nexo alive
/**************************************/
void TightenOpAtalsPM::timerFunc5000ms()
{
    DTdebug() << "op 5000ms tiemr" << nexoLinkOk << nexoHandleFlag << nexoAliveCount << nexoHandleFlag<< SYSS;
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
            DTdebug() << "3 times alive no ack disconnect  ";
            newConnects();
        }
    }
    nexoAliveCount++;
}

/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void TightenOpAtalsPM::disConnectDO()
{
    nexoLinkOk = false;
    DTdebug() << "OP disconnect 22222222222";
    newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpAtalsPM::newConnects()
{
    tcpSocket->abort(); //取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(ControllerIp_1,4545);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    DTdebug() << "PM the socket state is" << ControllerIp_1 << tcpSocket->state();
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DTdebug() << "OP link success  >>>>>>>>";
        nexoAliveCount = 0;
        timer5000ms->stop();
        nexoHandleFlag  = 0x00000001;  //start communication
        ctlNexoHandle();               //send CMD to nexo
        timer5000ms->start(5000);
    }
    else
    {
        nexoLinkOk = false;
        DTdebug() << "OP link fail XXXXXXXXXXXX" ;
    }
}

/***********************************/
//处理控制Nexo函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void TightenOpAtalsPM::ctlNexoHandle()
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
    /****************************************
    *******Atlas PF results data ACK********
    *******MID 0062 ************************
    ****************************************
    *******Atlas PM results data ACK********
    *******MID 0108 ************************
    ****************************************/
    else if(nexoHandleFlag&0x02000000)
    {
        nexoHandleFlag &= ~0x02000000;
#ifdef ATLAS_PF
        Tx_len = 21;
        sendBuf = "00200062001         ";
#else
        Tx_len = 22;
        sendBuf = "00210108            0";
#endif
    }
    /****************************************
     *******Alarm  ACK**********
     ****************************************/
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
    /******* MID 0060 ***********************/
    /****************************************/
    /*Atlas PM Last tightening results data subscribe*/
    /******* MID 0105 *******/
    /****************************************/
    else if(nexoHandleFlag&0x01000000)
    {       
#ifdef ATLAS_PF
        Tx_len = 21;
        sendBuf = "002000600011        ";
#else
        Tx_len = 21;
        sendBuf = "00200105            ";
#endif
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
//        if(!readMaxTightenIDFlag)
//        {
//            QByteArray tempBuf = QByteArray::number(intReadTighteningID,10);
//            sendBuf.replace(30-tempBuf.size(),tempBuf.size(),tempBuf);
//        }
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
    /**********       Alive       ***********/
    /****************************************/
    else if(nexoHandleFlag&0x00800000)//5S send Alive
    {
        nexoHandleFlag &= ~0x00800000;
        Tx_len = 21;
        sendBuf = "00209999001         ";
    }

 //   if(sendBuf.mid(4,4) != "9999")
       DTdebug() << "send      :" << sendBuf;// << "nexoHandleFlag :" << nexoHandleFlag ;
    tcpSocket->write(sendBuf,Tx_len);
    tcpSocket->waitForBytesWritten(300);

}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void TightenOpAtalsPM::revNexoData()
{   
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
    double doubleTorque = 0.0 ;
    int intAngle = 0;
    int intTorque = 0 ;
    QByteArray recBuf,hexBuf;//,hexBufTemp;

    recBuf.clear();
    recBuf = tcpSocket->readAll();
    nexoMID = recBuf.mid(4,4).toInt();
    revLen = recBuf.size();
    cmdLen = recBuf.mid(0,4).toInt();
    if(nexoMID == 900)
    {
        DTdebug() << "note:" << recBuf;
        revDataLen   = revLen;
        revFinishLen = cmdLen;
        curveBuf = recBuf;
        curveFirstFlag = 1;
        curveDataFlag = 1;   //receive curve
        idCode = "";
    }
    else
    {}
    //        DTdebug() << "/********************************************************************/";

    if(revLen == (cmdLen+1))              //length of the same
    {
        DTdebug() <<curveFirstFlag << curveDataFlag << "recevice len:" << revLen << cmdLen << "note:" << recBuf;
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
                DTdebug() << "nexo cmd error:" << cmdType << errCode;
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
            case 8:
                curveNullBuf = "null";
                if(angleSubFlag)
                {
                    angleSubFlag = 0;
                    timer5000ms->stop();
                    nexoHandleFlag = 0x00004000;   //torque subscribe
                    ctlNexoHandle();               //send CMD to nexo
                    timer5000ms->start(5000);
                }
                else
                {                   
                    nexoHandleFlag &= ~0x00004000;   //torque subscribe
                }
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
                    DTdebug() << " send next program";
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNumCh[0]);
                }
                break;
            case 43:    //tool enable
                nexoHandleFlag &= ~0x00000400;
                break;
            case 50:    //vin download
                timer5000ms->stop();
//                nexoHandleFlag = 0x00000002;//select program
                nexoHandleFlag = 0x00000400;//tool enable
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
            case 82:    //set time
                timer5000ms->stop();
                nexoHandleFlag = 0x00008000;     //angle subscribe
                ctlNexoHandle();                 //send CMD to nexo
                timer5000ms->start(5000);
                break;
            case 105:
                timer5000ms->stop();
                nexoHandleFlag = 0x00008000;     //angle subscribe
                ctlNexoHandle();                 //send CMD to nexo
                timer5000ms->start(5000);
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
            timer5000ms->stop();
//            nexoHandleFlag =  0x40000000;  //set time
            nexoHandleFlag = 0x01000000;//tightening results data subscription
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
            break;
        case 9999://应答不显示在PC 测试使用
            break;
        case 106://PM Rev result data
            timer5000ms->stop();
            nexoHandleFlag = 0x02000000;//result ACK
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);
            if(SYSS == "ING")
            {
                nexoProNO    = recBuf.mid(89,2);
#if 0
                //会出现解析为0
                torqueValue  = recBuf.mid(180,7);                           //torque value
                torqueValue = QByteArray::number(torqueValue.toDouble());
#else
                doubleTorque = recBuf.mid(180,7).toDouble() ;
                intTorque    = doubleTorque ;
                torqueValue  = QByteArray::number(intTorque,10) ;
#endif
//                angleValue   = recBuf.mid(189,7);                           //angle value
                doubleAngle = recBuf.mid(189,7).toDouble() + 0.5;
                intAngle = doubleAngle;
                angleValue = QByteArray::number(intAngle,10);
//                if(angleValue == "       " )
//                    angleValue = "0";

#if 0
                torqueValue = recBuf.mid(180+addr,7);                      //torque value
                intTorque   = torqueValue.toDouble() ;
                torqueValue = QByteArray::number(intTorque,10);

                angleValue = recBuf.mid(189+addr,7);
                intAngle    = angleValue.toDouble()+0.5;
                angleValue  = QByteArray::number(intAngle,10);             //angle value
#endif

                torqueMax = recBuf.mid(198,7);                             //torque high limit
                torqueMin = recBuf.mid(207,7);                             //torque low limit
                tighenTime   = recBuf.mid(68,19);                           //Torque time
                tighteningID  = recBuf.mid(30,10);                           //Tightening ID
                intReadTighteningID = tighteningID.toInt(&okFlag,10);
                if(intReadTighteningID == 0)
                {
                    DTdebug() << "data error ***********intReadTighteningID = 0****************";
                    return ;
                }
                if(recBuf[171] == '0')
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
                        DTdebug() << "*******" << nexoProNO << programNO ;
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
                if(boltCount == 0)                                 //ok count finished
                {
                    system("echo 1 > /sys/class/leds/OUTC1/brightness"); //PM cycle stop enable
                }
                DTdebug()  << tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime<<torqueMax<<torqueMin;
                DATA_STRUCT demo;
                demo.data_model[0] = QString(tighenTime.mid(0,10));
                demo.data_model[1] = QString(tighenTime.mid(11,8));
                demo.data_model[2] = QString(nexoResult);
                demo.data_model[3] = QString(torqueValue);
                demo.data_model[4] = QString(angleValue);
                demo.data_model[5] = QString(boltNumberBuf);
                demo.data_model[6] = QString(vinBuf);
                demo.data_model[7] = QString(tighteningID);
                demo.data_model[8] = "null";
                demo.data_model[9] = QString(nexoProNO);
                if(Factory =="BYDSZ" ||Factory=="BYDXA")
                {
                    demo.data_model[10] = "0";
                    demo.data_model[11] = AutoNO;
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
                    demo.data_model[15] = "1";
                }
                else if(Factory =="Dongfeng")
                {
                    demo.data_model[10] = "1";
                    if(manualMode)
                        demo.data_model[11] = "manual";
                    else
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
                    DTdebug() << "1111111111111111 send next program";
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNumCh[0]);
                }
            }
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
            curveBuf = curveBuf + recBuf;
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
            timer5000ms->stop();
            nexoHandleFlag |= 0x00002000;                 //results curve ACK
            ctlNexoHandle();                              //send CMD to nexo
            timer5000ms->start(5000);
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
                    DTdebug() << "11111curve file path:" <<fileName;
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
            timer5000ms->stop();
            nexoHandleFlag |= 0x00002000;                 //results curve ACK
            ctlNexoHandle();                              //send CMD to nexo
            timer5000ms->start(5000);
            revDataLen   = 0;
            revFinishLen = 0;
            curveFirstFlag = 0;
        }
        else
        {
            DTdebug() << "88888888888 data length" << revDataLen << revFinishLen;
            curveFirstFlag = 0;
        }
    }
    else
    {
        DTdebug() << "9999999999 data error"<< revLen << cmdLen << "note:" << recBuf;
    }

}










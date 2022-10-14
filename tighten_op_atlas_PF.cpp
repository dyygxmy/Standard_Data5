/**********************************************************************
 * file name  tighten_op_atlas_PF.c
 * describe   ：control enable
 *              Last tightening result data subscribe
 *
***********************************************************************/
#include "tighten_op_atlas_PF.h"

TightenOpAtalsPF::TightenOpAtalsPF(QObject *parent) :
    QObject(parent)
{
    isRepair = false;
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
    curveDataFlag = 0;
    sendNextEnableFlag = false;
    sendDisableFlag = false;
    Factory = factory;
    vari1 = "";
    oldVinCode = "";
    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpAtalsPF::tightenStart()
{
    DTdebug() << "tighten_op_atlas_PF thread start!!!"<<vari1;

    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    newConnects();
    timer5000ms->start(5000);
    system("echo 0 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 0  P1
    system("echo 0 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 0  P2
    system("echo 0 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 0  P3
    system("echo 1 > /sys/class/leds/OUTD5/brightness"); //Cleco Input4 0  P4
}
//使能
void TightenOpAtalsPF::sendReadOperate(bool enable,int n)
{
    FUNC() ;
    if(!enable) //
    {
        if(vari1 == "D")
        {
            return;
        }
        else if(vari1 == "IO")
        {
            system("echo 0 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 0  P1
            system("echo 0 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 0  P2
            system("echo 0 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 0  P3
        }
        else
        {
            timer5000ms->stop();
            nexoHandleFlag = 0x00000200;//nexo disable
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
        }
    }
    else
    {
        DTdebug() << "" << enable << n << vari1;
        Channel = "0";
        programNO = carInfor[0].proNo[n].toShort();// proNum.toShort();
        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        boltNokCount = 3;
        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
        vinBuf = VIN_PIN_SQL.toLatin1();
        if(vari1 == "IO")
        {
            if(programNO > 0)
            {
                if(programNO & 0x01)//
                {
                    system("echo 1 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 1  P1
                }
                else
                {
                    system("echo 0 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 0
                }
                if(programNO & 0x02)
                {
                    system("echo 1 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 1  P2
                }
                else
                {
                    system("echo 0 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 0
                }
                if(programNO & 0x04)
                {
                    system("echo 1 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 1  P3
                }
                else
                {
                    system("echo 0 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 0
                }
            }
        }
        else if(vari1 == "N")
        {
            if(nexoLinkOk)
            {
                timer5000ms->stop();
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();            //send CMD to nexo
                timer5000ms->start(5000);
            }
            DTdebug()<<" send enable "<<programNO;
            if(programNO > 0)
            {
                if(programNO & 0x01)//
                {
                    system("echo 1 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 1  P1
                }
                else
                {
                    system("echo 0 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 0
                }
                if(programNO & 0x02)
                {
                    system("echo 1 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 1  P2
                }
                else
                {
                    system("echo 0 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 0
                }
                if(programNO & 0x04)
                {
                    system("echo 1 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 1  P3
                }
                else
                {
                    system("echo 0 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 0
                }
            }
        }
        else if(vari1 == "P")
        {
            if(nexoLinkOk)
            {
                timer5000ms->stop();
                nexoHandleFlag = 0x00000002;//select program
                ctlNexoHandle();            //send CMD to nexo
                timer5000ms->start(5000);
            }
            DTdebug()<<"11111111111111 send enable 111111111111111111"<<programNO;
        }
        else//D
        {}
    }
}

/**************************************/
//nexo alive
/**************************************/
void TightenOpAtalsPF::timerFunc5000ms()
{
#if 1
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
#if 1
            emit IsTigntenReady(false);
#else
            emit IsTigntenReady(true);
#endif
            nexoLinkOk = false;
        }
        else
        {
            DTdebug() << "3 times alive no ack disconnect  ";
            newConnects();
        }
    }
    nexoAliveCount++;

#else
    emit IsTigntenReady(true);  //test
#endif
}

/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void TightenOpAtalsPF::disConnectDO()
{
//    nexoLinkOk = false;
    DTdebug() << "OP disconnect 22222222222";
//    newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpAtalsPF::newConnects()
{
    tcpSocket->abort(); //取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(ControllerIp_1,4545);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    DTdebug() << "PF the socket state is" << ControllerIp_1 << tcpSocket->state();
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
void TightenOpAtalsPF::ctlNexoHandle()
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
    /*******Atlas PF results data ACK********/
    /*******MID 0062 ************************/
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
    /******* MID 0060 ***********************/
    /****************************************/
    else if(nexoHandleFlag&0x01000000)
    {
        Tx_len = 21;
        sendBuf = "002000600011        ";
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
    /*********      VIN subscribe        **********/
    /****************************************/
    else if(nexoHandleFlag&0x00020000)
    {
        Tx_len = 21;
        sendBuf = "00200051001         ";
    }
    /****************************************/
    /*********      VIN ACK        **********/
    /****************************************/
    else if(nexoHandleFlag&0x00040000)
    {
        nexoHandleFlag &= ~0x00040000;
        Tx_len = 21;
        sendBuf = "00200053001         ";
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
    }         //send CMD to n
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
void TightenOpAtalsPF::revNexoData()
{
    bool okFlag;
    int nexoMID;
    int cmdType;
    int errCode;
    int revLen,cmdLen;
    double torqueMax = 0.0;
    double torqueMin = 0.0;
    QByteArray recBuf,vinCode = "";

    recBuf.clear();
    recBuf = tcpSocket->readAll();
    nexoMID = recBuf.mid(4,4).toInt();
    revLen = recBuf.size();
    cmdLen = recBuf.mid(0,4).toInt();
    //    if(nexoMID != 9999)
    //    {
    DTdebug() << "recevice len:" << revLen << cmdLen << "note:" << recBuf;
    //        DTdebug() << "/********************************************************************/";
    //    }
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
                    DTdebug() << "22222222222222 send next program";
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(nexoResult),0,groupNumCh[0]);
                }
                break;
            case 43:    //tool enable
                nexoHandleFlag &= ~0x00000400;
                break;
            case 50:    //vin download
                timer5000ms->stop();
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
                timer5000ms->start(5000);
                break;
            case 51:   //vin subscribe
                nexoHandleFlag &= ~0x00020000;//vin subscribe
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
                if(vari1 == "IO")
                {
                    nexoHandleFlag = 0x00000400;//tool enable
                }
                else
                {
                    timer5000ms->stop();
                    if(vari1 == "D")
                    {
                        nexoHandleFlag = 0x00020000;//vin subscribe
                    }
                    else
                    {
                        if(SYSS == "ING")
                        {
                            nexoHandleFlag = 0x00000400;//tool enable
                        }
                        else
                        {
                            nexoHandleFlag = 0x00000200;  //nexo disable
                        }
                    }
                    ctlNexoHandle();                 //send CMD to nexo
                    timer5000ms->start(5000);
                }
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
            //            nexoHandleFlag = 0x04000000; //subscription old tightening
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
            break;
        case 9999://应答不显示在PC 测试使用
            break;
        case 52:   //vin ack
            timer5000ms->stop();
            nexoHandleFlag = 0x00040000; //VIN ACK
            ctlNexoHandle();
            timer5000ms->start(5000);
            vinCode = recBuf.mid(20,17);
            if(vinCode.mid(0,3) == "LSV")
            {
                if( (vinCode == oldVinCode)&&(SYSS == "ING") )
                {
                    DTdebug()<<"vincode repeat "<<vinCode;
                }
                else
                {
                    emit sendVinToMain(vinCode);
                }
            }
            else
            {
                DTdebug()<<"error vincode"<<vinCode;
            }
            break;

        case 65:
            break;
        case 61://PF Rev result data
            timer5000ms->stop();
            nexoHandleFlag = 0x02000000;//result ACK
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);
            if(SYSS == "ING")
            {
                nexoProNO = recBuf.mid(90,3);
                torqueValue = recBuf.mid(140,6);                           //torque value
                torqueValue = QByteArray::number(torqueValue.toDouble()/100);
                angleValue  = recBuf.mid(169,5);                           //angle value
                angleValue = QByteArray::number(angleValue.toDouble());
                tighenTime   = recBuf.mid(176,19);                          //Torque time
                tighteningID = recBuf.mid(221,10);                           //Tightening ID
                intReadTighteningID = tighteningID.toInt(&okFlag,10);
                if(intReadTighteningID == 0)
                {
                    DTdebug() << "data error ***********intReadTighteningID = 0****************";
                    return ;
                }
                if(recBuf[107] == '0')
                {
                    nexoResult = "NOK";   //0=tightening NOK ,1=OK
                }
                else
                {
                    nexoResult = "OK" ;
                    if(vari1 == "D")
                    {
                        if(nexoProNO.toInt() != programNO)
                        {
                            nexoResult = "NOK" ;
                        }
                    }
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
                DTdebug()  << tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime;
                DATA_STRUCT demo;
                demo.data_model[0] = QString(tighenTime.mid(0,10));
                demo.data_model[1] = QString(tighenTime.mid(11,8));
                demo.data_model[2] = nexoResult;
                demo.data_model[3] = QString(torqueValue);
                demo.data_model[4] = QString(angleValue);
                demo.data_model[5] = QString(boltNumberBuf);
                demo.data_model[6] = QString(vinBuf);
                demo.data_model[7] = QString(tighteningID);
                demo.data_model[8] = "Curve is null";
                demo.data_model[9] = QString(nexoProNO);
                if(Factory =="BYDSZ" ||Factory=="BYDXA")
                {
                    demo.data_model[10] = "0";        //JobStatus
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
                    demo.data_model[15] = "1";       //Channel
                }
                else if((Factory == "SVW2")||(Factory == "Haima"))
                {
                    demo.data_model[10] = "1";       //Channel
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
                    demo.data_model[15] = "0";          //JobStatus
                    if(isRepair)
                    {
                        demo.data_model[16] = Line_ID;
                        demo.data_model[17] = Station;
                        demo.data_model[18] = WirelessIp;
                    }
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
                    demo.data_model[13] = QString::number(torqueMax);
                    demo.data_model[14] = QString::number(torqueMin);
                    demo.data_model[15] = "0";
                }
                else if(Factory =="BAIC")
                {
                    demo.data_model[10] = "1";
                    demo.data_model[11] = "NULL";
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
                    demo.data_model[16] = "0";
                    demo.data_model[17] = "0";
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
        DTdebug() << "9999999999 data error"<< revLen << cmdLen << "note:" << recBuf;
    }

}

void TightenOpAtalsPF::setRepair(bool temp)
{
    isRepair = temp;
}

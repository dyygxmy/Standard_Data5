/**********************************************************************
 * file name  tighten_op_cleco.c
  describe   ：control enable / program
 *             Last tightening result data subscribe
 *
***********************************************************************/
#include "tighten_op_cleco.h"

TightenOpCleco::TightenOpCleco(QObject *parent) :
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
    curveDataFlag = 0;
    sendVinFlag = 0;
    sendNextEnableFlag = false;
    sendDisableFlag = false;
    Factory = factory;
    vari_1 = "";

    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpCleco::tightenStart()
{
    DTdebug() << "tighten_op_cleco thread start!!!"<<vari_1;

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
    system("echo 0 > /sys/class/leds/OUTC1/brightness"); //disable
}
//使能
void TightenOpCleco::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
        if(vari_1 == "N" || vari_1 == "master"  || vari_1 == "slave")
        {
            timer5000ms->stop();
            if(n == 99)
            {
                nexoHandleFlag = 0x00000400;//tool enable
            }
            else
            {
                nexoHandleFlag = 0x00000200;//nexo disable
            }
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
        }
        system("echo 0 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 0  P1
        system("echo 0 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 0  P2
        system("echo 0 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 0  P3
        system("echo 0 > /sys/class/leds/OUTC1/brightness"); //disable
    }
    else
    {
        DTdebug() << vari_1 ;
        Channel = "1";
        programNO = carInfor[0].proNo[n].toShort();// proNum.toShort();
        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        boltNokCount = 3;
        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
        if(VIN_PIN_SQL.toLatin1() == vinBuf)
        {}
#if 0
        //        else
        //        {
        //            vinBuf = VIN_PIN_SQL.toLatin1();
        //            timer5000ms->stop();
        //            nexoHandleFlag = 0x00010000;//vin
        ////            nexoHandleFlag = 0x00000400;//tool enable
        //            ctlNexoHandle();            //send CMD to nexo
        //            timer5000ms->start(5000);;
        //        }
#else
        else
        {
            vinBuf = VIN_PIN_SQL.toLatin1();
        }
#endif
        if(vari_1 == "N")
        {
            timer5000ms->stop();
            nexoHandleFlag = 0x00000400;//tool enable
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);
        }
        else if(vari_1 == "master" || vari_1 == "slave")
        {
            DTdebug() << "master ;" ;
            timer5000ms->stop();
            nexoHandleFlag = 0x00010000;//vin
            ctlNexoHandle();            //send CMD to nexo
            timer5000ms->start(5000);;
        }
        if(nexoLinkOk)
        {
            if(programNO > 0)
            {
                system("echo 1 > /sys/class/leds/OUTC1/brightness"); //enable
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
        else
        {}
    }
}

/**************************************/
//nexo alive
/**************************************/
void TightenOpCleco::timerFunc5000ms()
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
void TightenOpCleco::disConnectDO()
{
    nexoLinkOk = false;
    DTdebug() << "OP disconnect 22222222222";
    newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpCleco::newConnects()
{
    tcpSocket->abort(); //取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(ControllerIp_1,4545);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
    DTdebug() << "CLECO the socket state is" << ControllerIp_1 << tcpSocket->state();
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
void TightenOpCleco::ctlNexoHandle()
{
    FUNC() ;
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
    /*******cleco results data ACK********/
    /*******MID 0062 ************************/
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
        DTdebug() << "set bolt number" ;
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
        DTdebug() << "send vin" ;
        Tx_len = 38;
        if(vinBuf.size() == 14)
        {
            vinBuf.append("***");
        }
        sendBuf = "00370050001         LSVAE45E7EN123***";
        sendBuf.replace(20,vinBuf.size(),vinBuf); //replace vincode
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
        DTdebug() << "enable" ;
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
void TightenOpCleco::revNexoData()
{
    FUNC() ;
    bool okFlag;
    int nexoMID;
    int cmdType;
    int errCode;
    int revLen,cmdLen;
    double torqueMax = 0.0;
    double torqueMin = 0.0;
    QByteArray recBuf;

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
    //#endif
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
            DTdebug() << cmdType ;
            errCount = 0;
            switch(cmdType)
            {
            //            case 3:
            //                nexoLinkOk = false;                //communication stop OK
            //                timer5000ms->stop();
            //                nexoHandleFlag  = 0x00000001;  //start communication
            //                ctlNexoHandle();               //send CMD to nexo
            //                timer5000ms->start(5000);
            //                break;
            case 18:     //select program
                timer5000ms->stop();
                nexoHandleFlag = 0x00000004;//set bolt number
                ctlNexoHandle();             //send CMD to nexo
                timer5000ms->start(5000);
                break;
            case 19:     //set bolt number
                //                nexoHandleFlag &= ~0x00000004;
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
                if(vari_1 == "N" || vari_1 == "master"  || vari_1 == "slave")
                {
                    timer5000ms->stop();
                    if(SYSS == "ING")
                    {
                        nexoHandleFlag = 0x00000400;//tool enable
                    }
                    else
                    {
                        nexoHandleFlag = 0x00000200;  //nexo disable
                    }
                    ctlNexoHandle();                  //send CMD to nexo
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
            ctlNexoHandle();             //send CMD to nexo
            timer5000ms->start(5000);
            break;
        case 9999://应答不显示在PC 测试使用
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
                if(nexoProNO != "099")       //99 program not count
                {
                    if(recBuf[107] == '0')
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
                            DTdebug() << "*******" << nexoProNO << programNO << nexoProNO.toInt() ;
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
                DATA_STRUCT demo;
                demo.data_model[0] = QString(tighenTime.mid(0,10));
                demo.data_model[1] = QString(tighenTime.mid(11,8));
                demo.data_model[2] = QString(nexoResult);
                demo.data_model[3] = QString(torqueValue);
                demo.data_model[4] = QString(angleValue);
                demo.data_model[5] = QString(boltNumberBuf);
                demo.data_model[6] = QString(vinBuf);
                demo.data_model[7] = QString(tighteningID);
                demo.data_model[8] = "Curve is null";
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
                else
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










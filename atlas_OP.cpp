/**********************************************************************
 * file name  tighten_op_atlas_PF.c
 * describe   ：control enable
 *              Last tightening result data subscribe
 *
***********************************************************************/
#include "atlas_OP.h"

AtlasOP::AtlasOP(QObject *parent) :
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
    Channel = "1";
    vari1 = "";
    ppFlag = false;
    oldVinCode = "NULL";
    timerNum = 5000;
    isVin = true ;
    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void AtlasOP::tightenStart()
{
    DTdebug() << "tighten_op_atlas_PF thread start!!!"<<vari1;

//    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
//    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    newConnects();
//    timer5000ms->start(timerNum);
    QTimer::singleShot(timerNum,this,SLOT(timerFunc5000ms()));
}

/**************************************/
//nexo alive
/**************************************/
void AtlasOP::timerFunc5000ms()
{
    DTdebug() << "op 5000ms tiemr" << nexoLinkOk << nexoHandleFlag << nexoAliveCount;
    if(nexoLinkOk)                   //nexo link
    {
        nexoHandleFlag |= 0x00800000;//Alive
        ctlNexoHandle();             //send CMD to nexo
    }
    else
    {}
    if(nexoAliveCount > 2)             //3 times alive not ack
    {
        if(nexoLinkOk )
        {
            emit IsTigntenReady(false);
            nexoLinkOk = false;
        }
        else
        {
            nexoAliveCount = 0;
            DTdebug() << "3 times alive no ack disconnect  ";
            newConnects();
        }
    }
    nexoAliveCount++;
    QTimer::singleShot(timerNum,this,SLOT(timerFunc5000ms()));
}

/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void AtlasOP::disConnectDO()
{
    DTdebug() << "OP disconnect 22222222222";
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void AtlasOP::newConnects()
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
        nexoHandleFlag  = 0x00000001;  //start communication
        ctlNexoHandle();               //send CMD to nexo
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
void AtlasOP::ctlNexoHandle()
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
    tcpSocket->waitForBytesWritten(2000);

}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void AtlasOP::revNexoData()
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
    QString strDataBegin = recBuf.mid(59,1) ;
    DTdebug() << "recevice len:" << revLen << cmdLen << "note:" << recBuf;
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
//                timer5000ms->stop();
                ctlNexoHandle();             //send CMD to nexo
//                timer5000ms->start(timerNum);
            }
            break;
        case 5:  //cmd send ok
            cmdType = recBuf.mid(20,4).toInt();
            errCount = 0;
            switch(cmdType)
            {
            case 18:     //select program
//                timer5000ms->stop();
                nexoHandleFlag = 0x00000004;//set bolt number
                ctlNexoHandle();             //send CMD to nexo
//                timer5000ms->start(timerNum);
                break;
            case 19:     //set bolt number
//                timer5000ms->stop();
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
//                timer5000ms->start(timerNum);
                break;
            case 42:    //tool disable
                nexoHandleFlag &= ~0x00000200;
                break;
            case 43:    //tool enable
                nexoHandleFlag &= ~0x00000400;
                break;
            case 50:    //vin download
//                timer5000ms->stop();
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
//                timer5000ms->start(timerNum);
                break;
            case 51:   //vin subscribe
                nexoHandleFlag &= ~0x00020000;//vin subscribe
                break;
            case 60:    //result subscribe
//                timer5000ms->stop();
                nexoHandleFlag = 0x40000000;//time set
                ctlNexoHandle();             //send CMD to nexo
//                timer5000ms->start(timerNum);
                break;
            case 64:    //read old result
                nexoHandleFlag &= ~0x04000000;
                break;
            case 70:    //alarm subscribe
                break;
            case 82:    //set time
                nexoHandleFlag &= ~0x40000000;//time set
                break;
            default:
                break;
            }
            break;
        case 2:
            nexoLinkOk = true;                //communication start OK
            emit IsTigntenReady(true);
//            timer5000ms->stop();
            nexoHandleFlag = 0x01000000;//tightening results data subscription
            ctlNexoHandle();             //send CMD to nexo
//            timer5000ms->start(timerNum);
            break;
        case 9999://应答不显示在PC 测试使用
            break;
        case 52:   //vin ack
            break;
        case 65:
            break;
        case 61://PF Rev result data
//            timer5000ms->stop();
            nexoHandleFlag = 0x02000000;//result ACK
            ctlNexoHandle();            //send CMD to nexo
//            timer5000ms->start(timerNum);

            if(strDataBegin == "L")
            {
                isVin = true ;
                vinBuf = recBuf.mid(59,17);
            }
            else if(strDataBegin == "C")
            {
                isVin = false ;
                vinBuf = recBuf.mid(59,14);
                strG9  = recBuf.mid(73,5) ;
            }
            else
            {
                DTdebug() << "The data is not vin(start with[L]) or rfid(start with C)" << strDataBegin<<recBuf.mid(59,17);
                return ;
            }

            nexoProNO = recBuf.mid(90,3);
            strNexoResult = recBuf[107] ;
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
            if(strNexoResult == "0")
            {
                nexoResult = "NOK";   //0=tightening NOK ,1=OK
            }
            else
            {
                nexoResult = "OK" ;
            }
            DTdebug()  << tighteningID <<vinBuf << strG9<< nexoProNO << torqueValue << angleValue << nexoResult << tighenTime;
//            nexoProNO = "001";
            if((vinBuf != oldVinCode)&&(vinBuf.trimmed() != ""))
            {
                oldVinCode = vinBuf;
                emit coming_VIN(vinBuf);
                screwid[0]="";
                bzero(&screwid_enable,sizeof(screwid_enable));
                if(isVin)
                {
                    ppFlag = matchVin();
                }
                else {
                    ppFlag = matchG9() ;
                }

            }
            if(ppFlag&&(vinBuf.trimmed() != ""))
            {
                screwid[0] = configParse(nexoProNO,nexoResult);
                DTdebug()<<"screwid:" << screwid[0];
                if(screwid[0] != "")
                {
                    DATA_STRUCT demo;
                    demo.data_model[0] = QString(tighenTime.mid(0,10));
                    demo.data_model[1] = QString(tighenTime.mid(11,8));
                    demo.data_model[2] = nexoResult;
                    demo.data_model[3] = QString(torqueValue);
                    demo.data_model[4] = QString(angleValue);
                    demo.data_model[5] = screwid[0];
                    demo.data_model[6] = QString(vinBuf);
                    demo.data_model[7] = QString(tighteningID);
                    demo.data_model[8] = "Curve is null";
                    demo.data_model[9] = QString(nexoProNO);
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
//                    if(isRepair)
//                    {
//                        demo.data_model[16] = Line_ID;
//                        demo.data_model[17] = Station;
//                        demo.data_model[18] = WirelessIp;
//                    }
                    QVariant DataVar;
                    DataVar.setValue(demo);

                    emit sendfromOpThread(DataVar);
                    if(screwid[0]!="100000000"&&screwid[0]!="200000000"&&screwid[0]!="300000000")
                    {
                        DTdebug() << screwid[0] ;
                        emit send_mainwindow(torqueValue,angleValue,nexoResult,1,screwid[0].toInt(),nexoProNO.toInt());
                    }
                    else if(screwid[0]=="200000000"||screwid[0]=="300000000")
                    {
                        emit VIN_Match_Wrong("1",nexoProNO);//匹配失败
                    }
                    else
                    {
                        emit VIN_Match_Wrong("NULL",nexoProNO);//匹配失败
                    }
                }
            }
            else
            {
                DTdebug()<<"match VIN FAIL: "<<vinBuf;
                vinBuf="";
                emit VIN_Match_Wrong("VIN","");
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

void AtlasOP::setRepair(bool temp)
{
    isRepair = temp;
}

bool AtlasOP::matchVin()
{
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    bool flags = false;
    int i = 1;
    for(i = 1;i < 21;i++)
    {
        QString temp = config->value(QString("carinfo").append(QString::number(i)).append("/VIN")).toString();
        if(temp.length()!=5)
        {
            continue;
        }
        int replacetmp = 0;
        flags = true;
        for(int k = 3;k < 8;k++)
        {
            replacetmp = k-3;
            if(temp.at(replacetmp) == '?')
            {
                continue;
            }
            else
            {
                if(vinBuf.at(k) != temp.at(replacetmp))
                {
                    flags = false;
                    break;
                }else
                {
                    continue;
                }
            }
        }
        if(flags)
        {
            whichar = i;
            break;
        }
    }
    if(flags)
    {
        for(int j = 1;j< 21;j++)
        {
            carInfor[0].proNo[j-1]  = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(j))).toString();
            carInfor[0].boltSN[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(j))).toString();
//            DTdebug()<<"&&&"<<carInfor[0].proNo[j-1]<<carInfor[0].boltSN[j-1];
        }
    }
    DTdebug()<<"&&&&&&&&&&&&&&&&*"<<flags;
    delete config;
    return flags;
}

bool AtlasOP::matchG9()
{
    FUNC() ;
    //匹配G9
    QString temp;
    bool flagss = false;
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    for(int i = 1;i < 101;i++)
    {
        temp = config->value(QString("carinfo").append(QString::number(i)).append("/G9")).toString();
        DTdebug() << temp <<strG9;

        if(temp.length()!=4)
            continue;

        flagss = true;
        for(int j=0;j<4;j++)
        {
            if(temp.at(j) == '?')
                continue;
            else
            {
                if(temp.at(j) == strG9.at(j))
                {
                    flagss = true;
                    continue;
                }
                else
                {
                    flagss = false;
                    break;
                }
            }
        }
        if(flagss)
        {
            whichar = i;
            break;
        }
    }
    if(flagss)
    {
        for(int j = 1;j< 21;j++)
        {
            carInfor[0].proNo[j-1]  = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(j))).toString();
            carInfor[0].boltSN[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(j))).toString();
//            DTdebug()<<"&&&"<<carInfor[0].proNo[j-1]<<carInfor[0].boltSN[j-1];
        }
    }

    delete config ;
    return flagss ;
}

QString AtlasOP::configParse(QString program_tmp,QString state_ok)
{
    FUNC() ;
    //匹配VIN码
    QString value = "";
    int j = 1;
    DTdebug()<<"*******program_tmp*****"<<program_tmp.toInt();
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    for(j = 1;j< 21;j++)
    {
        if(screwid_enable[j-1] == 3)
            continue;
        int program     = carInfor[0].proNo[j-1].toInt();
        DTdebug() << program ;
        if((program_tmp.toInt() == program)&&(program != 0))
        {
            value  = carInfor[0].boltSN[j-1];
            DTdebug() << value << j ;
            if(state_ok == "OK")
                screwid_enable[j-1] = 3;
            else if(!notDisqualificationSkip && state_ok == "NOK")
                screwid_enable[j-1]++;
            break;
        }
    }
    if(j == 21)
    {
         value = "300000000";
    }
    delete config;
    return value;
}


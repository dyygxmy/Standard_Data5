#include "tighten_op_rs232.h"

Tighten_OP_Rs232::Tighten_OP_Rs232(QObject *parent) :
    QObject(parent)
{
    isRepair = false;
    errCount = 0;
    programNO = 0;
    boltCount = 0;
    boltNokCount = 0;
    intReadTighteningID = 0;     //current read Tightening form nexo
    nexoHandleFlag = 0;
    nexoAliveCount = 0;
    controlLinkOk = false;
    revRsDataBuf.clear();
    cmdLen = 0 ;
    cmdMID = 0 ;
    opStart = "\x07\x09\x07\x09\x02";
    opEnd   = "\x03";
    vinBuf = "";
    groupNum = 0;
    vari1 = "";
    m_thread.start();
    this->moveToThread(&m_thread);
}
/*******************************************/
//线程开始
/*******************************************/
void Tighten_OP_Rs232::tightenStart()
{
    qDebug() << "Tighten_OP_Rs232 thread start!!!"<<vari1;

    myCom = new QextSerialPort("/dev/ttymxc3");
    timer5000ms=new QTimer;
    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
    if(comInit())
    {
        nexoHandleFlag  = 0x00000010;  //stop communication
        ctlNexoHandle();               //send CMD to nexo
    }
    timer5000ms->start(5000);
    system("echo 0 > /sys/class/leds/OUTC0/brightness"); //Cleco Input1 0  P1
    system("echo 0 > /sys/class/leds/OUTD7/brightness"); //Cleco Input2 0  P2
    system("echo 0 > /sys/class/leds/OUTD6/brightness"); //Cleco Input3 0  P3
    system("echo 1 > /sys/class/leds/OUTD5/brightness"); //Cleco Input4 0  P4
}
/*******************************************/
//RS232 com initialize
/*******************************************/
bool Tighten_OP_Rs232::comInit()
{
    myCom->setBaudRate((BaudRateType)9600);

    //设置数据位
    myCom->setDataBits((DataBitsType)8);

    //设置校验
    myCom->setParity(PAR_NONE);

    //设置停止位
    myCom->setStopBits(STOP_1);

    //设置数据流控制
    myCom->setFlowControl(FLOW_OFF);
    //设置延时
    myCom->setTimeout(10);

    if(myCom->open(QIODevice::ReadWrite))
    {
        qDebug()<<"serial open ok ";
        connect(myCom, SIGNAL(readyRead()), this, SLOT(slot_read_com3()));
        return true;
    }
    else
    {
        qDebug()<<"serial open fail ";
        return false;
    }
}
/*******************************************/
//send enable
/*******************************************/
void Tighten_OP_Rs232::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
        if(vari1 == "IO")
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
        groupNum = n;
        Channel = "1";
        programNO = carInfor[0].proNo[n].toInt();// proNum.toShort();
        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        boltNokCount = 3;
        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
        vinBuf = VIN_PIN_SQL.toLatin1();
        if(controlLinkOk)
        {
            timer5000ms->stop();
            if(vari1 == "IO")//IO control program
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
            else if(vari1 == "N")//only control enable
            {
                nexoHandleFlag = 0x00000400;//tool enable
            }
            else
            {
                nexoHandleFlag = 0x00000002;//select program
            }
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
void Tighten_OP_Rs232::timerFunc5000ms()
{
    qDebug() << "op 5000ms timer" << controlLinkOk << nexoHandleFlag << nexoAliveCount;
    if(controlLinkOk)                   //nexo link
    {
        if((nexoHandleFlag)&&(nexoHandleFlag != 0x00800000))//????????????
        {
            revRsDataBuf.clear();               //???????????
        }
        nexoHandleFlag |= 0x00800000;//Alive
        ctlNexoHandle();             //send CMD to nexo
    }
    else
    {}
    if(nexoAliveCount > 2)             //3 times alive not ack
    {
//        nexoAliveCount = 0;
        if(controlLinkOk )
        {
            qDebug()<<"emit controlLinkOk false";
            emit IsTigntenReady(false);
            controlLinkOk = false;
        }
        else
        {
            nexoAliveCount = 0;
            qDebug()<<"????? OP relink"<< revRsDataBuf.size()<<revRsDataBuf;//??????????
            revRsDataBuf.clear();
            qDebug()<<"send stop communication";
//        nexoHandleFlag  = 0x00000001;  //start communication
            nexoHandleFlag  = 0x00000010;  //stop communication
            ctlNexoHandle();               //send CMD to nexo
        }
    }
    nexoAliveCount++;
}


/***********************************/
//处理控制Nexo函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void Tighten_OP_Rs232::ctlNexoHandle()
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
        Tx_len = 27;
        sendBuf = opStart + "00200062001         ";// + opEnd;
    }
    /****************************************/
    /**********communication start***********/
    /****************************************/
    else if(nexoHandleFlag&0x00000001)
    {
        Tx_len = 27;
        sendBuf = opStart +"00200001            ";//+opEnd;
    }
    /****************************************/
    /**********communication stop***********/
    /****************************************/
    else if(nexoHandleFlag&0x00000010)
    {
        Tx_len = 27;
        sendBuf = opStart +"00200003            ";//+opEnd;
    }
    /****************************************/
    /*Last tightening results data subscribe*/
    /****************************************/
    else if(nexoHandleFlag&0x01000000)
    {
        Tx_len = 27;
        sendBuf = opStart + "002000600011        ";// + opEnd;
    }
    /****************************************/
    /*********      time set       **********/
    /****************************************/
    else if(nexoHandleFlag&0x40000000)
    {
        Tx_len = 46;
        sendBuf = opStart + "00390082001         2015-05-01:12:01:01";// + opEnd;
        QDateTime time = QDateTime::currentDateTime();
        QString strTime = time.toString("yyyy-MM-dd:hh:mm:ss");
        sendBuf.replace(25,19,strTime.toLatin1());
    }
    /****************************************/
    /*********history results upload request**********/
    /****************************************/
    else if(nexoHandleFlag&0x04000000)
    {
        Tx_len = 31;
        sendBuf = "00300064001         0000000000";
        QByteArray tempBuf = QByteArray::number(intReadTighteningID,10);
        sendBuf.replace(30-tempBuf.size(),tempBuf.size(),tempBuf);
    }
    /****************************************/
    /*********selected program**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000002)
    {
        Tx_len = 30;
        sendBuf = opStart + "00230018            001";// + opEnd;
        sendBuf[26] = programNO/10 + 0x30;
        sendBuf[27] = programNO%10 + 0x30;
    }
    /****************************************/
    /*********set bolt number**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000004)
    {
        Tx_len = 32;
        sendBuf = opStart + "00250019            00105";// + opEnd;
        sendBuf[26] = programNO/10 + 0x30;
        sendBuf[27] = programNO%10 + 0x30;
        sendBuf[28] = boltCount/10 + 0x30;
        sendBuf[29] = boltCount%10 + 0x30;
    }
    /****************************************/
    /*********      ID code        **********/
    /****************************************/
    else if(nexoHandleFlag&0x00010000)
    {
        Tx_len = 44;
        sendBuf = opStart + "00370150            LSVAE45E7EN123456";// + opEnd;
        sendBuf.replace(25,17,vinBuf); //replace vincode
    }
    /****************************************/
    /******** tool disable**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000200)
    {
        Tx_len = 27;
        sendBuf = opStart + "00200042001         ";//+ opEnd;
    }
    /****************************************/
    /******** tool enable**********/
    /****************************************/
    else if(nexoHandleFlag&0x00000400)
    {
        Tx_len = 27;
        sendBuf = opStart + "00200043001         ";// + opEnd;
    }         //send CMD to n
    /****************************************/
    /**********       Alive       ***********/
    /****************************************/
    else if(nexoHandleFlag&0x00800000)//5S send Alive
    {
        nexoHandleFlag &= ~0x00800000;
        Tx_len = 27;
        sendBuf = opStart + "00209999            ";// + opEnd;
    }
    sendBuf[Tx_len-1] = 0x03;

    int mmm = myCom->write(sendBuf,Tx_len);
    qDebug() << "send      :" << mmm << sendBuf ;

}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void Tighten_OP_Rs232::slot_read_com3()
{
    bool okFlag;
    int cmdType=0;
    int revLen;
    QByteArray recBuf;
    double torqueMax = 0.0;
    double torqueMin = 0.0;
    revLen=myCom->bytesAvailable();
    recBuf = myCom->readAll();
    if(revLen > 0)
    {
//        qDebug()<<"************recBuf***********"<<recBuf.size()<<recBuf;//??????
        revRsDataBuf.append(recBuf);
//        qDebug()<<"************revRsDataBuf*****"<<revRsDataBuf.size()<<revRsDataBuf;//???????
        while( (!revRsDataBuf.isEmpty())&&(revRsDataBuf.mid(0,1) != "\x02") )
        {
            qDebug()<< "****start not STX***" << revLen << revRsDataBuf;
            revRsDataBuf.replace(0,1,"");
        }
        if(revRsDataBuf.mid(0,1) == "\x02")
        {
            if(revRsDataBuf.size() > 5)
            {
                cmdLen = revRsDataBuf.mid(1,4).toInt() + 3;
                if(cmdLen <= revRsDataBuf.size())//?????????????
                {
                    nexoAliveCount = 0;
                    qDebug() << "recevice:" << revRsDataBuf.size()  << cmdLen << "note:" << revRsDataBuf;
                    cmdMID = revRsDataBuf.mid(5,4).toInt();
                    switch(cmdMID)
                    {
                    case 4:   //cmd send error
                        errCount ++;
                        if(errCount > 2)   //over 3 timer no send cmdType
                        {
                            errCount = 0;
                            nexoHandleFlag = 0; //clear status
                            qDebug() << "nexo cmd error:" << controlLinkOk;
                        }
                        else
                        {
                            if(nexoHandleFlag)
                            {
                                timer5000ms->stop();
                                ctlNexoHandle();
                                timer5000ms->start(5000);
                            }
                            qDebug() << "resend cmd 1111111";
                        }
                        break;
                    case 5:  //cmd send ok
                        errCount = 0;
                        cmdType = revRsDataBuf.mid(21,4).toInt();
                        switch(cmdType)
                        {
                        case 3:
                            nexoHandleFlag  = 0x00000001;//start communication
                            timer5000ms->stop();
                            ctlNexoHandle();
                            timer5000ms->start(5000);
                            break;
                        case 18:     //select program
                            nexoHandleFlag = 0x00000004;//set bolt number
                            //                            nexoHandleFlag = 0x00000400;//tool enable
                            timer5000ms->stop();
                            ctlNexoHandle();
                            timer5000ms->start(5000);
                            break;
                        case 19:     //set bolt number
                            nexoHandleFlag = 0x00000400;//tool enable
                            timer5000ms->stop();
                            ctlNexoHandle();
                            timer5000ms->start(5000);
                            break;
                        case 42:    //tool disable
                            nexoHandleFlag &= ~0x00000200;
                            break;
                        case 43:    //tool enable
                            nexoHandleFlag &= ~0x00000400;
                            break;
                        case 50:    //vin download
                            nexoHandleFlag = 0x00000002;//select program
                            timer5000ms->stop();
                            ctlNexoHandle();
                            timer5000ms->start(5000);
                            break;
                        case 60:    //result subscribe
                            nexoHandleFlag = 0x40000000;//time set
                            timer5000ms->stop();
                            ctlNexoHandle();
                            timer5000ms->start(5000);
                            break;
                        case 64:    //read old result
                            nexoHandleFlag &= ~0x04000000;
                            break;
                        case 82:    //set time
                            if(vari1 == "IO")
                            {
                                nexoHandleFlag = 0x00000400;//tool enable
                            }
                            else
                            {
                                if(SYSS == "ING")
                                {
                                    if(vari1 == "N")
                                    {
                                        nexoHandleFlag = 0x00000400;//tool enable
                                    }
                                    else
                                    {
                                        nexoHandleFlag = 0x00000002;//select program
                                    }
                                }
                                else
                                {
                                    nexoHandleFlag = 0x00000200;//nexo disable
                                }
                                timer5000ms->stop();
                                ctlNexoHandle();
                                timer5000ms->start(5000);
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 2:
                        controlLinkOk = true;             //communication start OK
                        nexoHandleFlag = 0x01000000;//tightening results data subscription
                        timer5000ms->stop();
                        ctlNexoHandle();             //send CMD to nexo
                        timer5000ms->start(5000);
                        if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
                        {
                            SYSS = "Ready";
                        }
                        emit IsTigntenReady(true);
                        break;
                    case 9999://应答不显示在PC 测试使用
                        break;
                    case 61://Rev result data
                        nexoHandleFlag = 0x02000000;//result ACK
                        timer5000ms->stop();
                        ctlNexoHandle();
                        timer5000ms->start(5000);
                        if(SYSS == "ING")
                        {
                            nexoProNO = revRsDataBuf.mid(91,3);
                            //                            torqueMin = recBuf.mid(116,6).toDouble()/100;
                            //                            torqueMax = recBuf.mid(124,6).toDouble()/100;
                            torqueValue = revRsDataBuf.mid(141,6);                           //torque value
                            torqueValue = QByteArray::number(torqueValue.toDouble()/100);
                            angleValue  = revRsDataBuf.mid(170,5);                           //angle value
                            angleValue  = QByteArray::number(angleValue.toInt());
                            tighenTime   = revRsDataBuf.mid(177,19);                          //Torque time
                            tighenTime[10]=0x20;
                            tighteningID = revRsDataBuf.mid(222,10);                           //Tightening ID
                            intReadTighteningID = tighteningID.toInt(&okFlag,10);
                            if(intReadTighteningID == 0)
                            {
                                qDebug() << "data error ***********intReadTighteningID = 0****************";
                                return ;
                            }
                            if(nexoProNO != "099")       //99 program not count
                            {
                                if(revRsDataBuf[108] == '0')
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
                            }
                            else
                            {
                                nexoResult = "NOK";                                  //99 program nok
                            }
                            qDebug()  << tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime;

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
                            if(isRepair)
                            {
                                demo.data_model[16] = Line_ID;
                                demo.data_model[17] = Station;
                                demo.data_model[18] = WirelessIp;
                            }

                            QVariant DataVar;
                            DataVar.setValue(demo);
                            emit sendfromworkthread(DataVar);
                            emit send_mainwindow(QString(torqueValue),QString(angleValue),nexoResult,0,groupNum);
                        }
                        break;
                    case 65://old result data
                        qDebug() << "mid 0065";
                        break;
                    default:
                        break;
                    }
//                    revRsDataBuf.clear();
                    revRsDataBuf=revRsDataBuf.right(revRsDataBuf.size()-cmdLen);
                    cmdLen = 0 ;
                }
                else
                {
//                    qDebug() << "data len no ok"  <<cmdLen << revRsDataBuf.size()<< revRsDataBuf;//???????????
                }
            }
        }
    }
}

void Tighten_OP_Rs232::setRepair(bool temp)
{
    isRepair = temp;
}

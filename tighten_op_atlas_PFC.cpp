/**********************************************************************
 * file name  tighten_op_atlas_PFC.c
  describe   ：control enable / program / vin
 *             Last tightening result data subscribe
 *             Last tightening result curve subscribe
 *             Old tightening result upload request
 *
***********************************************************************/
#include "tighten_op_atlas_PFC.h"
#define  DEBUG_OUT

TightenOpAtlasPFC::TightenOpAtlasPFC(QObject *parent) :
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
    tighteningID = "";
    Factory = factory;
    curve_AngleTorque = 0;
    curveNullBuf = "Curve is null";
    resetJobOffFlag = false;
    angleSubFlag = 0;
    vari_2 = "";
    vari_1 = "";

    m_thread.start();
    this->moveToThread(&m_thread);
}
//线程开始
void TightenOpAtlasPFC::tightenStart()
{
//    system("echo 1 > /sys/class/leds/OUTC1/brightness"); //PF disable = 1
    DTdebug() << "tighten_op_atlas_PFC thread start !!!"<<vari_1<<vari_2;
    timer5000ms=new QTimer;
    tcpSocket = new QTcpSocket;
    connect(timer5000ms,SIGNAL(timeout()),this,SLOT(timerFunc5000ms()));
    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(revNexoData()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDO()));
    newConnects();
    timer5000ms->start(5000);
}
//使能
void TightenOpAtlasPFC::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
        if(vari_1 == "N")
        {
            timer5000ms->stop();
            if(n==99)
            {
                nexoHandleFlag = 0x00000400;              //tool enable
            }
            else
            {
                nexoHandleFlag = 0x00000200;              //tool disable
            }
            ctlNexoHandle();
            timer5000ms->start(5000);
        }
        else if(vari_1 == "IO")
        {
            system("echo 1 > /sys/class/leds/OUTC1/brightness"); //PF disable = 1
            DTdebug()<<"PF disable = 1";
        }
        else if(n == 98)
        {
            if(vari_1 == "T")
            {
                timer5000ms->stop();
                nexoHandleFlag = 0x00080000;              //job batch incerment
                ctlNexoHandle();
                timer5000ms->start(5000);
            }
        }
        else if(n == 97)
        {
            if(vari_1 == "T")
            {
                timer5000ms->stop();
                nexoHandleFlag = 0x00200000;              //job off
                ctlNexoHandle();
                timer5000ms->start(5000);
            }
        }
        else if((ISRESET)||(n==99))
        {
            timer5000ms->stop();
            nexoHandleFlag = 0x00400000;                  //abort job
            ctlNexoHandle();
            timer5000ms->start(5000);
            DTdebug() << "********** abort job***********";
        }
    }
    else
    {
        Channel = "0";
        programNO = carInfor[0].proNo[n].toShort();// proNum.toShort();
        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        boltNokCount = 3;
        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();
        DTdebug() << "boltNumberBuf" << boltNumberBuf;
        if(VIN_PIN_SQL.toLatin1() == vinBuf)
        {
            sendVinFlag = 0;

            if(vari_1 == "N")
            {
                timer5000ms->stop();
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();
                timer5000ms->start(5000);
            }
        }
        else
        {
            if(vari_1 != "IO")
            {
                timer5000ms->stop();
                if(vari_1 == "N")
                {
                    nexoHandleFlag = 0x00000400;//tool enable
                }
                else if(vari_1 == "T")
                {
                    strJobID = QByteArray::number(10000+programNO).mid(1,2);
                    nexoHandleFlag = 0x00100000;           //select JOB
                    programNO %= 100;
                }
                else
                {
                    nexoHandleFlag = 0x00010000;           //ID code download
                }
                ctlNexoHandle();
                timer5000ms->start(5000);
            }
            sendVinFlag = 1;
        }
        if(vari_1 == "IO")
        {
            system("echo 0 > /sys/class/leds/OUTC1/brightness"); //PF disable = 0
            DTdebug()<<"PF disable = 0";
        }
        vinBuf = VIN_PIN_SQL.toLatin1();
        DTdebug() << "send enable" << programNO << boltCount<<strJobID ;
    }
}

/**************************************/
//nexo alive
/**************************************/
void TightenOpAtlasPFC::timerFunc5000ms()
{
#if 1
    DTdebug() << "op 5000ms tiemr" << nexoLinkOk << nexoHandleFlag << nexoAliveCount << nexoHandleFlag;
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
#else
    emit IsTigntenReady(true);  //test
#endif
}

/*******************************************************/
//nexo disconnect handle
/*******************************************************/
void TightenOpAtlasPFC::disConnectDO()
{
//    nexoLinkOk = false;
    DTdebug() << "OP disconnect 22222222222";
//    newConnects();
}
/*******************************************************/
//nexo bulid connect
/*******************************************************/
void TightenOpAtlasPFC::newConnects()
{
    tcpSocket->abort(); //取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(ControllerIp_1,4545);
    tcpSocket->waitForConnected(2000); //waitting 2000ms
#ifdef DEBUG_OUT
    DTdebug() << "PFC the socket state is" << ControllerIp_1 << tcpSocket->state();
#endif
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
void TightenOpAtlasPFC::ctlNexoHandle()
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
    /*******torque curve unsubscribe****************/
    /****************************************/
    else if(nexoHandleFlag&0x00000800)
    {
        nexoHandleFlag &= ~0x00000800;
        Tx_len = 35;
        sendBuf = "003400090010        09000010601002";
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
    }
    /****************************************/
    /********** abort job         ***********/
    /****************************************/
    else if(nexoHandleFlag&0x00400000)
    {
        Tx_len = 21;
        sendBuf = "00200127001         ";
    }
    /****************************************/
    /********** Job batch increment***********/
    /****************************************/
    else if(nexoHandleFlag&0x00080000)
    {
        Tx_len = 21;
        sendBuf = "00200128001         ";
    }
    /****************************************/
    /********Job off and off reset**** ******/
    /****************************************/
    else if(nexoHandleFlag&0x00200000)
    {
        Tx_len = 22;
        if(!resetJobOffFlag)
        {
            sendBuf = "00210130001         0";//job off
            resetJobOffFlag = true;
        }
        else
        {
            sendBuf = "00210130001         1";//reset job off
            resetJobOffFlag = false;
        }
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
    DTdebug() << "send      :" << sendBuf;// << "nexoHandleFlag :" << nexoHandleFlag ;
    tcpSocket->write(sendBuf,Tx_len);
   // tcpSocket->waitForBytesWritten(300);
       bool Send_Timeout_flag = tcpSocket->waitForBytesWritten(300);//加入300ms超时,防止发送超时阻塞

//        if(!Send_Timeout_flag)
//        {
//            DTdebug("PFc State:%d\n",tcpSocket->state());
//        }
        DTdebug() << "PFC sendbuf  finish!"<<"Send_Timeout_flag:"<<!Send_Timeout_flag;

}
/***********************************/
//处理接受Nexo数据处理函数
//len:收到的网络包长度  Rx_Buffer[]:收到的网络包数据  Tx_len:发送网络包长度  Tx_Buffer[]:发送网络包数据
/*************************************/
void TightenOpAtlasPFC::revNexoData()
{
//    bool okFlag;
    int nexoMID;
//    int cmdType;
//    int errCode;
    int alivelen = 0;
    int nullCount=0,i=0,j=0,k=0;
    int hByte,lByte;
    int revLen,cmdLen;
    QByteArray torqueMax ="";
    QByteArray torqueMin ="";
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
        //        DTdebug() << "note:" << recBuf;
    }
    else
    {}
    //        DTdebug() << "/********************************************************************/";

    if(revLen == (cmdLen+1))              //length of the same
    {
        DTdebug() <<curveFirstFlag << curveDataFlag << "recevice len:" << revLen << cmdLen << "note:" << recBuf;
        nexoAliveCount = 0;           //recvice nexo data
        receivenormaldata(revLen, cmdLen,nexoMID,recBuf);

    }
    else  if(curveDataFlag == 1)
    {
        DTdebug() <<curveFirstFlag << curveDataFlag << "recevice len:" << revLen << cmdLen<<curveBuf;
        //        DTdebug() <<"receive data:"<<curveBuf.size()<<curveBuf;
        if(curveFirstFlag == 0)
        {
            curveBuf = curveBuf + recBuf;
            revDataLen = revDataLen + revLen;
            DTdebug() << "more data:" << curveBuf.size() << revDataLen << revFinishLen;
        }
        if(revDataLen >= (revFinishLen<<1))//curve receive finished
        {
            for(k=0;k<2;k++)
            {
                curveType = curveBuf.mid(56,3).data();
                idCode = curveBuf.mid(20,10); //ID code
                DTdebug()<< "******curveType*********" <<curveType << idCode;
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
                    DTdebug()<< "**curve angle** curveType:" <<curveType << idCode;
                    if(curve_AngleTorque == 0)//curve is null
                    {
                        curve_AngleTorque = 1;//curve angle
                        curveBufTemp = "*";
                        curveBufTemp.append(curveBuf.mid(0,nullCount));
                        curveBufTemp.append(hexBuf);
                        curveBufTemp.append('|');
                        //                        DTdebug()<<"1111111 angle"<<curveBufTemp;

                        if(vari_2 != "1445")
                        {
//                                timer5000ms->stop();
                            nexoHandleFlag = 0x00004000;                  //torque subscribe
                            ctlNexoHandle();                              //send CMD to nexo
                            timer5000ms->start(5000);
                        }
                    }
                    else if(curve_AngleTorque == 2)//curve is torque
                    {
                        curveBufTemp = "*";
                        curveBufTemp.append(curveBuf.mid(0,nullCount));
                        curveBufTemp.append(hexBuf);
                        curveBufTemp.append('|');
                        //                        DTdebug()<<"222222 angle"<<recBuf;
                        curveBufTemp.append(curveBufTemp1);
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
                            tighteningID = "";
                            DTdebug() << "torque_angle curve file path:" <<fileName;//<<curveBufTemp;
                        }
                        else
                        {
                            DTdebug() << "tighteningID == null:";
                        }
                    }
                    else
                    {
                        curve_AngleTorque = 0;
                    }
                }
                else if(curveType == "001") //2 times torque
                {
                    DTdebug()<< "**curve torque** curveType:" <<curveType << idCode;
                    if(curve_AngleTorque == 0)//curve is null
                    {
                        curve_AngleTorque = 2;//curve torque
                        curveBufTemp1 = curveBuf.mid(0,nullCount);
                        curveBufTemp1.append(hexBuf);
                        curveBufTemp1.append('}');
                        //                        DTdebug()<<"1111111 torque"<<curveBufTemp1;

                        if(vari_2 != "1445")
                        {
//                                timer5000ms->stop();
                            nexoHandleFlag = 0x00008000;                  //angle subscribe
                            ctlNexoHandle();                              //send CMD to nexo
                            timer5000ms->start(5000);
                        }
                    }
                    else if(curve_AngleTorque == 1)//curve is angle
                    {
                        curveBufTemp.append(curveBuf.mid(0,nullCount));
                        curveBufTemp.append(hexBuf);
                        curveBufTemp.append('}');
                        //                        DTdebug()<<"2222222 torque"<<curveBufTemp;
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
                            tighteningID = "";
                            curve_AngleTorque = 0;
                            DTdebug() << "angle_torque curve file path:" <<fileName;//<<curveBufTemp;
                        }
                        else
                        {
                            DTdebug() << "tighteningID == null:";
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
                DTdebug()<< "**curve angle** curveType:" <<curveType << idCode;
                if(curve_AngleTorque == 0)//curve is null
                {
                    curve_AngleTorque = 1;//curve angle
                    curveBufTemp = "*";
                    curveBufTemp.append(curveBuf.mid(0,nullCount));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('|');
                    //                    DTdebug()<<"3333333 angle"<<curveBufTemp;
                    if(vari_2 != "1445")
                    {
//                            timer5000ms->stop();
                        nexoHandleFlag = 0x00004000;                  //torque subscribe
                        ctlNexoHandle();                              //send CMD to nexo
                        timer5000ms->start(5000);
                    }
                }
                else if(curve_AngleTorque == 2)//curve is torque
                {
                    curveBufTemp = "*";
                    curveBufTemp.append(curveBuf.mid(0,nullCount));
                    curveBufTemp.append(hexBuf);
                    curveBufTemp.append('|');
                    //                    DTdebug()<<"444444 angle"<<curveBufTemp;
                    curveBufTemp.append(curveBufTemp1);
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
                        tighteningID = "";
                        curve_AngleTorque = 0;
                        DTdebug() << "torque_angle curve file path:" <<fileName;//<<curveBufTemp;
                    }
                    else
                    {
                        DTdebug() << "tighteningID == null:";
                    }
                }
                else
                {
                    curve_AngleTorque = 0;
                }
            }
            else if(curveType == "001") //2 times Torque
            {
                DTdebug()<< "**curve torque** curveType:" <<curveType << idCode;
                if(curve_AngleTorque == 0)//curve is null
                {
                    curve_AngleTorque = 2;//curve torque

                    curveBufTemp1 = curveBuf.mid(0,nullCount);
                    curveBufTemp1.append(hexBuf);
                    curveBufTemp1.append('}');
                    //                    DTdebug()<<"3333333 torque"<<curveBufTemp1;
                    if(vari_2 != "1445")
                    {
//                            timer5000ms->stop();
                        nexoHandleFlag = 0x00008000;                  //angle subscribe
                        ctlNexoHandle();                              //send CMD to nexo
                        timer5000ms->start(5000);
                    }
                }
                else if(curve_AngleTorque == 1)//curve is angle
                {
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
                        tighteningID = "";
                        DTdebug() << "angle_torque curve file path:" <<fileName;//<<curveBufTemp;
                    }
                    else
                    {
                        DTdebug() << "tighteningID == null:";
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
        DTdebug() << "Receive multiple data sticky packets! ";

        if(nexoMID == 9999)//alive message
        {
            alivelen = 21;//alive message length
            nexoMID = recBuf.mid(4+alivelen,4).toInt();
            revLen = recBuf.size() - alivelen;
            cmdLen = recBuf.mid(0+alivelen,4).toInt();
            if(revLen ==(cmdLen+1) )
            {
                recBuf = recBuf.right(revLen);
                receivenormaldata(revLen,cmdLen, nexoMID,recBuf);
            }
            else
                DTdebug()<<"Pfc receive data error!";
        }
        else
        {
            DTdebug()<<"First data packet not alive,start NexoData!";
            receivenormaldata(revLen,cmdLen, nexoMID,recBuf);
        }
    }
}

    void TightenOpAtlasPFC::receivenormaldata(int revLen, int cmdLen, int nexoMID, QByteArray recBuf)
    {
        int cmdType;
        int errCode;
        bool okFlag;
        //int revLen,cmdLen;
        QByteArray torqueMax ="";
        QByteArray torqueMin ="";

        switch(nexoMID)
        {
        case 4:   //cmd send error         //send CMD to n
            cmdType = recBuf.mid(20,4).toInt();
            errCode = recBuf.mid(24,2).toInt();
            if((cmdType == 50)&&(errCode == 1))
            {
                sendVinFlag = 0;
                DTdebug() << "Vehicle ID Number cmd error,invalid data!";
            }
            else
            {
                errCount ++;
                if(errCount > 3)   //over 3 timer no send cmdType
                {
                    errCount = 0;
                    DTdebug() << "nexo cmd error 3 times";
                    nexoHandleFlag = 0;
                }
                else
                {
                    if((cmdType == 60)&&(errCode == 9))//Last tightening result subscription already exists
                    {
                        nexoHandleFlag = 0x40000000;//time set
                        DTdebug() << " Last tightening result data subscribe already exists!";
                    }
                    else
                    {
                        //                        经过测试可以发现这里是个bug，发送的命令为空,所以屏蔽
                        //                        timer5000ms->stop();
                        //                        ctlNexoHandle();             //send CMD to nexo
                        //                        timer5000ms->start(5000);
                        DTdebug() << "nexo cmd error:" << cmdType << errCode;
                    }
                }
            }
            break;
        case 5:  //cmd send ok
            cmdType = recBuf.mid(20,4).toInt();
            errCount = 0;
            switch(cmdType)
            {
            case 8:
            case 900:
                curveNullBuf = "null";
                nexoHandleFlag &= ~0x00008000;  //angle subscribe
                nexoHandleFlag &= ~0x00004000; //torque subscribe
                DTdebug()<<"****curveNullBuf*****:"<<curveNullBuf;
                if(angleSubFlag)
                {
                    angleSubFlag = 0;
                    if(vari_2 == "1445")
                    {
                                                timer5000ms->stop();
    //                    SenddataFlag = 1;
                        nexoHandleFlag = 0x00004000;   //torque subscribe
                        ctlNexoHandle();               //send CMD to nexo
                                                timer5000ms->start(5000);
    //                    SenddataFlag = 0;
                    }
                }
                break;
            case 18:     //select program
                //               nexoHandleFlag &= ~0x00000002;
                                 timer5000ms->stop();
    //            SenddataFlag = 1;
                nexoHandleFlag = 0x00000004;//set bolt number
                ctlNexoHandle();             //send CMD to nexo
                               timer5000ms->start(5000);
    //            SenddataFlag = 0;
                break;
            case 19:     //set bolt number
                //               nexoHandleFlag &= ~0x00000004;
                              timer5000ms->stop();
    //            SenddataFlag = 1;
                nexoHandleFlag = 0x00000400;//tool enable
                ctlNexoHandle();             //send CMD to nexo
                                timer5000ms->start(5000);
    //            SenddataFlag = 0;
                break;
            case 34:
                nexoHandleFlag &= ~0x00400000;//Job info subcrible
                break;
            case 38:
                nexoHandleFlag &= ~0x00100000; //select Job
                //                nexoHandleFlag = 0x00000400; //enable
                break;
            case 39://Job reset
                                timer5000ms->stop();
    //            SenddataFlag = 1;
                nexoHandleFlag = 0x00020000;                  //VIN subscribe
                ctlNexoHandle();
                                timer5000ms->start(5000);
    //            SenddataFlag = 0;
                break;
            case 42:    //tool disable
                nexoHandleFlag &= ~0x00000200;
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
            case 51:   //vin subscribe
                                timer5000ms->stop();
    //            SenddataFlag = 1;
                nexoHandleFlag = 0x00008000;  //angle subscribe
                ctlNexoHandle();
                                timer5000ms->start(5000);
    //            SenddataFlag = 0;
                curve_AngleTorque = 3;
                break;
            case 60:    //result subscribe
                angleSubFlag = 1;
                                timer5000ms->stop();
    //            SenddataFlag = 1;
                nexoHandleFlag = 0x40000000;//time set
                ctlNexoHandle();             //send CMD to nexo
                                timer5000ms->start(5000);
    //            SenddataFlag = 0;
                break;
            case 64:    //read old result
                nexoHandleFlag &= ~0x04000000;

                break;
            case 127:    //abort job
                if(ISRESET)
                {
                    ISRESET = false;
                    nexoHandleFlag &= ~0x00400000;            //abort job
                    //                    nexoHandleFlag = 0x00000200;            //disable
                }
                else
                {
                                        timer5000ms->stop();
    //                SenddataFlag = 1;
                    nexoHandleFlag = 0x00020000;                  //VIN subscribe
                    ctlNexoHandle();
                                        timer5000ms->start(5000);
    //                SenddataFlag = 0;
                }
                break;
            case 128:    //Job batch increment
                nexoHandleFlag &= ~0x00080000;            //job batch incerment
                break;
            case 130:    //Job off
                if(!resetJobOffFlag)
                {
                    nexoHandleFlag &= ~0x00200000;            //job off
                }
                else
                {
                                        timer5000ms->stop();
    //                SenddataFlag = 1;
                    nexoHandleFlag = 0x00200000;                //reset job off
                    ctlNexoHandle();
                                        timer5000ms->start(5000);
    //                SenddataFlag = 0;
                }
                break;
            case 70:    //alarm subscribe
                break;
            case 82:    //set time
                                timer5000ms->stop();
    //            SenddataFlag = 1;
                if(vari_1 == "N")
                {
                    nexoHandleFlag = 0x00000200;                  //disable
                }
                else if(vari_1 == "T")
                {
                    if(vari_2 == "V")
                    {
                        nexoHandleFlag = 0x00020000;                  //VIN subscribe
                    }
                    else
                    {
                        nexoHandleFlag = 0x00008000;                  //angle subscribe
                        curve_AngleTorque = 3;
                    }
                }
                else
                {
                    nexoHandleFlag = 0x00020000;                  //VIN subscribe
                }
                //                nexoHandleFlag = 0x00400000;                  //abort job
                ctlNexoHandle();
                                timer5000ms->start(5000);
    //            SenddataFlag = 0;
                break;
            default:
                break;
            }
            break;
        case 2:
            nexoLinkOk = true;                //communication start OK

            if(Disconnectflag)  //add 20171228
            {
                Disconnectflag = false;
                reconnectflag = true;
                DTdebug()<<"OP reconnect success!";
    //            requestCycle();
            }

            if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
            {
                SYSS = "Ready";
            }
            emit IsTigntenReady(true);
                        timer5000ms->stop();
    //        SenddataFlag = 1;
            nexoHandleFlag = 0x01000000;//tightening results data subscription
            ctlNexoHandle();             //send CMD to nexo
                        timer5000ms->start(5000);
    //        SenddataFlag = 0;
            break;
        case 9999://应答不显示在PC 测试使用
            break;
        case 71:    //alarm
                       timer5000ms->stop();
    //        SenddataFlag = 1;
            nexoHandleFlag = 0x00001000;
            ctlNexoHandle();            //send CMD to nexo
                        timer5000ms->start(5000);
    //        SenddataFlag = 0;
            break;
        case 76://system error status
                        timer5000ms->stop();
    //        SenddataFlag = 1;
            nexoHandleFlag = 0x10000000;//Status ACK ???????? not nexo ack
            ctlNexoHandle();             //send CMD to nexo
                        timer5000ms->start(5000);
    //        SenddataFlag = 0;
            break;
        case 52:   //vin ack
                        timer5000ms->stop();
    //        SenddataFlag = 1;
            nexoHandleFlag = 0x00040000;                  //VIN ACK
            ctlNexoHandle();
            timer5000ms->start(5000);
    //        SenddataFlag = 0;
            if(recBuf.size() >= 37)
            {
                if(vari_2 == "V")//
                {
                    DTdebug()<<"*****rev VIN*****QueueAlign**********";
                    emit QueueAlign(QString(recBuf.mid(20,17)),false,"VIN");
                }
                else
                {
                    if(sendVinFlag)//vin download
                    {
                        DTdebug()<<"**********vin download**********";
                        sendVinFlag = 0;
                    }
                    else//read vin
                    {
                        DTdebug()<<"**********QueueAlign**********";
                        emit QueueAlign(QString(recBuf.mid(20,17)),false,"VIN");
                    }
                }
            }
            break;
        case 61://Rev result data
                        timer5000ms->stop();
    //        SenddataFlag = 1;
            nexoHandleFlag = 0x02000000;//result ACK
            ctlNexoHandle();            //send CMD to nexo
                        timer5000ms->start(5000);
    //        SenddataFlag = 0;
            if(SYSS == "ING")
            {
                revDataLen   = 0;
                revFinishLen = 0;
                curveBuf = "";
                curveBufTemp ="";
                curveBufTemp1 = "";
                curveFirstFlag = 0;
                curveDataFlag = 0;   //receive curve
                idCode = "";
                curve_AngleTorque = 0;

                nexoProNO = recBuf.mid(90,3);  //wxm
                torqueValue = recBuf.mid(140,6);                           //torque value
                torqueValue = QByteArray::number(torqueValue.toDouble()/100);
                angleValue  = recBuf.mid(169,5);                           //angle value
                angleValue = QByteArray::number(angleValue.toDouble());
                tighenTime   = recBuf.mid(176,19);                          //Torque time
                tighteningID = recBuf.mid(221,10);                           //Tightening ID
                LastCycle = tighteningID.toInt();//add 20171228
                torqueMin = recBuf.mid(116,6);       //torque low limit
                torqueMin = QByteArray::number(torqueMin.toDouble()/100);
                torqueMax = recBuf.mid(124,6);       //torque high limit
                torqueMax = QByteArray::number(torqueMax.toDouble()/100);
                intReadTighteningID = tighteningID.toInt(&okFlag,10);
                intNexoMaxTighteningID = intReadTighteningID;
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
                DTdebug()  << tighteningID << nexoProNO << torqueValue << angleValue << nexoResult << tighenTime<<curveNullBuf;
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
                if(Factory =="BYDSZ" ||Factory=="BYDXA"||factory=="Haima")
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
                else if(Factory =="BAIC" || Factory =="NOVAT")
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
                    demo.data_model[13] = QString(torqueMax);
                    demo.data_model[14] = QString(torqueMin);
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
        case 900:
            revDataLen   = revLen;
            revFinishLen = cmdLen;
            curveBuf = recBuf;
            curveFirstFlag = 1;
            curveDataFlag = 1;   //receive curve
            idCode = "";
            break;
        case 65:
        {
            int tempcycle = recBuf.mid(23,10).toInt();//add 20171228
            DTdebug()<<"Old Tightening ID:"<<tempcycle;
            if(reconnectflag)
            {
                reconnectflag = false;
                if((tempcycle-LastCycle)>0)
                {
                    // LastCycle= tighteningID.toInt()-LastCycle;
                    DTdebug()<<"TightenOpAtlasPFC data missed "<<tempcycle-LastCycle;

                }
                else
                {
                    DTdebug()<<"TightenOpAtlasPFC data not missed ";
                }
            }
        }
            break;
        default:
            break;
        }
    }

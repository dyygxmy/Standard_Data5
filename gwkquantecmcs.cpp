#include "gwkquantecmcs.h"
#include "GlobalVarible.h"

GwkQuanTecMCS::GwkQuanTecMCS(QObject *parent) :
    QObject(parent)
{
    timerMs=new QTimer;
    connect(timerMs,SIGNAL(timeout()),this,SLOT(timerFuncMs()));
    timerMs->start(100);
    startCount = 0;
    vinCount = 0;
    curveTimerFlag = false;
    dataTimerFlag = false;
    curveTimerCount = 0;

    recFlag = false;
    strPeakAngle = "";
    strPeakTorque = "";
    strStopAngle = "";
    strStopTorque = "";
    strResult = "";
    strDateTime = "";
    gwkLinkFlag = false;
    reSendTimeCount = 0;
    reSendFlag = false;
    sendLen = 0;
    testCount = 0;
    proNameLen = 0;
    readProgramFlag = false;
    gwkProNO = "";
    gwk_thread.start();
    this->moveToThread(&gwk_thread);
}

/*********************************************/
//serial init
/*********************************************/
void GwkQuanTecMCS::serialComInit()
{
    qDebug() << "&&&&&& GWK thread start &&&" ;
    QString portName = "ttyUSB0";   //获取串口名
    myCom3 = new QextSerialPort("/dev/" + portName);
    connect(myCom3, SIGNAL(readyRead()), this, SLOT(slot_read_com()));

    //设置波特率
    myCom3->setBaudRate((BaudRateType)57600);

    //设置数据位
    myCom3->setDataBits((DataBitsType)8);

    //设置校验
    myCom3->setParity(PAR_NONE);

    //设置停止位
    myCom3->setStopBits(STOP_1);

    //设置数据流控制
    myCom3->setFlowControl(FLOW_OFF);
    //设置延时
    myCom3->setTimeout(20);

    if(myCom3->open(QIODevice::ReadWrite))
    {
        qDebug()<<"serial ttyUSB0 open ok ";
        if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
        {
            SYSS = "Ready";
        }
        emit IsTigntenReady(true);
        revRsDataBuf.clear();
        sendBuf[0]=0xeb;
        sendBuf[1] = 0;
        sendLen = 1;
        myCom3->write(sendBuf,sendLen);
        reSendFlag = true;     //send eb
        qDebug() << "send gwk init cmd:"<<sendBuf.toHex();
    }
    else
    {
        qDebug()<<"serial ttyUSB0 open fail XXXXXXXXXXXXXXX " ;
        emit IsTigntenReady(false);
    }
}
//使能
void GwkQuanTecMCS::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {}
    else
    {
        programNO = carInfor[0].proNo[n].toShort();// proNum.toShort();
        boltCount = carInfor[0].boltNum[n];//lsNum.toShort();
        boltNokCount = 3;
        boltNumberBuf = carInfor[0].boltSN[n].toLatin1();//SCREWID_SQL.toLatin1();

        qDebug() <<"********enable****** "<< VIN_PIN_SQL << boltNumberBuf <<programNO;
    }
}

/**************************************/
//100ms timer funcation
/**************************************/
void GwkQuanTecMCS::timerFuncMs()
{
    if(curveTimerFlag)
    {
        curveTimerCount ++;
        if((curveTimerCount>=10)&&((unsigned char)revRsDataBuf[revRsDataBuf.size()-1] == 0xE7)) //S
        {
            curveTimerCount = 0;
            curveTimerFlag = false;
            revRsDataBuf.clear();
            sendBuf[0] = 0xe0;  //read data cmd
            sendBuf[1] = 0;
            sendLen = 1;
            myCom3->write(sendBuf,sendLen);
            readProgramFlag = true;
            reSendFlag = true;  //send e0
            qDebug() << "send index cmd:"<<sendBuf.toHex();
        }
    }
    if(reSendFlag)
    {
        reSendTimeCount++;
        if(reSendTimeCount>20)//2s
        {
            if((unsigned char)sendBuf[0] == 0xe0)
            {
                readProgramFlag = true;
            }
            else if((unsigned char)sendBuf[0] == 0)
            {
                sendBuf[0] = 0xe0;
                sendLen = 1;
            }
            reSendTimeCount = 0;
            revRsDataBuf.clear();
            myCom3->write(sendBuf,sendLen);            
            qDebug() << "XXXXXXXXXXXXXXXXXX resend CMD XXXXXXXXXXXXXXXX:"<<sendBuf.toHex();
        }
    }
    testCount++;
    if(testCount>50)
    {
        testCount = 0;
        qDebug() <<"reSendFlag:"<<reSendFlag<<"reSendTimeCount:"<<reSendTimeCount<< "sendLen" <<sendLen<<"sendBuf"<<sendBuf.toHex();
    }
}
/*********************************************/
//serial receive data
/*********************************************/
void GwkQuanTecMCS::slot_read_com()
{
   //du读
    QByteArray revBuf;
    int revLen;
    revLen=myCom3->bytesAvailable();
    revBuf = myCom3->readAll();
    revRsDataBuf.append(revBuf);
    qDebug() << "rev len:" << revLen << revRsDataBuf.size() <<revRsDataBuf.toHex();
    if((unsigned char)revRsDataBuf[0] == 0xEB)
    {
        if((unsigned char)revRsDataBuf[2] == 0xD0) //curve subscribe success
        {
            qDebug() << "curreadProgramFlagve subscribe success"<<revRsDataBuf.size()<<revRsDataBuf.toHex();
            revRsDataBuf.clear();
            gwkLinkFlag = true;
            reSendTimeCount = 0;
            reSendFlag = false;  //send e0
        }
    }
    else if((unsigned char)revRsDataBuf[0] == 0xE5) //receive program parameter
    {
        if(revRsDataBuf.size()>=3)
        {
            proNameLen = revRsDataBuf[2];
            if(revRsDataBuf.size() == (35+proNameLen))
            {
                programName = revRsDataBuf.mid(3,proNameLen);
                programType = revRsDataBuf.mid(5+proNameLen,1);

                DownLimitValue = hexQBtyeArray_To_QString(revRsDataBuf.mid(8+proNameLen,2),2,100);
                upLimitValue   = hexQBtyeArray_To_QString(revRsDataBuf.mid(10+proNameLen,2),2,100);

                emit sendLimit(QString(programName),upLimitValue,DownLimitValue);
                qDebug() << "program parameter"<<programName<<programType.toHex()<<revRsDataBuf.size()<<revRsDataBuf.toHex();
                QByteArray sendBufTemp;
                revRsDataBuf.clear();
                sendBufTemp[0]=0xD0;
                myCom3->write(sendBufTemp,1);
            }
        }
    }
    else if((unsigned char)revRsDataBuf[0] == 0xE6) //receive curve
    {
        if((unsigned char)revRsDataBuf[revRsDataBuf.size()-1] == 0xE7)
        {
            curveTimerFlag = true;
        }
    }
    else if((unsigned char)revRsDataBuf[0] == 0xE0)//index data
    {
        if(revRsDataBuf.size()>=26)
        {
            programNumber = revRsDataBuf[25]; //program number
            if(programNumber == 0)
            {
            }
            else
            {
                if(revRsDataBuf.size()>(programNumber*31+26))  //program parameter min 31
                {
                    if(readProgramFlag)
                    {
                        readProgramFlag = false;
                        indexLen = 26;
                        for(int i=0;i<programNumber;i++)
                        {
                            proNameLen = revRsDataBuf[indexLen];
                            indexLen = indexLen + 31 + proNameLen;
                        }
                        indexLen += 9;
                        qDebug() <<"*****indexLen******" << indexLen;
                    }
                    if(revRsDataBuf.size() == indexLen)
                    {
                        reSendFlag = false;  //rev e0
                        reSendTimeCount = 0;
                        qDebug() << "indexLen len:" << programNumber << indexLen <<revRsDataBuf.toHex();
                        QByteArray nameTemp="";  //program name
                        QByteArray typeTemp="";  //program type
                        int addr=26;
                        proNameLen = revRsDataBuf[addr];
                        for(int i=0;i<programNumber;i++)
                        {
                            if(i>0)
                            {
                            }
                            qDebug() << "addr:"<<addr<<proNameLen;
                            nameTemp = revRsDataBuf.mid(1+addr,proNameLen);
                            typeTemp = revRsDataBuf.mid(3+proNameLen+addr,1);
                            QByteArray numTemp = revRsDataBuf.mid(30+proNameLen+addr,1); //data number of program
                            addr = addr + proNameLen + 31;
                            proNameLen = revRsDataBuf[addr];
                            qDebug() <<"***index:" <<i<<nameTemp<<typeTemp.toHex()<<numTemp.toHex();
                            if((nameTemp == programName) && (typeTemp == programType))
                            {
                                if((unsigned char)numTemp[0] > 0)
                                {
                                    readDataNO = i + 1;
                                    revRsDataBuf.clear();
                                    sendBuf[0] = 0xe3;
                                    sendBuf[1] = 0;
                                    sendLen = 1;
                                    myCom3->write(sendBuf,sendLen);
                                    reSendFlag = true;   //send e3
                                    qDebug() << "send read data cmd:"<<sendBuf.toHex();
                                    break;
                                }
                                else
                                {
                                    revRsDataBuf.clear();
                                    sendBuf[0]=0xeb;
                                    sendBuf[1] = 0;
                                    sendLen = 1;
                                    myCom3->write(sendBuf,sendLen);
                                    reSendFlag = true;   //send eb
                                    qDebug() << "send subscribe cuver cmd:"<<sendBuf.toHex();
                                }
                            }
                            else
                            {
                                if((i+1) == programNumber)
                                {
                                    revRsDataBuf.clear();
                                    sendBuf[0]=0xeb;
                                    sendBuf[1] = 0;
                                    sendLen = 1;
                                    myCom3->write(sendBuf,sendLen);
                                    reSendFlag = true;   //send eb
                                    qDebug() << "send subscribe cuver cmd:"<<sendBuf.toHex();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if((unsigned char)revRsDataBuf[0] == 0xE3)//start read data cmd
    {
        if((unsigned char)revRsDataBuf[1] == 0x01)
        {
            revRsDataBuf.clear();
            sendBuf[0]=0;
            sendBuf[1]=readDataNO;
            sendLen = 2;
            myCom3->write(sendBuf,sendLen);
            reSendFlag = true;   //send 000x
            reSendTimeCount = 0;
            qDebug() << "send 00 01 cmd:"<<sendBuf.toHex();
        }
    }
    else if((unsigned char)revRsDataBuf[0] == 0xEE)//data read ok ACK
    {
        if((unsigned char)revRsDataBuf[1] == 0x01)
        {
            QByteArray sendBufTemp;
            qDebug() << "rev ee len:" <<revRsDataBuf.toHex();
            revRsDataBuf.clear();
            sendBufTemp[0]=0;
            sendBufTemp[1]=readDataNO;
            myCom3->write(sendBufTemp,2);
            qDebug() << "send clear data cmd:"<<sendBufTemp.toHex();
        }
    }
//    else if((unsigned char)revRsDataBuf[0] == 0xEF)//data clear ok
//    {
//        if((unsigned char)revRsDataBuf[2] == 0xd0)
//        {
//            qDebug() << "rev ef len:" <<revRsDataBuf.toHex();
//            revRsDataBuf.clear();
//            sendBuf[0]=0xeb;
//            myCom3->write(sendBuf,1);
//            qDebug() << "send subscribe cuver cmd:"<<sendBuf.toHex();
//        }
//    }
    else if((unsigned char)revRsDataBuf[0] == 0) //receive data
    {
        recFlag = true;
        if(revRsDataBuf.size()>45)
        {
            proNameLen = revRsDataBuf[2];
            dataNumber = revRsDataBuf[32+proNameLen]; //bolt data number
            dataLen = dataNumber*25 + 2 + 33 + proNameLen;
            if(dataNumber == 0)
            {
                qDebug() << "rev data error:" << revRsDataBuf.size() <<revRsDataBuf.toHex();
                revRsDataBuf.clear();
                sendBuf[0]=0xe0;
                sendBuf[1] = 0;
                sendLen = 1;
                myCom3->write(sendBuf,sendLen);
                reSendFlag = true;     //send eb
                readProgramFlag = true;
                qDebug() << "programNumber = 0 send eb cmd:"<<sendBuf.toHex();
            }
            if(revRsDataBuf.size() == dataLen)
            {
                int addr;
                strDateTime = "";
                reSendFlag = false; //rev 000x
                reSendTimeCount = 0;
                gwkProNO = revRsDataBuf.mid(3,proNameLen);
                if(proNameLen>=3)
                {
                    gwkProNO = gwkProNO.right(2);
                }
                strTmin = hexQBtyeArray_To_QString(revRsDataBuf.mid(8+proNameLen,2),2,100);
                strTmax = hexQBtyeArray_To_QString(revRsDataBuf.mid(10+proNameLen,2),2,100);
                qDebug() << "rev data OK:" << gwkProNO << dataNumber << dataLen <<revRsDataBuf.toHex();
                for(int i=0;i<dataNumber;i++)
                {
                    addr = i*25;
                    strPeakAngle  = hexQBtyeArray_To_QString(revRsDataBuf.mid(proNameLen+33+addr,2),2,10);
                    strPeakTorque = hexQBtyeArray_To_QString(revRsDataBuf.mid(proNameLen+35+addr,2),2,100);
                    strStopAngle  = hexQBtyeArray_To_QString(revRsDataBuf.mid(proNameLen+39+addr,2),2,10);
                    strStopTorque = hexQBtyeArray_To_QString(revRsDataBuf.mid(proNameLen+41+addr,2),2,100);

                    if((unsigned char)revRsDataBuf[proNameLen+51+addr] == 0)
                    {
                        qDebug()<<"********PXX******"<<gwkProNO.toInt()<<programNO;
                        if(gwkProNO.toInt() == programNO)
                        {
                            strResult = "OK";
                        }
                        else
                        {
                            strResult = "NOK";
                        }
                    }
                    else
                    {
                        strResult = "NOK";
                    }
                    strDateTime = "20";
                    charTemp = revRsDataBuf[proNameLen+52+addr];
                    intTemp = charTemp + 100;
                    strDateTime.append(QString::number(intTemp,10).mid(1,2)).append(":");
                    charTemp = revRsDataBuf[proNameLen+53+addr];
                    intTemp = charTemp + 100;
                    strDateTime.append(QString::number(intTemp,10).mid(1,2)).append(":");
                    charTemp = revRsDataBuf[proNameLen+54+addr];
                    intTemp = charTemp + 100;
                    strDateTime.append(QString::number(intTemp,10).mid(1,2)).append(" ");
                    charTemp = revRsDataBuf[proNameLen+55+addr];
                    intTemp = charTemp + 100;
                    strDateTime.append(QString::number(intTemp,10).mid(1,2)).append(":");
                    charTemp = revRsDataBuf[proNameLen+56+addr];
                    intTemp = charTemp + 100;
                    strDateTime.append(QString::number(intTemp,10).mid(1,2)).append(":");
                    charTemp = revRsDataBuf[proNameLen+57+addr];
                    intTemp = charTemp + 100;
                    strDateTime.append(QString::number(intTemp,10).mid(1,2));
                    qDebug() << "peakAngle** A:"<<strPeakAngle;
                    qDebug() << "peakTorque* T:"<<strPeakTorque;
                    qDebug() << "stopAngle** A:"<<strStopAngle;
                    qDebug() << "stopTorque* T:"<<strStopTorque;
                    qDebug() << "strResult*****"<<strResult;
                    qDebug() << "strDateTime***"<<strDateTime;
                    qDebug() << "strTmax*******"<<strTmax;
                    qDebug() << "strTmin*******"<<strTmin;
                }
                if(SYSS == "ING" )
                {
                    DATA_STRUCT demo;
                    demo.data_model[0] = strDateTime.mid(0,10);
                    demo.data_model[1] = strDateTime.mid(11,8);
                    demo.data_model[2] = strResult;
                    demo.data_model[3] = strPeakTorque;
                    demo.data_model[4] = strPeakAngle;
                    demo.data_model[5] = boltNumberBuf;
                    demo.data_model[6] = VIN_PIN_SQL;
                    demo.data_model[7] = "0";    //tightening ID
                    demo.data_model[8] = "Curve is null";
                    demo.data_model[9] = (QString)gwkProNO;//QString::number(programNO);
                    demo.data_model[10] = "0";
                    demo.data_model[11] = AutoNO;
                    if(strResult == "OK")
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]++);
                    }
                    else
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]);
                    }
                    demo.data_model[13] = strTmax;
                    demo.data_model[14] = strTmin;
                    demo.data_model[15] = "0";
                    QVariant DataVar;
                    DataVar.setValue(demo);
                    emit sendfromworkthread(DataVar);
                    emit send_mainwindow(strPeakTorque,strPeakAngle,strResult,0,groupNumCh[0]);
                }

                revRsDataBuf.clear();
                QByteArray sendBufTemp;
                sendBufTemp[0]=0xee;
                myCom3->write(sendBufTemp,1);
                qDebug() << "send data ack cmd:"<<sendBufTemp.toHex();

            }
        }
    }
    else if((unsigned char)revRsDataBuf[0] == 0xd0)//data clear ok
    {
        qDebug() << "rev clear data ACK:" <<revRsDataBuf.toHex();
        revRsDataBuf.clear();
        sendBuf[0]=0xeb;
        sendBuf[1] = 0;
        sendLen = 1;
        myCom3->write(sendBuf,sendLen);
        reSendFlag = true;   //send eb
        qDebug() << "send subscribe cuver cmd:"<<sendBuf.toHex();
    }
}

/*********************************************/
// QByteArray hex to QString
/*********************************************/
QString GwkQuanTecMCS::hexQBtyeArray_To_QString(QByteArray buf,int len, int div)
{
    unsigned char charBuf[6];
    int i,intTemp = 0;
    for(i=0;i<len;i++)
    {
        intTemp = intTemp << 8;
        charBuf[i] = buf[i];
        intTemp += charBuf[i];
    }
    return (QString::number((double)intTemp/div));
//    charTemp = revRsDataBuf[41+addr];
//    intTemp = charTemp;
//    intTemp = intTemp << 8;
//    charTemp = revRsDataBuf[42+addr];
//    intTemp += charTemp;
//    strPeakAngle = QString::number((double)intTemp/10);
}


/**************************************/
//gkw reread data functiong
/**************************************/
void GwkQuanTecMCS::gwkReadDataFunc()
{
    reSendFlag = 0;
    reSendTimeCount = 0;
    revRsDataBuf.clear();
    sendBuf[0]=0xe0;
    sendBuf[1] = 0;
    sendLen = 1;
    myCom3->write(sendBuf,sendLen);
    reSendFlag = true;  //send e0
    readProgramFlag = true;
    qDebug() << "send gwk init cmd:"<<sendBuf.toHex();
}






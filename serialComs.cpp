#include "serialComs.h"
#include "qsettings.h"

SerialComs::SerialComs(QObject *parent) :
    QObject(parent)
{
    mMESUploadSVW2 = 0;
    mIoBoxIn[0] = 0;
    mIoBoxIn[1] = 0;

    whichindex = 0;
    this->moveToThread(&m_thread);
    m_thread.start();

    mAQCHERYState  = 0;

    connect(&mComTimer, SIGNAL(timeout()), this, SLOT(slot_ComTimeout()));
    mComTimer.setSingleShot(true);
    mComTimer.setInterval(200);

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    mPartCode = configIniRead->value("baseinfo/PartCode").toBool();
    delete configIniRead;
}

void SerialComs::comInit()
{
    system("echo 1 > /root/gpio/COM0");
    Factory = factory;
    seriallist.clear();
    Queuelock.lockForRead();
    if(isBarCode||isQueue||Factory =="Dongfeng")
    {
        Queuelock.unlock();
        const char *dev_name;
        if(Factory == "SVW2" && ControlType_1 == "GWK")
        {
            dev_name = "/dev/ttymxc0";
        }
        else if(ControlType_1 == "PF_232")
        {
            dev_name = "/dev/ttymxc0";
        }
        else
        {
#if 0
            //dev_name = "/dev/ttymxc3";
#else

            dev_name = "/dev/ttyS0";
#endif
        }
         //dev_name = "/dev/ttyUSB0";

        myCom = new QextSerialPort(dev_name);
        connect(myCom, SIGNAL(readyRead()), this, SLOT(readCom()));

        //设置波特率
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

        if(myCom->open(QIODevice::ReadWrite)){
            DTdebug()<<QObject::tr("serial open ok ") << dev_name;

        }else{
            DTdebug()<<QObject::tr("serial open fail ") << dev_name;
        }

        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);

        if(Factory == "NOVAT")
        {
            vinLen = 17;
            vinHead = "L";
        }

        else if(Factory == "GM" ||Factory == "Haima")
        {
            vinLen = configIniRead->value("baseinfo/vinLen").toInt();
            vinHead = configIniRead->value("baseinfo/vinHead").toByteArray();            
            delete configIniRead;
        }
        else if(isReplaceBarcode)
        {
            vinLen = 15;
            vinHead = "5";
        }
        else if(Factory == "SVW2" && ((ControlType_1 == "CS351")||(ControlType_1 == "Desoutter"))&& Special_Station)
        {
            vinLen = 14;
            vinHead = configIniRead->value("baseinfo/vinHead").toByteArray();   //vinHead = "7"
        }
        else
        {
            vinLen = 17;
            vinHead = "L";
        }
    }
    else
        Queuelock.unlock();
}


void SerialComs::readCom()
{
    if(gMessageboxShow)
    {
        myCom->readAll();
        mBuffer.clear();
        return;
    }

    if(Factory == "KFCHERY")
    {
        DTdebug() << "mIoBoxIn[0]" << mIoBoxIn[0];
        if(!mIoBoxIn[0])
        {
            myCom->readAll();
            return;
        }
    }

    mBuffer.append(myCom->readAll());
    mComTimer.start();
}

QString optionNum;

void SerialComs::slot_ComTimeout()
{
    if(Factory == "AQCHERY" || Factory == "KFCHERY")
    {
        if(mBuffer.endsWith(0x0d))
            buff = mBuffer.mid(0, mBuffer.size()-1);
        else
            buff = mBuffer;

        mBuffer.clear();
        DTdebug()<<  "factory = CHERY" <<  buff;

        int len_l = 17;
        int len_c = 15;
        if(Factory == "KFCHERY"){
            len_c = 17;
        }
        if(mPartCode == 1)
        {
            if(buff.size() != len_l)
            {
                DTdebug() << buff ;
                return;
            }
            isEqual = false;
            if(seriallist.isEmpty())
            {
                seriallist.push_back(buff);
                whichindex++;
            }
            else
            {
                QList<QString>::iterator i;
                for (i = seriallist.begin(); i != seriallist.end(); ++i)
                {
                    if(!(QString::compare(buff, *i, Qt::CaseSensitive)))
                    {
                        isEqual = true;
                        break;
                    }
                }
                if(!isEqual)
                {
                    if(seriallist.size() == 10)
                    {
                        seriallist[whichindex] = buff;
                    }
                    else
                    {
                        seriallist.push_back(buff);
                    }
                    whichindex++;
                    if(whichindex == 10)
                        whichindex = 0;
                }
            }
            DTdebug()<<"send PartCode SerialNumbs "<<buff;
            emit serialCom(buff,isEqual,"BarCode");
        }
        else
        {
            if(mAQCHERYState == 0)
            {
                if(buff.size() != len_l)
                    return;
                mAQCHERYState = 1;
                mAQCHERYString = buff;
                return;
            }
            else
            {
                if(buff.size() != len_c)
                    return;
                mAQCHERYState = 0;
                mAQCHERYString.append(buff);
            }
            isEqual = false;
            if(seriallist.isEmpty())
            {
                seriallist.push_back(mAQCHERYString);
                whichindex++;
            }
            else
            {
                QList<QString>::iterator i;
                for (i = seriallist.begin(); i != seriallist.end(); ++i)
                {
                    if(!(QString::compare(mAQCHERYString, *i, Qt::CaseSensitive)))
                    {
                        isEqual = true;
                        break;
                    }
                }
                if(!isEqual)
                {
                    if(seriallist.size() == 10)
                    {
                        seriallist[whichindex] = mAQCHERYString;
                    }
                    else
                    {
                        seriallist.push_back(mAQCHERYString);
                    }
                    whichindex++;
                    if(whichindex == 10)
                        whichindex = 0;
                }
            }
            DTdebug()<<"send SerialNumbs "<<mAQCHERYString;
            emit serialCom(mAQCHERYString,isEqual,"VIN+CarType");
        }
    }
    else if(Factory == "NOVAT")
    {
        int index = mBuffer.indexOf(0x03);
        if(index < 0)
        {
            return;
        }
        buff = mBuffer.mid(0, index+1);  //!...
        DTdebug() << buff ;
        mBuffer.clear();

        DTdebug()<<  "factory = NOVAT" <<  buff;
        int index1;
        index1 = buff.indexOf('!');
        if(index1 <= 0)
            return;
        QString tmpCode = buff.mid(1,index1-1);
        QByteArray serialNums = tmpCode.right(17).toLatin1();
        QString carMatch = tmpCode.left(8);
        optionNum = tmpCode.mid(0,3);  //match option bxcode

        //buff.clear();
        if((serialNums.size() >= vinLen)&&(serialNums[0]==vinHead[0]))
        {
            if(DebugMode)
            {
                emit sendDebug(QString(buff));
            }
            //CsIsConnect = true; //debug
            if(CsIsConnect && !ISmaintenance && SYSS!="ING"&& isBarCode) // notice by wxm 2020-8-3
            {

                isEqual = false;
                if(seriallist.isEmpty())
                {
                    seriallist.push_back(serialNums);
                    whichindex++;
                }
                else
                {
                    QList<QString>::iterator i;
                    for (i = seriallist.begin(); i != seriallist.end(); ++i)
                    {
                        if(!(QString::compare(serialNums, *i, Qt::CaseSensitive)))
                        {
                            isEqual = true;
                            break;
                        }
                    }
                    if(!isEqual)
                    {
                        if(seriallist.size() == 10)
                        {
                            seriallist[whichindex] = serialNums;
                        }
                        else
                        {
                            seriallist.push_back(serialNums);
                        }
                        whichindex++;
                        if(whichindex == 10)
                            whichindex = 0;
                    }
                }

                DTdebug()<<"send SerialNumbs "<<serialNums;
                emit serialCom(serialNums+carMatch,isEqual,"serialNums+carMatch");
            }
        }
        else
        {
            DTdebug()<<"wrong SerialNums "<<serialNums;
        }
    }
    else
    {
#if 0
        int index = mBuffer.indexOf(0x0d);
        if(index < 0)
        {
            DTdebug() << mBuffer ;
            return;
        }
        buff = mBuffer.mid(0, index+1);
        DTdebug() << buff ;
        mBuffer.clear();
#else
        if(mBuffer.size()<14)
        {
            DTdebug() << mBuffer ;
            mBuffer.clear();
            return;
        }
        buff = mBuffer;
#endif
        DTdebug() <<"buff.toHex():"<<buff.toHex();
        mBuffer.clear();
        if(Factory == "Benz"||((Factory =="BYDXA")&& isBarCode))   //用于长沙BYD
        {
            int len = 0;
            if(Factory == "Benz")
                len = 8;
            else if(Factory == "BYDXA")
                len = 18;
            //                    DTdebug()<<"buff<<"<<buff.size()<<buff;
            for(int i=0;i<buff.size();i++)
            {
                if((char)buff[i] == 0x0d)
                {
                    QString serialNums = buff.left(i);
                    buff.clear();
                    //                            buff = buff.right(buff.size()-i-1);
                    if (serialNums.size()== len)
                    {
                        if(CsIsConnect && !ISmaintenance && SYSS!="ING"&& isBarCode)
                        {
                            isEqual = false;
                            if(seriallist.isEmpty())
                            {
                                seriallist.push_back(serialNums);
                                whichindex++;
                            }
                            else
                            {
                                QList<QString>::iterator i;
                                for (i = seriallist.begin(); i != seriallist.end(); ++i)
                                {
                                    if(!(QString::compare(serialNums, *i, Qt::CaseSensitive)))
                                    {
                                        isEqual = true;
                                        break;
                                    }
                                }
                                if(!isEqual)
                                {
                                    if(seriallist.size() == 10)
                                    {
                                        seriallist[whichindex] = serialNums;
                                    }
                                    else
                                    {
                                        seriallist.push_back(serialNums);
                                    }
                                    whichindex++;
                                    if(whichindex == 10)
                                        whichindex = 0;
                                }
                            }

                            DTdebug()<<"send SerialNumbs "<<serialNums;
                            emit serialCom(serialNums,isEqual," ");
                        }
                    }
                    else
                    {
                        DTdebug()<<"wrong SerialNums "<<serialNums;
                    }
                    break;
                }
            }
            if(buff.size()>30)
            {
                buff.clear();
            }
        }
        else if(!manualMode)
        {
            if(Special_Station)
            {
                DTdebug()<< "Special_Station ="<<Special_Station;

                if((buff.size() >= 14)) //pin code 14byte
                {
                    QString serialNums;
                    QString pinCode = QString(buff.mid(0,(14)));
                    QString tmpWerk = pinCode.mid(0, 2);
                    QString tmpSPJ = pinCode.mid(2, 4);
                    QString tmpKNR = pinCode.mid(6);
                    //fis
                    if(mMESUploadSVW2)
                    {
                        serialNums = mMESUploadSVW2->GetVIN(tmpWerk, tmpSPJ, tmpKNR);
                        DTdebug()<< "mMESUploadSVW2 GetVIN ="<<serialNums;
                    }

                    if(serialNums.size() != 17)
                    {
                        DTdebug()<< "mMESUploadSVW2 GetVIN is error";
                        buff.clear();
                        return;
                    }

                    QString tmpCompareCode = pinCode; //serialNums

                    isEqual = false;
                    if(seriallist.isEmpty())
                    {
                        seriallist.push_back(tmpCompareCode);
                        whichindex++;
                    }
                    else
                    {
                        QList<QString>::iterator i;
                        for (i = seriallist.begin(); i != seriallist.end(); ++i)
                        {
                            if(!(QString::compare(tmpCompareCode, *i, Qt::CaseSensitive)))
                            {
                                isEqual = true;
                                break;
                            }
                        }
                        if(!isEqual)
                        {
                            if(seriallist.size() == 10)
                            {
                                seriallist[whichindex] = tmpCompareCode;
                            }
                            else
                            {
                                seriallist.push_back(tmpCompareCode);
                            }
                            whichindex++;
                            if(whichindex == 10)
                                whichindex = 0;
                        }
                    }

                    emit serialCom(pinCode+serialNums,isEqual,"VIN+FIS");
                    DTdebug() << "VIN+FIS code"<< buff;
                    buff.clear();
                }
                else
                {
                    buff.clear();
                }
            }
            else
            {
                while( (!buff.isEmpty())&&(!buff.startsWith(vinHead)) )
                {
                    buff.replace(0,1,"");
                }
                DTdebug()<<"not Start with :"<<vinHead<<buff.toHex();

                if((buff.size() >= vinLen)&&(buff[0]==vinHead[0]))
                {
                    DTdebug()<<SaveWhat<<isQueue;
                    buff[vinLen] = '\0';
                    QString serialNums = QString(buff);
                    DTdebug() << "tiaoma qiang"<< serialNums;
    #if 1
                    if(DebugMode)
                    {
                        emit sendDebug(QString(buff));
                    }
    #endif
                    Queuelock.lockForRead();
                    if(Factory == "Dongfeng")
                    {
                        if(SaveWhat == "delete_car")
                            emit deleteCar(serialNums);
                        else
                            emit alignCar(serialNums);
                    }
                    else if(SaveWhat == "delete_car")
                    {
                        Queuelock.unlock();
                        emit serialCom(serialNums,false,"VIN");
                    }
                    else if(isQueue)
                    {
                        Queuelock.unlock();
                        if(SaveWhat.mid(0,8) != "queue_no")
                        {
                            emit serialCom(serialNums,false,"VIN");
                        }
                    }

                    else if(CsIsConnect && !ISmaintenance  && SYSS!="ING"&& isBarCode)
                    {
                        Queuelock.unlock();
                        if((ControlType_1 == "SB356_PLC")&&(!PLCIsConnect))
                        {

                        }
                        else
                        {
                            if(SerialGunMode)
                            {
                                if(Factory == "Haima"||(Factory == "SVW2" && ControlType_1 == "GWK"))
                                    system("echo 1 > /sys/class/leds/control_uart1/brightness");
                                else
                                    system("echo 1 > /sys/class/leds/control_uart2/brightness");
                            }

                            isEqual = false;
                            if(seriallist.isEmpty())
                            {
                                seriallist.push_back(serialNums);
                                whichindex++;
                            }
                            else
                            {
                                QList<QString>::iterator i;
                                for (i = seriallist.begin(); i != seriallist.end(); ++i)
                                {
                                    if(!(QString::compare(serialNums, *i, Qt::CaseSensitive)))
                                    {
                                        isEqual = true;
                                        break;
                                    }
                                }
                                if(factory == "GZBAIC")
                                {
                                    isEqual = false ;
                                }

                                if(!isEqual)
                                {
                                    if(seriallist.size() == 10)
                                    {
                                        seriallist[whichindex] = serialNums;
                                    }
                                    else
                                    {
                                        seriallist.push_back(serialNums);
                                    }
                                    whichindex++;
                                    if(whichindex == 10)
                                        whichindex = 0;
                                }
                            }
                            DTdebug()<<"send SerialNumbs "<<serialNums<<isEqual;
                            emit serialCom(serialNums,isEqual,"BarCode");
                        }
                    }
                    else
                        Queuelock.unlock();
                    buff.clear();
                }
            }
        }
        else if(manualMode && Factory=="Dongfeng")
        {
            for(int i=0;i<buff.size();i++)
            {
                if((char)buff[i] == 0x0d)
                {
                    DTdebug()<<"&&&&&&&&&&&&&&&"<<buff;
                    QString serialNums = buff.left(i);
                    buff.clear();
                    while( (!serialNums.isEmpty())&&(!(((serialNums[0] >='a') && (serialNums[0]  <= 'z')) ||((serialNums[0]  >='A') && (serialNums[0]  <= 'Z'))||((serialNums[0]  >='0')&&(serialNums[0] <= '9'))) ) )
                    {
                        DTdebug()<<"not a number or letter"<<serialNums;
                        serialNums.replace(0,1,"");
                        //                    DTdebug() << "*******clear********" <<buff;
                    }

                    //                            buff = buff.right(buff.size()-i-1);
                    if (serialNums.size()==17)
                    {
                        if(CsIsConnect && !ISmaintenance  && SYSS!="ING" && (serialNums.left(1)!="L") )
                        {
                            isEqual = false;
                            if(seriallist.isEmpty())
                            {
                                seriallist.push_back(serialNums);
                                whichindex++;
                            }
                            else
                            {
                                QList<QString>::iterator i;
                                for (i = seriallist.begin(); i != seriallist.end(); ++i)
                                {
                                    if(!(QString::compare(serialNums, *i, Qt::CaseSensitive)))
                                    {
                                        isEqual = true;
                                        break;
                                    }
                                }
                                if(!isEqual)
                                {
                                    if(seriallist.size() == 10)
                                    {
                                        seriallist[whichindex] = serialNums;
                                    }
                                    else
                                    {
                                        seriallist.push_back(serialNums);
                                    }
                                    whichindex++;
                                    if(whichindex == 10)
                                        whichindex = 0;
                                }
                            }

                            DTdebug()<<"send SerialNumbs "<<serialNums<<isEqual;
                            emit serialCom(serialNums,isEqual,"BarCode");
                        }
                    }
                    else
                    {
                        DTdebug()<<"wrong SerialNums "<<serialNums;
                    }
                    break;
                }
            }
            if(buff.size()>30)
                buff.clear();
        }
    }
}

void SerialComs::slot_IoBoxInput(int pState)
{
    int tmpIn0 = pState & 0x00000001;
    int tmpIn1 = (pState>>1) & 0x00000001;

    DTdebug() << "tmpIn0" << tmpIn0;
    DTdebug() << "tmpIn1" << tmpIn1;

    if(tmpIn0 && (mIoBoxIn[0] != tmpIn0))//IO盒子有新的IN0(进站)信号时
    {
        GAbnormalOut_KF = true;
        mIoBoxIn[0] = 1;
        mIoBoxIn[1] = 0;
//        DTdebug() << "signalSetIoBox all off";
//        emit signalSetIoBox(0x7f);//0x7f实际没啥参考意义，只要不是0~7(指定输出IO点)的数都是全置0
    }
    if(tmpIn1 && (mIoBoxIn[1] != tmpIn1))//IO盒子有新的IN1(出站)信号时
    {
        mIoBoxIn[1] = 1;
        mIoBoxIn[0] = 0;
        DTdebug() << "mIoBoxIn[1]" << mIoBoxIn[1];
        if(GAbnormalOut_KF)//未拧紧完成，有出站信号就停线
        {
            DTdebug() << "signalSetIoBox out0";
            emit signalSetIoBox(0,true);//停线 0: 指定OUT0置1
        }
//        else
//        {
//            DTdebug() << "signalSetIoBox all off";
//            emit signalSetIoBox(0x7f);//0x7f实际没啥参考意义，只要不是0~7(指定输出IO点)的数都是全置0
//        }
    }
}


void SerialComs::slots_deleteCachedVIN(QByteArray code)
{
    QString code_str = QString(code);
    seriallist.removeAll(code_str);
}

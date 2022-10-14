#include "gwkPLC.h"

GwkThread::GwkThread(QObject *parent) :
    QObject(parent)
{
    channel = "";
    vinBuf = "";
    boltNumberBuf = "";
    programNO = "";
    boltNum    = 0;
    gwkStartFlag = false;
    gwkCloseFlag = false;
    revRsDataBuf = "";
    sendRsDataBuf = "";
    Cycle = 0;
    m_thread.start();
    this->moveToThread(&m_thread);
}

void GwkThread::gwkstart()
{
    qDebug() << "&&&&&& GWK thread start &&&" ;
    QString portName = "/dev/ttymxc3";   //获取串口名
    myCom3 = new QextSerialPort(portName);
    connect(myCom3, SIGNAL(readyRead()), this, SLOT(slot_read_com()));

    //设置波特率
    myCom3->setBaudRate((BaudRateType)9600);

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
        qDebug()<<"serial ttymxc3 open successs";
        if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
        {
            SYSS = "Ready";
        }
        emit IsTigntenReady(true);
        gwkSendCmdFun(0xAF);   //send stop cmd
    }
    else
    {
        qDebug()<<"serial ttymxc3 open fail XXXXXXXXXXXXXXX " ;
        emit IsTigntenReady(false);
    }

    QSettings *config = new QSettings("/config_bound.ini", QSettings::IniFormat);
    QString ToolName = config->value("/GWKPLC/ToolName").toString();
    short SnugTorque      = config->value("/GWKPLC/SnugTorque").toInt();
    short LooseTorque     = config->value("/GWKPLC/LooseTorque").toInt();
    short Angle           = config->value("/GWKPLC/Angle").toInt();
    short AngleMin        = config->value("/GWKPLC/AngleMin").toInt();
    short AngleMax        = config->value("/GWKPLC/AngleMax").toInt();
    short Torque          = config->value("/GWKPLC/Torque").toInt();
    TorqueMin             = config->value("/GWKPLC/TorqueMin").toInt();
    TorqueMax             = config->value("/GWKPLC/TorqueMax").toInt();
    short Parameter       = config->value("/GWKPLC/Parameter").toInt();
    short TightenMode     = config->value("/GWKPLC/TightenMode").toInt();
    delete config;
    sendRsDataBuf = ToolName.toLatin1();
    for(int i=ToolName.size();i<20;i++)
    {
        sendRsDataBuf[i] = 0;
    }
    sendRsDataBuf[20] = SnugTorque>>8;
    sendRsDataBuf[21] = SnugTorque;     //snug torque
    sendRsDataBuf[22] = LooseTorque>>8;
    sendRsDataBuf[23] = LooseTorque;     //loose torque
    sendRsDataBuf[24] = Angle>>8;
    sendRsDataBuf[25] = Angle;     //nominal angle
    sendRsDataBuf[26] = AngleMin>>8;
    sendRsDataBuf[27] = AngleMin;     //LAL
    sendRsDataBuf[28] = AngleMax>>8;
    sendRsDataBuf[29] = AngleMax;     //UAL
    sendRsDataBuf[30] = Torque>>8;
    sendRsDataBuf[31] = Torque;     //nominal Torque
    sendRsDataBuf[32] = TorqueMin>>8;
    sendRsDataBuf[33] = TorqueMin;     //LTL
    sendRsDataBuf[34] = TorqueMax>>8;
    sendRsDataBuf[35] = TorqueMax;     //UTL
    sendRsDataBuf[36] = 0;      //space for later extensions
    sendRsDataBuf[37] = Parameter;      //parameter   o Bit 0 always set to 1
    //            o Bit 1 set to 0 tightening direction CW
    //            o Bit 1 set to 1 tightening direction CCW
    //            o Bit 2 set to 0 torque measuring unit Nm
    //            o Bit 2 set to 1 torque measuring unit Footpound
    //            o Bit 4,5,6,7 always set to 0
    sendRsDataBuf[38] = TightenMode;   //tightening mode  torque = 72;angle = 73
    short sum = 0;
    for(int i=0;i<39;i++)
    {
        sum += sendRsDataBuf[i];
    }
    sendRsDataBuf[39] = sum >> 8;
    sendRsDataBuf[40] = sum;

    if(!db2.isOpen() || !QSqlDatabase::contains("GWKPlcMySQL") )
        mysql_open();
    if(db2.isOpen() && QSqlDatabase::contains("GWKPlcMySQL"))
    {
        if(!query2.exec("SELECT Cycle FROM "+Localtable +" Where Cycle !='-1' order by RecordID DESC limit 1"))
        {
            qDebug()<<"SELECT Cycle fail "<<query2.lastError();
        }
        else
        {
            if(query2.next())
            {
                Cycle = query2.value(0).toInt();
            }
            else
                qDebug()<<"There is no Cycle !=-1 ";
        }
    }
    else
    {
        qDebug()<<"db2 is not open or gwkPLC connect fail";
    }
}

// 本地mysql open
void GwkThread::mysql_open()
{
    if(QSqlDatabase::contains("GWKPlcMySQL")){
        db2 = QSqlDatabase::database("GWKPlcMySQL");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","GWKPlcMySQL");
        db2.setHostName("localhost");
        db2.setDatabaseName("Tighten");
        db2.setUserName("root");
        db2.setPassword("123456");
        query2 = QSqlQuery(db2);
    }

    if(!db2.open())
    {
        if(!db2.open())
        {
            qDebug()<< "gwkPLC localmysql "<< db2.lastError().text();
        }else
        {
            qDebug()<< "gwkPLC localmysql open ok 2";
        }
    }else
    {
        qDebug()<< "gwkPLC localmysql open ok 1";
    }
}

//串口读取
void GwkThread::slot_read_com()
{
    //从串口中读取数据
    QByteArray revBuf;
    int revLen;
    revLen=myCom3->bytesAvailable();
    revBuf = myCom3->readAll();
    revRsDataBuf.append(revBuf);
    qDebug()<<"*****com3 rev data:"<<revRsDataBuf.size()<<revRsDataBuf.toHex();
    if (revLen > 0)
    {
        if((unsigned char)revRsDataBuf[0] == 0xA3)
        {
            myCom3->write(sendRsDataBuf,41);
            qDebug()<<"***send com3 data:"<<sendRsDataBuf.size()<<sendRsDataBuf.toHex();
            revRsDataBuf.clear();
        }
        else if((unsigned char)revRsDataBuf[0] == 0xAF)
        {
            revRsDataBuf.clear();
        }
        else if((unsigned char)revRsDataBuf[0] == 0xD2)//config success
        {
            revRsDataBuf.clear();
        }
        else if((unsigned char)revRsDataBuf[0] == 0xD3)//config fail
        {
            revRsDataBuf.clear();
        }
        else
        {
            if(((char)revRsDataBuf[0] == 0x4d)&&(revRsDataBuf.size() >= 20)) //receive finished
            {
                QDateTime tighenTime = QDateTime::currentDateTime();//获取系统现在的时间
                qDebug()<<"*****rev data:"<<revRsDataBuf.size()<<revRsDataBuf;
                torqueValue = revRsDataBuf.mid(2,5);
                torqueValue = QByteArray::number(torqueValue.toDouble());
                angleValue  = revRsDataBuf.mid(10,3);
                angleValue = QByteArray::number(angleValue.toDouble());
                gwkStatus   = revRsDataBuf.mid(16,3);
                if((gwkStatus == "092")||(gwkStatus == "220"))
                {
                    gwkStatus = "OK";
                }
                else
                {
                    gwkStatus = "NOK";
                }
                if(SYSS == "ING")
                {
                    Cycle++;
                    qDebug() << "reslut:"<<Cycle<<torqueValue << angleValue << gwkStatus <<tighenTime.toString();
                    DATA_STRUCT demo;
                    demo.data_model[0] = tighenTime.toString("yyyy-MM-dd");
                    demo.data_model[1] = tighenTime.toString("hh:mm:ss");
                    demo.data_model[2] = QString(gwkStatus);
                    demo.data_model[3] = QString(torqueValue);
                    demo.data_model[4] = QString(angleValue);
                    demo.data_model[5] = boltNumberBuf;
                    demo.data_model[6] = vinBuf;
                    demo.data_model[7] = QString::number(Cycle);     //TightenID
                    demo.data_model[8] = "Curve is null";
                    demo.data_model[9] = programNO;
                    demo.data_model[10] = "0";    //channel

                    demo.data_model[11] = Type;
                    if(gwkStatus == "OK")
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]++);
                    }
                    else
                    {
                        demo.data_model[12] = QString::number(BoltOrder[0]);
                    }
                    demo.data_model[13] = QString::number(TorqueMax);
                    demo.data_model[14] = QString::number(TorqueMin);
                    demo.data_model[15] = "0";     //JobStatus

                    QVariant DataVar;
                    DataVar.setValue(demo);
                    emit sendfromworkthread(DataVar);
                    emit send_mainwindow(QString(torqueValue),QString(angleValue),QString(gwkStatus),0,groupNumCh[0]);
                }
                revRsDataBuf.clear();
            }
        }
    }
}

/****************************************************/
//funtion : gwk send cmd
/****************************************************/
void GwkThread::gwkSendCmdFun(char Cmd)
{
    QByteArray sendCmd =" ";
    sendCmd[0] = Cmd;
    myCom3->write(sendCmd,1);
    qDebug() << "****com3 send cmd: " << sendCmd.size()<<sendCmd.toHex();
}
/****************************************************/
//funtion : receive enable
/****************************************************/
void GwkThread::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
        revRsDataBuf.clear();
        gwkSendCmdFun(0xAF);   //send stop cmd
    }
    else
    {
        channel = "0";
        programNO = carInfor[0].proNo[n];// proNum.toShort();
        boltNumberBuf = carInfor[0].boltSN[n];//SCREWID_SQL.toLatin1();
        boltNum = carInfor[0].boltNum[n];//lsNum.toShort();
        if(VIN_PIN_SQL == vinBuf)
        {
        }
        else
        {
            vinBuf = VIN_PIN_SQL.toLatin1();
        }
        revRsDataBuf.clear();
        gwkSendCmdFun(0xA3);   //send start cmd
    }
}

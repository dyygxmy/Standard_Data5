#include "tighten_json_atlas_pm600.h"

TightenJsonAtlasPM600::TightenJsonAtlasPM600(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void TightenJsonAtlasPM600::init()
{
    qDebug() <<"PF600 thread start!!";
    QString ControllerIp = ControllerIp_1;
    isRepair = false;
    ErrorCode = true;
    sessionId = 0;
    torqueValue = "";
    angleValue = "";
    resultStatus = "NOK";
    programNO = "0";
    tightenTime = "";
    tighteningID = 0;
    tighteningID_Max = 0;
    aliveCount = 0;
    PF_Connect_Flag = false;
    powerONFlag = true;

    boltNumberBuf = "";
    vinBuf = "";
    boltCount = 0 ;
    proNum = 0;
    Channel = 0;

    manager = new QNetworkAccessManager(this);
    //    QNetworkRequest req;
    req.setUrl(QUrl("http://"+ControllerIp+"/cgi-bin/rcf.cgi"));
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    connect (&PostTimer,SIGNAL(timeout()),this,SLOT(dataPost()));
    PostTimer.start(1000);
    system("echo 0 > /sys/class/leds/OUTD5/brightness");       //input3
    system("echo 0 > /sys/class/leds/OUTD6/brightness");       //input2
    system("echo 0 > /sys/class/leds/OUTD7/brightness");       //input1
    system("echo 0 > /sys/class/leds/OUTC0/brightness");       //input0
}

//使能
void TightenJsonAtlasPM600::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
        system("echo 0 > /sys/class/leds/OUTD5/brightness");       //input3
        system("echo 0 > /sys/class/leds/OUTD6/brightness");       //input2
        system("echo 0 > /sys/class/leds/OUTD7/brightness");       //input1
        system("echo 0 > /sys/class/leds/OUTC0/brightness");       //input0
        qDebug() << "reset PF Tool Tightening disable = 1";
    }
    else
    {
        Channel = 1;
        proNum = carInfor[0].proNo[n].toShort();// proNum;
        boltCount = carInfor[0].boltNum[n];
        boltNumberBuf = carInfor[0].boltSN[n];//SCREWID_SQL;
        vinBuf = VIN_PIN_SQL;
        qDebug() << "send enable" << proNum << boltCount;
        if(proNum&0x01)
        {
            system("echo 1 > /sys/class/leds/OUTC0/brightness");       //input0
        }
        else
        {
            system("echo 0 > /sys/class/leds/OUTC0/brightness");       //input0
        }
        if(proNum&0x02)
        {
            system("echo 1 > /sys/class/leds/OUTD7/brightness");       //input1
        }
        else
        {
            system("echo 0 > /sys/class/leds/OUTD7/brightness");       //input1
        }
        if(proNum&0x04)
        {
            system("echo 1 > /sys/class/leds/OUTD6/brightness");       //input2
        }
        else
        {
            system("echo 0 > /sys/class/leds/OUTD6/brightness");       //input2
        }
        if(proNum&0x08)
        {
            system("echo 1 > /sys/class/leds/OUTD5/brightness");       //input3
        }
        else
        {
            system("echo 0 > /sys/class/leds/OUTD5/brightness");       //input3
        }
    }
}
/*************************************************/
//receive PF600 data
/*************************************************/
void TightenJsonAtlasPM600::finishedSlot(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        qDebug()<<"**********pf600 data:"<<bytes;
        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(bytes, &ok).toMap();
        if (!ok) 
        {
            qDebug()<<"An error occurred during parsing"<<bytes;
        }
        else 
        {
            aliveCount = 0;
            QVariantMap response = result["response"].toMap();
            if(response["errorCode"].toBool())
            {
                if(PF_Connect_Flag)
                {
                    PF_Connect_Flag = false;
                    emit IsTigntenReady(false);
                }
                ErrorCode = true;
                qDebug()<<"return error"<<bytes;
            }
            else
            {
                if(!PF_Connect_Flag)
                {
                    PF_Connect_Flag = true;
                    if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
                    {
                        SYSS = "Ready";
                    }
                    emit IsTigntenReady(true);
                }
                sessionId = response["sessionId"].toInt();
                ErrorCode = false;
                foreach (QVariant data, response["data"].toList())
                {
                    QVariantMap dataMap = data.toMap();
                    double torqueMax = 0.0;
                    double torqueMin = 0.0;
                    torqueValue = dataMap["finalTorque"].toString();
                    angleValue = dataMap["finalAngle"].toString();
                    programNO = dataMap["psetName"].toString().mid(5);
                    tighteningID = dataMap["id"].toInt();
                    dataMap["status"].toBool()?resultStatus="NOK":resultStatus="OK";//0=OK;1=NOK;
                    tightenTime = dataMap["timeStamp"].toString();
                    if(powerONFlag)
                    {
                        powerONFlag = false;
                        if(tighteningID > 0)
                        {
                            tighteningID_Max = tighteningID;
                            qDebug()<<"power on tighteningID_Max init:"<<tighteningID_Max;
                        }
                    }
                    if(resultStatus =="OK")
                    {
                        if(bound_enabled)
                        {
                            QSettings *config = new QSettings("/config_bound.ini", QSettings::IniFormat);
                            double torque_max = config->value("/ProNumber"+programNO+"/Torque_max").toDouble();
                            double torque_min = config->value("/ProNumber"+programNO+"/Torque_min").toDouble();
                            double Angle_max = config->value("/ProNumber"+programNO+"/Angle_max").toDouble();
                            double Angle_min = config->value("/ProNumber"+programNO+"/Angle_min").toDouble();
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
                                resultStatus = "NOK";
                            }
                        }
                    }
                    qDebug()<<"tightening result:"<<tighteningID_Max<<tighteningID<<programNO<<torqueValue<<angleValue<<tightenTime;
                    /*******************************
                     * 数据模型
                    data_model[0] = DATE_blc;
                    data_model[1] = TIME_blc;
                    data_model[2] = STATE_blc;
                    data_model[3] = MI_blc;
                    data_model[4] = WI_blc;
                    data_model[5] = SCREWID_SQL;
                    data_model[6] = VIN_PIN_SQL;
                    data_model[7] = Cycle_CSR;
                    data_model[8] = CURVE_STR;
                    data_model[9] = Program;
                    data_model[10] = JobStatus;
                    data_model[11] = AutoNO;
                    data_model[12] = Order;
                    data_model[13] = MaxValue;
                    data_model[14] = MinValue;
                    data_model[15] = Channel;
                    *******************************/
                    if((tighteningID_Max != tighteningID)&&(SYSS == "ING"))
                    {
                        tighteningID_Max = tighteningID;
                        DATA_STRUCT demo;
                        demo.data_model[0] = tightenTime.mid(0,10);
                        demo.data_model[1] = tightenTime.mid(11,8);
                        demo.data_model[2] = resultStatus;
                        demo.data_model[3] = torqueValue;
                        demo.data_model[4] = angleValue;
                        demo.data_model[5] = boltNumberBuf;
                        demo.data_model[6] = vinBuf;
                        demo.data_model[7] = QString::number(tighteningID);
                        demo.data_model[8] = "Curve is null";
                        demo.data_model[9] = programNO;
                        demo.data_model[10] = "0";
                        demo.data_model[11] = Type;
                        if(resultStatus == "OK")
                        {
                            demo.data_model[12] = QString::number(BoltOrder[0]++);
                        }
                        else
                        {
                            demo.data_model[12] = QString::number(BoltOrder[0]);
                        }
                        demo.data_model[13] = QString::number(torqueMax);
                        demo.data_model[14] = QString::number(torqueMin);
                        demo.data_model[15] = "1";
                        if(isRepair)
                        {
                            demo.data_model[16] = Line_ID;
                            demo.data_model[17] = Station;
                            demo.data_model[18] = WirelessIp;
                        }

                        QVariant DataVar;
                        DataVar.setValue(demo);
                        emit sendfromworkthread(DataVar);
                        emit send_mainwindow(torqueValue,angleValue,resultStatus,0,groupNumCh[0]);
                    }
                    else
                    {
                        qDebug()<<"***************data repeat*****************";
                    }
                }
            }
        }
    }
    else
    {
        qDebug()<<"PF600 handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        qDebug( "PF600 error found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        //         qDebug(qPrintable(reply->errorString()));
        qDebug()<<"PF600 error"<<reply->errorString();
    }
    reply->deleteLater();
    //     dataInsert();
}

void TightenJsonAtlasPM600::dataPost()
{
//    qDebug()<<"******1****request data:"<<aliveCount;
    QByteArray method = "";
    if(ErrorCode)
    {
        method = "method=controller_login&sessionId=0&username=hmi&password=hult&vsId=0";     //登录
    }
    else
    {
        method = "method=result_getResultList&sessionId="+QByteArray::number(sessionId)+"&count=1&vsId=4";     //获取数据
    }
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::ContentLengthHeader, method.length());

    manager->post(req,method);
    aliveCount ++;
    if(aliveCount>10)
    {
        aliveCount = 0;
        ErrorCode = true;
        if(PF_Connect_Flag)
        {
            PF_Connect_Flag = false;
            emit IsTigntenReady(false);
        }
        system("echo 0 > /sys/class/leds/OUTD5/brightness");       //input3
        system("echo 0 > /sys/class/leds/OUTD6/brightness");       //input2
        system("echo 0 > /sys/class/leds/OUTD7/brightness");       //input1
        system("echo 0 > /sys/class/leds/OUTC0/brightness");       //input0
        qDebug() << "reset PF Tool Tightening disable = 1";
    }
    qDebug()<<"******2****request data:"<<aliveCount<<method;
}

void TightenJsonAtlasPM600::setRepair(bool temp)
{
    isRepair = temp;
}



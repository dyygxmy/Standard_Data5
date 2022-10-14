#include "queueinterface.h"

QueueInterface::QueueInterface(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void QueueInterface::init()
{
    qDebug() <<"QueueInterface thread start!!";

    TIMEOUT = (5 * 1000);
    isReady = false;
    isFirst = true;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    enablePercentage = configIniRead->value("baseinfo/OffSet").toInt();
//    enablePercentage = 70;      //使能百分比
    QString rfidip = configIniRead->value("baseinfo/RfidIp").toString();
    QString rfidport = configIniRead->value("baseinfo/RfidPort").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    req_Getcardata.setUrl(QUrl("http://"+rfidip+":"+rfidport+"/getcardata?"));
    req_Operate.setUrl(QUrl("http://"+rfidip+":"+rfidport+"/operate?"));
//    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
//    connect (&GetTimer,SIGNAL(timeout()),this,SLOT(getcardata()));
//    GetTimer.start(1000);
    QTimer::singleShot(1000,this,SLOT(getcardata()));
}

void QueueInterface::delete_car(QString deleteCar)    //deleteCar or Align
{
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        QVariantMap dataMap;
        if(Station.right(1)=="L"||Station.right(1)=="R")
            dataMap.insert("station",Station.left(Station.size()-1));
        else
            dataMap.insert("station",Station);
        dataMap.insert("carCode",deleteCar);
        if(SaveWhat == "delete_car")
        {
            SaveWhat ="";
            dataMap.insert("order",3);
        }
        else
            dataMap.insert("order",5);

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);
        qDebug()<<"delete or align"<<"json"<<json;

        req_Operate.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_Operate.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            qDebug()<<"Set operate";
            QNetworkReply *reply =manager->post(req_Operate,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    emit sendQueueError(6);
                    qDebug()<< "reply timeout delete car fail "<<deleteCar;
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                qDebug()<<"************************************operate reply"<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    emit sendQueueError(4);
                    qDebug()<<"operate An error occurred during parsing"<<bytes;
                }
                else {
                    bool Result = result["result"].toBool();
                    if(!Result)
                    {
                        emit sendQueueError(5);
                        int resultCode = result["resultCode"].toInt();
                        qDebug()<< "delete or align car fail resultCode:"<<resultCode<<"deleteCar"<<deleteCar;
                    }
                    else if(Result)
                    {
                        qDebug()<< "delete or align car success:"<<deleteCar;
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                emit sendQueueError(3);
                qDebug()<<"operate handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                qDebug( "operate found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
                qDebug()<<reply->errorString();
            }

            reply->deleteLater();
        }
        else
        {
            emit sendQueueError(2);
            qDebug() << "Something went wrong:" << serializer.errorMessage();
        }
    }
    else
    {
        emit sendQueueError(1);
        WIFIlock.unlock();
        qDebug()<< "wifi unconnect delete car fail "<<deleteCar;
    }
}

void QueueInterface::IsTightenReady(int n,bool Ready)
{
    Q_UNUSED(n);
    isReady = Ready;
}

void QueueInterface::IsTightenReady(bool Ready)
{
    isReady = Ready;
}

void QueueInterface::getcardata()
{
    if(manualMode)
    {
        QTimer::singleShot(1000,this,SLOT(getcardata()));
        return;
    }
//    GetTimer.stop();
//    qDebug()<<"####################coming in";
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        QVariantMap dataMap;
//        if(Station.right(1)=="L"||Station.right(1)=="R")
//            dataMap.insert("station",Station.left(Station.size()-1));
//        else
        dataMap.insert("station",Station);
        dataMap.insert("Type",0);
        if(isReady)
            dataMap.insert("status",0);
        else
            dataMap.insert("status",1);

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);
        qDebug()<<"json"<<json;

        req_Getcardata.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_Getcardata.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            qDebug()<<"Set getcardata";
            QNetworkReply *reply =manager->post(req_Getcardata,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    qDebug()<<"getcardata reply timeout";
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                qDebug()<<"************************************getcardata reply"<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    qDebug()<<"getcardata An error occurred during parsing"<<bytes;
                }
                else {
                    bool isFront = true;
                    foreach (QVariant Value, result["Result"].toList())
                    {
                        QVariantMap value = Value.toMap();

                        QString toalVin = value["ToalVin"].toString();
                        QString testVin = value["TestVin"].toString();
                        int percentage = value["Percentage"].toInt();
                        bool align = value["Align"].toBool();
                        int tmpPercentage = enablePercentage;
                        if(isFront)
                        {
                            tmpPercentage = 100 - enablePercentage;
                            isFront = false;
                        }
                        if ((percentage > tmpPercentage) && ((toalVin+testVin) != VIN_PIN_SQL_RFID) && CsIsConnect)
                        {
                            if(align)
                            {
                                qDebug()<<"receive align";
                                if(SYSS=="ING")
                                {
                                    VIN_PIN_SQL_RFID = toalVin+testVin;
                                    sendAlign(toalVin,testVin);
                                }
                                break;
                            }
                            VIN_PIN_SQL_RFID = toalVin+testVin;
                            bool isEqual = false;
                            if(isFirst)
                            {
                                isFirst = false;
                                if(!db2.isOpen())
                                    mysqlopen();
                                if(db2.isOpen())
                                {
                                    if(!query2.exec("SELECT COUNT(*) FROM "+Localtable+" WHERE IDCode ='"+toalVin+"'"))
                                        qDebug()<<"QueueInterface SELECT count(*) fail "<<query2.lastError();
                                    else
                                    {
                                        if(query2.next())
                                        {
                                            if(query2.value(0).toInt()>0)
                                            {
                                                qDebug()<<"already exist equal"<<toalVin;
                                                isEqual = true;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    qDebug() << "QueueInterface mysql not open";
                                }
                                if(db2.isOpen())
                                    db2.close();
                            }

                            if(isEqual)
                            {
                                qDebug() << toalVin << "isEqual true" << testVin;
                            }
                            else
                            {
                                if(SYSS!="ING")
                                {
                                    qDebug() << toalVin << "not ING" << testVin;
                                    emit sendVinToUi(toalVin,false,testVin);
                                }
                                else
                                {
                                    //是ING状态，切断当前使能，等待工人确认
                                    qDebug() << toalVin << "ING" << testVin;
                                    RFIDlock.lockForWrite();
                                    if(rfidNextCom)
                                    {
                                        RFIDlock.unlock();
                                        qDebug()<<"next Car 2"<<VIN_PIN_SQL_RFID;
                                    }
                                    else
                                    {
                                        rfidNextCom = true;
                                        RFIDlock.unlock();
                                        qDebug()<<"next Car 1"<<VIN_PIN_SQL_RFID;
                                        qDebug()<<"emit sendGetCar"<<VIN_PIN_SQL_RFID;
                                        emit sendGetCar();
                                    }
                                }
                            }
                        }
                        else if((percentage > tmpPercentage) && ((toalVin+testVin) != VIN_PIN_SQL_RFID) && !CsIsConnect)
                        {
                            qDebug()<<"Tightening machine isn't connecting"<<toalVin<<testVin;
                            if(align)
                            {
                                qDebug()<<"receive align";
                                if(SYSS=="ING")
                                {
                                    VIN_PIN_SQL_RFID = toalVin+testVin;
                                    sendAlign(toalVin,testVin);
                                }
                            }
                        }
                        else
                            qDebug()<<"no use";
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                qDebug()<<"handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                qDebug( "found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
                qDebug()<<reply->errorString();
            }

            reply->deleteLater();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
        }
        else
        {
            qDebug() << "Something went wrong:" << serializer.errorMessage();
        }
    }
    else
        WIFIlock.unlock();
//    qDebug()<<"++++++++++++++++++ end";
//    GetTimer.start(1000);
    QTimer::singleShot(1000,this,SLOT(getcardata()));
//    qDebug()<<"------------------ end";
}

void QueueInterface::mysqlopen()
{
    if(QSqlDatabase::contains("InterfaceMysqlconnection")){
        db2 = QSqlDatabase::database("InterfaceMysqlconnection");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","InterfaceMysqlconnection");
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
            qDebug()<< "QueueInterface localmysql "<< db2.lastError().text();
        }else
        {
            qDebug()<< "QueueInterface localmysql ok2 ";
        }
    }else
    {
        qDebug()<< "QueueInterface localmysql ok1 ";
    }
}

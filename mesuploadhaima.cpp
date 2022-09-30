#include "mesuploadhaima.h"

MESUploadHaima::MESUploadHaima(QObject *parent)
    : QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void MESUploadHaima::init()
{
    DTdebug() <<"MESUploadHaima thread start!!";
    TIMEOUT = (5 * 1000);
    GetTimes = 0;
    isRepair = false;
    wrongRecordID = -1;
    wrongTimes = 0;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();

    int connectNet = configIniRead->value("baseinfo/connectNet").toInt();
    if(connectNet == 0)
        strIP = configIniRead->value("baseinfo/WirelessIp").toString() ;
    else if(connectNet == 1)
        strIP = configIniRead->value("baseinfo/LocalIp").toString() ;
    else if(connectNet == 2 )
        strIP = configIniRead->value("baseinfo/LocalIp2").toString() ;

    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    //    QNetworkRequest req;
    req_Submit.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/gettime"));
    req_RepairCheck.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/RepairCheck"));

    //    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    //    connect (&InsertTimer,SIGNAL(timeout()),this,SLOT(dataInsert()));
    //    InsertTimer.start(1000);
    QTimer::singleShot(1000,this,SLOT(dataInsert()));
}

void MESUploadHaima::dataInsert()
{
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();
        if(GetTimes == 0)
        {
            GetTimes++;
            QVariantMap dataMap;
            dataMap.insert("Type","Get");
            dataMap.insert("Target","GetCurrentTime");
            QJson::Serializer serializer;
            bool ok;
            QByteArray json = serializer.serialize(dataMap, &ok);

            req_Submit.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            req_Submit.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

            if (ok)
            {
                DTdebug() << "Get CurrentTime";
                QNetworkReply *reply = manager->post(req_Submit,json);
                reply->ignoreSslErrors();
                QTime t_time;
                t_time.start();

                bool timeout = false;

                while (!reply->isFinished()) {
                    QApplication::processEvents();
                    if (t_time.elapsed() >= TIMEOUT) {
                        timeout = true;
                        DTdebug()<<"CurrentTime reply timeout";
                        break;
                    }
                }
                if (!timeout && reply->error() == QNetworkReply::NoError)
                {
                    QByteArray bytes = reply->readAll();
                    //        DTdebug()<<bytes;

                    QJson::Parser parser;
                    bool ok;

                    QVariantMap result = parser.parse(bytes, &ok).toMap();
                    if (!ok) {
                        DTdebug()<<"Get CurrentTime An error occurred during parsing"<<bytes;
                    }
                    else
                    {
                        if(result["Result"].toBool())
                        {
                            if(result["Type"].toString()=="Get" && result["Target"].toString()=="GetCurrentTime")
                            {
                                QString datetime = result["CurrentTime"].toString();
                                system((QString("date -s \"") +datetime+QString("\" &")).toLatin1().data());
                                //将系统时间写入RTC
                                system("hwclock -w &");
                                DTdebug()<<"GetCurrentTime success "<<datetime;
                            }
                            else
                                DTdebug()<<"What's wrong"<<bytes;
                        }
                        else
                        {
                            DTdebug()<<"GetCurrentTime fail ";   //<<result["Error"].toString();
                            if(result["ErrorCode"].toString()== "1")
                            {
                                DTdebug()<<"Server Json parse fail"<<json;
                            }
                        }
                    }
                }
                else if(reply->error() != QNetworkReply::NoError)
                {
                    DTdebug()<<"GetCurrentTime handle errors here";
                    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                    //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                    DTdebug( )<<"GetCurrentTime found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                    DTdebug()<<reply->errorString();
                }

                reply->deleteLater();
            }
            else
            {
                DTdebug() << "Something went wrong:" << serializer.errorMessage();
            }
        }
        else
        {
            GetTimes++;
            if(GetTimes == 120)
                GetTimes = 0;
            if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                mysql_open();
            if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
            {
                QString sql= "";
                if(isRepair)
                    sql = "SELECT RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, Cycle, Operator, Program, JobStatus, Order_id, `MaxValue`, `MinValue`, Channel, Line_ID, Station, IpAddress FROM "+Localtable +" Where UploadMark =4 and Curve !='null' order by RecordID limit 1";
                else
                    sql = "SELECT RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, Cycle, Operator, Program, JobStatus, Order_id, `MaxValue`, `MinValue`, Channel FROM "+Localtable +" Where UploadMark =4 and Curve !='null' order by RecordID limit 1";

                if(!query2.exec(sql))
                {
                    DTdebug()<<"SELECT fail "<<query2.lastError();
                    //            DTdebug() << "Mysql error need to be repaired";
                    //            system(QString("/usr/local/mysql/bin/myisamchk -c -r /usr/local/mysql/var/Tighten/"+Localtable+".MYI").toLocal8Bit().data());
                    //            DTdebug() << "repaire Mysql";
                }
                else
                {
                    if(query2.next())
                    {
                        QVariantMap dataMap;
                        dataMap.insert("Type","Set");
                        dataMap.insert("Target","TighteningData");
                        QVariantMap data;
                        data.insert("Version",1);
                        data.insert("ID", query2.value(0).toInt());
                        data.insert("IDCode", query2.value(1).toString());
                        data.insert("ScrewID", query2.value(2).toString());
                        data.insert("Torque", query2.value(3).toDouble());
                        data.insert("Angle", query2.value(4).toDouble());
                        data.insert("Curve", query2.value(5).toString());
                        data.insert("TighteningStatus", query2.value(6).toString());
                        data.insert("TighteningTime", query2.value(7).toString().left(10)+" "+query2.value(7).toString().right(8));
                        data.insert("Cycle", query2.value(8).toInt());
                        data.insert("Operator", query2.value(9).toString());
                        data.insert("Program", query2.value(10).toInt());
                        data.insert("JobStatus", query2.value(11).toInt());
                        data.insert("IIO", query2.value(12).toInt());

                        QString maxValue = QString::number(query2.value(13).toDouble(), 10, 2);
                        QString minValue = QString::number(query2.value(14).toDouble(), 10, 2);
                        data.insert("MaxValue", maxValue);
                        data.insert("MinValue", minValue);
                        //                        data.insert("MaxValue", query2.value(13).toDouble());
                        //                        data.insert("MinValue", query2.value(14).toDouble());
                        data.insert("Channel", query2.value(15).toInt());
                        if(isRepair)
                        {
                            data.insert("Line_ID", query2.value(16).toInt());
                            data.insert("Station", query2.value(17).toString());
                            data.insert("IpAddress", query2.value(18).toString());
                        }
                        else
                        {
                            data.insert("Line_ID", Line_ID);
                            data.insert("Station", Station);
                            data.insert("IpAddress", strIP);
                        }
                        data.insert("Workshop", "Assembly");
                        data.insert("WorkShift", "");
                        dataMap.insert("Parameter",data);

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);

                        req_RepairCheck.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                        req_RepairCheck.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                        if (ok)
                        {
                            DTdebug() << "post RecordID:"<<query2.value(0).toInt()<<"Cycle:"<<query2.value(8).toInt();
                            QNetworkReply *reply = manager->post(req_RepairCheck,json);
                            reply->ignoreSslErrors();
                            QTime t_time;
                            t_time.start();

                            bool timeout = false;

                            while (!reply->isFinished()) {
                                QApplication::processEvents();
                                if (t_time.elapsed() >= TIMEOUT) {
                                    timeout = true;
                                    DTdebug()<<"insert reply timeout";
                                    break;
                                }
                            }
                            if (!timeout && reply->error() == QNetworkReply::NoError)
                            {
                                QByteArray bytes = reply->readAll();
                                //                                        DTdebug()<<bytes;

                                QJson::Parser parser;
                                bool ok;

                                QVariantMap result = parser.parse(bytes, &ok).toMap();
                                if (!ok) {
                                    DTdebug()<<"post Insert An error occurred during parsing"<<bytes;
                                }
                                else
                                {
                                    if(result["Result"].toBool())
                                    {
                                        if(result["Type"].toString()=="Set" && result["Target"].toString()=="TighteningData")
                                        {
                                            QString RecordID = result["ID"].toString();
                                            QString Cycle = result["Cycle"].toString();
                                            if(query2.exec("UPDATE "+Localtable+" SET UploadMark = 1,UploadTime = now() WHERE RecordID = "+RecordID))
                                            {
                                                DTdebug()<<"update 1 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                            }
                                            else
                                            {
                                                DTdebug()<<"update 1 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2.lastError();
                                            }
                                        }
                                        else
                                            DTdebug()<<"What's wrong"<<bytes;
                                    }
                                    else
                                    {
                                        //                                        if(result["ErrorCode"].toString()== "1")
                                        //                                        {
                                        //                                            DTdebug()<<"Server Json parse fail"<<json;
                                        //                                            if(query2.exec("UPDATE "+Localtable+" SET UploadMark = 7, UploadTime = now() WHERE RecordID = "+ID))
                                        //                                            {
                                        //                                                DTdebug()<<"update 7 success: RecordID:"<<ID;
                                        //                                            }
                                        //                                            else
                                        //                                            {
                                        //                                                DTdebug()<<"update 7 fail: RecordID:"<<ID<<query2.lastError();
                                        //                                            }
                                        //                                        }
                                        if(bytes == "")
                                        {
                                            DTdebug()<<"reply is null";
                                        }
                                        else
                                        {
                                            QString RecordID = result["ID"].toString();
                                            QString Cycle = result["Cycle"].toString();
                                            DTdebug()<<"Reinsert fail RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                            if(wrongRecordID != RecordID.toInt())
                                            {
                                                wrongRecordID = RecordID.toInt();
                                                wrongTimes++;
                                            }
                                            else
                                            {
                                                wrongTimes++;
                                                if(wrongTimes == 10)
                                                {
                                                    if(query2.exec("UPDATE "+Localtable+" SET UploadMark = 6, UploadTime = now() WHERE RecordID = "+RecordID))
                                                    {
                                                        wrongRecordID = -1;
                                                        wrongTimes = 0;
                                                        DTdebug()<<"update 6 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                                    }
                                                    else
                                                    {
                                                        DTdebug()<<"update 6 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2.lastError();
                                                    }
                                                }
                                                else if(wrongTimes >10)
                                                    wrongTimes = 0;
                                            }
                                        }
                                    }
                                }
                            }
                            else if(reply->error() != QNetworkReply::NoError)
                            {
                                DTdebug()<<"insert handle errors here";
                                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                                DTdebug( )<<"insert found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                                DTdebug()<<reply->errorString();
                            }

                            reply->deleteLater();
                        }
                        else
                        {
                            DTdebug() << "Something went wrong:" << serializer.errorMessage();
                        }
                    }
                }
            }
            else
            {
                DTdebug()<<"db2 is not open or ReinsertMySQL connect fail";
            }
        }
    }
    else
        WIFIlock.unlock();
    QTimer::singleShot(1000,this,SLOT(dataInsert()));
}


// 本地mysql open
void MESUploadHaima::mysql_open()
{
    if(QSqlDatabase::contains("ReinsertMySQL")){
        db2 = QSqlDatabase::database("ReinsertMySQL");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","ReinsertMySQL");
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
            DTdebug()<< "reinsert localmysql "<< db2.lastError().text();
        }else
        {
            DTdebug()<< "reinsert localmysql open ok 2";
        }
    }else
    {
        DTdebug()<< "reinsert localmysql open ok 1";
    }
}



void MESUploadHaima::receiveRepairVIN(QString repairVIN)    //deleteCar or Align
{
    DTdebug()<<"99999999999999999999999999999999999999999999999999999999";
    QVariantMap replyMap;
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        QVariantMap dataMap;
        dataMap.insert("Type","Get");
        dataMap.insert("Target","RepairCheck");
        dataMap.insert("IDCode",repairVIN);

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);
        DTdebug()<<"get repair"<<"json"<<json;

        req_RepairCheck.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_RepairCheck.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            DTdebug()<<"Get repair";
            QNetworkReply *reply =manager->post(req_RepairCheck,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    emit replyRepair(6,replyMap);
                    DTdebug()<< "reply timeout Get repair fail "<<repairVIN;
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                DTdebug()<<"************************************Get repair reply"<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    emit replyRepair(4,replyMap);
                    DTdebug()<<"Get repair An error occurred during parsing"<<bytes;
                }
                else {
                    DTdebug()<< "Get repair success:"<<repairVIN;
                    replyMap = result;
                    emit replyRepair(0,replyMap);
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                emit replyRepair(3,replyMap);
                DTdebug()<<"Get repair handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                DTdebug( )<<"Get repair found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                DTdebug()<<reply->errorString();
            }

            reply->deleteLater();
        }
        else
        {
            emit replyRepair(2,replyMap);
            DTdebug() << "Something went wrong:" << serializer.errorMessage();
        }
    }
    else
    {
        emit replyRepair(1,replyMap);
        WIFIlock.unlock();
        DTdebug()<< "wifi unconnect Get repair fail "<<repairVIN;
    }
}

void MESUploadHaima::setRepair(bool temp)
{
    isRepair = temp;
}


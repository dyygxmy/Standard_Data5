#include "mesuploadbaic.h"

MESUploadBAIC::MESUploadBAIC(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void MESUploadBAIC::init()
{
    Factory = factory;
    DTdebug() <<"MESUpload thread start!!"<<Factory;
    TIMEOUT = (5 * 1000);
    isFirst = true;
    GetTimes = 0;
    wrongRecordID = -1;
    wrongTimes = 0;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    if(factory == "AQCHERY")
        req_uploaddata.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/submit?"));
    else
        req_uploaddata.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/uploaddata?"));
    req_gettime.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/gettime?"));
//    reqFis.setUrl(QUrl("http://"+DataServerIp+":1900/getvins?"));
    QTimer::singleShot(1000,this,SLOT(dataInsert()));
}

void MESUploadBAIC::dataInsert()
{
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();
        if(GetTimes == 0)       //Time
        {
            GetTimes++;
            QVariantMap dataMap;
            dataMap.insert("Type","Get");
            dataMap.insert("Target","GetCurrentTime");
            QJson::Serializer serializer;
            bool ok;
            QByteArray json = serializer.serialize(dataMap, &ok);

            req_gettime.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            req_gettime.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

            if (ok)
            {
                DTdebug() << "Get CurrentTime";
                QNetworkReply *reply = manager->post(req_gettime,json);
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

        else if(GetTimes%6 == 0 && Factory == "BAIC")        //Fis
        {
            GetTimes++;
            if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                mysql_open();
            if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
            {
                if(isFirst)
                {
                    if(!query2.exec("CREATE TABLE IF NOT EXISTS "+tablePreview+
                                    " (RecordID integer primary key auto_increment, VIN varchar(17) not null,carType varchar(20) not null, "
                                    "tolNumber varchar(100) not null,UseFlag int not null,LocalDateTime datetime not null)"))
                    {
                        DTdebug()<<"create table "+tablePreview+" fail"<<query2.lastError();
                    }
                    else
                        isFirst = false;
                }

                if(!query2.exec("SELECT VIN FROM "+tablePreview + " order by RecordID desc limit 1"))
                {
                    DTdebug()<<"SELECT fail "<<query2.lastError();
                }
                else
                {
                    QVariantMap dataMap;
                    if(query2.next())
                    {
                        dataMap.insert("vin",query2.value(0).toString());
                    }
                    else
                        dataMap.insert("vin","");

                    QJson::Serializer serializer;
                    bool ok;
                    QByteArray json = serializer.serialize(dataMap, &ok);

                    reqFis.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                    reqFis.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                    if (ok)
                    {
                        DTdebug() << "post get FisPreview" << json;   //<<json;
                        QNetworkReply *reply = manager->post(reqFis,json);
                        reply->ignoreSslErrors();
                        QTime t_time;
                        t_time.start();

                        bool timeout = false;

                        while (!reply->isFinished())
                        {
                            QApplication::processEvents();
                            if (t_time.elapsed() >= TIMEOUT) {
                                timeout = true;
                                DTdebug()<<"FisPreview reply timeout";
                                break;
                            }
                        }
                        if (!timeout && reply->error() == QNetworkReply::NoError)
                        {
                            QByteArray bytes = reply->readAll();
                                    DTdebug()<<bytes;

                            QJson::Parser parser;
                            bool ok;

                            QVariantMap result = parser.parse(bytes, &ok).toMap();
                            if (!ok) {
                                DTdebug()<<"get FisPreview An error occurred during parsing"<<bytes;
                            }
                            else
                            {

                                DTdebug()<<"get FisPreview";

                                if(result["Result"].toBool())
                                {
                                    DTdebug()<<"get FisPreview";
                                    foreach (QVariant Value, result["VINS"].toList())
                                    {
                                        QVariantMap value = Value.toMap();
                                        if(!query2.exec("SELECT COUNT(*) FROM "+tablePreview))
                                            DTdebug()<<"SELECT COUNT(*) FAIL "<<query2.lastError();
                                        else
                                        {
                                            if(query2.next())
                                            {
                                                int numRows = query2.value(0).toInt();
                                                query2.prepare("insert into "+tablePreview+" (VIN,carType,tolNumber,UseFlag,LocalDateTime) values (?, ?, ?, 0, now())");
                                                if(numRows>7000)
                                                {
                                                    DTdebug()<<"delete fis data" ;

#if 0
                                                    query2.prepare("UPDATE "+tablePreview+
                                                                   " SET VIN =?,carType =?,tolNumber =?,UseFlag =0,LocalDateTime=now() WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+tablePreview+")as t)");
#endif
                                                    query2.prepare("delete from "+tablePreview+" where RecordID in (select x.RecordID from (select RecordID from "+tablePreview+" order by RecordID limit  100) as  x)");
                                                }
                                                else {

                                                }
                                                query2.addBindValue(value["vin"].toString());
                                                query2.addBindValue(value["carType"].toString());
                                                query2.addBindValue(value["tolNumber"].toString());

                                                if(!query2.exec())
                                                    DTdebug() <<"update FisPreview fail "<<query2.lastError();
                                            }
                                        }
                                    }
                                }
                                else
                                    DTdebug()<<"What's wrong"<<bytes;
                            }
                        }

                        else if(reply->error() != QNetworkReply::NoError)
                        {
                            DTdebug()<<"FisPreview handle errors here";
                            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                            DTdebug( )<<"FisPreview found error ....code: %d %d\n"<< statusCodeV.toInt()<<(int)reply->error();
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
            else
            {
                DTdebug()<<"db2 is not open or ReinsertMySQL connect fail";
            }
        }


        else       //Insert
        {
            DTdebug()<<"come in insert";
            GetTimes++;
            if(GetTimes == 120)
                GetTimes = 0;

            if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                mysql_open();
            if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
            {
                if(!query2.exec("SELECT RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, Cycle, Operator, Program, JobStatus, Order_id, `MaxValue`, `MinValue`, Channel, MaxAngle, MinAngle, Body_NO FROM "+Localtable +" Where UploadMark =4 and Curve !='null' order by RecordID limit 1"))
                {
                    DTdebug()<<"SELECT fail "<<query2.lastError();
                }
                else
                {
                    if(query2.next())
                    {
                        QString ID = query2.value(0).toString();
                        QVariantMap dataMap;
                        dataMap.insert("Type","Set");
                        dataMap.insert("Target","TighteningData");
                        QVariantMap data;
//                        data.insert("Version",1);
                        data.insert("ID", query2.value(0).toInt());
                        data.insert("IDCode", query2.value(1).toString());
                        data.insert("ScrewID", query2.value(2).toString());
                        data.insert("Torque", QString::number(query2.value(3).toDouble(),10, 2));
                        data.insert("Angle", QString::number(query2.value(4).toDouble(),10, 2));
                        data.insert("Curve", query2.value(5).toString());
                        data.insert("TighteningStatus", query2.value(6).toString());
                        data.insert("TighteningTime", query2.value(7).toString().left(10)+" "+query2.value(7).toString().right(8));
                        data.insert("Cycle", query2.value(8).toInt());
                        data.insert("Operator", query2.value(9).toString());
                        data.insert("Program", query2.value(10).toInt());
                        data.insert("JobStatus", query2.value(11).toInt());
                        data.insert("IIO", query2.value(12).toInt());

                        data.insert("Torque_Max", QString::number(query2.value(13).toDouble(),10, 2));
                        data.insert("Torque_Min", QString::number(query2.value(14).toDouble(),10, 2));


                        data.insert("Channel", query2.value(15).toInt());
                        data.insert("Angle_Max", QString::number(query2.value(16).toDouble(), 10, 2));
                        data.insert("Angle_Min", QString::number(query2.value(17).toDouble(), 10, 2));
                        data.insert("Line_ID", Line_ID);
                        data.insert("Station", Station);
                        data.insert("IpAddress", LocalIp2);   //北汽使用有线2 2017.09.26
                        data.insert("LineName", LineName);
//                        data.insert("Workshop", "Assembly");
//                        data.insert("WorkShift", "");
                        dataMap.insert("Parameter",data);

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);

//                        DTdebug()<<"@@@@@@@@@@"<<json;
                        req_uploaddata.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                        req_uploaddata.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                        if (ok)
                        {
                            DTdebug() << "post RecordID:"<<query2.value(0).toInt()<<"Cycle:"<<query2.value(8).toInt();
                            QNetworkReply *reply = manager->post(req_uploaddata,json);
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
                                        if(result["ErrorCode"].toString()== "1")
                                        {
                                            DTdebug()<<"Server Json parse fail"<<json;
                                            if(query2.exec("UPDATE "+Localtable+" SET UploadMark = 7, UploadTime = now() WHERE RecordID = "+ID))
                                            {
                                                DTdebug()<<"update 7 success: RecordID:"<<ID;
                                            }
                                            else
                                            {
                                                DTdebug()<<"update 7 fail: RecordID:"<<ID<<query2.lastError();
                                            }
                                        }
                                        else if(bytes == "")
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
void MESUploadBAIC::mysql_open()
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

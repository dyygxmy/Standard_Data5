#include "mesuploaddongfeng.h"

MESUploadDongfeng::MESUploadDongfeng(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void MESUploadDongfeng::init()
{
    qDebug() <<"MESUploadDongfeng thread start!!";
    enablePercentage = 70;      //使能百分比
    isFirst = true;
    GetTimes = 60;
    wrongRecordID = -1;
    wrongTimes = 0;
    TIMEOUT = (5 * 1000);
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    //    QNetworkRequest req;
    req_Submit.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/submit?"));
//    req_Getcardata.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/getcardata?"));
//    req_Operate.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/operate?"));
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
    connect (&InsertTimer,SIGNAL(timeout()),this,SLOT(dataInsert()));
    InsertTimer.start(1000);
}

void MESUploadDongfeng::finishedSlot(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        //        qDebug()<<bytes;

        QJson::Parser parser;
        bool ok;

        QVariantMap result = parser.parse(bytes, &ok).toMap();
        if (!ok) {
            qDebug()<<"An error occurred during parsing"<<bytes;
            //             qFatal("An error occurred during parsing");
            //             exit (1);
        }
        else {
            if(result["Type"].toString()=="Get" && result["Target"].toString()=="GetCurrentTime")
            {
                if(result["Result"].toBool())
                {
                    QString datetime = result["CurrentTime"].toString();
                    system((QString("date -s \"") +datetime+QString("\" &")).toLatin1().data());
                    //将系统时间写入RTC
                    system("hwclock -w &");
                    //                    if(datetime.mid(0,4).toInt()<2015)
                    //                        emit time_error(true);
                    //                    else
                    //                        emit time_error(false);
                    qDebug()<<"GetCurrentTime success "<<datetime;
                }
                else
                {
                    qDebug()<<"GetCurrentTime fail ";   //<<result["Error"].toString();
                }
            }
            else if(result["Type"].toString()=="Set" && result["Target"].toString()=="TighteningData")
            {
                if(result["Result"].toBool())
                {
                    QString RecordID = result["ID"].toString();
                    QString Cycle = result["Cycle"].toString();
                    if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                        mysql_open();
                    if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
                    {
                        if(query2.exec("UPDATE "+Localtable+" SET UploadMark = 1,UploadTime = now() WHERE RecordID = "+RecordID))
                        {
                            qDebug()<<"update 1 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                        }
                        else
                        {
                            qDebug()<<"update 1 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2.lastError();
                        }
                    }
                    else
                    {
                        qDebug()<<"db2 is not open or ReinsertMySQL connect fail Reinsert fail RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                    }
                }
                else
                {
                    QString RecordID = result["ID"].toString();
                    QString Cycle = result["Cycle"].toString();
                    qDebug()<<"Reinsert fail RecordID:"<<RecordID<<"Cycle:"<<Cycle;
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
                            if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                                mysql_open();
                            if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
                            {
                                if(query2.exec("UPDATE "+Localtable+" SET UploadMark = 6, UploadTime = now() WHERE RecordID = "+RecordID))
                                {
                                    wrongRecordID = -1;
                                    wrongTimes = 0;
                                    qDebug()<<"update 6 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                }
                                else
                                {
                                    qDebug()<<"update 6 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2.lastError();
                                }
                            }
                            else
                            {
                                qDebug()<<"db2 is not open or ReinsertMySQL connect fail Reinsert fail RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                            }
                        }
                        else if(wrongTimes >10)
                            wrongTimes = 0;
                    }
                }
            }
            else if(result["Type"].toString()=="Get" && result["Target"].toString()=="FisPreview")
            {
                if(result["Result"].toBool())
                {
                    if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                        mysql_open();
                    if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
                    {
                        qDebug()<<"get FisPreview";
                        foreach (QVariant Value, result["Value"].toList())
                        {
                            QVariantMap value = Value.toMap();
                            if(!query2.exec("SELECT COUNT(*) FROM "+tablePreview))
                                qDebug()<<"SELECT COUNT(*) FAIL "<<query2.lastError();
                            else
                            {
                                if(query2.next())
                                {
                                    int numRows = query2.value(0).toInt();
                                    if (numRows < 7000)
                                    {
                                        query2.prepare("insert into "+tablePreview+" (RecordID, Werk, SPJ, KNR, VIN ,LocalDateTime, FisMatch) values (?, ?, ?, ?, ?, now(),?)");
                                    }
                                    else
                                    {
                                        query2.prepare("UPDATE "+tablePreview+" SET RecordID =?, Werk =?, SPJ =?, KNR =?, VIN =?, LocalDateTime=now(), FisMatch =? WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+tablePreview+")as t)");
                                    }
                                    query2.addBindValue(value["RecordID"].toInt());
                                    query2.addBindValue(value["Werk"].toString());
                                    query2.addBindValue(value["SPJ"].toString());
                                    query2.addBindValue(value["KNR"].toString());
                                    query2.addBindValue(value["VIN"].toString());

                                    QJson::Serializer serializer;
                                    QByteArray match = serializer.serialize(value["FisMatch"],&ok);

                                    query2.addBindValue(match);
                                    if(!query2.exec())
                                        qDebug() <<"update FisPreview fail "<<query2.lastError();
                                }
                            }
                        }
                    }
                    else
                    {
                        qDebug()<<"db2 is not open or ReinsertMySQL connect fail update FisPreview fail";
                    }
                }
                else
                {
                    qDebug()<<"Get FisPreview fail ";//    <<result["Error"].toString();
                }
            }
        }

        //         QString string = QString::fromUtf8(bytes);

        //         qDebug()<<string.toUtf8();
    }
    else
    {
        qDebug()<<"handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        qDebug( "found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        //         qDebug(qPrintable(reply->errorString()));
        qDebug()<<reply->errorString();
    }
    reply->deleteLater();
    //     dataInsert();
}

void MESUploadDongfeng::dataInsert()
{
    InsertTimer.stop();
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

            req_Submit.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            req_Submit.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

            if (ok) {
                qDebug() << "Get CurrentTime";
                manager->post(req_Submit,json);
            } else {
                qDebug() << "Something went wrong:" << serializer.errorMessage();
            }
        }
        else if(GetTimes%6 == 0)        //Fis
        {
            GetTimes++;
            if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                mysql_open();
            if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
            {
                if(isFirst)
                {
                    if(!query2.exec("CREATE TABLE IF NOT EXISTS "+tablePreview+" (RecordID int not null primary key, Werk varchar(2) not null, SPJ varchar(4) not null, KNR varchar(8) not null, VIN varchar(17) not null,LocalDateTime datetime not null, FisMatch varchar(250) not null)"))
                    {
                        qDebug()<<"create table "+tablePreview+" fail"<<query2.lastError();
                    }
                    else
                        isFirst = false;
                }

                query2.exec("SELECT MAX(RecordID) FROM "+tablePreview+"");
                if(!query2.exec("SELECT MAX(RecordID) FROM "+tablePreview))
                {
                    qDebug()<<"SELECT fail "<<query2.lastError();
                }
                else
                {

                    QVariantMap dataMap;
                    dataMap.insert("Type","Get");
                    dataMap.insert("Target","FisPreview");
                    QVariantMap data;
                    if(query2.next())
                    {
                        data.insert("ID",query2.value(0).toInt());
                    }
                    else
                        data.insert("ID",0);
                    data.insert("Line_ID",Line_ID);
                    dataMap.insert("Parameter",data);

                    QJson::Serializer serializer;
                    bool ok;
                    QByteArray json = serializer.serialize(dataMap, &ok);

                    req_Submit.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                    req_Submit.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                    if (ok)
                    {
                        qDebug() << "post get FisPreview";   //<<json;
                        manager->post(req_Submit,json);
                    }
                    else
                    {
                        qDebug() << "Something went wrong:" << serializer.errorMessage();
                    }

                }
            }
            else
            {
                qDebug()<<"db2 is not open or ReinsertMySQL connect fail";
            }
        }
        else if(GetTimes%2 == 0)      //getcardata
        {
            GetTimes++;
            QVariantMap dataMap;
            dataMap.insert("station",Station);
            dataMap.insert("Type",0);

            QJson::Serializer serializer;
            bool ok;
            QByteArray json = serializer.serialize(dataMap, &ok);

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
                        InsertTimer.start(500);
                        qDebug()<<"reply timeout";
                        break;
                    }
                }

                if (!timeout && reply->error() == QNetworkReply::NoError) {
                    QByteArray bytes = reply->readAll();
                    qDebug()<<bytes;

                    QJson::Parser parser;
                    bool ok;

                    QVariantMap result = parser.parse(bytes, &ok).toMap();
                    if (!ok) {
                        qDebug()<<"An error occurred during parsing"<<bytes;
                    }
                    else {
                        foreach (QVariant Value, result["Result"].toList())
                        {
                            QVariantMap value = Value.toMap();

                            QString toalVin = value["ToalVin"].toString();
                            QString testVin = value["TestVin"].toString();
                            int percentage = value["Percentage"].toInt();
                            if ((percentage > enablePercentage) && ((toalVin+testVin) != VIN_PIN_SQL_RFID))
                            {
                                VIN_PIN_SQL_RFID = toalVin+testVin;
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
        else       //Insert
        {
            GetTimes++;
            if(GetTimes == 120)
                GetTimes = 0;

            if(!db2.isOpen() || !QSqlDatabase::contains("ReinsertMySQL") )
                mysql_open();
            if(db2.isOpen() && QSqlDatabase::contains("ReinsertMySQL"))
            {
                if(!query2.exec("SELECT RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, Cycle, Operator, Program, JobStatus, Order_id, `MaxValue`, `MinValue`, Channel FROM "+Localtable +" Where UploadMark =4 and Curve !='null' order by RecordID limit 1"))
                {
                    qDebug()<<"SELECT fail "<<query2.lastError();
                    //            qDebug() << "Mysql error need to be repaired";
                    //            system(QString("/usr/local/mysql/bin/myisamchk -c -r /usr/local/mysql/var/Tighten/"+Localtable+".MYI").toLocal8Bit().data());
                    //            qDebug() << "repaire Mysql";
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
                        data.insert("MaxValue", query2.value(13).toDouble());
                        data.insert("MinValue", query2.value(14).toDouble());
                        data.insert("Channel", query2.value(15).toInt());
                        data.insert("Line_ID", Line_ID);
                        data.insert("Station", Station);
                        data.insert("IpAddress", WirelessIp);
                        data.insert("Workshop", "Assembly");
                        data.insert("WorkShift", "");
                        dataMap.insert("Parameter",data);

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);

                        req_Submit.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                        req_Submit.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                        if (ok)
                        {
                            qDebug() << "post RecordID:"<<query2.value(0).toInt()<<"Cycle:"<<query2.value(8).toInt();
                            manager->post(req_Submit,json);
                        }
                        else
                        {
                            qDebug() << "Something went wrong:" << serializer.errorMessage();
                        }
                    }
                }
            }
            else
            {
                qDebug()<<"db2 is not open or ReinsertMySQL connect fail";
            }
        }
    }
    else
        WIFIlock.unlock();
    InsertTimer.start(500);
}

// 本地mysql open
void MESUploadDongfeng::mysql_open()
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
            qDebug()<< "reinsert localmysql "<< db2.lastError().text();
        }else
        {
            qDebug()<< "reinsert localmysql open ok 2";
        }
    }else
    {
        qDebug()<< "reinsert localmysql open ok 1";
    }
}

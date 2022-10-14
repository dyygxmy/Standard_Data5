#include "mesuploadsvw2.h"

MESUploadSVW2::MESUploadSVW2(QObject *parent) :
    QObject(parent)
{
    mGetFisOk = false;
    this->moveToThread(&m_thread);
    m_thread.start();
}

void MESUploadSVW2::init()
{
    Factory = factory;
    DTdebug() <<"MESUpload thread start!!"<<Factory;
    TIMEOUT = (5 * 1000);
    isFirst = true;
    GetTimes = 0;
    wrongRecordID = -1;
    wrongTimes = 0;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    mDataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    mServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    req.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/submit?"));
    QTimer::singleShot(1000,this,SLOT(dataInsert()));
    dataInsertCount = 0;

    mDataBase = mysql_open(QString("MESUploadSVW2"));
    if(!mDataBase.isOpen())
    {
        query2 = 0;
        DTdebug()<< "mDataBase is not open";
    }
    else
    {
        DTdebug()<< "mDataBase is open, create query";
        query2 = new QSqlQuery(mDataBase);

        DTdebug() << "MEBFisMatch" << MEBFisMatch;
        if(MEBFisMatch)
        {
            UpdateFisList();
        }
    }
}

void MESUploadSVW2::UpdateFisList()
{
    if(mDataBase.isOpen())
    {
        gFisVins.clear();
        gFisPins.clear();
        query2->exec("select KNR,VIN from "+tablePreview+" order by RecordID");
        QString tmpDebug;
        int tmpCount =0;
        while(query2->next())  //查到了匹配
        {
            QString tmpPin = query2->value(0).toString();
            QString tmpVin = query2->value(1).toString();
            //DTdebug() << "PIN: " << tmpPin << "   VIN: " << tmpVin;
            gFisVins.append(tmpVin);
            gFisPins.append(tmpPin);
            tmpDebug.append(QString("%1.PIN:%2,VIN:%3;").arg(tmpCount++).arg(tmpPin).arg(tmpVin));
        }
        {
            if(isRFID)
            {
                int tmpNewIndex = gFisPins.indexOf(gFisPin, 0);
                DTdebug() << "Search:" << gFisPin << tmpNewIndex;
                if(tmpNewIndex >= 0)
                {
                    gFisVinIndex = tmpNewIndex+1;
                }
                if(gFisVinIndex < 0)
                    gFisVinIndex = 0;
            }
            else if(isBarCode)
            {
                int tmpNewIndex = gFisVins.indexOf(gFisVin, 0);
                DTdebug() << "Search:" << gFisVin << tmpNewIndex;
                if(tmpNewIndex >= 0)
                {
                    gFisVinIndex = tmpNewIndex+1;
                }
                if(gFisVinIndex < 0)
                    gFisVinIndex = 0;
            }
        }
        DTdebug() << "select KNR,VIN from "<<tablePreview << " ; result = " << gFisVins.count() << tmpDebug;
    }
}

void MESUploadSVW2::dataInsert()
{
    dataInsertCount++;
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

            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

            if (ok)
            {
                DTdebug() << "Get CurrentTime";
                QNetworkReply *reply = manager->post(req,json);
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
        else if(GetTimes == 59 && Factory == "SVW2")      //Power
        {
            GetTimes++;
            QVariantMap dataMap;
            dataMap.insert("Type","Set");
            dataMap.insert("Target","Power");

            QVariantMap data;
            data.insert("TotalPower",112);
            data.insert("PeakPower",112);
            data.insert("OrdinaryPower",112);
            data.insert("ValleyPower",112);
            data.insert("ReTotalPower",112);
            data.insert("Voltage",112);
            data.insert("Electric",112);
            data.insert("TruePower",112);
            data.insert("FalsePower",112);
            data.insert("ApparentPower",112);
            data.insert("PowerFactor",112);
            data.insert("Frequency",112);
            data.insert("Years",112);
            data.insert("Days",112);
            data.insert("Minutes",112);
            data.insert("Address",112);
            data.insert("StationStatus",StationStatus);
            data.insert("Data2IP",WirelessIp);

            dataMap.insert("Parameter",data);

            QJson::Serializer serializer;
            bool ok;
            QByteArray json = serializer.serialize(dataMap, &ok);
           // DTdebug()<<"json:"<< json;
            req.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/submit?"));
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

            if (ok)
            {
                DTdebug()<<"Set Power";
                QNetworkReply *reply = manager->post(req,json);
                reply->ignoreSslErrors();
                QTime t_time;
                t_time.start();

                bool timeout = false;

                while (!reply->isFinished()) {
                    QApplication::processEvents();
                    if (t_time.elapsed() >= TIMEOUT) {
                        timeout = true;
                        DTdebug()<<"Power reply timeout";
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
                        DTdebug()<<"Set Power An error occurred during parsing"<<bytes;
                    }
                    else
                    {
                        if(!result["Result"].toBool())
                        {
                            DTdebug()<<"Set Power fail "; //<<result["Error"].toString();
                            if(result["ErrorCode"].toString()== "1")
                            {
                                DTdebug()<<"Server Json parse fail"<<json;
                            }
                        }
                    }
                }
                else if(reply->error() != QNetworkReply::NoError)
                {
                    DTdebug()<<"Power handle errors here";
                    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                    //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                    DTdebug( )<<"Power found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                    DTdebug()<<reply->errorString();
                }

                reply->deleteLater();
            }
            else
            {
                DTdebug() << "Something went wrong:" << serializer.errorMessage();
            }
        }
        else if(GetTimes == 51 && Factory == "SVW2")      //Andon
        {
            GetTimes++;
            QVariantMap dataMap;
            dataMap.insert("Type","Set");
            dataMap.insert("Target","Andon");

            QVariantMap data;
            data.insert("StationStatus",StationStatus);
            data.insert("Data2IP",WirelessIp);

            dataMap.insert("Parameter",data);

            QJson::Serializer serializer;
            bool ok;
            QByteArray json = serializer.serialize(dataMap, &ok);

            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

            if (ok)
            {
                DTdebug()<<"Set Andon";
                QNetworkReply *reply = manager->post(req,json);
                reply->ignoreSslErrors();
                QTime t_time;
                t_time.start();

                bool timeout = false;

                while (!reply->isFinished()) {
                    QApplication::processEvents();
                    if (t_time.elapsed() >= TIMEOUT) {
                        timeout = true;
                        DTdebug()<<"Andon reply timeout";
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
                        DTdebug()<<"Set Andon An error occurred during parsing"<<bytes;
                    }
                    else
                    {
                        if(!result["Result"].toBool())
                        {
                            DTdebug()<<"Set Andon fail "; //<<result["Error"].toString();
                            if(result["ErrorCode"].toString()== "1")
                            {
                                DTdebug()<<"Server Json parse fail"<<json;
                            }
                        }
                    }
                }
                else if(reply->error() != QNetworkReply::NoError)
                {
                    DTdebug()<<"Andon handle errors here";
                    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                    //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                    DTdebug( )<<"Andon found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                    DTdebug()<<reply->errorString();
                }

                reply->deleteLater();
            }
            else
            {
                DTdebug() << "Something went wrong:" << serializer.errorMessage();
            }
        }
        else if(GetTimes%6 == 0 && Factory == "SVW2")        //Fis
        {
            GetTimes++;

            if(!mDataBase.isOpen())
            {
                mDataBase = mysql_open(QString("MESUploadSVW2"));
                if(mDataBase.isOpen())
                    query2 = new QSqlQuery(mDataBase);
            }
            if(mDataBase.isOpen())
            {
                if(isFirst)
                {
                    if(!query2->exec("CREATE TABLE IF NOT EXISTS "+tablePreview+" (RecordID int not null primary key, Werk varchar(2) not null, SPJ varchar(4) not null, KNR varchar(8) not null, VIN varchar(17) not null,LocalDateTime datetime not null, FisMatch varchar(1000) not null)"))
                    {
                        DTdebug()<<"create table "+tablePreview+" fail"<<query2->lastError();
                    }
                    else
                        isFirst = false;
                }

                if(!query2->exec("SELECT MAX(RecordID) FROM "+tablePreview))
                {
                    DTdebug()<<"SELECT fail "<<query2->lastError();
                }
                else
                {
                    QVariantMap dataMap;
                    dataMap.insert("Type","Get");
                    dataMap.insert("Target","FisPreview");
                    QVariantMap data;
                    if(query2->next())
                    {
                        data.insert("ID",query2->value(0).toInt());
                    }
                    else
                        data.insert("ID",0);
                    data.insert("Line_ID",Line_ID);
                    dataMap.insert("Parameter",data);

                    QJson::Serializer serializer;
                    bool ok;
                    QByteArray json = serializer.serialize(dataMap, &ok);
                    req.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/submit?"));
                    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                    req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                    if (ok)
                    {
                        DTdebug() << "post get FisPreview";   //<<json;
                        QNetworkReply *reply = manager->post(req,json);
                        reply->ignoreSslErrors();
                        QTime t_time;
                        t_time.start();

                        bool timeout = false;

                        while (!reply->isFinished()) {
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
                                if(result["Result"].toBool())
                                {
                                    if(result["Type"].toString()=="Get" && result["Target"].toString()=="FisPreview")
                                    {
                                        DTdebug()<<"get FisPreview" << result["Value"].toString();
                                        bool tmpGetFlag = false;
                                        foreach (QVariant Value, result["Value"].toList())
                                        {
                                            QVariantMap value = Value.toMap();
                                            if(!query2->exec("SELECT COUNT(*) FROM "+tablePreview))
                                                DTdebug()<<"SELECT COUNT(*) FAIL "<<query2->lastError();
                                            else
                                            {
                                                if(query2->next())
                                                {
                                                    int numRows = query2->value(0).toInt();
                                                    if (numRows < 7000)
                                                    {
                                                        query2->prepare("insert into "+tablePreview+" (RecordID, Werk, SPJ, KNR, VIN ,LocalDateTime, FisMatch) values (?, ?, ?, ?, ?, now(),?)");
                                                    }
                                                    else
                                                    {
                                                        query2->prepare("UPDATE "+tablePreview+" SET RecordID =?, Werk =?, SPJ =?, KNR =?, VIN =?, LocalDateTime=now(), FisMatch =? WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+tablePreview+")as t)");
                                                    }
                                                    query2->addBindValue(value["RecordID"].toInt());
                                                    query2->addBindValue(value["Werk"].toString());
                                                    query2->addBindValue(value["SPJ"].toString());
                                                    query2->addBindValue(value["KNR"].toString());
                                                    query2->addBindValue(value["VIN"].toString());

                                                    QJson::Serializer serializer;
                                                    bool ok;
                                                    QByteArray match = serializer.serialize(value["FisMatch"],&ok);

                                                    if(ok)
                                                        query2->addBindValue(match);
                                                    else
                                                    {
                                                        query2->addBindValue(match);
                                                        DTdebug() << "match serialze fail:" << serializer.errorMessage()<<"FisMatch:"<<value["FisMatch"];
                                                    }

                                                    if(!query2->exec())
                                                        DTdebug() <<"update FisPreview fail "<<query2->lastError();

                                                    tmpGetFlag = true;
                                                    mGetFisOk = true;

                                                    DTdebug()<< "update FisPreview" << value["RecordID"].toInt() << value["Werk"].toString() << value["SPJ"].toString() << value["KNR"].toString() << value["VIN"].toString();
                                                }
                                            }
                                        }
                                        DTdebug()<< "MEBFisMatch" << MEBFisMatch <<"tmpGetFlag"<<tmpGetFlag;
                                        if(tmpGetFlag && MEBFisMatch)
                                        {
                                            UpdateFisList();
                                        }
                                    }
                                    else
                                        DTdebug()<<"What's wrong"<<bytes;
                                }
                                else
                                {
                                    DTdebug()<<"Get FisPreview fail ";//    <<result["Error"].toString();
                                    if(result["ErrorCode"].toString()== "1")
                                    {
                                        DTdebug()<<"Server Json parse fail"<<json;
                                    }
                                }
                            }
                        }
                        else if(reply->error() != QNetworkReply::NoError)
                        {
                            DTdebug()<<"FisPreview handle errors here";
                            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                            DTdebug( )<<"FisPreview found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
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

            if(!mDataBase.isOpen())
            {
                mDataBase = mysql_open(QString("MESUploadSVW2"));
                if(mDataBase.isOpen())
                    query2 = new QSqlQuery(mDataBase);
            }
            if(mDataBase.isOpen())
            {
                if(!query2->exec("SELECT RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, Cycle, Operator, Program, JobStatus, Order_id, `MaxValue`, `MinValue`, Channel, Type,"
                                "Torque1,Torque_Max1,Torque_Min1,Angle1,Angle_Max1,Angle_Min1,Torque2 ,Torque_Max2 ,Torque_Min2 ,Angle2 ,Angle_Max2 ,"
                                "Angle_Min2 ,Torque3 ,Torque_Max3 ,Torque_Min3 ,Angle3 ,Angle_Max3 ,Angle_Min3 ,Torque4 ,Torque_Max4 ,Torque_Min4 ,Angle4 ,"
                                "Angle_Max4 ,Angle_Min4 ,Torque5 ,Torque_Max5 ,Torque_Min5 ,Angle5 ,Angle_Max5 ,Angle_Min5 ,Torque_Max ,"
                                "Torque_Min ,Angle_Max ,Angle_Min "
                                " FROM "+Localtable +" Where UploadMark =4 and Curve !='null' order by RecordID limit 1"))
                {
                    DTdebug()<<"SELECT fail "<<query2->lastError();
                }
                else
                {
                    if(query2->next())
                    {
                        QString ID = query2->value(0).toString();
                        double d_tourque = query2->value(3).toDouble() ;
                        double d_angle = query2->value(4).toDouble() ;
                        QVariantMap dataMap;
                        dataMap.insert("Type","Set");
                        dataMap.insert("Target","TighteningData");
                        QVariantMap data;
                        data.insert("Version",1);
                        data.insert("ID", query2->value(0).toInt());
                        if(query2->value(16).toString() == "manual")
                        {
                            data.insert("IDCode", "");
                            data.insert("TestVin",query2->value(1).toString());
                        }
                        else
                            data.insert("IDCode", query2->value(1).toString());
                        data.insert("ScrewID", query2->value(2).toString());
                        data.insert("Torque", d_tourque);
                        data.insert("Angle",d_angle );
                        data.insert("Curve", query2->value(5).toString());
                        data.insert("TighteningStatus", query2->value(6).toString());
                        data.insert("TighteningTime", query2->value(7).toString().left(10)+" "+query2->value(7).toString().right(8));
                        data.insert("Cycle", query2->value(8).toInt());
                        data.insert("Operator", query2->value(9).toString());
                        data.insert("Program", query2->value(10).toInt());
                        data.insert("JobStatus", query2->value(11).toInt());
                        data.insert("IIO", query2->value(12).toInt());

                        QString maxValue = QString::number(query2->value(13).toDouble(), 10, 2);
                        QString minValue = QString::number(query2->value(14).toDouble(), 10, 2);

                        data.insert("MaxValue", maxValue);
                        data.insert("MinValue", minValue);

                        data.insert("Channel", query2->value(15).toInt());

                        data.insert("Type",query2->value(16).toString());
                        data.insert("Torque1", query2->value(17).toDouble());
                        data.insert("Torque_Max1", query2->value(18).toDouble());
                        data.insert("Torque_Min1", query2->value(19).toDouble());
                        data.insert("Angle1", query2->value(20).toDouble());
                        data.insert("Angle_Max1", query2->value(21).toDouble());
                        data.insert("Angle_Min1", query2->value(22).toDouble());
                        data.insert("Torque2", query2->value(23).toDouble());
                        data.insert("Torque_Max2", query2->value(24).toDouble());
                        data.insert("Torque_Min2", query2->value(25).toDouble());
                        data.insert("Angle2", query2->value(26).toDouble());
                        data.insert("Angle_Max2", query2->value(27).toDouble());
                        data.insert("Angle_Min2", query2->value(28).toDouble());
                        data.insert("Torque3", query2->value(29).toDouble());
                        data.insert("Torque_Max3", query2->value(30).toDouble());
                        data.insert("Torque_Min3", query2->value(31).toDouble());
                        data.insert("Angle3", query2->value(32).toDouble());
                        data.insert("Angle_Max3", query2->value(33).toDouble());
                        data.insert("Angle_Min3", query2->value(34).toDouble());
                        data.insert("Torque4", query2->value(35).toDouble());
                        data.insert("Torque_Max4", query2->value(36).toDouble());
                        data.insert("Torque_Min4", query2->value(37).toDouble());
                        data.insert("Angle4", query2->value(38).toDouble());
                        data.insert("Angle_Max4", query2->value(39).toDouble());
                        data.insert("Angle_Min4", query2->value(40).toDouble());
                        data.insert("Torque5", query2->value(41).toDouble());
                        data.insert("Torque_Max5", query2->value(42).toDouble());
                        data.insert("Torque_Min5", query2->value(43).toDouble());
                        data.insert("Angle5", query2->value(44).toDouble());
                        data.insert("Angle_Max5", query2->value(45).toDouble());
                        data.insert("Angle_Min5", query2->value(46).toDouble());
                        data.insert("Torque_Max", query2->value(47).toDouble());
                        data.insert("Torque_Min", query2->value(48).toDouble());
                        data.insert("Angle_Max", query2->value(49).toDouble());
                        data.insert("Angle_Min", query2->value(50).toDouble());

                        data.insert("Line_ID", Line_ID);
                        data.insert("Station", Station);
                        data.insert("IpAddress", WirelessIp);
                        data.insert("Workshop", "Assembly");
                        data.insert("WorkShift", "");
                        dataMap.insert("Parameter",data);

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);
                        DTdebug()<<"json:"<< json;

                        req.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/submit?"));
                        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                        req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                        if (ok)
                        {
                            DTdebug() << "post RecordID:"<<query2->value(0).toInt()<<"Cycle:"<<query2->value(8).toInt();
                            QNetworkReply *reply = manager->post(req,json);
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
                                            if(query2->exec("UPDATE "+Localtable+" SET UploadMark = 1,UploadTime = now() WHERE RecordID = "+RecordID))
                                            {
                                                DTdebug()<<"update 1 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                            }
                                            else
                                            {
                                                DTdebug()<<"update 1 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2->lastError();
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
                                            if(query2->exec("UPDATE "+Localtable+" SET UploadMark = 7, UploadTime = now() WHERE RecordID = "+ID))
                                            {
                                                DTdebug()<<"update 7 success: RecordID:"<<ID;
                                            }
                                            else
                                            {
                                                DTdebug()<<"update 7 fail: RecordID:"<<ID<<query2->lastError();
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
                                                    if(query2->exec("UPDATE "+Localtable+" SET UploadMark = 6, UploadTime = now() WHERE RecordID = "+RecordID))
                                                    {
                                                        wrongRecordID = -1;
                                                        wrongTimes = 0;
                                                        DTdebug()<<"update 6 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                                    }
                                                    else
                                                    {
                                                        DTdebug()<<"update 6 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2->lastError();
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
                    else
                    {
                        DTdebug() << "Can't find UploadMark =4";
                    }
                }

                if(!query2->exec("SELECT RecordID, Station, VEN, TYP, SNR, VNR, IPA, UploadTime, ECL, ERC, ERT, EST, KNR, LocalDateTime FROM TighteningDataErrors Where UploadMark =4 order by RecordID limit 1"))
                {
                    DTdebug()<<"SELECT fail from TighteningDataErrors"<<query2->lastError();
                }
                else
                {
                    if(query2->next())
                    {
                        curRecordID = query2->value(0).toInt();

                        QVariantMap dataMap;
                        dataMap.insert("Station", query2->value(1).toString());
                        dataMap.insert("VEN", query2->value(2).toString());
                        dataMap.insert("TYP", query2->value(3).toString());
                        dataMap.insert("SNR", query2->value(4).toString());
                        dataMap.insert("VNR", query2->value(5).toString());
                        dataMap.insert("IPA", query2->value(6).toString());
                        dataMap.insert("UploadTime", query2->value(7).toString());
                        dataMap.insert("ECL", query2->value(8).toString());
                        dataMap.insert("ERC", query2->value(9).toString());
                        dataMap.insert("ERT", query2->value(10).toString());
                        dataMap.insert("EST", query2->value(11).toString());
                        dataMap.insert("KNR", query2->value(12).toString());

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);
                        DTdebug()<<"json_updateError:"<< json;
                        req.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/uploadERR?"));
                        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                        req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());
                        if (ok)
                        {
                            QNetworkReply *reply = manager->post(req,json);
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
                                QJson::Parser parser;
                                bool ok;
                                QVariantMap result = parser.parse(bytes, &ok).toMap();
                                if (!ok) {
                                    DTdebug()<<"post Insert An error occurred during parsing"<<bytes;
                                }
                                else
                                {
                                    if(result["Result"].toInt())
                                    {
                                        if(query2->exec("UPDATE TighteningDataErrors SET UploadMark = 1,UploadTime = now() WHERE RecordID = "+QString::number(curRecordID)))
                                        {
                                            DTdebug()<<"update 1 success: RecordID:"<<curRecordID;
                                        }
                                        else
                                        {
                                            DTdebug()<<"update 1 fail: RecordID:"<<curRecordID<<query2->lastError();
                                        }
                                    }
                                    else
                                    {
                                        DTdebug()<<"Result = 0";
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
        if(dataInsertCount >= 360)
        {
            dataInsertCount = 0;
            DTdebug()<<"process UploadMark = 7";
            if(!mDataBase.isOpen())
            {
                mDataBase = mysql_open(QString("MESUploadSVW2"));
                if(mDataBase.isOpen())
                    query2 = new QSqlQuery(mDataBase);
            }
            if(mDataBase.isOpen())
            {
                if(!query2->exec("SELECT RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, Cycle, Operator, Program, JobStatus, Order_id, `MaxValue`, `MinValue`, Channel, Type,"
                                "Torque1,Torque_Max1,Torque_Min1,Angle1,Angle_Max1,Angle_Min1,Torque2 ,Torque_Max2 ,Torque_Min2 ,Angle2 ,Angle_Max2 ,"
                                "Angle_Min2 ,Torque3 ,Torque_Max3 ,Torque_Min3 ,Angle3 ,Angle_Max3 ,Angle_Min3 ,Torque4 ,Torque_Max4 ,Torque_Min4 ,Angle4 ,"
                                "Angle_Max4 ,Angle_Min4 ,Torque5 ,Torque_Max5 ,Torque_Min5 ,Angle5 ,Angle_Max5 ,Angle_Min5 ,Torque_Max ,"
                                "Torque_Min ,Angle_Max ,Angle_Min "
                                " FROM "+Localtable +" Where UploadMark =7 and Curve !='null' order by RecordID limit 1"))
                {
                    DTdebug()<<"SELECT UploadMark = 7 fail "<<query2->lastError();
                }
                else
                {
                    if(query2->next())
                    {
                        QString ID = query2->value(0).toString();
                        double d_tourque = query2->value(3).toDouble() ;
                        double d_angle = query2->value(4).toDouble() ;
                        QVariantMap dataMap;
                        dataMap.insert("Type","Set");
                        dataMap.insert("Target","TighteningData");
                        QVariantMap data;
                        data.insert("Version",1);
                        data.insert("ID", query2->value(0).toInt());
                        if(query2->value(16).toString() == "manual")
                        {
                            data.insert("IDCode", "");
                            data.insert("TestVin",query2->value(1).toString());
                        }
                        else
                            data.insert("IDCode", query2->value(1).toString());
                        data.insert("ScrewID", query2->value(2).toString());
                        data.insert("Torque", d_tourque);
                        data.insert("Angle",d_angle );
                        data.insert("Curve", query2->value(5).toString());
                        data.insert("TighteningStatus", query2->value(6).toString());
                        data.insert("TighteningTime", query2->value(7).toString().left(10)+" "+query2->value(7).toString().right(8));
                        data.insert("Cycle", query2->value(8).toInt());
                        data.insert("Operator", query2->value(9).toString());
                        data.insert("Program", query2->value(10).toInt());
                        data.insert("JobStatus", query2->value(11).toInt());
                        data.insert("IIO", query2->value(12).toInt());

                        QString maxValue = QString::number(query2->value(13).toDouble(), 10, 2);
                        QString minValue = QString::number(query2->value(14).toDouble(), 10, 2);

                        data.insert("MaxValue", maxValue);
                        data.insert("MinValue", minValue);
                        data.insert("Channel", query2->value(15).toInt());

                        data.insert("Type",query2->value(16).toString());

                        data.insert("Torque1", query2->value(17).toDouble());
                        data.insert("Torque_Max1", query2->value(18).toDouble());
                        data.insert("Torque_Min1", query2->value(19).toDouble());
                        data.insert("Angle1", query2->value(20).toDouble());
                        data.insert("Angle_Max1", query2->value(21).toDouble());
                        data.insert("Angle_Min1", query2->value(22).toDouble());
                        data.insert("Torque2", query2->value(23).toDouble());
                        data.insert("Torque_Max2", query2->value(24).toDouble());
                        data.insert("Torque_Min2", query2->value(25).toDouble());
                        data.insert("Angle2", query2->value(26).toDouble());
                        data.insert("Angle_Max2", query2->value(27).toDouble());
                        data.insert("Angle_Min2", query2->value(28).toDouble());
                        data.insert("Torque3", query2->value(29).toDouble());
                        data.insert("Torque_Max3", query2->value(30).toDouble());
                        data.insert("Torque_Min3", query2->value(31).toDouble());
                        data.insert("Angle3", query2->value(32).toDouble());
                        data.insert("Angle_Max3", query2->value(33).toDouble());
                        data.insert("Angle_Min3", query2->value(34).toDouble());
                        data.insert("Torque4", query2->value(35).toDouble());
                        data.insert("Torque_Max4", query2->value(36).toDouble());
                        data.insert("Torque_Min4", query2->value(37).toDouble());
                        data.insert("Angle4", query2->value(38).toDouble());
                        data.insert("Angle_Max4", query2->value(39).toDouble());
                        data.insert("Angle_Min4", query2->value(40).toDouble());
                        data.insert("Torque5", query2->value(41).toDouble());
                        data.insert("Torque_Max5", query2->value(42).toDouble());
                        data.insert("Torque_Min5", query2->value(43).toDouble());
                        data.insert("Angle5", query2->value(44).toDouble());
                        data.insert("Angle_Max5", query2->value(45).toDouble());
                        data.insert("Angle_Min5", query2->value(46).toDouble());


                        data.insert("Torque_Max", query2->value(47).toDouble());
                        data.insert("Torque_Min", query2->value(48).toDouble());
                        data.insert("Angle_Max", query2->value(49).toDouble());
                        data.insert("Angle_Min", query2->value(50).toDouble());

                        data.insert("Line_ID", Line_ID);
                        data.insert("Station", Station);
                        data.insert("IpAddress", WirelessIp);
                        data.insert("Workshop", "Assembly");
                        data.insert("WorkShift", "");
                        dataMap.insert("Parameter",data);

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);

                        req.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/submit?"));
                        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
                        req.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

                        if (ok)
                        {
                            DTdebug() << "post RecordID:"<<query2->value(0).toInt()<<"Cycle:"<<query2->value(8).toInt();
                            QNetworkReply *reply = manager->post(req,json);
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
                                DTdebug() << "Server Reply:" <<bytes;
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
                                            if(query2->exec("UPDATE "+Localtable+" SET UploadMark = 1,UploadTime = now() WHERE RecordID = "+RecordID))
                                            {
                                                DTdebug()<<"update 1 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                            }
                                            else
                                            {
                                                DTdebug()<<"update 1 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2->lastError();
                                            }
                                        }
                                        else
                                            DTdebug()<<"What's wrong"<<bytes;
                                    }
                                    else
                                    {
                                        if(result["ErrorCode"].toString()== "1")
                                        {
                                            DTdebug()<<"UploadMark = 7 upload fail, Server Json parse fail"<<json;
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
                                                    if(query2->exec("UPDATE "+Localtable+" SET UploadMark = 6, UploadTime = now() WHERE RecordID = "+RecordID))
                                                    {
                                                        wrongRecordID = -1;
                                                        wrongTimes = 0;
                                                        DTdebug()<<"update 6 success: RecordID:"<<RecordID<<"Cycle:"<<Cycle;
                                                    }
                                                    else
                                                    {
                                                        DTdebug()<<"update 6 fail: RecordID:"<<RecordID<<"Cycle:"<<Cycle<<query2->lastError();
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
                    else
                    {
                        DTdebug() << "Can't find UploadMark =4";
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

QString MESUploadSVW2::GetVIN(QString pWerk, QString pSPJ, QString pKNR)
{
    QString selectVin = QString("select VIN from ")+tablePreview+QString(" where Werk ='%1' and SPJ ='%2' and KNR ='%3'").arg(pWerk).arg(pSPJ).arg(pKNR);
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"GetVIN error!";
        return QString();
    }
    query2->exec(selectVin);
    if(query2->next())  //查到了匹配
    {
        DTdebug() << "****select VIN******" << selectVin;
        QByteArray VINCode;
        VINCode = query2->value(0).toByteArray();
        return QString(VINCode);
    }
    return QString();
}

QStringList MESUploadSVW2::GetVINs()
{
    QString selectVin = QString("select VIN from ")+tablePreview;
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"GetVIN error!";
        return QStringList();
    }
    query2->exec(selectVin);
    DTdebug() << "****select VIN******" << selectVin;

    QStringList tmpList;
    while(query2->next())  //查到了匹配
    {
        QString tmpVin = query2->value(0).toByteArray();
        DTdebug() << "VIN: " << tmpVin;
        tmpList.append(tmpVin);
    }
    return tmpList;
}


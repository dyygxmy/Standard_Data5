#include "mesuploadAQCHERY.h"

bool gPassbyResultState;

bool IsValid(QByteArray pData, QByteArray pMatch)
{
    int tmpDataLen = pData.size();
    int tmpMatchLen = pMatch.size();
    if(tmpMatchLen != tmpDataLen)
        return false;
    for(int i=0;i<pMatch.size();i++)
    {
        if(pMatch.at(i) == '?')
        {
            continue;
        }
        else if(pMatch.at(i) != pData.at(i))
        {
            return false;
        }
    }
    return true;
}

MESUploadAQCHERY::MESUploadAQCHERY(QObject *parent) :
    QObject(parent)
{
    mHttpCmd = E_CMD_None;
    this->moveToThread(&m_thread);
    m_thread.start();
}

void MESUploadAQCHERY::init()
{
    Factory = factory;
    DTdebug() <<"MESUpload thread start!!"<<Factory;
    TIMEOUT = (5 * 1000);
    isFirst = true;
    mNeedSetErrFalse = false;
    GetTimes = 0;
    wrongRecordID = -1;
    wrongTimes = 0;
    mResultState = 0;
    gPassbyResultState = 0;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    mDataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    mServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    mDeviceNo = configIniRead->value("baseinfo/DeviceNo").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    if(factory == "AQCHERY")
        req_uploaddata.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/submit?"));
    else
        req_uploaddata.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/uploaddata?"));
    req_gettime.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/gettime?"));

    reqVinInfo.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/GetVinInfoByDeviceNo?"));  //add by wxm 2021-2-22


    QTimer::singleShot(1000,this,SLOT(dataInsert()));

    mSendHttpTimer.setInterval(1000);
    connect(&mSendHttpTimer, SIGNAL(timeout()), this, SLOT(slot_SendHttp()));
    mSendHttpTimer.start();
}

void MESUploadAQCHERY::slot_SendHttp()
{
    if(mHttpCmd == E_CMD_None)
        return;

    QVariantMap dataMap;
    if(mHttpCmd == E_CMD_Ready)
    {
        reqUploadStationStatus.setUrl(QUrl("http://"+mDataServerIp+":"+mServerPort+"/UploadStationStatus?"));  //add by wxm 2021-2-22
        dataMap.insert("DeviceNo", mDeviceNo);
        dataMap.insert("Parameter", "Ready");
        dataMap.insert("Ready", 1);
    }

    QJson::Serializer serializer;
    bool ok;
    QByteArray json = serializer.serialize(dataMap, &ok);
    reqUploadStationStatus.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    reqUploadStationStatus.setHeader(QNetworkRequest::ContentLengthHeader, json.length());
    if(ok)
    {
        QNetworkReply *reply = manager->post(reqUploadStationStatus,json);
        DTdebug()<< "Http send json:" << json;
        reply->ignoreSslErrors();
        QTime t_time;
        t_time.start();
        bool timeout = false;
        while (!reply->isFinished()) {
            QApplication::processEvents();
            if (t_time.elapsed() >= TIMEOUT) {
                timeout = true;
                DTdebug()<<"Http reply timeout : "<<mHttpCmd;
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
                DTdebug()<<"Http reply parsing error"<<bytes;
            }
            else
            {
                if(result["Result"].toBool())
                {
                    DTdebug()<<"Http reply result success "<<QDateTime::currentDateTime().toString();
                    mResultState = 1;
                }
                else
                {
                    DTdebug()<<"Http reply result fail "<<QDateTime::currentDateTime().toString();
                }
            }
        }
        else if(reply->error() != QNetworkReply::NoError)
        {
            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
            DTdebug( )<<"Http reply error -- code: %d %d %s\n", statusCodeV.toInt(), (int)reply->error(), reply->errorString();
        }
        reply->deleteLater();
    }
    else
    {
        DTdebug() << "Http json content error:" << serializer.errorMessage();
    }
}

void MESUploadAQCHERY::slotSetErrFalse()
{
    slot_UploadVinVerify(0);
}

void MESUploadAQCHERY::slotUploadResult0Timer()
{
    slot_UploadStopTightening(0);
}


void MESUploadAQCHERY::PLCDisconnected()
{
    mResultState = 0;
    gPassbyResultState = 0;
    emit signalPLCHeartbeat(false);
}



void MESUploadAQCHERY::slot_SendPLCHeartbeat()
{
    {
        QVariantMap dataMap;
        dataMap.insert("DeviceNo", mDeviceNo);
        dataMap.insert("Parameter", "Heartbeat");
        dataMap.insert("Heartbeat", 1);
        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);

        reqUploadStationStatus.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reqUploadStationStatus.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            DTdebug() << "SendPLCHeartbeat";
            QNetworkReply *reply = manager->post(reqUploadStationStatus,json);
            DTdebug()<< "json_PLCHeartbeat:" << json;
            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    PLCDisconnected();
                    DTdebug()<<"SendPLCHeartbeat reply timeout";
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
                    DTdebug()<<"SendPLCHeartbeat An error occurred during parsing"<<bytes;
                    PLCDisconnected();
                }
                else
                {
                    if(result["Result"].toBool())
                    {
                        DTdebug()<<"SendPLCHeartbeat success "<<QDateTime::currentDateTime().toString();
                        emit signalPLCHeartbeat(true);
                    }
                    else
                    {
                        DTdebug()<<"SendPLCHeartbeat fail "<<QDateTime::currentDateTime().toString();
                        PLCDisconnected();
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                PLCDisconnected();
                DTdebug()<<"SendPLCHeartbeat handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                DTdebug( )<<"SendPLCHeartbeat found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                DTdebug()<<reply->errorString();
            }
            reply->deleteLater();
        }
        else
        {
            DTdebug() << "SendPLCHeartbeat Something went wrong:" << serializer.errorMessage();
        }
    }

    QTimer::singleShot(1000,this,SLOT(slot_SendPLCHeartbeat()));
}


void MESUploadAQCHERY::slot_SendReady()
{
    if(mResultState == 0)
    {

    }
    QTimer::singleShot(1000,this,SLOT(slot_SendReady()));
}


void MESUploadAQCHERY::slot_GetVinInfo()
{
    {
        QVariantMap dataMap;
        dataMap.insert("DeviceNo", mDeviceNo);
        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);

        reqVinInfo.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reqVinInfo.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            DTdebug() << "GetVinInfoByDeviceNo";
            QNetworkReply *reply = manager->post(reqVinInfo,json);
            DTdebug()<< "json_getVinInfo:" << json;
            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    DTdebug()<<"GetVinInfoByDeviceNo reply timeout";
                    break;
                }
            }
            if (!timeout && reply->error() == QNetworkReply::NoError)
            {
                QByteArray bytes = reply->readAll();
                QJson::Parser parser;
                bool ok;
                //test add by wxm
#if 0
                int index = bytes.indexOf("}");
                if(index > 0)
                {
                    bytes = bytes.mid(0, index+1);
                }

                //for test
                QVariantMap tmpTestMap;
                tmpTestMap.insert("Result", 1);
                tmpTestMap.insert("Station", "dddddd");
                tmpTestMap.insert("VIN", "LSVABC0T8AB123476");
                tmpTestMap.insert("JobID", "1234");
                QJson::Serializer tmpTestserializer;
                bool tmpTestMapok;
                bytes = tmpTestserializer.serialize(tmpTestMap, &tmpTestMapok);
#endif
                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    DTdebug()<<"GetVinInfoByDeviceNo An error occurred during parsing"<<bytes;
                }
                else
                {
                    if(result["Result"].toBool())
                    {
                        DTdebug()<<"GetVinInfoByDeviceNo success "<<QDateTime::currentDateTime().toString();
                        QString tmpStation = result["Station"].toString();
                        emit signalSetStation(tmpStation);
                        QString tmpVIN = result["VIN"].toString();
                        QString tmpCarType = result["JobID"].toString();

                        if(mNeedSetErrFalse && !mSetErrFalseTimer.isActive())
                        {
                            if(tmpVIN.isNull() && tmpCarType.isNull())
                            {
                                mSetErrFalseTimer.start();
                            }
                        }

                        if(tmpCarType == "0000")
                        {
                            mUploadResult0Timer.start();
                        }
                        if(tmpVIN.size() == 17)
                        {
                            isEqual = false;
                            if(seriallist.isEmpty())
                            {
                                seriallist.push_back(tmpVIN);
                                whichindex++;
                            }
                            else
                            {
                                QList<QString>::iterator i;
                                for (i = seriallist.begin(); i != seriallist.end(); ++i)
                                {
                                    if(!(QString::compare(tmpVIN, *i, Qt::CaseSensitive)))
                                    {
                                        isEqual = true;
                                        break;
                                    }
                                }
                                if(!isEqual)
                                {
                                    if(seriallist.size() == 10)
                                    {
                                        seriallist[whichindex] = tmpVIN;
                                    }
                                    else
                                    {
                                        seriallist.push_back(tmpVIN);
                                    }
                                    whichindex++;
                                    if(whichindex == 10)
                                        whichindex = 0;
                                }
                            }

                            DTdebug()<<"send SerialNumbs "<<tmpVIN;
                            if(!isEqual)
                            {
                                emit serialCom(tmpVIN+tmpCarType,isEqual,"VIN+CarType");
                            }
                            else
                            {
                                DTdebug()<<"isEqual == 1";
                            }
                        }
                    }
                    else
                    {
                        DTdebug()<<"GetVinInfoByDeviceNo fail "<<QDateTime::currentDateTime().toString();
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                DTdebug()<<"UploadStationStatus handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                DTdebug( )<<"UploadStationStatus found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
                DTdebug()<<reply->errorString();
            }
            reply->deleteLater();
        }
        else
        {
            DTdebug() << "UploadStationStatus Something went wrong:" << serializer.errorMessage();
        }
    }
    QTimer::singleShot(1000,this,SLOT(slot_GetVinInfo()));
}

void MESUploadAQCHERY::slot_UploadVinVerify(int pResult)
{
    QVariantMap dataMap;
    dataMap.insert("DeviceNo", mDeviceNo);
    dataMap.insert("Parameter", "VinErr");
    dataMap.insert("VinErr", pResult);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json = serializer.serialize(dataMap, &ok);

    reqUploadStationStatus.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    reqUploadStationStatus.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

    if (ok)
    {
        DTdebug() << "slot_UploadVinVerify";
        QNetworkReply *reply = manager->post(reqUploadStationStatus,json);
        DTdebug()<< "json_vinVerify:" << json;
        reply->ignoreSslErrors();
        QTime t_time;
        t_time.start();

        bool timeout = false;

        while (!reply->isFinished()) {
            QApplication::processEvents();
            if (t_time.elapsed() >= TIMEOUT) {
                timeout = true;
                DTdebug()<<"slot_UploadVinVerify reply timeout";
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
                DTdebug()<<"slot_UploadVinVerify An error occurred during parsing"<<bytes;
            }
            else
            {
                if(result["Result"].toBool())
                {
                    DTdebug()<<"slot_UploadVinVerify success "<<QDateTime::currentDateTime().toString();
                    if(pResult == 1)
                    {
                        mNeedSetErrFalse = true;
                    }
                    if(pResult == 0)
                    {
                        if(mNeedSetErrFalse)
                        {
                            mSetErrFalseTimer.stop();
                            mNeedSetErrFalse = false;
                        }
                    }
                }
                else
                {
                    DTdebug()<<"slot_UploadVinVerify fail "<<QDateTime::currentDateTime().toString();
                }
            }
        }
        else if(reply->error() != QNetworkReply::NoError)
        {
            DTdebug()<<"slot_UploadVinVerify handle errors here";
            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
            DTdebug( )<<"slot_UploadVinVerify found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
            DTdebug()<<reply->errorString();
        }
        reply->deleteLater();
    }
    else
    {
        DTdebug() << "slot_UploadVinVerify Something went wrong:" << serializer.errorMessage();
    }
}

void MESUploadAQCHERY::slot_UploadBeginTightening()
{
    QVariantMap dataMap;
    dataMap.insert("DeviceNo", mDeviceNo);
    dataMap.insert("Parameter", "Done_OK_Tighting");
    dataMap.insert("Done", 0);
    dataMap.insert("OK", 0);
    dataMap.insert("Tighting", 1);
    QJson::Serializer serializer;
    bool ok;
    QByteArray json = serializer.serialize(dataMap, &ok);

    reqUploadStationStatus.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    reqUploadStationStatus.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

    if (ok)
    {
        DTdebug() << "slot_UploadBeginTightening";
        QNetworkReply *reply = manager->post(reqUploadStationStatus,json);
         DTdebug()<< "json_beginTightening:" << json;
        reply->ignoreSslErrors();
        QTime t_time;
        t_time.start();

        bool timeout = false;

        while (!reply->isFinished()) {
            QApplication::processEvents();
            if (t_time.elapsed() >= TIMEOUT) {
                timeout = true;
                DTdebug()<<"slot_UploadBeginTightening reply timeout";
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
                DTdebug()<<"slot_UploadBeginTightening An error occurred during parsing"<<bytes;
            }
            else
            {
                if(result["Result"].toBool())
                {
                    DTdebug()<<"slot_UploadBeginTightening success "<<QDateTime::currentDateTime().toString();
                }
                else
                {
                    DTdebug()<<"slot_UploadBeginTightening fail "<<QDateTime::currentDateTime().toString();
                }
            }
        }
        else if(reply->error() != QNetworkReply::NoError)
        {
            DTdebug()<<"slot_UploadBeginTightening handle errors here";
            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
            DTdebug( )<<"slot_UploadBeginTightening found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
            DTdebug()<<reply->errorString();
        }
        reply->deleteLater();
    }
    else
    {
        DTdebug() << "slot_UploadBeginTightening Something went wrong:" << serializer.errorMessage();
    }
}

void MESUploadAQCHERY::slot_UploadStopPassby()
{
    if(gPassbyResultState == 0)
        slot_UploadStopTightening(2);  //add by wxm 2021-3-10
    QTimer::singleShot(1000,this,SLOT(slot_UploadStopPassby()));  //add by wxm 2021-3-11
}

void MESUploadAQCHERY::slot_UploadStopTightening(int pResult)
{
    QVariantMap dataMap;
    dataMap.insert("DeviceNo", mDeviceNo);
    if(pResult == 0)
    {
        dataMap.insert("Parameter", "Done_OK_Tighting");
        dataMap.insert("Done", 1);
        dataMap.insert("OK", 1);
        dataMap.insert("Tighting", 0);
    }
    else if(pResult == 1)
    {
        dataMap.insert("Parameter", "Passby");
        dataMap.insert("Passby", 1);
        dataMap.insert("Tighting", 0);
    }
    else
    {
        dataMap.insert("Parameter", "Passby");
        dataMap.insert("Passby", 0);
        dataMap.insert("Tighting", 0);
    }
    QJson::Serializer serializer;
    bool ok;
    QByteArray json = serializer.serialize(dataMap, &ok);

    reqUploadStationStatus.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    reqUploadStationStatus.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

    if (ok)
    {
        DTdebug() << "slot_UploadStopTightening";
        QNetworkReply *reply = manager->post(reqUploadStationStatus,json);
        DTdebug()<< "json_stopTightening:" << json;
        reply->ignoreSslErrors();
        QTime t_time;
        t_time.start();

        bool timeout = false;

        while (!reply->isFinished()) {
            QApplication::processEvents();
            if (t_time.elapsed() >= TIMEOUT) {
                timeout = true;
                DTdebug()<<"slot_UploadStopTightening reply timeout";
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
                DTdebug()<<"slot_UploadStopTightening An error occurred during parsing"<<bytes;
            }
            else
            {
                if(result["Result"].toBool())
                {
                    DTdebug()<<"slot_UploadStopTightening success "<<QDateTime::currentDateTime().toString();

                    if(pResult == 2)
                    {
                        gPassbyResultState = 1;
                    }

                    if(pResult == 0)
                    {
                        mUploadResult0Timer.stop();
                    }
                }
                else
                {
                    DTdebug()<<"slot_UploadStopTightening fail "<<QDateTime::currentDateTime().toString();
                }
            }
        }
        else if(reply->error() != QNetworkReply::NoError)
        {
            DTdebug()<<"slot_UploadStopTightening handle errors here";
            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
            DTdebug( )<<"slot_UploadStopTightening found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error();
            DTdebug()<<reply->errorString();
        }
        reply->deleteLater();
    }
    else
    {
        DTdebug() << "slot_UploadStopTightening Something went wrong:" << serializer.errorMessage();
    }
}

void MESUploadAQCHERY::dataInsert()
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
                        data.insert("LineName", LineName); //add by wxm 2020/11/4
//                        data.insert("Workshop", "Assembly");
//                        data.insert("WorkShift", "");
                        dataMap.insert("Parameter",data);

                        QJson::Serializer serializer;
                        bool ok;
                        QByteArray json = serializer.serialize(dataMap, &ok);

//                        DTdebug()<<"@@@@@@@@@@"<<json;
                        DTdebug()<<"json:"<< json;   //add by wxm 2020/6/1

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
void MESUploadAQCHERY::mysql_open()
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

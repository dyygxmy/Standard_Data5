#include "baicinfointerface.h"

BaicInfoInterface::BaicInfoInterface(QObject *parent) :
    QObject(parent)
{
    qDebug() <<"BaicInfoInterface start!!"<<factory;
    TIMEOUT = (5 * 1000);
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);
    req_Getproduction.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/getproduction?"));
    req_ErrorMessage.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/ErrorMessage?"));
}

QString BaicInfoInterface::getProduction(QString Vin)
{
    QString Production_Dis = "";
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        QVariantMap dataMap;
        dataMap.insert("Type","Get");
        dataMap.insert("Target","Production_Information");
        dataMap.insert("CarNumber",Vin);
        dataMap.insert("Code_status",1);

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);
        qDebug()<<"get Production json"<<json;

        req_Getproduction.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_Getproduction.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            qDebug()<<"Get Production";
            QNetworkReply *reply =manager->post(req_Getproduction,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    qDebug()<< "reply timeout Get Production fail "<<Vin;
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                qDebug()<<"************************************ Get Production"<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    qDebug()<<"Get Production An error occurred during parsing"<<bytes;
                }
                else {
                    bool Result = result["result"].toBool();
                    if(!Result)
                    {
//                        int resultCode = result["resultCode"].toInt();
                        qDebug()<< "Get Production fail result = 0";
                    }
                    else if(Result)
                    {
                        Body_NO = result["CarNumber"].toString();
                        Production_Dis = result["Production_Dis"].toString();
                        qDebug()<< "Get Production success:"<<Vin;
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                qDebug()<<"Get Production handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                qDebug( "Get Production found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
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
    {
        WIFIlock.unlock();
        qDebug()<< "wifi unconnect get Production fail";
    }
    return Production_Dis;
}

bool BaicInfoInterface::ErrorMessage(QString ErrorData)
{
    bool isSuccess = false;
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        QVariantMap dataMap;
        dataMap.insert("Type","Set");
        dataMap.insert("ErrorData",ErrorData);
        dataMap.insert("ErrorBy","dat@");

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);
        qDebug()<<"get Production json"<<json;

        req_ErrorMessage.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_ErrorMessage.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            qDebug()<<"Error Message";
            QNetworkReply *reply = manager->post(req_ErrorMessage,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    qDebug()<< "reply timeout Error Message fail "<<ErrorData;
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                qDebug()<<"************************************ Error Message"<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    qDebug()<<"Error Message An error occurred during parsing"<<bytes;
                }
                else {
                    bool Result = result["result"].toBool();
                    if(!Result)
                    {
                        qDebug()<< "Error Message fail"<<ErrorData;
                    }
                    else if(Result)
                    {
                        isSuccess = true;
                        qDebug()<< "Error Message success:"<<ErrorData;
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                qDebug()<<"Error Message handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                qDebug( "Error Message found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
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
    {
        WIFIlock.unlock();
        qDebug()<< "wifi unconnect Error Message fail";
    }
    return isSuccess;
}

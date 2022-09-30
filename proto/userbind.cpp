/*******************************************************
 * 创建人：Claire
 * 创建时间：2019年2月19日
 * 功能说明：用于株洲北汽用户的登录注销。可扩展
********************************************************/

#include "userbind.h"

userBind::userBind(QObject *parent) :
    QObject(parent)
{
    TIMEOUT = (5 * 1000);



    manager = new QNetworkAccessManager(this);
#if 0
    req_Getproduction.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/getproduction?"));
    req_ErrorMessage.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/ErrorMessage?"));
#endif
}

bool userBind::setUrlRequest(userBind::requestType type ,
                       QVariantMap dataMap)
{
    DTdebug() << type << dataMap;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;

    switch (type)
    {
    case login:
        req_Getproduction.setUrl(QUrl("http://"+DataServerIp+":51568/api/user/Login?"));
        req_ErrorMessage.setUrl(QUrl("http://"+DataServerIp+":51568/ErrorMessage?"));
        break;

    case logout:
        req_Getproduction.setUrl(QUrl("http://"+DataServerIp+":51568/api/user/Logout?"));
        req_ErrorMessage.setUrl(QUrl("http://"+DataServerIp+":51568/ErrorMessage?"));
        break;

    default:
//        req_Getproduction.setUrl(QUrl("https://api.liwenyang.cn/index.php?"));
//        req_ErrorMessage.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/ErrorMessage?"));
        break;
    }
    //logintest
//    WIFI_STATE = true ;
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);
        //logintest
//        req_Getproduction.setUrl(QUrl("https://api.liwenyang.cn/index.php?"));

        req_Getproduction.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_Getproduction.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            DTdebug()<<"Get Production";
            QNetworkReply *reply =manager->post(req_Getproduction,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    bindStatus = true;
                    DTdebug()<< "reply timeout Get Production fail ";
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError)
            {
                QByteArray bytes = reply->readAll();
                DTdebug()<<" result "<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();

                if (!ok) {
                    DTdebug()<<" An error occurred during parsing"<<bytes;
                }
                else
                {
                    bindStatus = result["result"].toBool();
                    DTdebug() << bindStatus ;
                    if(type == userBind::login)
                    {
                        if(bindStatus)
                        {
                            QVariantMap value = result["data"].toMap();
//                            userID = value["UserId"].toInt();
                            userName = value["UserName"].toString();
                        }
                        else
                        {
                            //重新登录
                        }
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                DTdebug()<<"Get Production handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                DTdebug( ) << "Get Production found error ....code: %d %d\n"<< statusCodeV.toInt()<<(int)reply->error();
                DTdebug()<<reply->errorString();

                //重新登录
                bindStatus=false;
            }

            reply->deleteLater();
        }
        else
        {
            DTdebug() << "Something went wrong:" << serializer.errorMessage();
            //重新登录
            bindStatus = false ;
        }
    }
    else
    {
        WIFIlock.unlock();
        bindStatus = true ;
        DTdebug()<< "wifi unconnect login fail";

    }

    DTdebug() << bindStatus ;
    return bindStatus;

}


bool userBind::ErrorMessage(QString ErrorData)
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
        DTdebug()<<"get Production json"<<json;

        req_ErrorMessage.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        req_ErrorMessage.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            DTdebug()<<"Error Message";
            QNetworkReply *reply = manager->post(req_ErrorMessage,json);

            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished()) {
                QApplication::processEvents();
                if (t_time.elapsed() >= TIMEOUT) {
                    timeout = true;
                    DTdebug()<< "reply timeout Error Message fail "<<ErrorData;
                    break;
                }
            }

            if (!timeout && reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                DTdebug()<<"************************************ Error Message"<<bytes;

                QJson::Parser parser;
                bool ok;

                QVariantMap result = parser.parse(bytes, &ok).toMap();
                if (!ok) {
                    DTdebug()<<"Error Message An error occurred during parsing"<<bytes;
                }
                else {
                    bool Result = result["result"].toBool();
                    if(!Result)
                    {
                        DTdebug()<< "Error Message fail"<<ErrorData;
                    }
                    else if(Result)
                    {
                        isSuccess = true;
                        DTdebug()<< "Error Message success:"<<ErrorData;
                    }
                }
            }
            else if(reply->error() != QNetworkReply::NoError)
            {
                DTdebug()<<"Error Message handle errors here";
                QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
                DTdebug( )<<"Error Message found error "<< statusCodeV.toInt()<<(int)reply->error();
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
        WIFIlock.unlock();
        DTdebug()<< "wifi unconnect Error Message fail";
    }
    return isSuccess;
}

/********************************************************************
 * 创 建 人：Claire
 * 创建时间：2019年5月27日
 * 邮   箱：xiaoyu_xiaoyu_xy@163.com
 * 功   能：目前用于大众二厂通过VIN获取车辆信息
 *******************************************************************/
#include "getcarinformation.h"

getCarInformation::getCarInformation(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
    init();
}

void getCarInformation::init()
{
    HTTPTIMEOUT = (3 * 1000);
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;

    manager = new QNetworkAccessManager(this);

    getVIN.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/GetFisPreviewByVinOrPin?"));

    if(!db2.isOpen() || !QSqlDatabase::contains("GetCarInfo") )
        mysql_open();
    if(db2.isOpen() && QSqlDatabase::contains("GetCarInfo"))
    {
        QString strCreateTable = "CREATE TABLE IF NOT EXISTS "+tablePreview+
                " (RecordID int not null primary key, "
                "Werk varchar(2) not null, "
                "SPJ varchar(4) not null, "
                "KNR varchar(8) not null, "
                "VIN varchar(17) not null,"
                "LocalDateTime datetime not null, "
                "FisMatch varchar(1000) not null)";

        if(!query2.exec(strCreateTable))
        {
            DTdebug()<<"create table "+tablePreview+" fail"<<query2.lastError();
        }
    }
    else
    {
        DTdebug()<<"db2 is not open or GetCarInfo connect fail";
    }

}

//从服务器获取数据
void getCarInformation::getData(QString vin,bool flag,int whichCar)
{
    FUNC() ;
    DTdebug() << WIFI_STATE ;
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        //向服务器请求数据
        QVariantMap  dataMap ;
        dataMap.insert("Vin_Pin",vin);

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(dataMap, &ok);

        getVIN.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        getVIN.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

        if (ok)
        {
            QNetworkReply *reply = manager->post(getVIN,json);
            reply->ignoreSslErrors();
            QTime t_time;
            t_time.start();

            bool timeout = false;

            while (!reply->isFinished())
            {
                QApplication::processEvents();
                if (t_time.elapsed() >= HTTPTIMEOUT) {
                    timeout = true;
//                    Q_EMIT FisSendSerial(vin,"timeout");
                    DTdebug()<<"Fis reply timeout";
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
                    DTdebug()<<"get FisPreview An error occurred during parsing"<<bytes;
                }
                else
                {
                    DTdebug()<<"get FisPreview";

                    if(result["Result"].toBool())
                    {
                        foreach (QVariant Value, result["Value"].toList())
                        {
                            QVariantMap value = Value.toMap();
                            if(!query2.exec("truncate "+tablePreview))
                                DTdebug()<<"truncate FAIL "<<query2.lastError();
                            else
                            {
                                query2.prepare("insert into "+tablePreview+
                                               " (RecordID, Werk, SPJ, KNR, VIN ,LocalDateTime, FisMatch) values (?, ?, ?, ?, ?, now(),?)");
                                query2.addBindValue(value["RecordID"].toInt());
                                query2.addBindValue(value["Werk"].toString());
                                query2.addBindValue(value["SPJ"].toString());
                                query2.addBindValue(value["KNR"].toString());
                                query2.addBindValue(value["VIN"].toString());

//                                Q_EMIT FisSendSerial(strVin,strCarType);        //to do

                                QJson::Serializer serializer;
                                bool ok;
                                QByteArray match = serializer.serialize(value["FisMatch"],&ok);

                                if(ok)
                                    query2.addBindValue(match);
                                else
                                {
                                    query2.addBindValue(match);
                                    DTdebug() << "match serialze fail:" << serializer.errorMessage()<<"FisMatch:"<<value["FisMatch"];
                                }

                                if(!query2.exec())
                                    DTdebug() <<"update FisPreview fail "<<query2.lastError();
                            }
                        }
                    }
                    else
                    {
                        DTdebug()<<"have not vin"<<bytes<<vin;
//                        Q_EMIT FisSendSerial("","");
                    }
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
    else
    {
        WIFIlock.unlock();
//        Q_EMIT FisSendSerial(vin,"timeout");
        DTdebug()<< "wifi unconnect get Production fail";
    }
    Q_EMIT requst(flag,whichCar);
}

// 本地mysql open
void getCarInformation::mysql_open()
{
    FUNC();
    if(QSqlDatabase::contains("GetCarInfo")){
        db2 = QSqlDatabase::database("GetCarInfo");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","GetCarInfo");
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



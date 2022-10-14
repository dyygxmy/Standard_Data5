/************************************************************
 * 创建时间：20180723
 * 创 建 人：Claire
 * 功   能：暂时仅用于郑州海马的队列模式，
           用于从服务器获取数据，保存至本地数据库
           如果可以，后期所有队列操作都由该文件进行操作
 ***********************************************************/

#include "queuethread.h"

queueThread::queueThread(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
    init();

}

void queueThread::init()
{
    HTTPTIMEOUT = (5 * 1000);
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    QString ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    delete configIniRead;
    manager = new QNetworkAccessManager(this);

    getVIN.setUrl(QUrl("http://"+DataServerIp+"/Handler.ashx?"));
    errorMessage.setUrl(QUrl("http://"+DataServerIp+":"+ServerPort+"/ErrorMessage?"));

    mDataBase = mysql_open(QString("queueThread"));
    if(mDataBase.isOpen())
    {
        query2 = QSqlQuery(mDataBase);
        if(!query2.exec("CREATE TABLE IF NOT EXISTS "+tablePreview+
                        " (RecordID integer primary key auto_increment, VIN varchar(17) not null,carType varchar(20) not null, "
                        "UseFlag int not null,LocalDateTime datetime not null)"))
        {
            DTdebug()<<"create table "+tablePreview+" fail"<<query2.lastError();
        }
    }
    else
    {
        DTdebug()<<"mDataBase is not open or ReinsertMySQL connect fail";
    }

    QTimer::singleShot(1000,this,SLOT(firstGetData()));

}

void queueThread::firstGetData()
{
    if(WIFI_STATE)
    {
        if(!mDataBase.isOpen())
        {
            DTdebug()<<"mDataBase is not open";
        }
        else if(!query2.exec("SELECT VIN,UseFlag FROM "+tablePreview + " order by RecordID desc limit 1"))
        {
            DTdebug()<<"SELECT fail "<<query2.lastError();
        }
        else
        {
            if(query2.next())
            {
                QString strVin = query2.value(0).toString();
                bool flag =  query2.value(1).toBool();
                if(flag)
                {
                    getData(0,strVin);
                }
                else {
                    getData(1,strVin);
                }

            }
            else {
                DTdebug() << "select error"<<query2.lastError() ;
            }
        }
    }
    else
    {
        QTimer::singleShot(1000,this,SLOT(firstGetData()));
    }
}

//从服务器获取数据
void queueThread::getData(int type,QString vin)
{
    FUNC() ;
    DTdebug() << WIFI_STATE ;
    WIFIlock.lockForRead();
    if(WIFI_STATE)//服务器
    {
        WIFIlock.unlock();

        //向服务器请求数据
        QVariantMap  dataMap ;

        dataMap.insert("Station",Station);
        dataMap.insert("NVin",vin);
        dataMap.insert("vinType",type);

        dataMap.insert("Type","Get");
        dataMap.insert("Target","QueuePreview");
//        dataMap.insert("Parameter",dataMap);

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
                    Q_EMIT FisSendSerial(vin,"timeout");
                    DTdebug()<<"HaiMaVin reply timeout";
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
                            QString strVin = value["VIN"].toString();
                            QString strCarType = value["carModel"].toString();
                            if(!mDataBase.isOpen())
                            {
                                DTdebug()<<"mDataBase is not open";
                            }
                            else if(!query2.exec("SELECT COUNT(*) FROM "+tablePreview))
                            {
                                DTdebug()<<"SELECT COUNT(*) FAIL "<<query2.lastError();
                            }
                            else
                            {
                                if(query2.next())
                                {
                                    int numRows = query2.value(0).toInt();
                                    if(numRows>7000)
                                    {
                                        query2.prepare("delete from "+tablePreview+" where RecordID in (select x.RecordID from (select RecordID from "+tablePreview+" order by RecordID limit  100) as  x)");
                                    }
                                    else {

                                    }
                                    query2.prepare("insert into "+tablePreview+" (VIN,carType,UseFlag,LocalDateTime) values (?, ?, 0, now())");
                                    query2.addBindValue(strVin);
                                    query2.addBindValue(strCarType);
                                    DTdebug()<<strVin<<strCarType;
                                    Q_EMIT FisSendSerial(strVin,strCarType);

                                    if(!query2.exec())
                                        DTdebug() <<"update FisPreview fail "<<query2.lastError();
                                }

                            }
                        }
                    }
                    else
                    {
                        DTdebug()<<"have not vin"<<bytes<<vin;
                        _type = type ;
                        _strVin = vin ;
                        Q_EMIT FisSendSerial("","");
                        QTimer::singleShot(1000,this,SLOT(getCarInfoAgain()));
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
        Q_EMIT FisSendSerial(vin,"timeout");
        DTdebug()<< "wifi unconnect get Production fail";
    }
}

void queueThread::getCarInfoAgain()
{
    getData(_type,_strVin);
}

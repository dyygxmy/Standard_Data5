#include "fisupdatequeue.h"
#include <QSqlRecord>
#include <QSettings>
#include "GlobalVarible.h"
FisUpdateQueue::FisUpdateQueue(QObject *parent) :
    QObject(parent)
{
    fis_thread.start();
    this->moveToThread(&fis_thread);
}

void FisUpdateQueue::myfistimer()
{
    qDebug()<< "fisupdate thread start";
    fisupdateFunc();
}

//50秒更新Fis
void FisUpdateQueue::fisupdateFunc()
{
    while(1)
    {
        if(true)
        {
            QSqlDatabase db2=QSqlDatabase::addDatabase("QMYSQL","mysqlconnection");
            db2.setHostName("localhost");
            db2.setDatabaseName("Tighten");
            db2.setUserName("root");
            db2.setPassword("123456");
            QSqlQuery query2 = QSqlQuery(db2);
            QSqlQuery query3 = QSqlQuery(db2);

            if(!db2.open())
            {
                if(!db2.open())
                {
                    qDebug()<< "fisupdate localmysql "<< db2.lastError().text();
                }else
                {
                    qDebug()<< "fisupdate localmysql open ok 2";
                }
            }else
            {
                qDebug()<< "fisupdate localmysql open ok 1";
            }

            Queuelock.lockForRead();
            if(isQueue && !ISmaintenance && CsIsConnect)
            {
                if(QueueIsNull)
                {
                    qDebug()<<"Queue is null";
                    Queuelock.unlock();
                    //            if(!db2.isOpen()||!QSqlDatabase::contains("mysqlconnection"))
                    //                mysql_open();
                    if(db2.isOpen()||QSqlDatabase::contains("mysqlconnection"))
                    {
                        if(!query2.exec("select VIN from "+tablePreview+" where RecordID =(select min(RecordID) from "+tablePreview+" where UseFlag = 0)"))
                            qDebug() <<"fis 148 row "  << query2.lastError();
                        else
                        {
                            if(query2.next())
                            {
                                qDebug()<<"&&&&&&&&&&  FisSendSerial "<<query2.value(0).toString();
                                Queuelock.lockForWrite();
                                QueueIsNull = false;
                                Queuelock.unlock();
                                emit FisSendSerial(query2.value(0).toString());
                            }
                            else
                                qDebug() << "fis is null";
                        }
                    }
                    else
                        qDebug() << "fis open fail";
                }
                else
                {
                    qDebug()<<"Queue is not null";
                    Queuelock.unlock();
                }
            }
            else
            {
                qDebug()<<"!isQueue or ISmaintenance or !CsIsConnect"<<isQueue<<ISmaintenance<<CsIsConnect;
                Queuelock.unlock();
            }
            //    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlconnection"))
            //        mysql_open();
            if(db2.isOpen()||QSqlDatabase::contains("mysqlconnection"))
            {
                if(query2.exec("select AutoNO, VIN from TaskDeleted where Flag = 0 "))
                {
                    while(query2.next())
                    {
                        if(query3.exec("UPDATE "+tablePreview+" SET UseFlag = 2 where AutoNO ='"+query2.value(0).toString()+"' or VIN = '"+query2.value(1).toString()+"'"))
                        {
                            if(query3.numRowsAffected() == 1)
                                qDebug()<<"Update success 2 AutoNO:"+query2.value(0).toString()+" VIN:"+query2.value(1).toString();
                            else if(query3.numRowsAffected() == 0)
                                qDebug()<<"There is no 2 AutoNO:"+query2.value(0).toString()+" VIN:"+query2.value(1).toString();
                        }
                        else
                        {
                            qDebug()<<"Update 2 fail AutoNO:"+query2.value(0).toString()+" VIN:"+query2.value(1).toString()<<query3.lastError();
                        }
                    }
                }
                else
                {
                    qDebug()<<"224"<<query2.lastError();
                }
            }
            if(db2.isOpen())
                db2.close();
        }
        QSqlDatabase::removeDatabase("mysqlconnection");
        sleep(20);
    }
}
//更新列
void FisUpdateQueue::update_column(QString column)
{
    WIFIlock.lockForRead();
    if(WIFI_STATE)
    {
        WIFIlock.unlock();
        if(true)
        {
            QSqlDatabase db1=QSqlDatabase::addDatabase("QODBC","SQLServerconnection");
            db1.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=1;SQL_ATTR_CONNECTION_TIMEOUT=1");
            db1.setDatabaseName("sqltighten");
            db1.setPort(1433);
            db1.setUserName(SqlUserName);
            db1.setPassword(SqlPassword);
            QSqlQuery query1 = QSqlQuery(db1);

            if(!db1.open())
            {
                if(!db1.open())
                {
                    qDebug()<<"sqlserver fisupdate "<<db1.lastError().text();
                }
                else
                {
                    qDebug()<< "sqlserver fisupdate open ok 2";
                }
            }else
            {
                qDebug()<< "sqlserver fisupdate open ok 1";
            }

            QSqlDatabase db2=QSqlDatabase::addDatabase("QMYSQL","mysqlconnection");
            db2.setHostName("localhost");
            db2.setDatabaseName("Tighten");
            db2.setUserName("root");
            db2.setPassword("123456");
            QSqlQuery query2 = QSqlQuery(db2);
            QSqlQuery query3 = QSqlQuery(db2);

            if(!db2.open())
            {
                if(!db2.open())
                {
                    qDebug()<< "fisupdate localmysql "<< db2.lastError().text();
                }else
                {
                    qDebug()<< "fisupdate localmysql open ok 2";
                }
            }else
            {
                qDebug()<< "fisupdate localmysql open ok 1";
            }

            if(db1.isOpen() && db2.isOpen() && QSqlDatabase::contains("SQLServerconnection") && QSqlDatabase::contains("mysqlconnection"))
            {
                bool isColumnName = false;
                query1.exec("select column_name from information_schema.columns where table_name='dbo.TaskPreview'");
                while(query1.next())
                {
                    if(column == query1.value(0).toString())
                        isColumnName = true;
                }
                if(isColumnName)
                {
                    if(query2.exec("select RecordID from "+tablePreview+" order by RecordID DESC LIMIT 1000"))
                    {
                        while(query2.next())
                        {
                            query1.exec("select "+column+" from dbo.TaskPreview where RecordID="+query2.value(0).toString());
                            query1.next();
                            if(!query3.exec("UPDATE "+tablePreview+" SET "+column+" = \'"+query1.value(0).toString()+"\' WHERE RecordID = "+query2.value(0).toString()))
                                qDebug()<<"235"<<query3.lastError();
                        }
                    }
                    else
                        qDebug()<<"239"<<query2.lastError();
                }
            }

            if(db1.isOpen())
                db1.close();
            if(db2.isOpen())
                db2.close();
        }
        QSqlDatabase::removeDatabase("SQLServerconnection");
        QSqlDatabase::removeDatabase("mysqlconnection");
    }
    else
    {
        WIFIlock.unlock();
        qDebug()<<"wifi unconnect update_column old datas fail";
    }
}

//查询时间
void FisUpdateQueue::QueryTime()
{
    WIFIlock.lockForRead();
    if(WIFI_STATE)
    {
        WIFIlock.unlock();
        if(true)
        {
            QSqlDatabase db1=QSqlDatabase::addDatabase("QODBC","SQLServerconnection");
            db1.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=1;SQL_ATTR_CONNECTION_TIMEOUT=1");
            db1.setDatabaseName("sqltighten");
            db1.setPort(1433);
            db1.setUserName(SqlUserName);
            db1.setPassword(SqlPassword);
            QSqlQuery query1 = QSqlQuery(db1);

            if(!db1.open())
            {
                if(!db1.open())
                {
                    qDebug()<<"sqlserver fisupdate "<<db1.lastError().text();
                }
                else
                {
                    qDebug()<< "sqlserver fisupdate open ok 2";
                }
            }else
            {
                qDebug()<< "sqlserver fisupdate open ok 1";
            }


            if(db1.isOpen()&&QSqlDatabase::contains("SQLServerconnection"))
            {
                query1.exec("select CONVERT(VARCHAR(20),getdate(),120)");
                query1.next();
                QString datetime = query1.value(0).toString();
                emit sendTime(datetime);
            }

            if(db1.isOpen())
                db1.close();
        }
        QSqlDatabase::removeDatabase("SQLServerconnection");
    }
    else
    {
        WIFIlock.unlock();
    }
}


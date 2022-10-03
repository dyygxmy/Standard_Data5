#include "fisupdate.h"
FisUpdate::FisUpdate(QObject *parent) :
    QObject(parent)
{
    Factory = factory;
    fis_thread.start();
    this->moveToThread(&fis_thread);
}

void FisUpdate::myfistimer()
{
    qDebug()<< "fisupdate thread start";
    fisupdateFunc();
}

//50秒更新Fis
void FisUpdate::fisupdateFunc()
{
    while(1)
    {
        if(Factory == "SVW3")
        {
            if(true)
            {
                QSqlDatabase db2=QSqlDatabase::addDatabase("QMYSQL","andon_connection_local");
                db2.setHostName("localhost");
                db2.setDatabaseName("Tighten");
                db2.setUserName("root");
                db2.setPassword("123456");
                QSqlQuery query2 = QSqlQuery(db2);

                if(!db2.open())
                {
                    if(!db2.open())
                    {
                        qDebug()<< "andon local mysql "<< db2.lastError().text();
                        return;
                    }
                    else
                    {
                        qDebug()<< "andon local mysql open ok 2";
                    }
                }
                else
                {
                    qDebug()<< "andon local mysql open ok 1";
                }

                if(db2.isOpen() && QSqlDatabase::contains("andon_connection_local"))
                {
                    if(!query2.exec("CREATE TABLE IF NOT EXISTS andonLocal (id int primary key auto_increment,StationStatus int)"))
                    {
                        qDebug()<<"create tabel fail "<<query2.lastError();
                        return;
                    }

                    query2.exec("select count(*) from andonLocal");
                    query2.next();
                    int num=query2.value(0).toInt();
                    bool result = false;
                    if(num)
                        result=query2.exec("UPDATE andonLocal set StationStatus ="+QString::number(StationStatus)+" where id= 1");
                    else
                        result=query2.exec("insert into andonLocal(StationStatus) values(0);");
                    if(!result)
                        qDebug()<<"query2"<<query2.lastError();
                    else
                    {
                        if(query2.numRowsAffected()==1)
                        {
                            qDebug()<< "updata andon local mysql success"<<StationStatus;
                        }
                        else
                        {
                            qDebug()<<"Status not Change or Ip wrong update andon local mysql fail";
                        }
                    }
                }
                else
                    qDebug()<<"andon update fail";
                if(db2.isOpen())
                    db2.close();
            }
            QSqlDatabase::removeDatabase("andon_connection_local");
        }
        sleep(50);
    }
}

void FisUpdate::update_column(QString column)
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
                qDebug()<<"update history 1000";
                bool isColumnName = false;
                QString aff = "";
                if(Factory == "SVW3")
                {
                    aff= "select column_name from information_schema.columns where table_name='FisPreview'";
                }
                else if(Factory == "Ningbo")
                {
                    aff= "select column_name from information_schema.columns where table_name='pnr'";
                }
                if(!query1.exec(aff))
                    qDebug()<<"286 query1.error"<<query1.lastError();
                while(query1.next())
                {
                    qDebug()<<"query1.value(0):"<<query1.value(0).toString();
                    if(column == query1.value(0).toString())
                        isColumnName = true;
                }
                if(isColumnName)
                {
                    qDebug()<<"isColumnName column:"<<column;
                    if(query2.exec("select RecordID from "+tablePreview+" order by RecordID DESC LIMIT 1000"))
                    {
                        while(query2.next())
                        {
                            if(Factory == "SVW3")
                            {
                                aff= "select "+column+" from Data.FisPreview where RecordID="+query2.value(0).toString();
                            }
                            else if(Factory == "Ningbo")
                            {
                                aff= "select "+column+" from dbo.pnr where ID="+query2.value(0).toString();
                            }
                            if(!query1.exec(aff))
                                qDebug()<<"select column fail query1.lastError:"<<query1.lastError();
                            query1.next();
                            if(!query3.exec("UPDATE "+tablePreview+" SET "+column+" = \'"+query1.value(0).toString()+"\' WHERE RecordID = "+query2.value(0).toString()))
                                qDebug()<<"update column query3"<<query3.lastError();
                        }
                    }
                    else
                        qDebug()<<"update column query2"<<query2.lastError();
                }
                else
                    qDebug()<<"isn't ColumnName";
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
void FisUpdate::QueryTime()
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

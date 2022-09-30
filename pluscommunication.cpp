#include "pluscommunication.h"
#include "GlobalVarible.h"

PlusCommunication::PlusCommunication(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&plus_thread);
    plus_thread.start();
}
/***************************************************/
//plus init
/***************************************************/
void PlusCommunication::PlusStart()
{
//    sharedMemory.setKey("QSharedMemoryIDCode");
    Idcode_temp="";
//    QString fileName = "/var/PlusFlag";
//    QFile file(fileName);
//    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
//    {
//        qDebug()   << "Cannot open wifi file2 for Writing";
//    }
//    else
//    {
//        file.write("0");
//        file.close();
//    }

    QByteArray stationTemp;
    qDebug()  << "plus Thread start...";
    tcpServer = new QTcpServer(this);
//    if(!tcpServer->listen(QHostAddress::Any,5002))
//    {
//        qDebug() << tcpServer->errorString();
//    }
//    else
//    {}
    if(!tcpServer->listen(QHostAddress("192.168.2.12"),5002))
    {
        qDebug() << tcpServer->errorString();
    }
    else
    {}
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(DataReceived()));
    heartTimer = new QTimer(this);
    connect(heartTimer,SIGNAL(timeout()),this,SLOT(TimerFunc()));
    heartTimer->start(1000);

    stationID = "000000";
    stationTemp = Station.toLatin1();
    if(stationTemp.size()>6)
    {
        stationID = stationTemp.right(6);
    }
    else if(stationTemp.size()<6)
    {
        stationID.replace(6-stationTemp.size(),stationTemp.size(),stationTemp);
    }
    else
    {
        stationID = stationTemp;
    }
    requestJobFlag = false;
    plusSynFlag = false;
    JobInsertFlag = false;
    Cycle_Id = 0;
    timerCount1= 0;
    timerCount2= 0;
    timerValue = 2;
    requestJobBuf = "00126NSRIAFLITPNR         IW97000095                                        FPxxxxxxTM                                AASC";
    requestJobBuf.replace(78,6,stationID);    //replace station ID
    dataHeadBuf = "000000218ISRIAEQFP" + stationID +"ITPNR         IW97000095                                        TLJ TN01AASCAW01";
    boltInforBuf = "QI3040030-130-000-T2-01   QAIO  FEGUT QS00USANLG0000DA2016-12-26ZT10:10:54AP02";
    torque_AngleBuf = "PIDMPW+130.10     PIDWPW+232        ";
}
/***************************************************/
//close TcpServer connection
/***************************************************/
void PlusCommunication::closePortTcpServer()
{
    emit sendPlusFlag(false);
    if(tcpServer->hasPendingConnections())
    {
        qDebug()<<"Plus_pTcpSocket connected or connecting";
        Plus_pTcpSocket->abort();
        delete Plus_pTcpSocket;
    }
}
/***************************************************/
//timer funcation send vin to plus
/***************************************************/
void PlusCommunication::TimerFunc()
{
//    qDebug() << "1000ms timer plusSynFlag:" << plusSynFlag;

//    QByteArray IDcode="";

//    if (!sharedMemory.attach()) {
//        qDebug()<<"no IDCode Request";
//        return;
//    }
//    else
//    {
//        QBuffer buffer;
//        QDataStream in(&buffer);

//        sharedMemory.lock();
//        buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
//        buffer.open(QBuffer::ReadOnly);
//        in >> IDcode;
//        sharedMemory.unlock();

//        sharedMemory.detach();
//        qDebug()<<"get IDCode:"<<IDcode;
//    }
//    QFile IDcodeFile("/var/IDCode");
//    QByteArray IDcode="";
//    if(IDcodeFile.open(QIODevice::ReadOnly|QIODevice::Text))
//    {
//        IDcode= IDcodeFile.readAll();
//        IDcodeFile.close();
//        qDebug()<<"IDcode len:"<<IDcode.size()<<IDcode;
//    }
//    else
//    {
//        qDebug()   << "Cannot open IDcodeFile for Read";
//    }
//    if(IDcode.size() >= 8)
//    {
//        IDcode = IDcode.mid(0,8);
//        if(!IDcodeFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
//        {
//            qDebug()   << "Cannot open IDcodeFile for Write";
//        }
//        else
//        {
//            IDcodeFile.close();
//        }
//    }
    QString IDcode="";
    if(Idcode_temp != "")
    {
        IDcode = Idcode_temp;
        Idcode_temp = "";
    }
    if(plusSynFlag)
    {
        aliveCount ++;
        timerCount1++;
        timerCount2++;
        if(IDcode.size()>=8)
        {
            timerCount2 = 0;
            IDcodeRequestJob(IDcode);//1s quick requiet JOB
        }
        else if(timerCount1 >= timerValue) //2s request job
        {
            timerCount1 = 0;
//            MysqlHandleFunc();
        }
        else if(timerCount2 >= 3) //3s upload data
        {
            timerCount2 = 0;
            dataUpload();
        }
        if(aliveCount>70)
        {
            plusSynFlag = false;
            aliveCount = 0;
            closePortTcpServer(); //close Port TcpServer
        }
    }
}
/***************************************************/
//link success
/***************************************************/
void PlusCommunication::DataReceived()
{
    qDebug() << "*******PLUS 5002 port connect success*******";
    Plus_pTcpSocket = tcpServer->nextPendingConnection();
    connect(Plus_pTcpSocket, SIGNAL(readyRead()),this, SLOT(PlusMessage()));

    sendBuf = "000000020NSRSYNS0000"; //contorol send first Syn
    Tx_Len = 21;
    Plus_pTcpSocket->write(sendBuf,Tx_Len);
    Plus_pTcpSocket->waitForBytesWritten();
    qDebug() << "send first SYN len:" << Tx_Len << "note:"<<sendBuf;
}

/***************************************************/
//receive data and Ack
/***************************************************/
void PlusCommunication::PlusMessage()
{
    aliveCount = 0;
    revBuf = Plus_pTcpSocket->readAll();
    qDebug() << "PLUS to data len:" << revBuf.size() << "note:"<<revBuf;
    if(revBuf.size() >= 16)
    {
        if(revBuf.mid(9,7) == "NRSSYNR") //plus send first Syn
        {
            sendBuf = "Q00000000NRSSYNR";
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send data len:" << sendBuf.size() << "note:"<<sendBuf;

            sendBuf = "000000020NSRSYNS0000"; //contorol send second Syn
            Tx_Len = 21;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send data len:" << sendBuf.size() << "note:"<<sendBuf;
        }
        else if(revBuf.mid(9,7) == "ARSSYNS") //plus send thirt Syn
        {
            sendBuf = "Q00000000ARSSYNS";
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send data len:" << sendBuf.size() << "note:"<<sendBuf;
            qDebug() << "SYN success";
            plusSynFlag = true;
            Cycle_Id = 0;
            emit sendPlusFlag(true);
//            QString fileName = "/var/PlusFlag";
//            QFile file(fileName);
//            if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
//            {
//                qDebug()   << "Cannot open wifi file2 for Writing";
//            }
//            else
//            {
//                file.write("1");
//                file.close();
//            }
        }
        else if(revBuf.mid(9,7) == "NRSUEBW") //plus send alive
        {
            str_Cycle_Id_Plus = revBuf.mid(0,4);
            sendBuf = "Q00000000NRSUEBW";
            sendBuf.replace(5,4,str_Cycle_Id_Plus);
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send data len:" << sendBuf.size() << "note:"<<sendBuf;
        }
        else if(revBuf.mid(9,7) == "ARSIAAL")//PLUS ACK JOB
        {
            requestJobFlag = false;
            IDcodeInfor = "";
            JobInfor = "";
            if(revBuf.mid(30,2) == "IW")
            {
                IDcodeInfor = revBuf.mid(32,8); //IDcode
            }
            if(revBuf.mid(88,2) == "TM")
            {
                JobInfor = revBuf.mid(90,8); //get JOB
            }
            JobInsertFlag = true;
            str_Cycle_Id_Plus = revBuf.mid(0,4);
//            sendBuf = "Q0000xxxxNRSUEBW";
            sendBuf = "Q0000xxxxARSIAAL";
            sendBuf.replace(5,4,str_Cycle_Id_Plus);
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send job ACK len:" << sendBuf.size() << "note:"<<sendBuf;
        }
        else if(revBuf.mid(9,7) == "NSRIAPL")//Error IDcode
        {
            IDcodeInfor = revBuf.mid(32,8); //IDcode
            JobInfor = "44444444"; //get JOB
            JobInsertFlag = true;
            str_Cycle_Id_Plus = revBuf.mid(0,4);
//            sendBuf = "Q0000xxxxNRSUEBW";
            sendBuf = "Q0000xxxxNSRIAPL";
            sendBuf.replace(5,4,str_Cycle_Id_Plus);
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send error IDcode ACK len:" << sendBuf.size() << "note:"<<sendBuf;
        }
        else if(revBuf.mid(9,7) == "NSRIARL")//Error Station
        {
            IDcodeInfor = revBuf.mid(32,8); //IDcode
            JobInfor = "55555555"; //get JOB
            JobInsertFlag = true;
            str_Cycle_Id_Plus = revBuf.mid(0,4);
            sendBuf = "Q0000xxxxNSRIARL";
            sendBuf.replace(5,4,str_Cycle_Id_Plus);
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send error Station ACK len:" << sendBuf.size() << "note:"<<sendBuf;
        }
        else if(revBuf.mid(9,7) == "ISRIAEQ")//data ACK
        {
            if(revBuf.mid(0,5) == "Q0000")//data insert success
            {
                if(!db2.isOpen() || !QSqlDatabase::contains("mysqlconnection"))
                    mysqlOpen();
                if(db2.isOpen() && QSqlDatabase::contains("mysqlconnection"))
                {
                    QString msg="";
                    for(int i=0;i<RecodeIDList.size();i++)
                    {
                        if(query2.exec("UPDATE BenzTighteningDatas SET PlusMark = 0 WHERE RecordID = "+RecodeIDList[i]))
                        {
                            msg= "pluscommunication update success RecordID:" + RecodeIDList[i];
                            qDebug()<<msg;
                        }
                        else
                        {
                            qDebug()<<query2.lastError();
                            msg= "pluscommunication update fail RecordID:" + RecodeIDList[i];
                            qDebug()<<msg;
                        }
                    }
                }
            }
            else
            {
                qDebug()<<"data insert fail";
            }
        }
        else if(revBuf.mid(0,1) != "Q")//Error CMD
        {           
            QByteArray temp="";
            str_Cycle_Id_Plus = revBuf.mid(0,4);
            temp = revBuf.mid(9,7);
            sendBuf = "Q0000xxxx";;
            sendBuf.replace(5,4,str_Cycle_Id_Plus);
            sendBuf.append(temp);
            Tx_Len = 17;
            Plus_pTcpSocket->write(sendBuf,Tx_Len);
            Plus_pTcpSocket->waitForBytesWritten();
            qDebug() << "send error CMD ACK len:" << sendBuf.size() << "note:"<<sendBuf;
        }
        if(JobInsertFlag)
        {
            JobInsertFlag = false;
            if((IDcodeInfor == "")||(JobInfor == ""))
            {}
            else
            {
                qDebug()<<"rev PLUS JOB:"<<IDcodeInfor<<JobInfor;
                emit SendJOB(JobInfor);
//                if(!db2.isOpen() || !QSqlDatabase::contains("mysqlconnection"))
//                    mysqlOpen();
//                if(db2.isOpen() && QSqlDatabase::contains("mysqlconnection"))
//                {
//                    QString MysqlMax = "1";
//                    if(!query2.exec("SELECT MAX(RecordID) FROM "+tablePreview))
//                        qDebug()<<"select Max(RecordID) fail"<<query2.lastError();
//                    else
//                    {
//                        if(query2.next())
//                            MysqlMax = QString::number(query2.value(0).toInt()+1) ;
//                    }

//                    query2.exec("SELECT COUNT(*) FROM "+tablePreview+"");
//                    query2.next();
//                    int numRows = query2.value(0).toInt();
//                    if (numRows < 7000)
//                    {
//                        query2.prepare("insert into "+tablePreview+" (RecordID, IDCode, Job, LocalDateTime) values (?, ?, ?, now())");
//                    }
//                    else
//                    {
//                        query2.prepare("UPDATE "+tablePreview+" SET RecordID=? IDCode =?, Job =?, LocalDateTime=now() WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+tablePreview+")as t)");
//                    }
//                    query2.addBindValue(MysqlMax);
//                    query2.addBindValue(IDcodeInfor);
//                    query2.addBindValue(JobInfor);

//                    if(!query2.exec())
//                        qDebug() <<"insert or update fail IDcodeInfor:"<<IDcodeInfor<<"JobInfor"<<JobInfor<<query2.lastError();
//                    else
//                        qDebug() <<"insert or update success IDcodeInfor:"<<IDcodeInfor<<"JobInfor"<<JobInfor;
//                    //                emit SendJOB(QString(IDcodeInfor),QString(JobInfor));//send IDcode and JOB
//                }
//                else
//                    qDebug()<<"qb2 not open insert fail IDcodeInfor:"<<IDcodeInfor<<"JobInfor"<<JobInfor;
            }
        }
    }
}

/***************************************************/
//request JOB
/***************************************************/
void PlusCommunication::IDcodeRequestJob(QString IDcode)
{
    if(plusSynFlag)
    {
        Cycle_Id++;
        if(Cycle_Id > 9999)
        {
            Cycle_Id = 0;
        }
        str_Cycle_Id = QByteArray::number(Cycle_Id+10000).mid(1,4);
        requestJobBuf.replace(28,8,IDcode.toLatin1().left(8));       //replace IDcode
        sendBuf = str_Cycle_Id.append(requestJobBuf);
        Tx_Len = 127;
        Plus_pTcpSocket->write(sendBuf,Tx_Len);
        Plus_pTcpSocket->waitForBytesWritten();
        qDebug() << "send data len:" << sendBuf.size() << "note:"<<sendBuf;
    }
}

/***************************************************/
//upload data to plus
/***************************************************/
void PlusCommunication::dataUpload()
{
    if(!db2.isOpen() || !QSqlDatabase::contains("mysqlconnection"))
        mysqlOpen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlconnection"))
    {
        if(!query2.exec("select t1.RecordID,t1.IDCode,t1.ScrewID,t1.TighteningStatus,t1.TighteningTime,t1.Torque,t1.Angle from  BenzTighteningDatas t1 inner join (Select IDCode from BenzTighteningDatas where PlusMark=1 and (JobStatus =1 or JobStatus=2) LIMIT 1) t2 on t1.IDCode=t2.IDCode where PlusMark=1"))
            qDebug()<<"select t1 fail "<<query2.lastError();
        else
        {
            int boltNum = 100;
            QByteArray TightenTorque="";
            QByteArray TightenAngle="";
            QByteArray temp = "";
            QByteArray IDcode;
            sendBuf = dataHeadBuf;
            RecodeIDList.clear();
            while(query2.next())
            {
                RecodeIDList<<query2.value(0).toString();
                IDcode = query2.value(1).toByteArray();
                QByteArray ScrewID = query2.value(2).toByteArray();
                QByteArray TightenResult = query2.value(3).toByteArray();
                if(TightenResult == "OK")
                {
                    TightenResult = "IO  ";
                }
                else
                {
                    TightenResult = "NIO ";
                }
                QByteArray TightenData = query2.value(4).toString().mid(0,10).toLatin1();
                QByteArray TightenTime = query2.value(4).toString().mid(11,8).toLatin1();
//                qDebug()<<"11111111DataTime"<<query2.value(4).toByteArray();
//                intTemp = query2.value(5).toByteArray();
//                TightenTorque = QByteArray::number(intTemp);
//                intTemp = query2.value(6).toByteArray();
//                TightenAngle = QByteArray::number(intTemp);
                TightenTorque = query2.value(5).toByteArray();
                TightenAngle = query2.value(6).toByteArray();
                if((TightenTorque == "-1")||(TightenAngle == "-1"))
                {
                    TightenTorque = "0";
                    TightenAngle = "0";
                }
                temp = "+           ";
//                temp = "000000000000";
                boltNum++;
                boltInforBuf.replace(2,ScrewID.size(),ScrewID).replace(28,4,TightenResult).replace(54,10,TightenData).replace(66,8,TightenTime);
                temp.replace(1,TightenTorque.size(),TightenTorque);
                torque_AngleBuf.replace(6,12,temp);//torque
                temp = "+           ";
//                temp = "000000000000";
                temp.replace(1,TightenAngle.size(),TightenAngle);
                torque_AngleBuf.replace(24,12,temp);//angle
                sendBuf.append(boltInforBuf).append(torque_AngleBuf);
//                qDebug()<<"222222222"<<boltNum<<boltInforBuf;
//                qDebug()<<"333333333"<<boltNum<<torque_AngleBuf;
            }
            if(boltNum > 100) //data do not upload
            {
                Cycle_Id++;
                if(Cycle_Id > 9999)
                {
                    Cycle_Id = 0;
                }
                str_Cycle_Id = QByteArray::number(Cycle_Id+10000).mid(1,4);
                sendLen = QByteArray::number(sendBuf.size() + 100000).right(5);
                sendBuf.replace(0,4,str_Cycle_Id).replace(4,5,sendLen).replace(40,8,IDcode).replace(102,2,QByteArray::number(boltNum+100).right(2));
                Tx_Len = sendBuf.size()+1;
                Plus_pTcpSocket->write(sendBuf,Tx_Len);
                Plus_pTcpSocket->waitForBytesWritten();
                qDebug() << "send data len:" << Tx_Len << "note:"<<sendBuf;
            }
        }
    }
}

void PlusCommunication::receiveRequestJob(QString idcode)
{
    Idcode_temp = idcode;
}

//void PlusCommunication::quickRequest()
//{
//    QFile IDCode("/var/IDCode");
//    if(!IDCode.open(QIODevice::ReadWrite | QIODevice::Text))
//        qDebug()<<"Cannot open IDCode file for ReadWrite";
//    else
//        IDCode.readAll();
//}

/***************************************************/
//mysql handle request job
/***************************************************/
//void PlusCommunication::MysqlHandleFunc()
//{
////    if(requestJobFlag)
////    {
////        qDebug() << "request job ING >>>>>>>>>>";
////        return;
////    }
//    GlobalVarible::WIFIlock.lockForRead();
//    bool state = GlobalVarible::wifiState();
//    qDebug()<< "I am alive "<<state;
//    if(state)
//    {
//        GlobalVarible::WIFIlock.unlock();
//        if(true)
//        {
//            QSqlDatabase db1=QSqlDatabase::addDatabase("QODBC","SQLServerconnection");
//            db1.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=1;SQL_ATTR_CONNECTION_TIMEOUT=1");
//            db1.setDatabaseName("sqlcurve");
//            db1.setPort(1433);
//            db1.setUserName(GlobalVarible::SqlUserName);
//            db1.setPassword(GlobalVarible::SqlPassword);
//            QSqlQuery query1 = QSqlQuery(db1);

//            if(!db1.open())
//            {
//                if(!db1.open())
//                {
//                    qDebug()<<"sqlserver pluscommunication "<<db1.lastError().text();
//                }
//                else
//                {
//                    qDebug()<< "sqlserver pluscommunication open ok 2";
//                }
//            }else
//            {
//                qDebug()<< "sqlserver pluscommunication open ok 1";
//            }

//            if(!db2.isOpen() || !QSqlDatabase::contains("mysqlconnection"))
//                mysqlOpen();

//            if(db2.isOpen() && QSqlDatabase::contains("mysqlconnection"))
//            {
//                if(!query2.exec("CREATE TABLE IF NOT EXISTS BenzTaskPreview(RecordID int not null primary key, IDCode varchar(28) not null, Job varchar(32) not null, LocalDateTime datetime not null)"))
//                {
//                    qDebug()<<"create table fail"<<query2.lastError();
//                }

//                if(db1.isOpen() && db2.isOpen() && QSqlDatabase::contains("SQLServerconnection") && QSqlDatabase::contains("mysqlconnection"))
//                {
//                    //更新时间
//                    if(!query1.exec("select CONVERT(VARCHAR(20),getdate(),120)"))
//                        qDebug()<<"update datetime"<<query1.lastError();
//                    else if(query1.next())
//                    {
//                        QString datetime = query1.value(0).toString();
//                        system((QString("date -s \"") +datetime+QString("\" &")).toLatin1().data());

//                        //将系统时间写入RTC
//                        system("hwclock -w &");
//                    }

//                    if(!query2.exec("SELECT IDCode FROM "+GlobalVarible::tablePreview+" order by RecordID DESC limit 1"))
//                        qDebug()<<"select local IDCode fail"<<query2.lastError();
//                    else
//                    {
//                        QString aff="";
//                        if(query2.next())
//                        {
//                            aff= "SELECT Top 1 RecordID FROM Data.IDCodes WHERE IDCode ='"+query2.value(0).toString()+"' order by RecordID DESC";
//                            if(!query1.exec(aff))
//                                qDebug() << "select RecordID fail" << query1.lastError();
//                            else
//                            {
//                                if(query1.next())
//                                {
////                                    qDebug()<<"RecordID"<<query1.value(0).toString();
//                                    aff = "SELECT TOP 1 IDCode FROM Data.IDCodes where RecordID >"+query1.value(0).toString()+" order by RecordID";
//                                }
//                            }
//                        }
//                        else
//                            aff= "SELECT TOP 1 IDCode FROM Data.IDCodes order by RecordID";
//                        if(!query1.exec(aff))
//                            qDebug()<<"select SQLServer IDCode fail"<<query1.lastError();
//                        else
//                        {
//                            if(query1.next())
//                            {
//                                requestJobFlag = true;
//                                IDcodeRequestJob(query1.value(0).toString());//request JOB
//                            }
//                        }
//                    }
//                }
//                else
//                {
//                    qDebug()<<"database not open";
//                }

//                if(db1.isOpen())
//                    db1.close();
//            }
//            else
//            {
//                qDebug()<<"db2 not open or unconnected";
//            }
//        }
//        QSqlDatabase::removeDatabase("SQLServerconnection");
//    }
//    else
//    {
//        GlobalVarible::WIFIlock.unlock();
//    }

//}

void PlusCommunication::mysqlOpen()
{
    if(QSqlDatabase::contains("mysqlconnection"))
        db2 = QSqlDatabase::database("mysqlconnection");
    else
    {
        db2 = QSqlDatabase::addDatabase("QMYSQL","mysqlconnection");
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
            qDebug()<< "pluscommunication localmysql "<< db2.lastError().text();
        }else
        {
            qDebug()<< "pluscommunication localmysql open ok 2";
        }
    }else
    {
        qDebug()<< "pluscommunication localmysql open ok 1";
    }
}



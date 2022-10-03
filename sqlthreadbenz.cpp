#include "sqlthreadbenz.h"

SqlThreadBenz::SqlThreadBenz(QObject *parent) :
    QObject(parent)
{
    m_thread.start();
    this->moveToThread(&m_thread);
}

void SqlThreadBenz::sqlinit()
{
    db2=QSqlDatabase::addDatabase("QMYSQL","mysqlconnections");
    db2.setHostName("localhost");
    db2.setDatabaseName("Tighten");
    db2.setUserName("root");
    db2.setPassword("123456");
    query2 = QSqlQuery(db2);

    isFirst = true;
    NOKflag = false;

    controllerIp1 = ControllerIp_1;
    controllerIp2 = ControllerIp_2;
}

void SqlThreadBenz::mysqlopen()
{
    if(QSqlDatabase::contains("mysqlconnections")){
        db2 = QSqlDatabase::database("mysqlconnections");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","mysqlconnections");
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
            qDebug()<< "SqlThread localmysql "<< db2.lastError().text();
            //**************打不开重启mysql********************
            emit send_mysqlerror();

        }else
        {
            qDebug()<< "SqlThread localmysql ok 2";
        }
    }else
    {
        qDebug()<< "SqlThread localmysql ok 1";
    }
}

void SqlThreadBenz::Useropen()
{
    if(QSqlDatabase::contains("UserConnections")){
        db1 = QSqlDatabase::database("UserConnections");
    }else{
        db1 = QSqlDatabase::addDatabase("QMYSQL","UserConnections");
        db1.setHostName("localhost");
        db1.setDatabaseName("User");
        db1.setUserName("root");
        db1.setPassword("123456");
        query1 = QSqlQuery(db1);
    }

    if(!db1.open())
    {
        if(!db1.open())
        {
            qDebug()<< "SqlThread UserConnections open fail"<< db1.lastError().text();
            //**************打不开重启mysql********************
            //            emit send_mysqlerror();

        }else
        {
            qDebug()<< "SqlThread UserConnections ok 2";
        }
    }else
    {
        qDebug()<< "SqlThread UserConnections ok 1";
    }
}

void SqlThreadBenz::Userclose()
{
    if(db1.isOpen())
        db1.close();
}

void SqlThreadBenz::sqlclose()
{
    if(db2.isOpen())
        db2.close();
}

void SqlThreadBenz::sqlinsert(QVariant DataVar)
{
    DATA_STRUCT dataStruct;
    dataStruct = DataVar.value<DATA_STRUCT>();
    QString data_model[17];
    for(int i = 0; i<17; i++)
    {
        data_model[i] = dataStruct.data_model[i];
    }
    qDebug() << "here is sqlthread";
    /*******************************
     * 数据模型
    data_model[0] = DATE_blc;
    data_model[1] = TIME_blc;
    data_model[2] = STATE_blc;
    data_model[3] = MI_blc;
    data_model[4] = WI_blc;
    data_model[5] = SCREWID_SQL;
    data_model[6] = VIN_PIN_SQL;
    data_model[7] = Cycle_CSR;
    data_model[8] = CURVE_STR;
    data_model[9] = Program;
    data_model[10] = JobStatus;
    data_model[11] = Type;
    data_model[12] = Order;
    data_model[13] = MaxValue;
    data_model[14] = MinValue;
    data_model[15] = Channel;
    data_model[16] = IpAddress;
    *******************************/
    //qDebug() << data_model[0] << data_model[1] <<data_model[2] <<data_model[3] <<data_model[4] <<data_model[5] <<data_model[6] ;
    //bool datasql = false;
    // bool curvesql = false;
    int uploadmark = 0;
    QString msg = "";
    QString WorkShift = "";
    if(!db1.isOpen()||!QSqlDatabase::contains("UserConnections"))
        Useropen();
    if(db1.isOpen() && QSqlDatabase::contains("UserConnections"))
    {
        //        data_model[1].replace(2,1,":").replace(5,1,":");
        //        qDebug()<<data_model[1];
        if(!query1.exec("SELECT Remark from WorkShift where StartTime<'"+data_model[1]+"' and EndTime >'"+data_model[1]+"'"))
            qDebug()<<"sqlthreadbenz select WorkShiftName fail "<<query1.lastError();
        else
        {
            if(query1.next())
                WorkShift = query1.value(0).toString();
            else
                WorkShift = "null";
        }
    }
    else
        qDebug()<<"db1 not open or unconnected";

    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlconnections"))
        mysqlopen();
//    int screwid_sql = data_model[5].toInt();
    int Cycle = data_model[7].toInt();
    int Program = data_model[9].toInt();
    int Order = data_model[12].toInt();
    int Channel = data_model[15].toInt();
    int JobStatus = data_model[10].toInt();

    if(Order == BoltTotalNum && data_model[2] == "OK")
    {
        if(NOKflag)
            JobStatus = 2;
        else
            JobStatus = 1;
        NOKflag = false;
    }

    double MaxValue = data_model[13].toDouble();
    double MinValue = data_model[14].toDouble();

    if(data_model[5].size()<5)
    {
        msg= QString("Screwid<5 discard the data: IDCode:")+data_model[6]+QString("||ScrewID:")+data_model[5]+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||JobStatus:")+data_model[10]+QString("||Type:")+data_model[11]+QString("||Order:")+data_model[12]+QString("||MaxValue:")+data_model[13]+QString("||MinValue:")+data_model[14]+QString("||Channel:")+data_model[15];
        qDebug()<<msg;
        return;
    }

    qDebug() << "Program:" << Program;

    //    if(curvesql && datasql)
    //        uploadmark = 1;//all successful
    //    else if(curvesql && !datasql)
    //        uploadmark = 2;//data not successful
    //    else if(!curvesql && datasql)
    //        uploadmark = 3;
    //    else if(!curvesql && !datasql)
    uploadmark = 3;

    //**************************本地数据库*******************************************
    if(db2.isOpen() && QSqlDatabase::contains("mysqlconnections"))
    {
        if(isFirst)
        {
            if(!query2.exec("CREATE TABLE IF NOT EXISTS BenzTighteningDatas (RecordID int not null primary key auto_increment, IDCode varchar(128) not null,ScrewID varchar(28) null,Torque real not null,Angle real not null,Curve text not null,TighteningStatus char(3) not null,TighteningTime datetime not null,LocalDateTime datetime not null,Operator char(15) null,UploadMark tinyint not null,UploadTime datetime null,Cycle int null,Program int null, JobStatus int null, Type nvarchar(20) not null, IIO int null, `MaxValue` float null, `MinValue` float null, Channel int null, ErrorCode varchar(10) null,Workshop nvarchar(50) null, WorkShift nvarchar(50) null, IpAddress varchar(20) null, PlusMark tinyint null)"))
            {
                qDebug()<<"create table BenzTighteningDatas fail"<<query2.lastError();
                emit send_mysqlerror();
            }
            else
                isFirst = false;
        }

        query2.exec("SELECT MAX(RecordID) FROM "+Localtable);
        query2.next();
        int RecordIDMax = query2.value(0).toInt();

        query2.exec("SELECT COUNT(*) FROM "+Localtable);
        query2.next();
        int numRows = query2.value(0).toInt();
        if (numRows < 10000)
        {
            query2.prepare("INSERT INTO "+Localtable+"(RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, LocalDateTime, Operator, UploadMark, UploadTime, Cycle, Program, JobStatus, Type, IIO, MaxValue, MinValue, Channel, ErrorCode, Workshop, WorkShift, IpAddress, PlusMark)"
                           "VALUES (?, ?, ?, ?, ?, ?, ?, ?, now(), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        }
        else
        {
            //            query2.prepare("UPDATE "+Localtable+" SET RecordID =?, IDCode =?, ScrewID =?, Torque =?, Angle =?, Curve =?, TighteningStatus =?, TighteningTime =?, LocalDateTime =now(), Operator =?, UploadMark =?, UploadTime =?, Cycle =?, Program =?, JobStatus =?, Type =?, IIO =?, MaxValue = ?, MinValue = ?, Channel =?, ErrorCode =? WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+Localtable+")as t)");
            query2.prepare("UPDATE "+Localtable+" SET RecordID =?, IDCode =?, ScrewID =?, Torque =?, Angle =?, Curve =?, TighteningStatus =?, TighteningTime =?, LocalDateTime =now(), Operator =?, UploadMark =?, UploadTime =?, Cycle =?, Program =?, JobStatus =?, Type =?, IIO =?, MaxValue = ?, MinValue = ?, Channel =?, ErrorCode =?, Workshop = ?, WorkShift = ?, IpAddress = ?, PlusMark = ? WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+Localtable+")as t)");
        }
        query2.addBindValue(RecordIDMax+1);
        query2.addBindValue(data_model[6]);
        query2.addBindValue(data_model[5]);
        query2.addBindValue(data_model[3]);
        query2.addBindValue(data_model[4]);
        query2.addBindValue(data_model[8]);
        query2.addBindValue(data_model[2]);
        query2.addBindValue(data_model[0]+" "+data_model[1]);
        //query2.addBindValue(now());
        query2.addBindValue(Operator);
        query2.addBindValue(uploadmark);
        query2.addBindValue("");
        query2.addBindValue(Cycle);
        query2.addBindValue(Program);
        query2.addBindValue(JobStatus);
        query2.addBindValue(data_model[11]);
        query2.addBindValue(Order);
        query2.addBindValue(MaxValue);
        query2.addBindValue(MinValue);
        query2.addBindValue(Channel);
        query2.addBindValue("0");
        query2.addBindValue("Assembly");     //车间
        query2.addBindValue(WorkShift);  //班次
        query2.addBindValue(data_model[16]);
        query2.addBindValue(1);

        bool inserttmp = query2.exec();
        int numRowsAffected=query2.numRowsAffected();

        QString temp = data_model[6]+QString("||ScrewID:")+data_model[5]+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||JobStatus:")+data_model[10]+QString("||AutoNO:")+data_model[11]+QString("||Order:")+data_model[12]+QString("||MaxValue:")+data_model[13]+QString("||MinValue:")+data_model[14]+QString("||Channel:")+data_model[15]+QString("||IpAddress:")+data_model[16];

        if(!inserttmp)
        {
            qDebug() << query2.lastError().text();
            if(!db2.isOpen())
                mysqlopen();
            if(!query2.exec())
            {
                if(!query2.exec())
                {
                    //插入3次不成功存文件
                    msg= QString("SqlThread mysql insert false: IDCode:")+temp;
                    qDebug()<<msg;
                    qDebug()<<"need to repair table BenzTighteningDatas";
                    //重启mysql 服务记录 缓存文件
                    emit send_mysqlerror();
                }
                else
                {
                    msg= QString("SqlThread mysql insert success3: IDCode:")+temp;
                    qDebug()<<msg;
                }
            }
            else
            {
                msg= QString("SqlThread mysql insert success2: IDCode:")+temp;
                qDebug()<<msg;
            }
        }
        else if(inserttmp && numRowsAffected ==0)
        {
            msg= QString("SqlThread mysql insert false: IDCode:")+temp;
            qDebug()<<msg;
            qDebug()<<"need to optimize table BenzTighteningDatas";
            emit send_mysqlerror();
        }
        else if(inserttmp && numRowsAffected >0)
        {
            msg= QString("SqlThread mysql insert success1: IDCode:")+temp;
            qDebug()<<msg;
        }
        sqlclose();
    }
    else
    {
        qDebug()<<"db2 not open or unconnected";
    }
}


void SqlThreadBenz::configOne(QString screwid, QString vin_pin_sql, QString protmp, int Channel)
{
    //    QString data_model[17];
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    DATA_STRUCT demo;
    demo.data_model[0] = time.toString("yyyy-MM-dd");
    demo.data_model[1] = time.toString("hh:mm:ss");
    demo.data_model[2] = "NOK";
    demo.data_model[3] = "-1";
    demo.data_model[4] = "-1";
    demo.data_model[5] = screwid;
    demo.data_model[6] = vin_pin_sql;
    demo.data_model[7] = "-1";
    demo.data_model[8] = "Curve is null";
    demo.data_model[9] = protmp;
    demo.data_model[10] = "0";
    demo.data_model[11] = Type;
    demo.data_model[12] = QString::number(BoltOrder[Channel-1]++);
    demo.data_model[13] = "0";
    demo.data_model[14] = "0";
    demo.data_model[15] = QString::number(Channel);
    if(Channel == 1)
        demo.data_model[16] = controllerIp1;
    else if(Channel == 2)
        demo.data_model[16] = controllerIp2;


    NOKflag = true;
    if((BoltOrder[Channel-1]-1) == BoltTotalNum)
        demo.data_model[10] = "2";

    QVariant DataVar;
    DataVar.setValue(demo);
    sqlinsert(DataVar);

}

/***********************************************/
//multi channel NOK all
/***********************************************/
void SqlThreadBenz::receiveNokAll(int ch)
{
    //    QString data_model[17];
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    for(int i=0;i<ch;i++)
    {
        for(int j=0;j<20;j++)
        {
            for(int k=0;k<carInfor[i].boltNum[j];k++)
            {
                DATA_STRUCT demo;
                demo.data_model[0] = time.addSecs(k*(i+1)*(j+1)).toString("yyyy-MM-dd");
                demo.data_model[1] = time.addSecs(k*(i+1)*(j+1)).toString("hh:mm:ss");
                demo.data_model[2] = "NOK";
                demo.data_model[3] = "-1";
                demo.data_model[4] = "-1";
                demo.data_model[5] = carInfor[i].boltSN[j];
                demo.data_model[6] = VIN_PIN_SQL;
                demo.data_model[7] = "-1";
                demo.data_model[8] = "Curve is null";
                demo.data_model[9] = carInfor[i].proNo[j];
                demo.data_model[10] = "0";
                demo.data_model[11] = Type;
                demo.data_model[12] = QString::number(BoltOrder[i]++);
                demo.data_model[13] = "0";
                demo.data_model[14] = "0";
                demo.data_model[15] = QString::number(i+1);
                if(i == 0)
                    demo.data_model[16] = controllerIp1;
                else if(i == 1)
                    demo.data_model[16] = controllerIp2;

                NOKflag = true;
                if((BoltOrder[i]-1) == BoltTotalNum)
                    demo.data_model[10] = "2";
                QVariant DataVar;
                DataVar.setValue(demo);
                sqlinsert(DataVar);
            }
        }
    }
}

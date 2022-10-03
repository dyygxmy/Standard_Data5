#include "sqlthreadsvw2.h"

SqlThreadSVW2::SqlThreadSVW2(QObject *parent) :
    QObject(parent)
{
    m_thread.start();
    this->moveToThread(&m_thread);
}

void SqlThreadSVW2::sqlinit()
{
    db2=QSqlDatabase::addDatabase("QMYSQL","mysqlconnections");
    db2.setHostName("localhost");
    db2.setDatabaseName("Tighten");
    db2.setUserName("root");
    db2.setPassword("123456");
    query2 = QSqlQuery(db2);
    isFirst = true;
    isFirstError = true;
    Factory = factory;
    NOKflag = false;

    changeColumnName();
}

void SqlThreadSVW2::changeColumnName()
{
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlconnections"))
        mysqlopen();

    if(isFirst)
    {
        query2.exec("select * from information_schema.`COLUMNS` where TABLE_SCHEMA = 'Tighten' and TABLE_NAME = 'SVW2TighteningDatas' and COLUMN_NAME='Torque_Min2'");
        query2.next();
        QString temp = query2.value(0).toString();
        if(temp.isEmpty())
        {
            query2.exec("alter table "+ Localtable +" add (`Torque1` real null,`Torque_Max1` real null,`Torque_Min1` real null,`Angle1` real null,`Angle_Max1` real null,`Angle_Min1` real null,`Torque2` real null,`Torque_Max2` real null,`Torque_Min2` real null,`Angle2` real null,`Angle_Max2` real null,"
                        "`Angle_Min2` real null,`Torque3` real null,`Torque_Max3` real null,`Torque_Min3` real null,`Angle3` real null,`Angle_Max3` real null,`Angle_Min3` real null,`Torque4` real null,`Torque_Max4` real null,`Torque_Min4` real null,`Angle4` real null,"
                        "`Angle_Max4` real null,`Angle_Min4` real null,`Torque5` real null,`Torque_Max5` real null,`Torque_Min5` real null,`Angle5` real null,`Angle_Max5` real null,`Angle_Min5` real null,`Torque_Max` real null,"
                        "`Torque_Min` real null,`Angle_Max` real null,`Angle_Min` real null)");
        }
    }
}

/*
     数据库关闭
*/
void SqlThreadSVW2::sqlclose()
{
    if(db2.isOpen())
        db2.close();
}
//本地mysql open
void SqlThreadSVW2::mysqlopen()
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
            DTdebug()<< "SqlThreadSVW2 localmysql "<< db2.lastError().text();
            //**************打不开重启mysql********************
            emit send_mysqlerror();

        }else
        {
            DTdebug()<< "SqlThreadSVW2 localmysql ok 2";
        }
    }else
    {
        DTdebug()<< "SqlThreadSVW2 localmysql ok 1";
    }
}

QString SqlThreadSVW2::addzero(int m, QString s)
 {
    int i;
    QString t=s;
    for(i=0; i< m-s.length(); i++)
    {
        t.prepend("0");
    }
    return t;
 }


/* 数据模型
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
data_model[10] = Channel;
data_model[11] = Type;
data_model[12] = Order;
data_model[13] = MaxValue;
data_model[14] = MinValue;
data_model[15] = JobStatus;*/

void SqlThreadSVW2::sqlinserterror(QVariant DataVar)
{
    QVariantMap tmpMap = DataVar.toMap();
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlconnections"))
        mysqlopen();
    //**************************本地数据库*******************************************
    if(db2.isOpen() && QSqlDatabase::contains("mysqlconnections"))
    {
        if(isFirstError)
        {
            if(!query2.exec("CREATE TABLE IF NOT EXISTS TighteningDataErrors (RecordID int not null primary key auto_increment, UploadMark tinyint not null,Station varchar(28) null, VEN varchar(28) null, TYP varchar(28) null, SNR varchar(28) null, VNR varchar(28) null, IPA varchar(28) null, UploadTime datetime not null, ECL varchar(28) null, ERC varchar(28) null, ERT varchar(28) null, EST varchar(28) null, KNR varchar(28) null, LocalDateTime datetime not null)"))
            {
                DTdebug()<<"create table TighteningDataErrors fail"<<query2.lastError();
                emit send_mysqlerror();
            }
            else
                isFirstError = false;
        }

        query2.exec("SELECT MAX(RecordID) FROM TighteningDataErrors");
        query2.next();
        int RecordIDMax = query2.value(0).toInt();

        query2.exec("SELECT COUNT(*) FROM TighteningDataErrors");
        query2.next();
        int numRows = query2.value(0).toInt();
        if (numRows < 10000)
        {
            query2.prepare("INSERT INTO TighteningDataErrors(RecordID, UploadMark, Station, VEN, TYP, SNR, VNR, IPA, UploadTime, ECL, ERC, ERT, EST, KNR, LocalDateTime)"
                           "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        }
        else
        {
            query2.prepare("UPDATE TighteningDataErrors SET RecordID =?, UploadMark =?, Station =?, VEN =?, TYP =?, SNR =?, VNR =?, IPA =?, UploadTime =?, ECL =?, ERC =?, ERT =?, EST =?, KNR =?, LocalDateTime =? WHERE RecordID = (select Min(t.RecordID) from (select RecordID from TighteningDataErrors)as t)");
        }

        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        QString tmpStation = configIniRead->value("baseinfo/StationId").toString();
        configIniRead->deleteLater();

        QString tmpKNR = tmpMap.value("KNR").toString();
        if(tmpKNR.isEmpty())
            tmpKNR = "1";

        query2.addBindValue(RecordIDMax+1);
        query2.addBindValue("4");
        query2.addBindValue(tmpStation);
        query2.addBindValue(tmpMap.value("VEN").toString());
        query2.addBindValue(tmpMap.value("TYP").toString());
        query2.addBindValue(tmpMap.value("SNR").toString());
        query2.addBindValue(tmpMap.value("VNR").toString());
        query2.addBindValue(tmpMap.value("IPA").toString());
        query2.addBindValue(tmpMap.value("UploadTime").toString());
        query2.addBindValue(tmpMap.value("ECL").toString());
        query2.addBindValue(tmpMap.value("ERC").toString());
        query2.addBindValue(tmpMap.value("ERT").toString());
        query2.addBindValue(tmpMap.value("EST").toString());
        query2.addBindValue(tmpKNR);
        query2.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        bool inserttmp = query2.exec();
        if(!inserttmp)
        {
            qDebug()<<"insert error" << query2.lastError();
        }
        sqlclose();
    }
    else
    {
        qDebug()<<"db2 not open or unconnected";
    }
}

void SqlThreadSVW2::sqlinsert(QVariant DataVar)
{

    //QString *data_model = data_models;
    DATA_STRUCT dataStruct;
    dataStruct = DataVar.value<DATA_STRUCT>();
    QString data_model[50];
    QString tmpString;
    for(int i = 0; i<50; i++)
    {
        data_model[i] = dataStruct.data_model[i];
        tmpString.append(data_model[i] + QString("  "));
    }
    DTdebug() << "here is sqlthread";
    DTdebug() << "DataVar_sqlinsert" << tmpString;


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
    data_model[10] = Channel;
    data_model[11] = Type;
    data_model[12] = Order;
    data_model[13] = MaxValue;
    data_model[14] = MinValue;
    data_model[15] = JobStatus;
    *******************************/
    //DTdebug() << data_model[0] << data_model[1] <<data_model[2] <<data_model[3] <<data_model[4] <<data_model[5] <<data_model[6] ;
    //bool datasql = false;
    // bool curvesql = false;
    double Torque1,Torque_Max1,Torque_Min1,Angle1,Angle_Max1,Angle_Min1,Torque2,Torque_Max2,Torque_Min2,Angle2,Angle_Max2,
            Angle_Min2,Torque3,Torque_Max3,Torque_Min3,Angle3,Angle_Max3,Angle_Min3,Torque4,Torque_Max4,Torque_Min4,Angle4,
            Angle_Max4,Angle_Min4,Torque5,Torque_Max5,Torque_Min5,Angle5,Angle_Max5,Angle_Min5,Torque_Max,
            Torque_Min,Angle_Max,Angle_Min=0;

    int uploadmark = 0;
    QString msg="";
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlconnections"))
        mysqlopen();
    QString screwid_sql = data_model[5];
    int Cycle = data_model[7].toInt();
    int Program = data_model[9].toInt();
    int Channel = data_model[10].toInt();
    int Order = data_model[12].toInt();
    int JobStatus = data_model[15].toInt();
    QString statusBlc = data_model[2] ;

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

    Torque1 = data_model[16].toDouble() ;
    Torque_Max1 = data_model[17].toDouble() ;
    Torque_Min1 = data_model[18].toDouble() ;
    Angle1 = data_model[19].toDouble() ;
    Angle_Max1 = data_model[20].toDouble() ;
    Angle_Min1 = data_model[21].toDouble() ;

    Torque2 = data_model[22].toDouble() ;
    Torque_Max2 = data_model[23].toDouble() ;
    Torque_Min2 = data_model[24].toDouble() ;
    Angle2 = data_model[25].toDouble() ;
    Angle_Max2 = data_model[26].toDouble() ;
    Angle_Min2 = data_model[27].toDouble() ;

    Torque3 = data_model[28].toDouble() ;
    Torque_Max3 = data_model[29].toDouble() ;
    Torque_Min3 = data_model[30].toDouble() ;
    Angle3 = data_model[31].toDouble() ;
    Angle_Max3 = data_model[32].toDouble() ;
    Angle_Min3 = data_model[33].toDouble() ;

    Torque4 = data_model[34].toDouble() ;
    Torque_Max4 = data_model[35].toDouble() ;
    Torque_Min4 = data_model[36].toDouble() ;
    Angle4 = data_model[37].toDouble() ;
    Angle_Max4 = data_model[38].toDouble() ;
    Angle_Min4 = data_model[39].toDouble() ;

    Torque5 = data_model[40].toDouble() ;
    Torque_Max5 = data_model[41].toDouble() ;
    Torque_Min5 = data_model[42].toDouble() ;
    Angle5 = data_model[43].toDouble() ;
    Angle_Max5 = data_model[44].toDouble() ;
    Angle_Min5 = data_model[45].toDouble() ;


    Torque_Max = data_model[46].toDouble() ;
    Torque_Min = data_model[47].toDouble() ;
    Angle_Max = data_model[48].toDouble() ;
    Angle_Min = data_model[49].toDouble() ;



    if(screwid_sql.isEmpty())
    {
        msg= QString("Screwid=0 discard the data : IDCode:")+data_model[6]+QString("||ScrewID:")+screwid_sql+QString("||Torque:")+data_model[3]+
                QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString
                ("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::
                number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||Channel:")+data_model[10];
        DTdebug()<<msg;
        return;
    }

    DTdebug() << "Program:" << Program;

    //    if(curvesql && datasql)
    //        uploadmark = 1;//all successful
    //    else if(curvesql && !datasql)
    //        uploadmark = 2;//data not successful
    //    else if(!curvesql && datasql)
    //        uploadmark = 3;
    //    else if(!curvesql && !datasql)

#if 0
    if(Program==99 || screwid_sql=="100000000" || screwid_sql=="200000000" || screwid_sql=="300000000" || screwid_sql=="400000000")
        uploadmark = 1;
    else
    {
        if(isReplaceBarcode)
            uploadmark = 5;
        else
            uploadmark = 4;
    }
#else
    if(statusBlc == "LSN" || screwid_sql=="100000000" || screwid_sql=="200000000" || screwid_sql=="300000000" || screwid_sql=="400000000")
    {
        DTdebug() << "upInversion" << upInversion;
        if( upInversion )
            uploadmark = 4;
        else
            uploadmark = 1;
    }

    else
    {
        if(isReplaceBarcode)
            uploadmark = 5;
        else
            uploadmark = 4;
    }

#endif


    //**************************本地数据库*******************************************
    if(db2.isOpen() && QSqlDatabase::contains("mysqlconnections"))
    {
        DTdebug()<<"2222222222222222222222222222";
        if(isFirst)
        {
            QString tmpSqlStr = QString("CREATE TABLE IF NOT EXISTS ")+Localtable+QString(" (RecordID int not null primary key auto_increment, IDCode varchar(128) not null,ScrewID varchar(28) null,Torque real not null,Angle real not null,"
                                                                         "Curve text not null,TighteningStatus char(3) not null,TighteningTime datetime not null,LocalDateTime datetime not null,Operator char(15) null,UploadMark tinyint not null,UploadTime datetime null,Cycle int null,Program int null, Channel int null, Type char(20) not null, Order_id int null, `MaxValue` float null, `MinValue` float null, JobStatus int null,"
                                                                         "Torque1 real null,Torque_Max1 real null,Torque_Min1 real null,Angle1 real null,Angle_Max1 real null,Angle_Min1 real null,Torque2 real null,Torque_Max2 real null,Torque_Min2 real null,Angle2 real null,Angle_Max2 real null,"
                                                                         "Angle_Min2 real null,Torque3 real null,Torque_Max3 real null,Torque_Min3 real null,Angle3 real null,Angle_Max3 real null,Angle_Min3 real null,Torque4 real null,Torque_Max4 real null,Torque_Min4 real null,Angle4 real null,"
                                                                         "Angle_Max4 real null,Angle_Min4 real null,Torque5 real null,Torque_Max5 real null,Torque_Min5 real null,Angle5 real null,Angle_Max5 real null,Angle_Min5 real null,Torque_Max real null,"
                                                                         "Torque_Min real null,Angle_Max real null,Angle_Min real null) DEFAULT CHARSET=utf8");

             DTdebug()<<tmpSqlStr;
            if(!query2.exec(tmpSqlStr))
            {
                DTdebug()<<"11111111111111111111111111";
                DTdebug()<<tmpSqlStr;
                DTdebug()<<"create table "+Localtable+" fail"<<query2.lastError();
                emit send_mysqlerror();
            }
            else
            {
                DTdebug()<<"3333333333333333333333333333333";
                DTdebug()<<tmpSqlStr;
                isFirst = false;
            }
        }
        DTdebug()<<"444444444444444444444444444444444";

        query2.exec("SELECT MAX(RecordID) FROM "+Localtable);
        query2.next();
        int RecordIDMax = query2.value(0).toInt();

        query2.exec("SELECT COUNT(*) FROM "+Localtable);
        query2.next();
        int numRows = query2.value(0).toInt();
#if 0
        if (numRows < saveTightenResultNum)
        {
            query2.prepare("INSERT INTO "+Localtable+"(RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, LocalDateTime, Operator, UploadMark, UploadTime, Cycle, Program, Channel, Type, Order_id, `MaxValue`, `MinValue`, JobStatus)"
                           "VALUES (?, ?, ?, ?, ?, ?, ?, ?, now(), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        }
        else
        {
            query2.prepare("UPDATE "+Localtable+" SET RecordID =?, IDCode =?, ScrewID =?, Torque =?, Angle =?, Curve =?, TighteningStatus =?, TighteningTime =?, LocalDateTime =now(), Operator =?, UploadMark =?, UploadTime =?, Cycle =?, Program =?, Channel =?, Type =?, Order_id =?, `MaxValue` =?, `MinValue` =?, JobStatus =?  WHERE RecordID = (select Min(t.RecordID) from (select RecordID from "+Localtable+")as t)");
        }
#else
        if(numRows>saveTightenResultNum)
        {
            DTdebug()<<"delete tighten data" ;
            query2.exec("delete from "+Localtable+" where RecordID in (select x.RecordID from (select RecordID from "+Localtable+" order by RecordID limit  100) as  x) and UploadMark=1");
        }
        else
        {

        }
        query2.prepare("INSERT INTO "+Localtable+"(RecordID, IDCode, ScrewID, Torque, Angle, Curve, TighteningStatus, TighteningTime, LocalDateTime, Operator, UploadMark, UploadTime, Cycle, Program, Channel, Type, Order_id, `MaxValue`, `MinValue`, JobStatus,"
                       "Torque1,Torque_Max1,Torque_Min1,Angle1,Angle_Max1,Angle_Min1,Torque2,Torque_Max2,Torque_Min2,Angle2,Angle_Max2,"
                       "Angle_Min2,Torque3,Torque_Max3,Torque_Min3,Angle3,Angle_Max3,Angle_Min3,Torque4,Torque_Max4,Torque_Min4,Angle4,"
                       "Angle_Max4,Angle_Min4,Torque5,Torque_Max5,Torque_Min5,Angle5,Angle_Max5,Angle_Min5,Torque_Max,"
                       "Torque_Min,Angle_Max,Angle_Min)"
                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?, now(), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
#endif
        query2.addBindValue(RecordIDMax+1);
        query2.addBindValue(data_model[6]);
        if(screwid_sql.size() == 7)     //马头 螺栓数量配多个的
            query2.addBindValue(screwid_sql+addzero(2,QString::number(Order)));
        else
            query2.addBindValue(screwid_sql);
        query2.addBindValue(data_model[3]);
        query2.addBindValue(data_model[4]);
        if(data_model[8].isEmpty())
        {
            query2.addBindValue("Curve is null");
        }
        else
            query2.addBindValue(data_model[8]);
        query2.addBindValue(data_model[2]);

        QString tmpCurDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        if(tmpCurDateTime.isEmpty())
            tmpCurDateTime = "1970-01-01 00:00:01";

        //柳汽是alats op协议，和控制器的时间同步有问题，也可能是同步我写错了。就先这样用吧。
        if(factory== "Dongfeng")
        {
            if(data_model[0]=="" || data_model[1]=="")
                query2.addBindValue(data_model[0]+" "+data_model[1]);
            else
                query2.addBindValue(tmpCurDateTime);
        }
        else {
            if(data_model[0]=="" || data_model[1]=="")
                query2.addBindValue(tmpCurDateTime);
            else
            {
                tmpCurDateTime = data_model[0]+" "+data_model[1];
                query2.addBindValue(tmpCurDateTime);
            }
        }


        DTdebug() << "sqlTheeadsvw2 uploadmark:" << uploadmark;

        //query2.addBindValue(now());   //LocalDateTime
        query2.addBindValue(Operator);
        query2.addBindValue(uploadmark);
        query2.addBindValue(tmpCurDateTime);  //"1970-01-01 00:00:01"
        query2.addBindValue(Cycle);
        query2.addBindValue(Program);
        query2.addBindValue(Channel);
        if(data_model[11].isNull())
            data_model[11] = "NULL";
        query2.addBindValue(data_model[11]);
        query2.addBindValue(Order);
        query2.addBindValue(MaxValue);
        query2.addBindValue(MinValue);
        query2.addBindValue(JobStatus);

        if(factory=="SVW2")
        {
            query2.addBindValue(Torque1);
            query2.addBindValue(Torque_Max1);
            query2.addBindValue(Torque_Min1);
            query2.addBindValue(Angle1);
            query2.addBindValue(Angle_Max1);
            query2.addBindValue(Angle_Min1);

            query2.addBindValue(Torque2);
            query2.addBindValue(Torque_Max2);
            query2.addBindValue(Torque_Min2);
            query2.addBindValue(Angle2);
            query2.addBindValue(Angle_Max2);
            query2.addBindValue(Angle_Min2);

            query2.addBindValue(Torque3);
            query2.addBindValue(Torque_Max3);
            query2.addBindValue(Torque_Min3);
            query2.addBindValue(Angle3);
            query2.addBindValue(Angle_Max3);
            query2.addBindValue(Angle_Min3);

            query2.addBindValue(Torque4);
            query2.addBindValue(Torque_Max4);
            query2.addBindValue(Torque_Min4);
            query2.addBindValue(Angle4);
            query2.addBindValue(Angle_Max4);
            query2.addBindValue(Angle_Min4);

            query2.addBindValue(Torque5);
            query2.addBindValue(Torque_Max5);
            query2.addBindValue(Torque_Min5);
            query2.addBindValue(Angle5);
            query2.addBindValue(Angle_Max5);
            query2.addBindValue(Angle_Min5);


            query2.addBindValue(Torque_Max);
            query2.addBindValue(Torque_Min);

            query2.addBindValue(Angle_Max);
            query2.addBindValue(Angle_Min);
        }

        bool inserttmp = query2.exec();
        int numRowsAffected=query2.numRowsAffected();

        if(!inserttmp)
        {
            DTdebug()<<"insert error" << query2.lastError();
            if(!db2.isOpen())
                mysqlopen();
            if(!query2.exec())
            {
                if(!query2.exec())
                {
                    //插入3次不成功存文件
                    msg= QString("SqlThreadSVW2 mysql insert false: IDCode:")+data_model[6]+QString("||ScrewID:")+screwid_sql+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||Channel:")+data_model[10];
                    DTdebug()<<msg;
                    DTdebug()<<"need to repair table "+Localtable;
                    //重启mysql 服务记录 缓存文件
                    emit send_mysqlerror();
                }
                else
                {
                    msg= QString("SqlThreadSVW2 mysql insert success3: IDCode:")+data_model[6]+QString("||ScrewID:")+screwid_sql+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||Channel:")+data_model[10];
                    DTdebug()<<msg;
                }
            }
            else
            {
                msg= QString("SqlThreadSVW2 mysql insert success2: IDCode:")+data_model[6]+QString("||ScrewID:")+screwid_sql+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||Channel:")+data_model[10];
                DTdebug()<<msg;
            }
        }
        else if(inserttmp && numRowsAffected ==0)
        {
            msg= QString("SqlThreadSVW2 mysql insert false: IDCode:")+data_model[6]+QString("||ScrewID:")+screwid_sql+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||Channel:")+data_model[10];
            DTdebug()<<msg;
            DTdebug()<<"need to optimize table "+Localtable;
            emit send_mysqlerror();

        }
        else if(inserttmp && numRowsAffected >0)
        {
            msg= QString("SqlThreadSVW2 mysql insert success1: IDCode:")+data_model[6]+QString("||ScrewID:")+screwid_sql+QString("||Torque:")+data_model[3]+QString("||Angle:")+data_model[4]+QString("||TighteningStatus:")+data_model[2]+QString("||TighteningTime:")+data_model[0]+QString("||")+data_model[1]+QString("||LocalDateTime:")+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")+QString("||Operator:")+Operator+QString("||UploadMark:")+QString::number(uploadmark)+QString("||Cycle:")+data_model[7]+QString("||Program:")+data_model[9]+QString("||Channel:")+data_model[10];
            DTdebug()<<msg;
        }
        sqlclose();
    }
    else
    {
        DTdebug()<<"db2 not open or unconnected";
    }
}

/***********************************************/
//multi channel NOK all
/***********************************************/
void SqlThreadSVW2::receiveNokAll(int ch, int ch2)
{
    //    QString data_model[16];
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间

    if(ch2 == 2)
    {
        for(int i=0;i<ch;i++)
        {
            for(int j=0;j<20;j++)
            {
                for(int k=0;k<carInfor[i].boltNum[j];k++)
                {
                    DATA_STRUCT demo;
                    demo.data_model[0] = time.addSecs(k*(j+1)*(i+1)).toString("yyyy-MM-dd");
                    demo.data_model[1] = time.addSecs(k*(j+1)*(i+1)).toString("hh:mm:ss");
                    demo.data_model[2] = "AK2";
                    demo.data_model[3] = "-2";
                    demo.data_model[4] = "-2";
                    demo.data_model[5] = carInfor[i].boltSN[j];
                    if(VIN_PIN_SQL_of_SpecialStation.isEmpty())
                        demo.data_model[6] = VIN_PIN_SQL;
                    else
                        demo.data_model[6] = VIN_PIN_SQL_of_SpecialStation;
                    //demo.data_model[6] = VIN_PIN_SQL;
                    demo.data_model[7] = "-1";
                    demo.data_model[8] = "Curve is null";
                    demo.data_model[9] = carInfor[i].proNo[j];
                    demo.data_model[10] = "0";

                    if( Factory=="Dongfeng" && manualMode)
                        demo.data_model[11] = "manual";
                    else
                        demo.data_model[11] = Type;

                    demo.data_model[12] = QString::number(BoltOrder[0]++);
                    demo.data_model[13] = "0";
                    demo.data_model[14] = "0";
                    demo.data_model[15] = "0";

                    NOKflag = true;
                    if((BoltOrder[0]-1) == BoltTotalNum)
                        demo.data_model[15] = "2";
                    QVariant DataVar;
                    DataVar.setValue(demo);
                    sqlinsert(DataVar);


                }
            }
        }
    }
    else if(ch2 == 1)
    {
        for(int i=0;i<ch;i++)
        {
            for(int j=0;j<20;j++)
            {
                for(int k=0;k<carInfor[i].boltNum[j];k++)
                {
                    DATA_STRUCT demo;
                    demo.data_model[0] = time.addSecs(k*(j+1)*(i+1)).toString("yyyy-MM-dd");
                    demo.data_model[1] = time.addSecs(k*(j+1)*(i+1)).toString("hh:mm:ss");
                    demo.data_model[2] = "AK1";
                    demo.data_model[3] = "-1";
                    demo.data_model[4] = "-1";
                    demo.data_model[5] = carInfor[i].boltSN[j];
                    if(VIN_PIN_SQL_of_SpecialStation.isEmpty())
                        demo.data_model[6] = VIN_PIN_SQL;
                    else
                        demo.data_model[6] = VIN_PIN_SQL_of_SpecialStation;
                    //demo.data_model[6] = VIN_PIN_SQL;
                    demo.data_model[7] = "-1";
                    demo.data_model[8] = "Curve is null";
                    demo.data_model[9] = carInfor[i].proNo[j];
                    demo.data_model[10] = "0";

                    if( Factory=="Dongfeng" && manualMode)
                        demo.data_model[11] = "manual";
                    else
                        demo.data_model[11] = Type;

                    demo.data_model[12] = QString::number(BoltOrder[0]++);
                    demo.data_model[13] = "0";
                    demo.data_model[14] = "0";
                    demo.data_model[15] = "0";

                    NOKflag = true;
                    if((BoltOrder[0]-1) == BoltTotalNum)
                        demo.data_model[15] = "2";
                    QVariant DataVar;
                    DataVar.setValue(demo);
                    sqlinsert(DataVar);
                }
            }
        }
    }
}

void SqlThreadSVW2::configOne(QString screwid, QString vin_pin_sql, QString protmp,int Channel)
{
    Q_UNUSED(Channel);
    //    QString data_model[11];
    DATA_STRUCT demo;
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    demo.data_model[0] = time.toString("yyyy-MM-dd");
    demo.data_model[1] = time.toString("hh:mm:ss");
    demo.data_model[2] = "AK1";
    demo.data_model[3] = "-1";
    demo.data_model[4] = "-1";
    demo.data_model[5] = screwid;
    if(VIN_PIN_SQL_of_SpecialStation.isEmpty())
        demo.data_model[6] = vin_pin_sql;
    else
        demo.data_model[6] = VIN_PIN_SQL_of_SpecialStation;
    //demo.data_model[6] = vin_pin_sql;
    demo.data_model[7] = "-1";
    demo.data_model[8] = "Curve is null";
    demo.data_model[9] = protmp;
    demo.data_model[10] = "0";

    if( Factory=="Dongfeng" && manualMode)
        demo.data_model[11] = "manual";
    else
        demo.data_model[11] = Type;

    demo.data_model[12] = QString::number(BoltOrder[0]++);
    demo.data_model[13] = "0";
    demo.data_model[14] = "0";
    demo.data_model[15] = "0";

    NOKflag = true;
    if((BoltOrder[0]-1) == BoltTotalNum)
        demo.data_model[15] = "2";
    QVariant DataVar;
    DataVar.setValue(demo);
    sqlinsert(DataVar);
}
/***********************************************/
//NOK one group of PLC Data2
/***********************************************/
void SqlThreadSVW2::configOneGroup(QString vin_pin_sql,QString *screwid,QString *proNO)
{
    int i;
    QString screwidBuf[2],proNOBuf[2];
    //    QString data_model[11];
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    for(i=0;i<2;i++)
    {
        screwidBuf[i] = screwid[i];
        proNOBuf[i] = proNO[i];
    }
    for(i=0;i<2;i++)
    {
        DATA_STRUCT demo;
        demo.data_model[0] = time.addSecs(i).toString("yyyy-MM-dd");
        demo.data_model[1] = time.addSecs(i).toString("hh:mm:ss");
        demo.data_model[2] = "AK1";
        demo.data_model[3] = "-1";
        demo.data_model[4] = "-1";
        demo.data_model[5] = screwidBuf[i];
        if(VIN_PIN_SQL_of_SpecialStation.isEmpty())
            demo.data_model[6] = vin_pin_sql;
        else
            demo.data_model[6] = VIN_PIN_SQL_of_SpecialStation;
        //demo.data_model[6] = vin_pin_sql;
        demo.data_model[7] = "-1";
        demo.data_model[8] = "Curve is null";
        demo.data_model[9] = proNOBuf[i];
        demo.data_model[10] = "0";

        demo.data_model[11] = Type;
        demo.data_model[12] = QString::number(BoltOrder[0]++);
        demo.data_model[13] = "0";
        demo.data_model[14] = "0";
        demo.data_model[15] = "0";

        NOKflag = true;
        if((BoltOrder[0]-1) == BoltTotalNum)
            demo.data_model[15] = "2";
        QVariant DataVar;
        DataVar.setValue(demo);
        sqlinsert(DataVar);
    }
}


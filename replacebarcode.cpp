#include "replacebarcode.h"

ReplaceBarcode::ReplaceBarcode(QObject *parent) :
    QObject(parent)
{
    replace_thread.start();
    this->moveToThread(&replace_thread);
}

//打开数据库
void ReplaceBarcode::myreplace()
{
    DTdebug()<< "ReplaceBarcode thread start";
    db2=QSqlDatabase::addDatabase("QMYSQL","mysql_replace");
    db2.setHostName("localhost");
    db2.setDatabaseName("Tighten");
    db2.setUserName("root");
    db2.setPassword("123456");
    query2 = QSqlQuery(db2);

    list << "" << "";
    serialNums = "";
    on_actionOpen_triggered();
//    flag_l = false;
}

//打开串口
void ReplaceBarcode::on_actionOpen_triggered()
{
    FUNC() ;
    QString portName = "ttymxc0" /*"ttyUSB0"*/;   //获取串口名
    myCom = new QextSerialPort("/dev/" + portName);
    connect(myCom, SIGNAL(readyRead()), this, SLOT(readMyCom()));

    //设置波特率
    myCom->setBaudRate((BaudRateType)9600);

    //设置数据位
    myCom->setDataBits((DataBitsType)8);

    //设置校验
    myCom->setParity(PAR_NONE);

    //设置停止位
    myCom->setStopBits(STOP_1);

    //设置数据流控制
    myCom->setFlowControl(FLOW_OFF);
    //设置延时
    myCom->setTimeout(TIME_OUT);

    if(myCom->open(QIODevice::ReadWrite)){
        DTdebug()<<QObject::tr("serial open ok ") << portName;

    }else{
        DTdebug()<<QObject::tr("serial open fail ") << portName;
    }
}

//读取数据
void ReplaceBarcode::readMyCom()
{
    FUNC();
    //list[0] VIN  list[1] 零件码

    QString VIN = "";
    QString code = "";
    QByteArray temp = myCom->readAll();

    DTdebug() << "com temp " << temp ;

    if(!temp.isEmpty())
    {
        serialNums = serialNums+temp;

        //回车等清除
        while(!serialNums.startsWith("L") && !serialNums.startsWith("5") && !serialNums.startsWith("2") &&!serialNums.isEmpty())
        {
            serialNums.replace(0,1,"");
        }
//        DTdebug()<<serialNums;


        while(serialNums.size()>14)
        {
            //零件码
            if(serialNums.startsWith("551"))
            {
                if(serialNums.size() == 15)
                {
                    code = serialNums;
                    serialNums.clear();
                }
                else if(serialNums.size()>15)
                {
                    code = serialNums.left(15);
                    serialNums.replace(0,15,"");
                    while(!serialNums.startsWith("L") && !serialNums.startsWith("5") && !serialNums.isEmpty())
                    {
                        serialNums.replace(0,1,"");
                    }
                }
                qDebug()<<"new code"<<code;
                if(code != list[1])
                {
                    list[1] = code;
                    if(list[0] != "")
                    {
                        repalace(list[0],list[1]);
                        list[0] = "";
                        list[1] = "";
                    }
                }
                else if(code == list[1])
                    DTdebug()<<"code repeat";
            }
            //VIN码
            else if(serialNums.startsWith("LS"))
            {
                DTdebug( ) << "VIN" << list[1];
                if(list[1] == "")
                {
                    QVariantMap statusNull ;
                    DTdebug( ) << "replaceCodeNull" ;
                    serialNums = "" ;
                    statusNull.insert("status","replaceCodeNull") ;
                    statusNull.insert("text","请先扫零件码" ) ;
                    Q_EMIT signalReplaceStatus(statusNull);
                    return;
                }

                if(serialNums.size() == 17)
                {
                    VIN =serialNums;
                    serialNums.clear();
                }
                else if(serialNums.size()>17)
                {
                    VIN =serialNums.left(17);
                    serialNums.replace(0,17,"");
                    while(!serialNums.startsWith("L") && !serialNums.startsWith("5") && !serialNums.isEmpty())
                    {
                        serialNums.replace(0,1,"");
                    }
                }
                else
                {
                    return;
                }
                DTdebug()<<"new VIN"<<VIN;
                if(VIN != list[0])
                {
                    list[0]=VIN;
                    if(list[1] !="")
                    {
                        repalace(list[0],list[1]);
                        list[0]="";
                        list[1]="";
                    }
                }
                else if(VIN == list[0])
                    DTdebug()<<"VIN repeat";
            }
            else if(serialNums.startsWith("2"))
            {
                emit signalQRCodeFromSerial(serialNums);
            }
            else
            {
                qDebug() << "What are you ???????  " << serialNums;
                serialNums="";
            }
        }
//        qDebug()<<"out";
    }
}


void ReplaceBarcode::mysqlopen()
{
    if(QSqlDatabase::contains("mysql_replace")){
        db2 = QSqlDatabase::database("mysql_replace");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","mysql_replace");
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
            qDebug()<< "ReplaceBarcode localmysql "<< db2.lastError().text();
            //**************打不开重启mysql********************
            //            emit send_mysqlerror();
        }else
        {
            qDebug()<< "ReplaceBarcode localmysql ok 2";
        }
    }else
    {
        qDebug()<< "ReplaceBarcode localmysql ok 1";
    }
}

void ReplaceBarcode::sqlclose()
{
    if(db2.isOpen())
        db2.close();
}



void ReplaceBarcode::repalace(QString VIN,QString code)
{
    if(!db2.isOpen()||!QSqlDatabase::contains("mysql_replace"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysql_replace"))
    {
        bool update_tmp = query2.exec("UPDATE "+Localtable+" SET UploadMark = 4,TighteningTime=now(),IDcode = '"+VIN+"' WHERE IDCode = '"+code+"' AND TighteningStatus!='OK'");
        int numRowsAffected=query2.numRowsAffected();

        bool updateTmpOK = query2.exec("UPDATE "+Localtable+" SET UploadMark = 4,TighteningTime=now()+2,IDcode = '"+VIN+"' WHERE IDCode = '"+code+"'' AND TighteningStatus='OK'");
        int numRowsAffectedOK=query2.numRowsAffected();

        if((!update_tmp)||(!updateTmpOK))
        {
            DTdebug()<<"parts code replace fail,code:"+code +"VIN:"+ VIN +" Error:" << query2.lastError();
            QVariantMap statusFail ;
            statusFail.insert("status","replaceFail") ;
            statusFail.insert("text","替换失败") ;
            Q_EMIT signalReplaceStatus(statusFail);
        }
        else if(update_tmp &&updateTmpOK && (numRowsAffectedOK>0) && (numRowsAffected > 0))
        {
            DTdebug()<<"VIN:"+VIN+" replace parts code:"+code+" numRowsAffected:"+QString::number(numRowsAffected);

            QString str = code + "\r\n成功替换为\r\n" +VIN ;
            QVariantMap statusSuccess ;
            statusSuccess.insert("status", "replaceSuccess") ;
            statusSuccess.insert("text" , str) ;
            Q_EMIT signalReplaceStatus(statusSuccess);
        }
        else if((update_tmp && numRowsAffected == 0) || (updateTmpOK && numRowsAffectedOK==0))
        {
            DTdebug()<< "Don't have the bar code";

            QVariantMap statusFail ;
            statusFail.insert("status","replaceFail") ;
            statusFail.insert("text","零件码不存在") ;
            Q_EMIT signalReplaceStatus(statusFail);
        }
    }
}

#if 0
void ReplaceBarcode::slot_read_com()
{
    QString VIN = "";
    QString code = "";
    //从串口中读取数据
    int  nTmp = serial_Com.read_datas_tty(Ser_Fd, buff, 200, 20);
    DTdebug() << "nTmp come in " << nTmp << QString(buff);
    if(nTmp > 0)
    {
        flag_l = true;
        serialNums = serialNums+QString(QLatin1String(buff));
        nTmp=0;
    }
    else
    {
        if(flag_l)
            nTmp = 10;
        else
            nTmp = 0;
        flag_l = false;
    }
    DTdebug()<<"nTmp"<<nTmp<<serialNums;

    bzero(&buff,sizeof(buff));

    if (0 < nTmp && serialNums.size() < 22 && serialNums.size() > 13)
    {
        if(serialNums.size()>15)
        {
            VIN = serialNums.mid(0,17);
            DTdebug()<<"new VIN"<<VIN;
            if(VIN != list[0])
            {
                list[0]=VIN;
                DTdebug()<<"list[0]"<<list[0];
                if(list[1] !="")
                {
                    repalace(list[0],list[1]);
                    list[0]="";
                    list[1]="";
                }
            }
            else if(VIN == list[0])
                DTdebug()<<"VIN repeat";
            serialNums.clear();
        }
        else
        {
            if(serialNums.mid(0,1)=="T")
            {
                code = serialNums.mid(0,14);
                DTdebug()<<"new code"<<code;
                if(code != list[1])
                {
                    list[1]=code;
                    if(list[0] != "")
                    {
                        repalace(list[0],list[1]);
                        list[0]="";
                        list[1]="";
                    }
                }
                else if(code == list[1])
                    DTdebug()<<"code repeat";
                serialNums.clear();
            }
        }
    }
    else if(0 < nTmp && serialNums.size() == 36)
    {
        VIN = serialNums.mid(0,17);
        DTdebug()<<"two new VINs:"<<VIN<<" "<<serialNums.mid(18,17);
        if(VIN != list[0])
        {
            list[0]=VIN;
            DTdebug()<<"list[0]"<<list[0];
            if(list[1] !="")
            {
                repalace(list[0],list[1]);
                list[0]="";
                list[1]="";
            }
            list[0]=serialNums.mid(18,17);
        }
        else if(VIN == list[0])
        {
            DTdebug()<<"VIN repeat"<<VIN;
            if(list[0]==serialNums.mid(18,17))
                DTdebug()<<"VIN repeat"<<serialNums.mid(18,17);
            else
                list[0]=serialNums.mid(18,17);
        }
        serialNums.clear();
    }
    else if(0 < nTmp && serialNums.size() == 33)
    {
        if(serialNums.mid(0,1)=="T")
        {
            code = serialNums.mid(0,14);
            VIN = serialNums.mid(15,17);
            DTdebug()<<"new code"<<code;
            DTdebug()<<"new VIN"<<VIN;
            if(code == list[1])
            {
                DTdebug()<<"code repeat" <<code;
            }
            list[1]=code;
            if(list[0] != "")
            {
                repalace(list[0],list[1]);
                list[0]=VIN;
                list[1]="";
            }
            else if(list[0] == "")
            {
                repalace(VIN,code);
                list[0]="";
                list[1]="";
            }
        }
        else if(serialNums.mid(0,1)=="L")
        {
            VIN = serialNums.mid(0,17);
            code = serialNums.mid(18,14);
            DTdebug()<<"new VIN"<<VIN;
            DTdebug()<<"new code"<<code;
            if(VIN == list[0])
            {
                DTdebug()<<"VIN repeat" <<VIN;
            }
            list[0]=VIN;
            if(list[1] != "")
            {
                repalace(list[0],list[1]);
                list[0]="";
                list[1]=code;
            }
            else if(list[1] == "")
            {
                repalace(VIN,code);
                list[0]="";
                list[1]="";
            }
        }
        serialNums.clear();
    }
    else if( 0 < nTmp && serialNums.size() == 30)
    {
        code = serialNums.mid(0,14);
        DTdebug()<<"two new codes:"<<code<<" "<<serialNums.mid(15,14);
        if(code != list[1])
        {
            list[1]=code;
            DTdebug()<<"list[1]"<<list[1];
            if(list[0] !="")
            {
                repalace(list[0],list[1]);
                list[0]="";
                list[1]="";
            }
            list[1]=serialNums.mid(16,15);
        }
        else if(code == list[1])
        {
            DTdebug()<<"code repeat"<<code;
            if(list[1]==serialNums.mid(16,15))
                DTdebug()<<"code repeat"<<serialNums.mid(16,15);
            else
                list[1]=serialNums.mid(16,15);
        }
        serialNums.clear();
    }
    else if(0 < nTmp)
    {
        DTdebug()<<serialNums<<"    what?????????????????????????"<<serialNums.size();
        serialNums.clear();
    }
}
#endif

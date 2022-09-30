#include "ftpdataparse.h"

ftpDataParse::ftpDataParse(QObject *parent) :
    QObject(parent)
{
    StartBolt = 0;
    parseJsonData_thread.start();
    this->moveToThread(&parseJsonData_thread);
}
void ftpDataParse::ftpStart()
{
    qDebug() << "FTP JSON Data parse thread start!!!!!!!!!!!!" ;
    QString path;
    path="/tmp/Curve/FO";
    QDir dir(path);
    if(!dir.exists())
    {
        dir.mkpath(path);//创建多级目录
    }
    path="/tmp/wrongCurve/";
    QDir dir_wrong(path);
    if(!dir_wrong.exists())
    {
        dir_wrong.mkpath(path);//创建多级目录
    }
    //    channel_is_true = false;
    connect(&ftpupload,SIGNAL(timeout()),this,SLOT(ftptimeouts()));
    ftpupload.start(1000);
    systemRunStatus = "OK";
}

void ftpDataParse::ftptimeouts()
{
    FindFile("/tmp/Curve");
}

void ftpDataParse::revRunStatus(QString status)
{
    systemRunStatus = status;
}

//使能
void ftpDataParse::sendReadOperate(bool enable,int n)
{
    if(!enable) //
    {
    }
    else
    {
        StartBolt = n;
    }
}

void ftpDataParse::FindFile(QString path)
{
    //qDebug() << "find file1";
    QDir dir(path);
    if (!dir.exists())
        return ;
    dir.setFilter(QDir::Dirs|QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
    int i=0;
    do{
        QFileInfo fileInfo = list.at(i);
        QString fileNm = fileInfo.fileName();
        qDebug()<<"fileInfo"<<fileInfo.filePath()<<fileNm;
 /*       if(isMaster)
        {
            if( fileNm.left(3)=="SEQ" && fileNm.right(fileNm.size()-4).toInt()>10 )
            {
                i++;
                continue;
            }
        }
        else
        {
            if( fileNm.left(3)=="SEQ" && fileNm.right(fileNm.size()-4).toInt()<11 )
            {
                i++;
                continue;
            }
        }*/


        if(fileNm=="."||fileNm=="..")
        {
            i++;
            continue;
        }
        bool bisDir=fileInfo.isDir();
        if(bisDir)
        {
            //nFiles++;
            // qDebug() << QString("%1 %2 %3").arg(fileInfo.size(), 10)
            // qDebug() << "find file2";
//            qDebug()<<"find dir"<<fileInfo.filePath();
            FindFile(fileInfo.filePath());
//            qDebug() << "find fil3";
        }
        else
        {
            //nFiles++;
            // qDebug() << QString("%1 %2 %3").arg(fileInfo.size(), 10)
            // .arg(fileInfo.fileName(),10).arg(fileInfo.path())<<endl;
            qDebug() << "find file parsing!!" << fileInfo.filePath();
            parseFile(fileInfo.absoluteFilePath(),fileInfo.baseName());

        }
        i++;
    }while(i<list.size());
}

void ftpDataParse::parseFile(QString filepath,QString fileName)
{
    Q_UNUSED(fileName);
    if(systemRunStatus != "ING")
    {
        system(QString("mv "+ filepath+" /tmp/wrongCurve/").toLocal8Bit().data());
        qDebug() << "systemRunStatus != ING move file "+filepath+" to /tmp/wrongCurve";
        return;
    }
    //    int num = fileName.mid(21,3).toInt();
    QJson::Parser parser;
    bool ok;

    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        qDebug() << "Cannot open testdsn file for Reading";
    }
    QByteArray json = file.readAll();
    QByteArray tempjson = json.right(10);
    QByteArray rightJson = "";
    for(int i=0;i<tempjson.size();i++)
    {
        if(tempjson.at(i) != 0x0d && tempjson.at(i) != 0x0a && tempjson.at(i) != 0x20)
            rightJson.append(tempjson[i]);
    }
//    qDebug()<<"rightJson"<<rightJson<<rightJson.right(3);
    if(rightJson.right(3) == "}]}")
    {
        QMap<QString,int>::iterator iter;
        for(iter = wrongMap.begin(); iter != wrongMap.end(); ++iter)
        {
            if(filepath == iter.key())
            {
                wrongMap.erase(iter);
                qDebug()<<"delete "+filepath+" from wrongMap";
                break;
            }
        }

        QVariantMap result = parser.parse(json, &ok).toMap();
        if (!ok) {
            qDebug()<<"An error occurred during parsing"<<json;
            system(QString("mv "+ filepath+" /tmp/wrongCurve/").toLocal8Bit().data());
            qDebug() << "parser fail move file "+filepath+" to /tmp/wrongCurve";
            return;
        }

        for(int m=0;m<result["no of channels"].toInt();m++)
        {
            QString Date_time,DATE_blc,TIME_blc,STATE_blc,MI_blc="",WI_blc="",Cycle_CSR,Program_blc,Channel,Curve;
            QString Torque_Max="",Torque_Min="",Angle_Max="",Angle_Min="";
            QVariantMap curveMap = result["channels"].toList().at(m).toMap();
//            Curve = result["channels"].toList().at(m).toString();
            QVariantList curveList;
            curveList << curveMap;
            QJson::Serializer serializer;
            QByteArray curveJson = serializer.serialize(curveList,&ok);
            Curve = curveJson.mid(1,curveJson.length()-2);
//            qDebug() << "curve*********" << Curve;
            STATE_blc = curveMap["result"].toString();
            Channel = QString::number(curveMap["node id"].toDouble()*10);
            Program_blc = curveMap["prg nr"].toString();
            Date_time = curveMap["date"].toString();
            DATE_blc = Date_time.mid(0,10);
            TIME_blc = Date_time.mid(11,8);
            Cycle_CSR = curveMap["cycle"].toString();

            QVariantMap tightening_steps=curveMap["tightening steps"].toList().last().toMap();
            int size = tightening_steps["tightening functions"].toList().size();
            qDebug()<<"Channel"<<Channel<<"tightening functions"<<size<<"program"<<Program_blc;
            for(int i=0;i<size;i++)
            {
                QVariantMap tightening_functions=tightening_steps["tightening functions"].toList().at(i).toMap();
                if(tightening_functions["name"].toString()=="MF AngleMax")
                {
                    WI_blc = tightening_functions["act"].toString();
                    Angle_Max = tightening_functions["nom"].toString();
                }
                else if(tightening_functions["name"].toString()=="MF AngleMin")
                {
                    Angle_Min = tightening_functions["nom"].toString();
                }
                else if(tightening_functions["name"].toString()=="MF TorqueMax")
                {
                    MI_blc = tightening_functions["act"].toString();
                    Torque_Max = tightening_functions["nom"].toString();
                }
                else if(tightening_functions["name"].toString()=="MF TorqueMin")
                {
                    Torque_Min = tightening_functions["nom"].toString();
                }
            }
            if(MI_blc == "")
                MI_blc = "0";
            if(WI_blc  == "")
                WI_blc = "0";
            if(Angle_Max == "")
                Angle_Max = "0";
            if(Angle_Min == "")
                Angle_Min = "0";
            if(Torque_Max == "")
                Torque_Max = "0";
            if(Torque_Min == "")
                Torque_Min = "0";
            qDebug()<<"***********"<<Channel<<Program_blc;
//            if(systemRunStatus == "ING")
//            {
//                return;
//            }
            DATA_STRUCT demo;
            demo.data_model[0] = DATE_blc;
            demo.data_model[1] = TIME_blc;
            demo.data_model[2] = STATE_blc;
            demo.data_model[3] = MI_blc;
            demo.data_model[4] = WI_blc;
            demo.data_model[5] = carInfor[0].boltSN[StartBolt+m];
            demo.data_model[6] = VIN_PIN_SQL;
            demo.data_model[7] = Cycle_CSR;
            demo.data_model[8] = Curve;
            demo.data_model[9] = Program_blc;
            demo.data_model[10] = Channel;
            demo.data_model[11] = Body_NO;
            if(STATE_blc == "OK")
            {
                demo.data_model[12] = QString::number(BoltOrder[0]++);
            }
            else
            {
                demo.data_model[12] = QString::number(BoltOrder[0]+m);
            }
            demo.data_model[13] = Torque_Max;
            demo.data_model[14] = Torque_Min;
            demo.data_model[15] = "0";
            demo.data_model[16] = Angle_Max;
            demo.data_model[17] = Angle_Min;
            for(int i=0;i<18;i++)
            {
                if(i == 8)
                    continue;
                qDebug()<<"emit date_model:"<<i<<demo.data_model[i];
            }
            QVariant DataVar;
            DataVar.setValue(demo);
            emit sendfromjsonthread(DataVar);
            emit send_mainwindow(MI_blc,WI_blc,STATE_blc,m);
        }
        unlink(filepath.toLocal8Bit().data());//delete file
        qDebug() << "delete file "+filepath;
    }
    else
    {
        qDebug() << filepath <<" is incomplete or wrong";
        QMap<QString,int>::iterator iter;
        bool newfile = true;
        for(iter=wrongMap.begin(); iter != wrongMap.end(); ++iter)
        {
            if(filepath == iter.key())
            {
                newfile = false;
                //                wrongMap[filepath]++;
                ++iter.value();
                if(iter.value()==5)
                {
                    wrongMap.erase(iter);
                    qDebug()<<"delete "+filepath+" from wrongMap >5 times";
                    system(QString("mv "+ filepath+" /tmp/wrongCurve/").toLocal8Bit().data());
                    //                    unlink(filepath.toLocal8Bit().data());
                    qDebug() << "move file "+filepath+" to /tmp/wrongCurve";
                }
                break;
            }
        }
        if(newfile)
        {
            wrongMap[filepath]=1;
            qDebug()<<"add newfile "+filepath+" to wrongMap";
        }
    }
}


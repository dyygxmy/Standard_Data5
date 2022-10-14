#include "rfidcontroler.h"
#include "mainwindow.h"

RfidControler::RfidControler(QObject *parent) :
    QObject(parent)
{
    RFIDIsConnect = false;
    Factory = factory;
    for(int i =0;i<10;i++)
        seriallist[i] = "";
    whichindex = 0;
    isLogin = false;
    plusFlag = false;
    isFirst = true;
    revBuf = "";
    sendBuf = "";
    systemRunStatusFlag  = false ;
    controllerStatusFlag = false ;
    RFID_Enable_Flag = false ;
    this->moveToThread(&m_thread);
    m_thread.start();
}

void RfidControler::rfidInit()
{
    DTdebug()<< "rfidcontroller thread start"<<vari_1<<rfidip<<rfidport;

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    RFIDIngNotIn = configIniRead->value("baseinfo/RFIDING").toBool();
    delete configIniRead;

    if(vari_1 == "slave")
    {
        rfidport = 4545;
    }
    m_pTcpSocket = new QTcpSocket;
    connect(m_pTcpSocket,SIGNAL(readyRead()),this,SLOT(receivemss()));
    //    connect(m_pTcpSocket,SIGNAL(connected()),this,SLOT(connectedDo()));
    connect(m_pTcpSocket,SIGNAL(disconnected()),this,SLOT(disconnectedDo()));
    connect(&pingTimer,SIGNAL(timeout()),this,SLOT(pingTimers()));
    connect(m_pTcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    if(RFID_Enable_Flag)
    {
        pingTimer.start(5000);
    }
}

void RfidControler::newConnect()
{
    m_pTcpSocket->abort();                        //取消已有的连接
    m_pTcpSocket->connectToHost(rfidip,rfidport); //连接到主机
    m_pTcpSocket->waitForConnected(2000);         //waitting 2000ms
    DTdebug() << "RFID the socket state is" << rfidip << m_pTcpSocket->state();
    if(m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        DTdebug() << "RFID link success  >>>>>>>>";
        emit rfidConnected(true);
        RFIDIsConnect = true;
    }
    else
    {
        DTdebug() << "RFID link fail XXXXXXXXXXXX" ;
    }
}

void RfidControler::displayError(QAbstractSocket::SocketError )
{
    // DTdebug() << "df" << m_pTcpSocket->errorString(); //输出错误信息
    RFIDIsConnect = false;
}

void RfidControler::connectedDo()
{
    DTdebug() << "rfid connect success";
    pingTimer.stop();
    emit rfidConnected(true);
    RFIDIsConnect = true;

}

void RfidControler::disconnectedDo()
{
    DTdebug() << "rfidconnect dis";
    emit rfidConnected(false);
    RFIDIsConnect = false;
    //    pingTimer.start(5000);
}
void RfidControler::rfidDisconnectDo(bool istrue)
{
    if(!istrue)
    {
        disconnectedDo();
    }
}

void RfidControler::receiveLogin(bool tmp)
{
    isLogin = tmp;
}

void RfidControler::receivePlusFlag(bool tmp)
{
    plusFlag = tmp;
}
/************************************************/
//receive system run status
/************************************************/
void RfidControler::receiveSystemStatus(QString status)
{
    if(status == "ING")
    {
        systemRunStatusFlag = true; //ING
    }
    else
    {
        systemRunStatusFlag = false; //no ING
    }
}
/************************************************/
//send system run status
/************************************************/
void RfidControler::sendSystemStaus(int cmd,bool vari1,bool vari2)
{
    unsigned char sum = 0;
    sendBuf[0] = 0x02;
    sendBuf[1] = cmd;
    sendBuf[2] = 0x04;           //len
    if(cmd == 0x01)
    {
        sendBuf[3] = 0;
        sendBuf[4] = 0;
    }
    else
    {
        sendBuf[3] = 0x30 + vari1;
        sendBuf[4] = 0x30 + vari2;
    }
    for(int i=2;i<5;i++)
    {
        sum += sendBuf[i];
    }
    sendBuf[5] = sum;
    sendBuf[6] = 0x03;
    m_pTcpSocket->write(sendBuf,7);
    DTdebug()<<"*****slave send*****:"<<sendBuf.toHex();
}



void RfidControler::pingTimers()
{
    if(!RFIDIsConnect)
    {
        newConnect();
    }
}
/**************************************************/
//receive data
//cmd : 0x02   0x01   0x1c  0x31 xxxxxxxxxxxxxxxxxxxxxxxxx 0x 0x03
//      start  cmd    len  pin code enable    data 25bit      end
//ACK : 0x02   0x01   0x04       0x00 0x00                 0x 0x03
//          receive ACK cmd      error code
//ACK : 0x02   0x81   0x04       0x31 0x31                 0x 0x03
//                   system status and controller status
/**************************************************/
void RfidControler::receivemss()
{
    QString tmpPin;
    QString tmpG9;

    if(gMessageboxShow)
    {
        m_pTcpSocket->readAll();
        return;
    }

    if(vari_1 == "slave")
    {
        revBuf = m_pTcpSocket->readAll();
        DTdebug()<<"*****Rfid slave reveive data *****:"<<revBuf.toHex();
        if(revBuf.size() >= 31)
        {
            if( ((unsigned char)revBuf[0] == 0x02)&&((unsigned char)revBuf[30] == 0x03) )
            {
                if((unsigned char)revBuf[3] == 0x31)//pin code
                {
                    tempPin = QString(revBuf.mid(4,14));
                    tempG9  = QString(revBuf.mid(18,4));

                    //南京需要先校验再判断来车
                    if((!MainWindow::pinCodeRightVerify(tempPin.toLocal8Bit(),8)) &&
                           (! MainWindow::pinCodeVerify(tempPin.toLocal8Bit(),14)))//PIN code verify
                    {
                        DTdebug()<<" RFID pinCodeRightVerify 8 byte and pinCodeVerify 14 byte is false";
                        return;
                    }

                    emit sendPinToUi(tempPin,false,tempG9);
                    sendSystemStaus(0x01,false,false);
                }
                else
                {
                    sendSystemStaus(0x81,systemRunStatusFlag,controllerStatusFlag);//alive
                }
            }
        }
        revBuf.clear();
    }
    else //master and other
    {
        QString hex = QString(m_pTcpSocket->readAll());
        QByteArray ret;
        QString &org = hex;
        DTdebug()<<"*****Rfid master and other receive data:"<<hex;
        if(Factory == "Benz" || Factory == "Haima")
        {
            if(!isLogin||!plusFlag)
            {
                return;
            }
            tempPin = hex;
            tempG9 = "";
        }
        else if(Factory == "BAIC")
        {
            if(!isLogin)
                return;
            //获取的VIN码
            //tempPin = hex.mid(2,17);
            if(hex.size()<17)//长度小于17位不理睬
                return ;
            tempPin = hex.mid(0,17);
            tempG9 = "";
        }
        else
        {
            hex = hex.trimmed();
            int n =2;
            int size= org.size();
            int space= qRound(size*1.0/n+0.5)-1;
            if(space<=0)
                return ;
            for(int i=0,pos=n;i<space;++i,pos+=(n+1))
            {
                org.insert(pos,' ');
            }
            QStringList sl=hex.split(" ");
            foreach(QString s,sl)
            {
                if(!s.isEmpty())
                    ret.append((char)s.toInt(0,16)&0xFF);
            }
            QString pin = QString(ret);
            tmpPin = pin.mid(2,14);
            tmpG9  = pin.mid(16,4);
            DTdebug() << "************RFID***********"<<tmpPin<< tmpG9;
            if(tmpPin== "" || tmpG9 == "")
            {
                DTdebug() << "What's wrong ?" << hex;
                return;
            }
            //南京需要先校验再判断来车
            if((!MainWindow::pinCodeRightVerify(tmpPin.toLocal8Bit(),8)) &&
                   (!MainWindow::pinCodeVerify(tmpPin.toLocal8Bit(),14)))//PIN code verify
            {
                DTdebug()<<" RFID pinCodeRightVerify 8 byte and pinCodeVerify 14 byte is false";
                return;
            }

            //tempPin = tmpPin;
            //tempG9  = tmpG9;
        }
        if(vari_1 == "master")
        {
            tempPin = tmpPin;
            tempG9  = tmpG9;

            isEqual = pinCodeRepeatHandle(tempPin);
            if(isEqual)
            {
                DTdebug()  << "PIN code invalid repetition" << tempPin << tempG9;
            }
            else
            {
                if( (!systemRunStatusFlag)&&(controllerStatusFlag) ) //master system not ING and controller ready
                {
                    DTdebug() << "system not ING:" << tempPin << tempG9;
                    emit sendPinToUi(tempPin,isEqual,tempG9);
                }
                else
                {
                    DTdebug() << "***** master ING or controll no ready******" << tempPin << tempG9;
                    emit sendPinToMasterTcp(tempPin,isEqual,tempG9);
                }
            }
        }
        else
        {
            if(!controllerStatusFlag || workmode || !isRFID  || Factory == "Dongfeng" ||(systemRunStatusFlag && RFIDIngNotIn))
            {
                DTdebug()<<"************error*************"<< tmpPin << tmpG9 << controllerStatusFlag << workmode << isRFID << (systemRunStatusFlag && RFIDIngNotIn);
            }
            else
            {
                isEqual = pinCodeRepeatHandle(tmpPin);
                if(isEqual)
                {
                    DTdebug()  << "PIN code invalid repetition" << tmpPin << tmpG9;
                }
                else
                {
                    DTdebug() << "PIN code valid:"  << tmpPin << tmpG9;
                    if(!systemRunStatusFlag) //system not ING
                    {
                        tempPin = tmpPin;
                        tempG9  = tmpG9;
                        DTdebug() << "system not ING:" << tmpPin << tmpG9;
                        emit sendPinToUi(tmpPin,isEqual,tmpG9);
                    }
                    else
                    {
                        //是ING状态，切断当前使能，等待工人确认
                        DTdebug() << tmpPin << "ING" << tmpG9;
                        RFIDlock.lockForWrite();
                        if(rfidNextCom)
                        {
                            RFIDlock.unlock();
                            VIN_PIN_SQL_RFID_previous = VIN_PIN_SQL_RFID;
                            VIN_PIN_SQL_RFID = tmpPin+tmpG9;
                            DTdebug()<<"next Car 2"<<VIN_PIN_SQL_RFID;
                        }
                        else
                        {
                            tempPin = tmpPin;
                            tempG9  = tmpG9;
                            DTdebug() << unCacheRfidFlag ;
                            if(unCacheRfidFlag)
                            {
                                rfidNextCom = false ;
                                DTdebug() << "false" ;
                            }
                            else
                            {
                                rfidNextCom = true;
                                DTdebug() << "ture" ;
                            }

                            RFIDlock.unlock();
                            VIN_PIN_SQL_RFID = tmpPin+tmpG9;
//                            VIN_PIN_SQL_RFID = "" ;
                            DTdebug()<<"next Car 1"<<VIN_PIN_SQL_RFID;
                            DTdebug()<<"emit sendGetCar"<<VIN_PIN_SQL_RFID;
                            emit sendGetCar();
                        }
                    }
                }
            }
        }
    }
}

void RfidControler::mysqlopen()
{
    if(QSqlDatabase::contains("rfidmysqlconnection")){
        db2 = QSqlDatabase::database("rfidmysqlconnection");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","rfidmysqlconnection");
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
            DTdebug()<< "rfid localmysql "<< db2.lastError().text();
        }else
        {
            DTdebug()<< "rfid localmysql ok2 ";
        }
    }else
    {
        DTdebug()<< "rfid localmysql ok1 ";
    }
}
/**************************************************/
//pin code repeat handle funcation
/**************************************************/
bool RfidControler::pinCodeRepeatHandle(QString code)
{
    bool isEqual = false;

    for (int i = 0; i < 10; ++i)
    {
        if(code == seriallist[i])
        {
            isEqual = true;
            return isEqual;
        }

//        DTdebug() << seriallist[i];
    }

    if(!isEqual)
    {
        seriallist[whichindex++] = code;
        if(whichindex == 10)
        {
            whichindex = 0;
        }
    }
    if(isFirst)
    {
        isFirst = false;
        if(!db2.isOpen())
            mysqlopen();
        if(db2.isOpen())
        {
            if(!query2.exec("SELECT COUNT(*) FROM "+Localtable+" WHERE IDCode ='"+code+"'"))
                DTdebug()<<"rfid SELECT count(*) fail "<<query2.lastError();
            else
            {
                if(query2.next())
                {
                    if(query2.value(0).toInt()>0)
                    {
                        DTdebug()<<"Localmysql already exist PIN code"<<tempPin;
                        isEqual = true;
                    }
                }
            }
        }
        else
        {
            DTdebug() << "rfid mysql not open";
        }
        if(db2.isOpen())
            db2.close();
    }
    return isEqual;
}

void RfidControler::pinCodeRepeatRemove(QString code)
{
    for (int i = 0; i < 10; ++i)
    {
        if(code == seriallist[i])
        {
            seriallist[i].clear();
        }
    }
}
/**************************************************/
//receive controller status
// =true;controller ready
// =false;controller no ready
/**************************************************/
void RfidControler::revControllerStatus(bool ctlStatusFlag)
{
    controllerStatusFlag = ctlStatusFlag;
}
/**************************************************/
//clear pin code buf
/**************************************************/
void RfidControler::clearCodeBuf()
{
    for (int i = 0; i < 10; i++)
    {
        seriallist[i] = "";
    }
    whichindex = 0;
    DTdebug()<<"clear RFID pin code buf";
}


void RfidControler::slotDeletArray()
{
//    DTdebug() << whichindex ;

//    DTdebug() << seriallist[whichindex-1] ;

    seriallist[--whichindex] = "" ;
}

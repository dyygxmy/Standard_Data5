#include "tightenthread.h"

TightenThread::TightenThread(QObject *parent) :
    QObject(parent)
{
    cycleId =1;
    sendCysleID = 0;
    config351count = 0;
    port4700LinkFlag = false;
    port4700DisconnectFlag = false;
    cs351InitFlag = false;
    timerCount = 0;
    timerCount1 = 0;
    sendEnableFlag = false;
    enableFlag = false;

    //    resetTemp="<MSL_MSG><PNR>21</PNR><RST/><SYN><DAT>2014-12-23</DAT><TIM>15:20:00</TIM></SYN><SDR><SER>1</SER></SDR><PDC><CIO>5</CIO><CNO>5</CNO></PDC><PID><PRT>T</PRT><PI1/></PID><TOL><VAL>0</VAL></TOL></MSL_MSG></root:ROOT>";
    //    resetTemp_timewrong="<MSL_MSG><PNR>21</PNR><RST/><SDR><SER>1</SER></SDR><PDC><CIO>5</CIO><CNO>5</CNO></PDC><PID><PRT>T</PRT><PI1/></PID><TOL><VAL>0</VAL></TOL></MSL_MSG></root:ROOT>";
    //    enableTemp         = "<MSL_MSG><PNR>21</PNR><SDR><SER>1</SER></SDR><PDC><CIO>5</CIO><CNO>5</CNO></PDC><PID><PRT>T</PRT><PI1>VIN12345678901234</PI1></PID><PRS><PRG>1</PRG> <SIO>2</SIO> <MNO>2</MNO><NUT>0</NUT><KNR>1</KNR> </PRS><TOL><VAL>1</VAL><KNR>1</KNR> </TOL><STR><VAL>1</VAL><KNR>1</KNR> </STR></MSL_MSG></root:ROOT>";

    xmlhead            = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root:ROOT xmlns:root=\"http://pdps.in.audi.vwg/legacy_schema/20.7.2/root\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://pdps.in.audi.vwg/legacy_schema/20.7.2/root root.xsd\">";
    aliveTemp          = "<MSL_MSG><PNR>21</PNR><SDR><SER>1</SER></SDR></MSL_MSG></root:ROOT>";
    enableTemp         = "<MSL_MSG><PNR>21</PNR><SDR><SER>1</SER></SDR><PDC><CIO>5</CIO><CNO>5</CNO></PDC><PID><PRT>T</PRT><PI1>VIN12345678901234</PI1></PID><PRS><PRG>1</PRG> <SIO>2</SIO> <MNO>2</MNO> <NUT>0</NUT><KNR>1</KNR> </PRS><TOL><VAL>1</VAL><KNR>1</KNR> </TOL></MSL_MSG></root:ROOT>";   //<SIO>2</SIO>需要拧紧几个 <MNO>2</MNO>最多NOK次数
    socketSpannerTemp  = "<MSL_MSG><NUT><NID>%1</NID></NUT></MSL_MSG></root:ROOT>";
    strInitializeXML   = "<MSL_MSG><PNR>21</PNR><SYN><DAT>date</DAT><TIM>time</TIM></SYN><SDR><SMG>1</SMG><SER>1</SER><SPC>1</SPC><SAL>1</SAL><SDS>1</SDS><SNU>1</SNU></SDR></MSL_MSG></root:ROOT>" ;
    resetTemp          = "<MSL_MSG><PNR>21</PNR><SYN><DAT>2014-12-23</DAT><TIM>15:20:00</TIM></SYN><SDR><SER>1</SER></SDR><PDC><CIO>5</CIO><CNO>5</CNO></PDC><PID><PRT>T</PRT><PI1/></PID><TOL><VAL>0</VAL><KNR>1</KNR> </TOL></MSL_MSG></root:ROOT>";
        //  resetTemp          = "<MSL_MSG><PNR>21</PNR><EVT><DAT>2020-10-28</DAT><TIM>15:12:11.206</TIM><STS><ONC><TOL><VAL>0</VAL></TOL><RDY>1</RDY><KNR>1</KNR></ONC></STS></EVT></MSL_MSG></root:ROOT>" ;
  resetTemp_timewrong= "<MSL_MSG><PNR>21</PNR><SDR><SER>1</SER></SDR><PDC><CIO>5</CIO><CNO>5</CNO></PDC><PID><PRT>T</PRT><PI1/></PID><TOL><VAL>0</VAL><KNR>1</KNR> </TOL></MSL_MSG></root:ROOT>";
  //  resetTemp_timewrong= "<MSL_MSG><PNR>21</PNR><EVT><DAT>2020-10-28</DAT><TIM>15:12:11.206</TIM><STS><ONC><TOL><VAL>0</VAL></TOL><RDY>1</RDY><KNR>1</KNR></ONC></STS></EVT></MSL_MSG></root:ROOT>" ;

    if(channelNum != "" ||channelNum != "1")
    {
        if(channelNum.size() == 1)
        {
            enableTemp.replace(192,1,channelNum).replace(228,1,channelNum);
            resetTemp.replace(182,1,channelNum);
            resetTemp_timewrong.replace(131,1,channelNum);
        }
        else if(channelNum.size() == 2)
        {
            enableTemp.replace(192,8,channelNum+"</KNR>").replace(228,8,channelNum+"</KNR>");
            resetTemp.replace(182,8,channelNum+"</KNR>");
            resetTemp_timewrong.replace(131,8,channelNum+"</KNR>");
        }
    }


    this->moveToThread(&m_thread);
    m_thread.start();

    QTimer::singleShot(60000,this,SLOT(slot_SYNTime()));
}

void TightenThread::slot_SYNTime()
{
    QDateTime tmpCurrentDataTime = QDateTime::currentDateTime();
    if((tmpCurrentDataTime.time().hour() == 4)&&(tmpCurrentDataTime.time().minute() == 30))
    {
        sendCmdToCs351(6);
    }
    QTimer::singleShot(60000,this,SLOT(slot_SYNTime()));
}

//线程开始
void TightenThread::tightenStart()
{
    FUNC( ) ;
    DTdebug() <<"Tighten thread start!!";
    ctlType = ControlType_1;
    tcpSocket =new QTcpSocket;
#if 0
    QString strInitializeXMLTemp = xmlhead + strInitializeXML ;
    strInitializeXMLTemp.replace("date",QDateTime::currentDateTime().toString("yyyy-MM-dd")) ;
    strInitializeXMLTemp.replace("time",QDateTime::currentDateTime().toString("hh:mm:ss")) ;

    DTdebug() << strInitializeXMLTemp ;
    tcpSocket->write(strInitializeXMLTemp.toLatin1());
#endif


    connect(tcpSocket, SIGNAL(readyRead()),this, SLOT(recsocket()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(receivedisconnect()));
    timer351 = new QTimer(this);
    connect(timer351,SIGNAL(timeout()),this,SLOT( newConnect()));
    timer351->start(250);

}
/******************************************************/
//************通道4700连接351   1000ms timer
/******************************************************/
void TightenThread::newConnect()
{
    timerCount++;
    timerCount1++;
    if(sendEnableFlag)
    {
        if(!enableFlag)
        {
            sendCmdMessage(1);//send reset
        }
        else
        {
            sendCmdMessage(2);//send enable
        }
        DTdebug()<<enableFlag;
        sendEnableFlag = false;
        timerCount = 0;
    }
    else
    {
        if((ctlType == "CS351")||(ctlType == "Desoutter"))
        {
            if(timerCount>=20)
            {
                timerCount = 0;
                DTdebug() << "heat 5000ms 4700 port" <<  cs351InitFlag  << port4700LinkFlag << ISmaintenance;
                if(!port4700LinkFlag)
                {
                    tcpSocket->abort(); //取消已有的连接
                    tcpSocket->connectToHost(ControllerIp_1,4700);
                    if(tcpSocket->waitForConnected(2000))
                    {
                        port4700LinkFlag = true;
                        DTdebug() << "4700 port connect success";
                    }
                    else
                    {
                        emit IsTigntenReady(false);  //发信号断开 mainwindo
                        //DTdebug() << "4700 port connect fail";
                        DTdebug() << "4700 port connect fail"<< tcpSocket->errorString();
                    }
                }
                else  //send alive
                {
                    if(cs351InitFlag)
                    {
                        config351count++;
                        if(config351count < 5)
                        {
#if 1

                            sendCmdToCs351(3);//send alive
#else
                            Cs351Heartbeat();
#endif
                        }
                        else
                        {
                            DTdebug() << "4700 heart time out";
                            emit disconnectTellPortB();
                            config351count = 0;
                            port4700LinkFlag = false;
                            port4700DisconnectFlag = true;
                            emit IsTigntenReady(false);  //发信号断开 mainwindow
                            cs351InitFlag  = false;
                        }
                    }
                }
            }
        }
        else
        {
            if(timerCount>=20)//5000ms
            {
                timerCount = 0;
                DTdebug() << "heat 5000ms 4700 port" <<  cs351InitFlag  << port4700LinkFlag << ISmaintenance;
                if(!port4700LinkFlag)
                {
                    tcpSocket->abort(); //取消已有的连接
                    tcpSocket->connectToHost(ControllerIp_1,4700);
                    if(tcpSocket->waitForConnected(2000))
                    {
                        port4700LinkFlag = true;
                        DTdebug() << "4700 port connect success";
                    }
                    else
                    {
                        emit IsTigntenReady(false);  //发信号断开 mainwindo
                        DTdebug() << "4700 port connect fail";
                    }
                }
            }
            if(timerCount1>=4)//1000ms
            {
                timerCount1 = 0;
                if(port4700LinkFlag)  //send alive
                {
                    if(cs351InitFlag)
                    {
                        config351count++;
                        if(config351count < 12)//12000ms
                        {
#if 1
                            sendCmdToCs351(3);//send alive
#else
                            Cs351Heartbeat();
#endif
                        }
                        else
                        {
                            DTdebug() << "4700 heart time out";
                            emit disconnectTellPortB();
                            config351count = 0;
                            port4700LinkFlag = false;
                            port4700DisconnectFlag = true;
                            emit IsTigntenReady(false);  //发信号断开 mainwindow
                            cs351InitFlag  = false;
                        }
                    }
                }
            }
        }
    }
}
/******************************************************/
//port B disconnect tell port A
/******************************************************/
void TightenThread::portBSendPortA()
{
    tcpSocket->abort();
    port4700LinkFlag = false;
    emit IsTigntenReady(false); //mainwindow display cs351 no ready
    config351count = 0;
    cs351InitFlag  = false;
}
//************通道4710连接上351他通知
void TightenThread::receivehavedconnect(bool flag)
{
    if(!RDYY) //cs351 no ready
    {
        if(!flag)
        {
            DTdebug() << "gun not ready";
            emit IsTigntenReady(false); //mainwindow display cs351 no ready
        }
        else
        {
            if(tcpSocket->state() == QAbstractSocket::ConnectedState)
            {
                DTdebug() << "cs351 4700  %%%%%%%%%%%%%%% port init()" << cs351InitFlag;
//                if(!cs351InitFlag)
//                {
                    cs351InitFlag = true;//data power on
                    cycleId = 1;
                    sendCmdMessage(4);//send reset cmd

                    if(IOFlag)
                    {
                        //不发送额外报文
                    }
                    else {
                        sendCmdMessage(6);
                    }
//                }
            }
        }
    }
    else
    {
        if(SYSS == "ING" || SYSS == "OK" || SYSS == "NOK")
        {

        }
        else
        {
            SYSS = "Ready";
        }
        if(port4700DisconnectFlag) //断线重连
        {
            if(SYSS == "ING")
            {
                emit ReconnectSendOperate();
            }
            DTdebug() << "******cs351 reconnect*******";
        }
        if(!CsIsConnect)
        {
            emit IsTigntenReady(true);   //mainwindow display cs351 no ready
            port4700DisconnectFlag = false;
            DTdebug() << "******cs351 connect success**********";
        }
    }
}
/**************************输出错误信息**************************/
void TightenThread::displayError(QAbstractSocket::SocketError)
{
    DTdebug() << "df" << tcpSocket->errorString(); //输出错误信息
}
/**********************************************
*351主动断开 处理 or alive no ack over 3 times
**********************************************/
void TightenThread::receivedisconnect()
{
    DTdebug() << "here 4700 port disconnect";
    emit disconnectTellPortB();
    config351count = 0;
    port4700LinkFlag = false;
    port4700DisconnectFlag = true;
    emit IsTigntenReady(false);  //发信号断开 mainwindow
//    timer351->stop();
//    timer351->start(5000);
}

/******************************************************/
//***************mianwondow emit 发送使能
/******************************************************/
void TightenThread::sendReadOperate(bool enable,int n)
{
    Q_UNUSED(n);

    if(sendEnableFlag)
    {
        if(enableFlag != enable)
        {
            if(!enableFlag)
            {
                sendCmdMessage(1);//send reset
            }
            else
            {
                sendCmdMessage(2);//send enable
            }
            DTdebug()<<enableFlag;
        }
    }

    sendEnableFlag = true;
    enableFlag = enable;
}
/******************************************************
 * type = 1   发送 切断使能
 * type = 2   发送 使能
 * type = 3   发送 alive
 * type = 4   发送 复位
 * type = 5   发送套筒号
******************************************************/
void TightenThread::sendCmdMessage(int type)
{
    FUNC( ) ;
    DTdebug() << type ;
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        if(type == 1) //send reset
        {
            sendCmdToCs351(1);//send disable
            DTdebug() << "send operate disable" << carInfor[0].proNo[groupNumCh[0]] << carInfor[0].boltNum[groupNumCh[0]];
        }
        else if(type == 2)//send enable
        {
            sendCmdToCs351(2);
            DTdebug() << "send operate" << carInfor[0].proNo[groupNumCh[0]] << carInfor[0].boltNum[groupNumCh[0]];
        }
        else if(type == 3) //send alive
        {
            sendCmdToCs351(3);
        }
        else if( type == 5)
        {
            sendCmdToCs351(5);
        }
        else if(type == 6)
        {
            sendCmdToCs351(6);
        }
        else//send disable
        {
             sendCmdToCs351(4);//send reset
        }
    }
}
/*****************************************************
 * 4700 port send cmd to cs351
 * cmd = 1 send disable;
 * cmd = 2 send enable;
 * cmd = 3 send alive;
 * cmd = 4 send reset;
 * cmd = 5 send socketSpannerNum
*****************************************************/

bool gSendAliveFlag = false;
void TightenThread::sendCmdToCs351(int type)
{
    FUNC() ;
    DTdebug( ) << "sendCmdToCs351" <<type << IOFlag  << CsIsConnect;
    QString xmlCmdHead = "55AA0001000000000002000000000000";
    QString Data_Xml_T = "";
    QString strProNo = carInfor[0].proNo[groupNumCh[0]];     //程序号
    QString strBoltNum = QString::number(carInfor[0].boltNum[groupNumCh[0]]);

    if(type == 1) // send disable
    {
        Data_Xml_T = xmlhead+resetTemp_timewrong;
        xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
        Data_Xml_T = xmlCmdHead + Data_Xml_T;
        tcpSocket->write(Data_Xml_T.toLatin1());
        tcpSocket->waitForBytesWritten(300);
        DTdebug() << "4700 send disable:" <<Data_Xml_T.mid(0,32);
    }
    else if(type == 4) // send reset
    {
        QDateTime cur = QDateTime::currentDateTime();
        QString current_date = cur.toString("yyyy-MM-dd");
        QString current_time = cur.toString("hh:mm:ss");
        if(cur.date().year()>2015)
        {
            DTdebug()<<"send reset cmd";
            Data_Xml_T = xmlhead+resetTemp;
            xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
            Data_Xml_T.replace(278,current_date.size(),current_date).replace(299,current_time.size(),current_time);
        }
        else
        {
            DTdebug()<<"send reset cmd_timewrong";
            Data_Xml_T = xmlhead+resetTemp_timewrong;
            xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
        }
        Data_Xml_T = xmlCmdHead + Data_Xml_T;
        tcpSocket->write(Data_Xml_T.toLatin1());
        tcpSocket->waitForBytesWritten(300);
        DTdebug() << "4700 send reset:" <<Data_Xml_T.mid(0,32);
    }
    else if(type == 2)//send enable
    {
        Data_Xml_T = xmlhead+enableTemp;
        //VIN_PIN_SQL = QString("LSVABC5LXAB123455");
        //VIN_PIN_SQL = QString("***78202073600472");
        if(VIN_PIN_SQL.size()>=17)
        {
            Data_Xml_T.replace(313+35,17,VIN_PIN_SQL);
        }
        else if(isReplaceBarcode)
        {
            Data_Xml_T.replace(313+35,17,"**"+VIN_PIN_SQL);
        }
        else
        {
            Data_Xml_T.replace(313+35,17,"***"+VIN_PIN_SQL);
        }
        int len1 = 0;
        if(strProNo.length() == 1)
        {
            len1 = 7;
        }
        else
        {
            len1 = 8;
        }
        Data_Xml_T.replace(352+35,len1,strProNo+"</PRG>");
        QString SIOtemp,MNOtemp;
        if(strBoltNum.toInt()>9)
            SIOtemp = strBoltNum+"</SIO>";
        else
            SIOtemp = strBoltNum+"</SIO> ";
        Data_Xml_T.replace(365+35,8,SIOtemp);//7 or 8 replace 8

        if(strBoltNum.toInt()>1)
            MNOtemp = "20</MNO>";
        else
            MNOtemp = "2</MNO> ";
        Data_Xml_T.replace(365+35+13,8,MNOtemp); //马头的同一个程序发送多颗，最多不合格次数设为20

        xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
        DTdebug()<<"send enable >> CsIsConnect:" ;
        if(CsIsConnect)
        {
            sendCysleID = cycleId;
            Data_Xml_T = xmlCmdHead + Data_Xml_T;
            tcpSocket->write(Data_Xml_T.toLatin1());
            tcpSocket->waitForBytesWritten(300);
            DTdebug() << "4700 send enable:" <<Data_Xml_T/*.mid(0,32)*/;
        }
    }

    //该功能仅xml协议的套筒选择功能使用 不使用&&判断，为了避免IOFlag条件下，执行出错
    else if(type == 5)
    {
        if(!IOFlag)
        {
            int socketSpannerNum = carInfor[preChannel].ttNum[groupNumCh[preChannel]];
            socketSpannerTemp.replace("%1",QString(char(socketSpannerNum+64))) ;

            Data_Xml_T = xmlhead+socketSpannerTemp;
            xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
            Data_Xml_T = xmlCmdHead + Data_Xml_T;
            tcpSocket->write(Data_Xml_T.toLatin1());
            DTdebug() << "4710 send enable tpye==5" << Data_Xml_T;
            socketSpannerTemp  = "<MSL_MSG><NUT><NID>%1</NID></NUT></MSL_MSG></root:ROOT>";
        }
    }

    //初始化信息
    else if(type == 6)
    {

#if 0
        Data_Xml_T = xmlhead + strInitializeXML ;
        xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
        Data_Xml_T = xmlCmdHead + Data_Xml_T ;
#if 0
        strInitializeXMLTemp.replace("date",QDateTime::currentDateTime().toString("yyyy-MM-dd")) ;
        strInitializeXMLTemp.replace("time",QDateTime::currentDateTime().toString("hh:mm:ss")) ;
#endif
        DTdebug() << Data_Xml_T;
        tcpSocket->write(Data_Xml_T.toLatin1());
#else
        QDateTime cur = QDateTime::currentDateTime();
        QString current_date = cur.toString("yyyy-MM-dd");
        QString current_time = cur.toString("hh:mm:ss");
        if(cur.date().year()>2015)
        {
            DTdebug()<<"send reset cmd";
            Data_Xml_T = xmlhead+strInitializeXML;
            Data_Xml_T.replace("date",/*current_date.size(),*/current_date).replace("time",/*current_time.size(),*/current_time);
            xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
            DTdebug() << Data_Xml_T ;
        }
        else
        {
            DTdebug()<<"send reset cmd_timewrong";
            Data_Xml_T = xmlhead+resetTemp_timewrong;
            xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
        }


        Data_Xml_T = xmlCmdHead + Data_Xml_T;
        tcpSocket->write(Data_Xml_T.toLatin1());
        tcpSocket->waitForBytesWritten(300) ;
        DTdebug() << "4700 send reset:" <<Data_Xml_T;
#endif
    }

    else //send alive
    {
        if(!gSendAliveFlag)
        {
            gSendAliveFlag = true;
            Data_Xml_T = xmlhead+aliveTemp;
            xmlCmdHead = getHeaderFunc(cycleId,Data_Xml_T.size(),type);
            Data_Xml_T = xmlCmdHead + Data_Xml_T;
            tcpSocket->write(Data_Xml_T.toLatin1());
            tcpSocket->waitForBytesWritten(300);
            DTdebug() << "4700 send alive:" <<Data_Xml_T.mid(0,32);
        }
        else
        {
            DTdebug() << "4700 no send alive";
        }
    }

    cycleId ++;
    if(cycleId == 10000)
    {
        cycleId = 1;
    }
}


/***************************************************
 * get 32 data head
 * alive  Typ = 0003 ; other Typ = 0001
 ***************************************************/
QString TightenThread::getHeaderFunc(int mid,int size,int type)
{
//    QByteArray xmlCmdHead = "55AA 0001 00000000 0002 0000 00000000";
    QString xmlCmdHead = "55AA";                        //55AA
    QString strTemp1 = "0000";
    QString strTemp2 ="";
    strTemp2 = QString::number(mid);
    strTemp1.replace(4-strTemp2.size(),strTemp2.size(),strTemp2);
    xmlCmdHead = xmlCmdHead + strTemp1;                 //

    strTemp1 = "00000000";
    strTemp2 = QString::number(size);
    strTemp1.replace(8-strTemp2.size(),strTemp2.size(),strTemp2);
    xmlCmdHead = xmlCmdHead + strTemp1;

    if(type == 3)//alive
    {
        xmlCmdHead = xmlCmdHead + "0003";
    }
    else
    {
        xmlCmdHead = xmlCmdHead + "0001";
    }

    xmlCmdHead = xmlCmdHead + "000000000000";
    return xmlCmdHead;
}

/******************************************************
 *4710 port receive cs351 data
******************************************************/
void TightenThread::recsocket()
{
    config351count = 0;
    QByteArray a = tcpSocket->readAll();
    DTdebug() << "4700 port receive head" << a.mid(0,32);
    gSendAliveFlag = false;
    if( a.mid(20,4).toInt() != 0)
    {
        DTdebug() << "4700 port error head" << a;
        if(a.mid(20,4).toInt() == 1)
        {
            cycleId = 1;
        }
        else
        {
        }
    }
}


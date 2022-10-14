#include "mainwindow.h"
#include "rootdialog.h"
#include "worker.h"
#include "worker_356.h"
#include "GlobalVarible.h"
#include "serialComs.h"
#include "./TestInput_Cursor/inputmethod.h"
#include "fisupdate.h"
#include "fisupdatequeue.h"
#include "communication.h"
#include "tightenthread.h"
#include "tighten_op_net.h"
#include "tighten_op_cleco.h"
#include "tighten_op_atlas_PM.h"
#include "tighten_op_atlas_PF.h"
#include "tighten_op_atlas_PFC.h"
#include "tighten_op_atlas_PMM.h"
#include "tighten_op_atlas_IRMM.h"
#include "tighten_op_net_ch1.h"
#include "tighten_op_net_ch2.h"
#include "tighten_op_rs232.h"
#include "gwkquantecmcs.h"
#include "gwkPLC.h"
#include "ftphandle.h"
#include "logdelete.h"
#include "xmlcreate.h"
#include "inputevents.h"
#include "wificonnect.h"
#include "reinsertserverdb.h"
#include "reinsertserverdbningbo.h"
#include "reinsertserverdbbyd.h"
#include "taotong.h"
#include "sqlthread.h"
#include "sqlthreadbyd.h"
#include "sqlthreadbenz.h"
#include "split.h"
#include "rfidcontroler.h"
#include "ftpjson.h"
#include "ftpjsonparse.h"
#include "ftpdataparse.h"
#include "connectplc.h"
#include "cardread.h"
#include "usermanagement.h"
#include "quickrequest.h"
#include "mesuploadhaima.h"
#include "logdelete.h"
#include "pluscommunication.h"
#include "sqlthreadsvw2.h"
#include "mesuploadsvw2.h"
#include "replacebarcode.h"
#include "tighten_json_atlas_pm600.h"
#include "queueinterface.h"
#include "sqlthreadbaic.h"
#include "mesuploadbaic.h"
#include "mastertcp.h"
#include "ftpclient.h"
#include "wirelesslocation.h"
#include "locationparsejson.h"
#include "locationserver.h"
#include "xmldata.h"
#include "atlas_OP.h"
#include "location.h"
#include "leuzerecv.h"
#include "testalive.h"
#include "queuethread.h"
#include "serial/serialthread.h"
#include "proto/matchfeaturecode.h"
#include "tightenProto/alatspm4000.h"
#include "sqlthreadgzbaic.h"
#include "getcarinformation.h"
#include "lightoperate.h"
#include "tighten_op_net_curve.h"
#include "mesuploadAQCHERY.h"
#include "iobox.h"

#include <QWSServer>

bool GAbnormalOut_KF = true;//开封项目异常出站
bool gRfidParseDone = true;
bool gMessageboxShow = false;

bool gNexoLinkOk = false;
bool isJS = false;               //是否是技术员
bool isNormal = false;           //状态是否正常
bool workmode = false;           //工作模式与否
bool CsIsConnect = false;        //CS351拧紧枪是否连接
bool RFIDIsConnect = false;      //RFID是否连接
bool PLCIsConnect = false;       //PLC connect
bool RDYY  = false;              //CS351是否准备
bool ISmaintenance = false;      //是否在点击配置页面中
bool ISRESET  = false;           //是否复位
bool ISWARNING = false;          //警告
bool Curve_Is_Null = false;      //曲线是否为空
bool TaoTongState = false;       //是否为套筒模式
bool WIFI_STATE = false;         //wifi状态
bool SerialGunMode = true;       //条码枪模式 wired barcode and wireless barcode
bool DebugMode = false;          //调试模式
bool battery = true;             //电池与否
bool bound_enabled = false;      //是否启用扭矩角度 控制
bool QueueIsNull = true;         //队列为空标识
bool rfidNextCom = false;        //rfid下辆车已进站
//bool isParse = true;
int Line_ID = 1;                 //工线
int Y1V  = 1;                    //扭矩存储
//int SerialMode = 0;
bool restart_enabled = false;
QString SYSS = "";               //data2运行状态  Ready  ING  OK NOK
QString SaveWhat="";             //保存参考变量
QString VIN_PIN_SQL= "";         //条码
QString VIN_PIN_SQL_of_SpecialStation= "";
QString VIN_PIN_SQL_RFID = "";   //RFID条码
QString VIN_PIN_SQL_RFID_previous = "";   //倒数第二辆
QString AutoNO = "";
QString Type = "";
QString Body_NO = "";
QString SCREWID_SQL= "";         //螺栓编号
//QByteArray time_info ="";
QString lsNum="";                //额定螺栓数量
QString SqlUserName = "";
QString SqlPassword ="";
QString WirelessIp ="";
QString LocalIp = "";
QString LocalIp2 = "";
QString ControllerIp_1 ="";
QString ControllerIp_2 ="";
int BoltOrder[2];
int QualifiedNum=0;              //合格数量
int TimeLeft = 0;                //进度条剩余时间
int enablenumberLeft = 0;        //剩余螺栓数量
//int BackTimeLeft = 0;            //归位剩余时间
int TAOTONG = -1;
int StationStatus= 0;
QString Operator = "";
QString info[7] ={};
QString status[D_BOLTNUM][5]={};
QString Version="";
QString Station="";
QString LineName ="";
QString Equip="";
QString variable1="";
QReadWriteLock lock;
QReadWriteLock RFIDlock;
QReadWriteLock deletelock;
QReadWriteLock WIFIlock;
QReadWriteLock Queuelock;
QQueue<QVector<QString> > queue;
QString ControlType_1= "";
QString ControlType_2= "";
QString factory = "";
QString tablePreview="";
QString Localtable="";
int BoltTotalNum = 0;   //螺栓总数
//bool NOKflag = false;          //是否有NOK
CAR_INFOR carInfor[4];
int preChannel=0;
int groupNumCh[4];
QString channelNum = "1";
bool isRFID = false;      //RFID模式
bool isBarCode = false;   //条码模式RS232
bool isQueue = false;     //队列模式
bool BAICQueue = false;   //北汽的队列模式
bool isServerTip = false;  //与服务器断开连接后，界面是否需要提示”网络异常”
bool Special_Station =false;  //MEB是否是特殊工位
bool MEBFisMatch = false;
bool NOK_Skip = false;
bool ReversalFlag = false;
bool isOutFlag = false;   //output
bool isReplaceBarcode = false;   //条码替换
bool isQRVerify = false ;
bool manualMode = false;   //手动模式
//QString main_variable1="";
QString RFID_IP = "";
int     RFID_Port = 0;
bool isRepair = false;
int locationStatus = 0;
bool upInversion = false;   //默认不上传反转数据
int socketSpannerStatus = 0;
bool IOFlag = false ;
bool unCacheRfidFlag=false;    //默认缓存
bool notDisqualificationSkip = false ;//南京仪表板机械手3次不合格是否切换到下一颗螺栓?0：切 1：不切 默认false,即切
QString userName = "";
QString userID = "";
int taotongNum = 0 ;
int saveTightenResultNum = 0 ;

QStringList   gFisVins;
QStringList   gFisPins;
int           gFisVinIndex;
QString       gFisVin;
QString       gFisPin;

#include "qextserial/qextserialport.h"

QextSerialPort  *gExtSerialPort = 0;
QSqlDatabase mysql_open(QString pConnName)
{
    QSqlDatabase     tmpDataBase;
    //return tmpDataBase;

    //if(!tmpDataBase.isValid())
    {
        if(QSqlDatabase::contains(pConnName))
        {
            DTdebug()<< pConnName << " Contain";
            tmpDataBase = QSqlDatabase::database(pConnName);
        }
        else
        {
            DTdebug()<< pConnName << " New";
            tmpDataBase=QSqlDatabase::addDatabase("QMYSQL",pConnName);
            tmpDataBase.setHostName("localhost");
            tmpDataBase.setDatabaseName("Tighten");
            tmpDataBase.setUserName("root");
            tmpDataBase.setPassword("123456");
        }
    }
    if(!tmpDataBase.isOpen())
    {
        if(!tmpDataBase.open())
        {
            DTdebug()<< pConnName << " Tighten open fail: "<< tmpDataBase.lastError().text();
        }else
        {
            DTdebug()<< pConnName << " Tighten open ok";
        }
    }
    else
    {
        DTdebug()<< pConnName << " has opened";
    }
    return tmpDataBase;
}

#define OP_ONLY_SEND_ENABLE
/******************日志打印级别函数****************************/
void outputMessage(QtMsgType type, const char *msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        abort();
    }
    if(text != QString("Warning:"))
    {
        QString message = QString("[%1] %2 %3").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz ddd")).arg(text).arg(msg);
        QDateTime time = QDateTime::currentDateTime();
        QString date = time.toString("yyyy-MM-dd_hh");

        QFile file(QString("/log/")+date+QString(".log"));
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << message << endl;
        file.flush();
        file.close();
    }
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv, QApplication::GuiClient);
//        QWSServer::setBackground(QColor(0,0,0,0));
//#ifdef ARM
    // arm use only
//    QWSServer::setCursorVisible(false);

    qInstallMsgHandler(outputMessage);                                  //重定向日志到文件

//#endif
//    QApplication::setOverrideCursor(Qt::BlankCursor);                   //去掉光标

    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(utf8);
    QTextCodec::setCodecForLocale(utf8);
    QTextCodec::setCodecForCStrings(utf8);

    DTdebug()<<"!!!!!!";
    qRegisterMetaType<QVariant>("QVariant");
    system("dos2unix /config.ini /config_bound.ini");
    /**
   *Data2程序main函数开始初始化一些参数变量，无线网卡ip
   **/

    gFisVins.clear();
    gFisPins.clear();
    gFisVinIndex = 0;

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);

    if(!QSettings("/config.ini", QSettings::IniFormat).contains("baseinfo/offVOLTAGE")){
        QSettings("/config.ini", QSettings::IniFormat).setValue("baseinfo/offVOLTAGE",3);
    }
    if(!QSettings("/config.ini", QSettings::IniFormat).contains("baseinfo/offTime")){
        QSettings("/config.ini", QSettings::IniFormat).setValue("baseinfo/offTime",30);
    }


    configIniRead->value("baseinfo/serialMode").toInt()?SerialGunMode=true:SerialGunMode=false;
    configIniRead->value("baseinfo/isRFID").toInt()?isRFID=true:isRFID=false;
    configIniRead->value("baseinfo/isBarCode").toInt()?isBarCode=true:isBarCode=false;
    Queuelock.lockForWrite();
    configIniRead->value("baseinfo/isQueue").toInt()?isQueue=true:isQueue=false;
    configIniRead->value("baseinfo/BAICQueue").toInt()?BAICQueue=true:BAICQueue=false;
    configIniRead->value("baseinfo/isServerTip").toInt()?isServerTip=true:isServerTip=false;
    configIniRead->value("baseinfo/Special_Station").toInt()?Special_Station=true:Special_Station=false;
    configIniRead->value("baseinfo/MEBFisMatch").toInt()?MEBFisMatch=true:MEBFisMatch=false;
    configIniRead->value("baseinfo/NOK_Skip").toInt()?NOK_Skip=true:NOK_Skip=false;
    unCacheRfidFlag = configIniRead->value("baseinfo/unCacheRfidFlag").toBool();
    saveTightenResultNum  = configIniRead->value("baseinfo/saveNum").toInt();
    if(saveTightenResultNum == 0)
    {
        saveTightenResultNum = 10000 ;
    }
    Queuelock.unlock();
    //    Operator = configIniRead->value("baseinfo/Opera"2018-11-28 17:03:28:686" function is start
    Station = configIniRead->value("baseinfo/StationId").toString();
    if(Station.right(1)=="L"||Station.right(1)=="R")
        Equip = Station.right(1);
    else
        Equip = "";
    Line_ID = configIniRead->value("baseinfo/Line_ID").toInt();
    LineName = configIniRead->value("baseinfo/LineName").toString();

    Y1V = configIniRead->value("baseinfo/Y1V").toInt();
    configIniRead->value("baseinfo/bound").toInt()?bound_enabled=true:bound_enabled=false;
    SqlUserName = configIniRead->value("baseinfo/SQLUserName").toString();
    SqlPassword = configIniRead->value("baseinfo/SQLPassword").toString();
    variable1 = configIniRead->value("baseinfo/APSSID").toString();  //variable1 value
    QString variable2 = configIniRead->value("baseinfo/APpsk").toString();   //variable2 value
    WirelessIp = configIniRead->value("baseinfo/WirelessIp").toString();
    LocalIp = configIniRead->value("baseinfo/LocalIp").toString();
    LocalIp2 = configIniRead->value("baseinfo/LocalIp2").toString();
    ControllerIp_1 = configIniRead->value("baseinfo/ControllerIp_1").toString();
    ControllerIp_2 = configIniRead->value("baseinfo/ControllerIp_2").toString();
    factory = configIniRead->value("baseinfo/factory").toString();
    channelNum = configIniRead->value("baseinfo/channelNO").toString();
    IOFlag  = configIniRead->value("baseinfo/IOFlag").toBool();
    notDisqualificationSkip = configIniRead->value("baseinfo/notDisqualificationSkip").toBool();
    QString StationName = configIniRead->value("baseinfo/StationName").toString();

    system("echo 1 > /root/gpio/COM0");

    QString dev_name = "/dev/ttyUSB1";

    gExtSerialPort = new QextSerialPort(dev_name);
    //设置波特率
    gExtSerialPort->setBaudRate(BAUD9600);
    //设置数据位
    gExtSerialPort->setDataBits(DATA_8);
    //设置校验
    gExtSerialPort->setParity(PAR_NONE);
    //设置停止位
    gExtSerialPort->setStopBits(STOP_1);
    //设置数据流控制
    gExtSerialPort->setFlowControl(FLOW_OFF);
    //设置延时
    gExtSerialPort->setTimeout(10);
    if(gExtSerialPort->open(QIODevice::ReadWrite)){
        DTdebug()<<QObject::tr("serial open ok ") << dev_name;
    }else{
        DTdebug()<<QObject::tr("serial open fail ") << dev_name;
    }

    if((factory!="AQCHERY")&&(factory!="NOVAT")&&(factory!="KFCHERY")&&(factory!="BYDSZ")&&(factory!="BYDXA") && (factory!="Ningbo") && (factory != "Benz") && (factory != "GM") && (factory != "Haima")&& (factory != "SVW2") && (factory != "Dongfeng") && (factory != "BAIC") && (factory != "GZBAIC"))
    {
        factory="SVW3";
    }
    ControlType_1 = configIniRead->value("baseinfo/ControlType_1").toString();
    ControlType_2 = configIniRead->value("baseinfo/ControlType_2").toString();
    if(ControlType_1=="")
    {
        ControlType_1="CS351";
        configIniRead->setValue("baseinfo/ControlType_1","CS351");
    }
    if(ControlType_2=="")
    {
        ControlType_2="OFF";
        configIniRead->setValue("baseinfo/ControlType_2","OFF");
    }
    restart_enabled = configIniRead->value("baseinfo/restart_enabled").toBool();
    if(factory=="Benz")
    {
        for(int i=1;i<21;i++)
        {
            if( (configIniRead->value("carinfo"+QString::number(i)+"/Out1").toBool())||(configIniRead->value("carinfo"+QString::number(i)+"/Out2").toBool())||
                    (configIniRead->value("carinfo"+QString::number(i)+"/Out3").toBool())||(configIniRead->value("carinfo"+QString::number(i)+"/Out4").toBool()) )
            {
                isOutFlag = true;
                break;
            }
        }
    }
    isReplaceBarcode = configIniRead->value("baseinfo/isReplaceBarcode").toBool();
    RFID_IP   = configIniRead->value("baseinfo/RfidIp").toString();
    RFID_Port = configIniRead->value("baseinfo/RfidPort").toInt();
    isRepair = configIniRead->value("baseinfo/isRepair").toBool();
    isQRVerify = configIniRead->value("baseinfo/isQRVerify").toBool();

    Version = "VI22.10.13.03";

    qDebug()<<"factory:" <<factory << "ControlType_1:" << ControlType_1 <<"ControlType_2:"<<ControlType_2<< "Version:"<<Version<<"Station:"<<Station<<"LineName:"<<LineName;

    //20190725
//    int connectNet = configIniRead->value("baseinfo/connectNet").toInt();
//    if(connectNet == 0)
//        system((QString("ifconfig wlan0 ")+configIniRead->value("baseinfo/WirelessIp").toString()+" netmask "+configIniRead->value("baseinfo/netmask").toString()+" &").toLatin1().data());
//    else
//        system((QString("ifconfig wlan0 ")+configIniRead->value("baseinfo/WirelessIp").toString()+" netmask 255.255.255.0 &").toLatin1().data());
//    if(connectNet == 1)
//        system((QString("ifconfig eth0 ")+configIniRead->value("baseinfo/LocalIp").toString()+" netmask "+configIniRead->value("baseinfo/netmask").toString()+"  &").toLatin1().data());
//    else
//        system((QString("ifconfig eth0 ")+configIniRead->value("baseinfo/LocalIp").toString()+" netmask 255.255.255.0 &").toLatin1().data());
//    if(connectNet == 2 )
//        system((QString("ifconfig eth1 ")+configIniRead->value("baseinfo/LocalIp2").toString()+" netmask "+configIniRead->value("baseinfo/netmask").toString()+"  &").toLatin1().data());
//    else
//        system((QString("ifconfig eth1 ")+configIniRead->value("baseinfo/LocalIp2").toString()+" netmask 255.255.255.0 &").toLatin1().data());


    QString freetds = "/usr/local/arm/freetds/etc/freetds.conf";
    QFile file1(freetds);
    if(!file1.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        qDebug()   << "Cannot open testdsn file1 for Reading";
    }
    else
    {
        QString str1 (file1.readAll());
        if(str1.contains("[testdsn1]", Qt::CaseInsensitive)&&str1.contains("[testdsn2]", Qt::CaseInsensitive))
        {
            str1.replace(QRegExp("\\[testdsn1\\]\\s*host = \\S*"),QString("[testdsn1]\n\thost = ")+configIniRead->value("baseinfo/CurveServerIp").toString());
            str1.replace(QRegExp("\\[testdsn2\\]\\s*host = \\S*"),QString("[testdsn2]\n\thost = ")+configIniRead->value("baseinfo/DataServerIp").toString());
            str1.replace(QRegExp("\\[testdsn3\\]\\s*host = \\S*"),QString("[testdsn3]\n\thost = ")+configIniRead->value("baseinfo/AndonServerIp").toString());
        }
        file1.close();
        if(!file1.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            qDebug()   << "Cannot open testdsn file1 for Writing";
        }
        else
        {
            file1.write(str1.toUtf8());
            file1.close();
        }
    }

    QString wpa_conf = "/etc/wpa_supplicant.conf";
    QFile file2(wpa_conf);
    if(!file2.open(QIODevice::ReadOnly| QIODevice::Text)){
        qDebug()   << "Cannot open wifi file2 for Reading";
    }
    else
    {
        QString str2 (file2.readAll());
        if(str2.contains("ssid=", Qt::CaseInsensitive)&&str2.contains("psk=", Qt::CaseInsensitive)){
            str2.replace(QRegExp("\nssid=\\S*"),QString("\nssid=\"")+configIniRead->value("baseinfo/SSID").toString()+QString("\""));
            str2.replace(QRegExp("psk=\\S*"),QString("psk=\"")+configIniRead->value("baseinfo/psk").toString()+QString("\""));
        }
        file2.close();
        if(!file2.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            qDebug()   << "Cannot open wifi file2 for Writing";
        }
        else
        {
            file2.write(str2.toUtf8());
            file2.close();
        }
    }

    QString Master_slave = configIniRead->value("baseinfo/Master_slave").toString();
    QString slave = configIniRead->value("baseinfo/slave1").toString();

    delete configIniRead;
    //    time_info.resize(7);
    QString main_variable1 = variable1;
    QString k_power;                            //select power mode
    QFile power("/var/power");
    power.open(QIODevice::ReadOnly);
    QTextStream in_power(&power);
    in_power >> k_power;
    if(k_power=="11")
        battery=false;
    power.close();

    system("mv -f /curveFile/* /curveFileDel/"); //move error curve

    if(factory=="SVW3" || factory=="Ningbo")     //select local mysql
    {
        tablePreview="FisPreview";
        Localtable="TighteningDatas";
    }
    else if(factory=="BYDSZ"  || factory=="BYDXA" || factory =="GM")
    {
        tablePreview="TaskPreview";
        Localtable="BYDTighteningDatas";
    }
    else if(factory=="Benz")
    {
        tablePreview="BenzTaskPreview";
        Localtable="BenzTighteningDatas";
    }
    else if(factory=="Haima")
    {
        tablePreview="HMFisPreview";    //海马的条码是实时获取的，数据库算暂存
        if(isRepair)
            Localtable = "HaimaRepairTighteningDatas";
        else
            Localtable = "HaimaTighteningDatas";
    }
    else if(factory =="SVW2")
    {
        tablePreview="SVW2FisPreview";
        Localtable="SVW2TighteningDatas";
    }
    else if(factory =="Dongfeng")
    {
        tablePreview="";
        Localtable="DFTighteningDatas";
    }
    else if(factory =="BAIC")
    {
        tablePreview="BAICFis";
        Localtable="BAICTighteningDatas";
    }
    else if(factory == "GZBAIC")
    {
        tablePreview="";
        Localtable="GZBAICTighteningDatas";
    }
    else if(factory == "NOVAT")
    {
        tablePreview="";
        Localtable="NOVATTighteningDatas";
    }
    else if(factory == "AQCHERY")
    {
        tablePreview="AQCHERYFisPreview";
        Localtable="AQCHERYTighteningDatas";
    }
    else if(factory == "KFCHERY")
    {
        tablePreview="KFCHERYFisPreview";
        Localtable="KFCHERYTighteningDatas";
    }

    /*
      * MainWindow:主界面UI
      * 用来显示状态 不可有阻塞操作
    */
    Newconfiginfo *newconfiginfo = new Newconfiginfo;
    MainWindow *w = new MainWindow ;
    if(main_variable1 == "T")
    {
        if(factory != "BAIC" && factory != "Haima"/*&& factory != "GZBAIC"*/)
        {
            w->JobOffShow();
        }
    }

    RootDialog m ;
    m.show();
    m.init(w,newconfiginfo);

    QObject::connect(newconfiginfo,SIGNAL(closeconfig()),w,SLOT(initBack()),Qt::AutoConnection);
    /*
      * Wificonnect:wifi网络状态判断
      * 包括服务器 rfid连接状态5秒检测一下
    */

    LocationParseJson * locationParseJson = new LocationParseJson;   //读定位配置文件
    QVariantMap currentLocation = locationParseJson->parseJson();
    delete locationParseJson;

    locationStatus = currentLocation.value("locationStatus").toInt();  
    if(locationStatus == 2)  //主机
    {
#if 0
        WirelessLocation *wirelessLocation = new WirelessLocation;  //连接基站，接收数据
        QTimer::singleShot(0,wirelessLocation,SLOT(initLocation()));
#endif
        LocationServer * locationServer = new LocationServer;  //每个Data都要开服务端接收标签位置信息和电量  主机用127.0.0.1
        QTimer::singleShot(0,locationServer,SLOT(init()));
#if 0
        QObject::connect(locationServer,SIGNAL(sendTagStatus(bool,int,QPointF,QPointF,QPointF,QPointF,QPointF,bool)),w,SLOT(revTagStatus(bool,int,QPointF,QPointF,QPointF,QPointF,QPointF,bool)),Qt::AutoConnection);  //接收到的标签位置信息和电量发送给界面
  		QObject::connect(newconfiginfo,SIGNAL(SendCalibration()),wirelessLocation,SLOT(Calibration_cell()),Qt::AutoConnection);
        QObject::connect(wirelessLocation,SIGNAL(SendCalibrationsatus(int)),newconfiginfo,SLOT(Cell_Calibration_status(int)),Qt::AutoConnection);
        QObject::connect(wirelessLocation,SIGNAL(sendoffsetlist(QVariantList)),newconfiginfo,SLOT(Receiveoffsetlist(QVariantList)),Qt::AutoConnection);
#endif
    }
    else if(locationStatus == 1)  //从机
    {
        LocationServer *locationServer = new LocationServer;   //每个Data都要开服务端接收标签位置信息和电量
        QTimer::singleShot(0,locationServer,SLOT(init()));
        QObject::connect(locationServer,SIGNAL(sendTagStatus(bool,int,QPointF,QPointF,QPointF,QPointF,QPointF,bool)),w,SLOT(revTagStatus(bool,int,QPointF,QPointF,QPointF,QPointF,QPointF,bool)),Qt::AutoConnection);  //接收到的标签位置信息和电量发送给界面 add lw 2017/8/21
    }

    Wificonnect *Wifi_Connect = NULL;
    Wifi_Connect = new Wificonnect;
    QTimer::singleShot(0,Wifi_Connect,SLOT(Wifi_ConnectStart()));

    LogDelete logDelete ;
    QTimer::singleShot(0,&logDelete,SLOT(init()));

    TightenThread       *tighten_xml = NULL;
    Worker    *work = NULL;

    FisUpdate *fisupdate = NULL;
    FisUpdateQueue *fisupdatequeue = NULL;

    SqlThread *sqlthread = NULL;
    SqlThreadBYD * sqlthreadbyd = NULL;
    SqlThreadBenz * sqlthreadbenz = NULL;
    SqlThreadSVW2 * sqlthreadsvw2 = NULL;

    UserManagement * userManagement = NULL;
    PlusCommunication *PlusCom = NULL;

    QueueInterface    *queueInterface = NULL;
    SqlThreadBAIC     *sqlthreadBAIC = NULL;
    sqlThreadGZBAIC   *sqlthreadGZBAIC = NULL ;

    MESUploadSVW2 *mesUploadSVW2 = 0;


    /*
      * Communication:与上位机通讯线程
      * 可配置线程
    */
    Communication communication;
    QTimer::singleShot(0,&communication,SLOT(mycommunicationtimer()));

    /*
      * InputEvents:中断线程
      * 主要响应钥匙的操作，后期添加中断可在此线程操作
    */
    InputEvents inputevents ;
    QTimer::singleShot(0,&inputevents,SLOT(InputEventStart()));

    /*
      * TaoTong:套筒操作线程
      * 套筒操作
    */
    TaoTong taotong;
    QTimer::singleShot(0,&taotong,SLOT(T_start()));

    /*
      * Split:多屏幕显示线程
      * 接收从机数据分屏显示
    */
    Split split;
    QTimer::singleShot(0,&split,SLOT(mysplit_timer()));

    /*
      * SerialComs:RS232串口线程
      * 接收条码枪扫描条码
    */
    SerialComs    *serial = NULL;

    /*
      * RfidControler:RFID控制线程
      * 接收RFID条码
    */
    RfidControler *rfid = NULL;
    serial = new SerialComs;




    //    ReInsertServerDb *reinsert = NULL;
    //    ReInsertServerDbNingBo *reinsertningbo = NULL;
    //    ReInsertServerDbBYD *reinsertbyd = NULL;
    if(factory=="SVW3" || factory=="Ningbo")
    {
        /*
          * FisUpdate:FIS更新线程
          * 与服务器FIS交互定期从server拉取fis数据
        */
        fisupdate = new FisUpdate;                             //PC FisPreview
        QTimer::singleShot(0,fisupdate,SLOT(myfistimer()));
        /*
          * SqlThread:本地存储数据线程
          * 与Worker线程交互拿到数据也存储到本地mysql
        */
        sqlthread = new SqlThread;
        QTimer::singleShot(0,sqlthread,SLOT(sqlinit()));       //Local Mysql

        if(ControlType_1 == "SB356")
        {
            FtpJson *ftpjson = new FtpJson;
            QTimer::singleShot(0,ftpjson,SLOT(myjson()));
            QObject::connect(ftpjson,SIGNAL(sendfromjsonthread(QVariant)),sqlthread,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(ftpjson,SIGNAL(send_mainwindow(QString,QString,QString,int,int,int)),w,SLOT(fromFtpJsonData(QString,QString,QString,int,int,int)),Qt::AutoConnection);
            QObject::connect(ftpjson,SIGNAL(coming_VIN(QString)),w,SLOT(VIN_coming(QString)),Qt::AutoConnection);
            QObject::connect(ftpjson,SIGNAL(VIN_Match_Wrong(QString,QString)),w,SLOT(Wrong_Match_VIN(QString,QString)),Qt::AutoConnection);
            QObject::connect(Wifi_Connect,SIGNAL(sb356connect(bool)),w,SLOT(SB356Connect(bool)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "SB356_PLC")
        {
            FtpJsonParse *ftpjsonParse = new FtpJsonParse;
            QTimer::singleShot(0,ftpjsonParse,SLOT(ftpStart()));
            QObject::connect(ftpjsonParse,SIGNAL(sendfromjsonthread(QVariant)),sqlthread,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(ftpjsonParse,SIGNAL(send_mainwindow(QString,QString,QString,QString,QString,QString)),w,SLOT(fromsecondthreaddata(QString,QString,QString,QString,QString,QString)),Qt::AutoConnection);
            //            QObject::connect(ftpjson,SIGNAL(coming_VIN(QString)),w,SLOT(VIN_coming(QString)),Qt::AutoConnection);
            QObject::connect(ftpjsonParse,SIGNAL(VIN_Match_Wrong(QString,QString)),w,SLOT(Wrong_Match_VIN(QString,QString)),Qt::AutoConnection);
            QObject::connect(Wifi_Connect,SIGNAL(sb356connect(bool)),w,SLOT(SB356Connect(bool)),Qt::AutoConnection);
            ConnectPLC *connectPLC = new ConnectPLC;
            QTimer::singleShot(0,connectPLC,SLOT(PLCStart()));
            QObject::connect(w,SIGNAL(vinSendPLC(int,int,QString)),connectPLC,SLOT(revVinFunc(int,int,QString)),Qt::AutoConnection);
            QObject::connect(connectPLC,SIGNAL(nokBolt(int)),w,SLOT(nokWhichBolt(int)),Qt::AutoConnection);
            QObject::connect(connectPLC,SIGNAL(PLCStatus(int)),w,SLOT(PLCIsReady(int)),Qt::AutoConnection);
        }
        else if((ControlType_1 == "CS351")||(ControlType_1 == "Nexo") ||(ControlType_1 == "Desoutter"))
        {
            /*
              * Worker:工作线程
              * 用来接收CS351 B端口发来心跳曲线
            */
            work = new Worker;
            QTimer::singleShot(0,work,SLOT(fun1()));               //cs351 port 4710
            /*
              * TightenThread:控制CS351线程
              * 发送复位 心跳 使能命令同时与UI交互
            */
            tighten_xml = new TightenThread;
            QTimer::singleShot(0,tighten_xml,SLOT(tightenStart()));//cs351 port 4700
        }
    }
    else if(factory=="BYDSZ"||factory=="BYDXA")
    {
        fisupdatequeue = new FisUpdateQueue;                         //PC TaskPreview
        QTimer::singleShot(0,fisupdatequeue,SLOT(myfistimer()));

        sqlthreadbyd = new SqlThreadBYD;                             //Local Mysql
        QTimer::singleShot(0,sqlthreadbyd,SLOT(sqlinit()));

        //        reinsertbyd = new ReInsertServerDbBYD;                        //PC SQL Server
        //        QTimer::singleShot(0,reinsertbyd,SLOT(reinsert_start()));

        DeleteCarSQL *deletecarsql = new DeleteCarSQL;                //Delete car
        QTimer::singleShot(0,deletecarsql,SLOT(SQL_Delete_Car()));

        if(ControlType_1 == "CS351")
        {
            TightenOpNet *tighten_opnet = new TightenOpNet;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            //            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString)),&w,SLOT(fromsecondthreaddata(QString,QString,QString)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "Cleco")
        {
            TightenOpCleco *tighten_opnet = new TightenOpCleco;

            tighten_opnet->vari_1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "PFC")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtlasPFC  *tighten_opnet = new TightenOpAtlasPFC;
//            Testalive *test_alive5s = new Testalive;//add 20171228

            tighten_opnet->vari_2 = variable2;
            tighten_opnet->vari_1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
//            QTimer::singleShot(0,test_alive5s,SLOT(start_timer5s()));//add 20171228
//            QObject::connect(test_alive5s,SIGNAL(sendalive5s()),tighten_opnet,SLOT(timerFunc5000ms()),Qt::AutoConnection);//add 20171228

            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(QueueAlign(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "PM")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtalsPM    *tighten_opnet = new TightenOpAtalsPM;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "PMM")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtalsPMM  *tighten_opnet = new TightenOpAtalsPMM;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);

        }
        else if(ControlType_1 =="PFD")
        {
            TightenOpAtalsPF    *tighten_opnet = new TightenOpAtalsPF;
            tighten_opnet->vari1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
    }
    else if(factory=="Benz")  //to do
    {
        //        QuickRequest *quickRequest = new QuickRequest;
        //        QTimer::singleShot(0,quickRequest,SLOT(requestStart()));
        PlusCom = new PlusCommunication;
        QTimer::singleShot(0,PlusCom,SLOT(PlusStart()));            //plus communication

        CardRead *cardRead = new CardRead;
        QTimer::singleShot(0,cardRead,SLOT(comInit()));

        userManagement = new UserManagement;
        m.initUser(userManagement);

        QObject::connect(PlusCom,SIGNAL(SendJOB(QString)),w,SLOT(receiveJob(QString)),Qt::AutoConnection);
        QObject::connect(PlusCom,SIGNAL(sendPlusFlag(bool)),w,SLOT(receivePlusFlag(bool)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendRequestJob(QString)),PlusCom,SLOT(receiveRequestJob(QString)),Qt::AutoConnection);
        QObject::connect(cardRead,SIGNAL(sendCardNumber(int)),userManagement,SLOT(receiveCardNumber(int)),Qt::AutoConnection);
        //        QObject::connect(quickRequest,SIGNAL(sendJob(QString)),w,SLOT(searchJobNum(QString)),Qt::AutoConnection);
        //        QObject::connect(quickRequest,SIGNAL(JobError(int)),w,SLOT(receiveJobError(int)),Qt::AutoConnection);
        //        QObject::connect(w,SIGNAL(requestJob(QString)),quickRequest,SLOT(request(QString)),Qt::AutoConnection);
        //        QObject::connect(w,SIGNAL(sendError(bool,bool,bool)),quickRequest,SLOT(receiveErrorCode(bool,bool,bool)),Qt::AutoConnection);


        sqlthreadbenz = new SqlThreadBenz;                             //Local Mysql
        QTimer::singleShot(0,sqlthreadbenz,SLOT(sqlinit()));

        if(isOutFlag)
        {
            ConnectPLC *connectPLC = new ConnectPLC;
            QTimer::singleShot(0,connectPLC,SLOT(PLCStart()));
            QObject::connect(w,SIGNAL(vinSendPLC(int,int,QString)),connectPLC,SLOT(revVinFunc(int,int,QString)),Qt::AutoConnection);
            //            QObject::connect(connectPLC,SIGNAL(PLCStatus(int)),w,SLOT(PLCIsReady(int)),Qt::AutoConnection);
        }
        if(ControlType_1 == "PFC" || ControlType_2 == "PFC")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));
        }

        if(ControlType_1 != "OFF")
        {
            TightenOpNetCh1 *tighten_opnetCh1 = new TightenOpNetCh1;
            QTimer::singleShot(0,tighten_opnetCh1,SLOT(tightenStartCh1()));
            QObject::connect(tighten_opnetCh1,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbenz,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCh1,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCh1,SIGNAL(IsTigntenReady(int,bool)),w,SLOT(TightenIsReady(int,bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnetCh1,SLOT(sendReadOperateCh1(bool,int)),Qt::AutoConnection);
        }
        if(ControlType_2 != "OFF")
        {
            TightenOpNetCh2 *tighten_opnetCh2 = new TightenOpNetCh2;
            QTimer::singleShot(0,tighten_opnetCh2,SLOT(tightenStartCh2()));
            QObject::connect(tighten_opnetCh2,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbenz,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCh2,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCh2,SIGNAL(IsTigntenReady(int,bool)),w,SLOT(TightenIsReady(int,bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate2(bool,int)),tighten_opnetCh2,SLOT(sendReadOperateCh2(bool,int)),Qt::AutoConnection);
        }
    }
    else if(factory=="GM")
    {
        //        fisupdate = new FisUpdate;                             //PC FisPreview
        //        QTimer::singleShot(0,fisupdate,SLOT(myfistimer()));

        sqlthreadbyd = new SqlThreadBYD;                             //Local Mysql
        QTimer::singleShot(0,sqlthreadbyd,SLOT(sqlinit()));

        GwkQuanTecMCS *gwkSerial = new GwkQuanTecMCS;
        QTimer::singleShot(0,gwkSerial,SLOT(serialComInit()));
        QObject::connect(gwkSerial,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
        QObject::connect(gwkSerial,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
        QObject::connect(gwkSerial,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
        QObject::connect(gwkSerial,SIGNAL(sendLimit(QString,QString,QString)),w,SLOT(displayLimit(QString,QString,QString)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendOperate1(bool,int)),gwkSerial,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(gwkInit()),gwkSerial,SLOT(gwkReadDataFunc()),Qt::AutoConnection);
    }
    else if(factory=="Haima")
    {
        system("/etc/data2/restart &");
        newconfiginfo->setRepair(isRepair);

        sqlthreadbyd = new SqlThreadBYD;                             //Local Mysql
        sqlthreadbyd->setRepair(isRepair);
        QTimer::singleShot(0,sqlthreadbyd,SLOT(sqlinit()));

        MESUploadHaima *mesUploadHaima = new MESUploadHaima;              //data upload
        queueThread    *queueHaima    = new queueThread ;
        mesUploadHaima->setRepair(isRepair);
        QTimer::singleShot(0,mesUploadHaima,SLOT(init()));


        userManagement = new UserManagement;
        m.initUser(userManagement);
        CardRead *cardRead = new CardRead;
        QTimer::singleShot(0,cardRead,SLOT(comInit()));

        serialThread *serial = new serialThread;
        if(StationName.contains("分装线"))
        {
            QTimer::singleShot(0,serial,SLOT(initSerial()));
            QObject::connect(serial,SIGNAL(signalSerialCom(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
        }

        QObject::connect(newconfiginfo,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
        QObject::connect(newconfiginfo,SIGNAL(sendRepairVIN(QString)),mesUploadHaima,SLOT(receiveRepairVIN(QString)),Qt::AutoConnection);
        QObject::connect(mesUploadHaima,SIGNAL(replyRepair(int,QVariantMap)),newconfiginfo,SLOT(receiveRepairReply(int,QVariantMap)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(signalUpdateQueueSql()),queueHaima, SLOT(getData())) ;
        QObject::connect(cardRead,SIGNAL(sendCardNumber(int)),userManagement,SLOT(receiveCardNumber(int)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(signalGetCarInfo(int,QString)),queueHaima,SLOT(getData(int,QString)));
        QObject::connect(queueHaima,SIGNAL(FisSendSerial(QString,QString)),w,SLOT(slotGetQueueNum(QString,QString)));

        if(ControlType_1 =="PF_232")
        {
            Tighten_OP_Rs232 *tighten_op_rs232 = new Tighten_OP_Rs232;
            tighten_op_rs232->setRepair(isRepair);
            tighten_op_rs232->vari1 = main_variable1;
            QTimer::singleShot(0,tighten_op_rs232,SLOT(tightenStart()));
            QObject::connect(tighten_op_rs232,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_op_rs232,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_op_rs232,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_op_rs232,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if(ControlType_1 =="PFD")
        {
            TightenOpAtalsPF *tighten_opnet = new TightenOpAtalsPF;
            tighten_opnet->setRepair(isRepair);
            tighten_opnet->vari1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if(ControlType_1 =="PM600")
        {
            TightenJsonAtlasPM600    *tighten_json_pm600 = new TightenJsonAtlasPM600;
            tighten_json_pm600->setRepair(isRepair);
            QTimer::singleShot(0,tighten_json_pm600,SLOT(init()));
            QObject::connect(tighten_json_pm600,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_json_pm600,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_json_pm600,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);     //????????是否连接怎么判断？
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_json_pm600,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if (ControlType_1 == "PFC")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtlasPFC  *tighten_opnet = new TightenOpAtlasPFC;
            tighten_opnet->vari_2 = variable2;
            tighten_opnet->vari_1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));

            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadbyd,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(QueueAlign(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);

        }
    }
    else if(factory=="SVW2")
    {
        /*
          * SqlThread:本地存储数据线程
          * 与Worker线程交互拿到数据也存储到本地mysql
        */
        sqlthreadsvw2 = new SqlThreadSVW2;
        QTimer::singleShot(0,sqlthreadsvw2,SLOT(sqlinit()));       //Local Mysql

        mesUploadSVW2 = new MESUploadSVW2;              //data upload
        QTimer::singleShot(0,mesUploadSVW2,SLOT(init()));

        getCarInformation *getCarInfo = new getCarInformation;
        QObject::connect(w,SIGNAL(signalGetCarInfoFast(QString,bool,int)),getCarInfo,SLOT(getData(QString,bool,int)));
        QObject::connect(getCarInfo,SIGNAL(requst(bool,int)),w,SLOT(VinPinCodeView(bool,int)));

        qDebug() << "factory==SVW2";

        if((ControlType_1 == "CS351")||(ControlType_1 == "Nexo")||(ControlType_1 == "PF_XML")||(ControlType_1 == "Desoutter"))
        {
            /*
              * Worker:工作线程
              * 用来接收CS351 B端口发来心跳曲线
            */
            work = new Worker;
            QTimer::singleShot(0,work,SLOT(fun1()));               //cs351 port 4710
            /*
              * TightenThread:控制CS351线程
              * 发送复位 心跳 使能命令同时与UI交互
            */
            tighten_xml = new TightenThread;
            QTimer::singleShot(0,tighten_xml,SLOT(tightenStart()));//cs351 port 4700
            QObject::connect(&logDelete,SIGNAL(signalSyncTime(int)),tighten_xml,SLOT(sendCmdMessage(int)));
        }
        else if(ControlType_1 =="PFD")//SR
        {
            if(main_variable1 == "op")
            {
                AtlasOP *OP_data = new AtlasOP;
                OP_data->vari1 =  main_variable1;
                QTimer::singleShot(0,OP_data,SLOT(tightenStart()));
                QObject::connect(OP_data,SIGNAL(sendfromOpThread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
                QObject::connect(OP_data,SIGNAL(send_mainwindow(QString,QString,QString,int,int,int)),w,SLOT(fromFtpJsonData(QString,QString,QString,int,int,int)),Qt::AutoConnection);
                QObject::connect(OP_data,SIGNAL(coming_VIN(QString)),w,SLOT(VIN_coming(QString)),Qt::AutoConnection);
                QObject::connect(OP_data,SIGNAL(VIN_Match_Wrong(QString,QString)),w,SLOT(Wrong_Match_VIN(QString,QString)),Qt::AutoConnection);
                QObject::connect(OP_data,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            }
            else if(main_variable1 == "xml")
            {
                xmlData *xml_data = new xmlData;
                xml_data->vari1 = main_variable1;
                QTimer::singleShot(0,xml_data,SLOT(xmlInit()));
                QObject::connect(xml_data,SIGNAL(sendfromjsonthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
                QObject::connect(xml_data,SIGNAL(send_mainwindow(QString,QString,QString,int,int,int)),w,SLOT(fromFtpJsonData(QString,QString,QString,int,int,int)),Qt::AutoConnection);
                QObject::connect(xml_data,SIGNAL(coming_VIN(QString)),w,SLOT(VIN_coming(QString)),Qt::AutoConnection);
                QObject::connect(xml_data,SIGNAL(VIN_Match_Wrong(QString,QString)),w,SLOT(Wrong_Match_VIN(QString,QString)),Qt::AutoConnection);
                QObject::connect(xml_data,SIGNAL(havedconnect(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            }
            else
            {
                TightenOpAtalsPF *tighten_opnet = new TightenOpAtalsPF;
                tighten_opnet->vari1 =  main_variable1;
                QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
                QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
                QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
                QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
                QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
                QObject::connect(tighten_opnet,SIGNAL(sendVinToMain(QString)),w,SLOT(autoSearchVinNum(QString)),Qt::AutoConnection);
            }
        }
        else if(ControlType_1 == "Cleco")
        {
            TightenOpCleco *tighten_opnet = new TightenOpCleco;
            tighten_opnet->vari_1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }

        else if(ControlType_1 =="GWK")//GWK
        {
            GwkThread *tighten_opnet = new GwkThread;
            QTimer::singleShot(0,tighten_opnet,SLOT(gwkstart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
//        else if(ControlType_1 == "SB356")
//        {
//            FtpJson *ftpjson = new FtpJson;
//            QTimer::singleShot(0,ftpjson,SLOT(myjson()));
//            QObject::connect(ftpjson,SIGNAL(sendfromjsonthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
//            QObject::connect(ftpjson,SIGNAL(send_mainwindow(QString,QString,QString,int,int,int)),w,SLOT(fromFtpJsonData(QString,QString,QString,int,int,int)),Qt::AutoConnection);
//            QObject::connect(ftpjson,SIGNAL(coming_VIN(QString)),w,SLOT(VIN_coming(QString)),Qt::AutoConnection);
//            QObject::connect(ftpjson,SIGNAL(VIN_Match_Wrong(QString,QString)),w,SLOT(Wrong_Match_VIN(QString,QString)),Qt::AutoConnection);
//            QObject::connect(Wifi_Connect,SIGNAL(sb356connect(bool)),w,SLOT(SB356Connect(bool)),Qt::AutoConnection);
//        }
        else if((ControlType_1 == "SB356"))
        {
            /*
              * Worker:工作线程
              * 用来接收CS351 B端口发来心跳曲线
            */
            Worker356 *work356 = new Worker356;
            QTimer::singleShot(0,work356,SLOT(fun1()));               //cs351 port 4710
            /*
              * TightenThread:控制CS351线程
              * 发送复位 心跳 使能命令同时与UI交互
            */

            QObject::connect(work356,SIGNAL(havedconnect(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);

            QObject::connect(work356,SIGNAL(sendfromjsonthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);

            QObject::connect(work356,SIGNAL(send_mainwindow(QString,QString,QString,int,int,int)),w,SLOT(fromFtpJsonData(QString,QString,QString,int,int,int)),Qt::AutoConnection);
            QObject::connect(work356,SIGNAL(coming_VIN(QString)),w,SLOT(VIN_coming(QString)),Qt::AutoConnection);
            QObject::connect(work356,SIGNAL(VIN_Match_Wrong(QString,QString)),w,SLOT(Wrong_Match_VIN(QString,QString)),Qt::AutoConnection);
        }
        if(isReplaceBarcode)
        {
            ReplaceBarcode *replaceBarcode = new ReplaceBarcode;
            QTimer::singleShot(0,replaceBarcode,SLOT(myreplace()));
            QObject::connect(replaceBarcode,SIGNAL(signalReplaceStatus(QVariantMap)),w,SLOT(slotReplaceBarcode(QVariantMap))) ;
        }

    }
    else if(factory=="Dongfeng")
    {
        sqlthreadsvw2 = new SqlThreadSVW2;
        QTimer::singleShot(0,sqlthreadsvw2,SLOT(sqlinit()));       //Local Mysql

        queueInterface = new QueueInterface;
        QTimer::singleShot(0,queueInterface,SLOT(init()));

        if(ControlType_1 == "CS351"||(ControlType_1 == "Desoutter"))
        {
            work = new Worker;
            QTimer::singleShot(0,work,SLOT(fun1()));               //cs351 port 4710

            tighten_xml = new TightenThread;
            QTimer::singleShot(0,tighten_xml,SLOT(tightenStart()));//cs351 port 4700
        }
        else if(ControlType_1 == "PFC")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtlasPFC  *tighten_opnet = new TightenOpAtlasPFC;
//            Testalive *test_alive5s = new Testalive;//add 20171228
            tighten_opnet->vari_2 = variable2;
            tighten_opnet->vari_1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
//            QTimer::singleShot(0,test_alive5s,SLOT(start_timer5s()));//add 20171228
//            QObject::connect(test_alive5s,SIGNAL(sendalive5s()),tighten_opnet,SLOT(timerFunc5000ms()),Qt::AutoConnection);//add 20171228

            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),queueInterface,SLOT(IsTightenReady(bool)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "PM")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtalsPM    *tighten_opnet = new TightenOpAtalsPM;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),queueInterface,SLOT(IsTightenReady(bool)),Qt::AutoConnection);

        }
        else if(main_variable1 == "IR")
        {
            //            TightenOpAtalsPF    *tighten_opnet = new TightenOpAtalsPF;
            //            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            //            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            //            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            //            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            //            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int)),w,SLOT(fromsecondthreaddataIRMM(QString,QString,QString,int)),Qt::AutoConnection);



            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtalsIRMM *tighten_opnet[5];
            QStringList list = ControllerIp_1.split(".",QString::SkipEmptyParts);
            int IP_Right = list[list.size()-1].toInt();
            for(int n=0;n<5;n++)
            {
                tighten_opnet[n] = new TightenOpAtalsIRMM;
                list[3] = QString::number(IP_Right + n);
                tighten_opnet[n]->controllerIP = list.join(".");
                tighten_opnet[n]->ch = n;
                QTimer::singleShot(0,tighten_opnet[n],SLOT(tightenStart()));
                QObject::connect(tighten_opnet[n],SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
                QObject::connect(tighten_opnet[n],SIGNAL(send_mainwindow(QString,QString,QString,int)),w,SLOT(fromsecondthreaddataIRMM(QString,QString,QString,int)),Qt::AutoConnection);
                QObject::connect(tighten_opnet[n],SIGNAL(IsTigntenReady(int,bool)),w,SLOT(TightenIsReady(int,bool)),Qt::AutoConnection);
                QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet[n],SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
                QObject::connect(tighten_opnet[n],SIGNAL(IsTigntenReady(int,bool)),queueInterface,SLOT(IsTightenReady(int,bool)),Qt::AutoConnection);
            }
        }
        else if(ControlType_1 == "PMM")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtalsPMM    *tighten_opnet = new TightenOpAtalsPMM;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),queueInterface,SLOT(IsTightenReady(bool)),Qt::AutoConnection);
        }

        mesUploadSVW2 = new MESUploadSVW2;              //data upload
        QTimer::singleShot(0,mesUploadSVW2,SLOT(init()));


        QObject::connect(queueInterface,SIGNAL(sendVinToUi(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
        QObject::connect(queueInterface,SIGNAL(sendAlign(QString,QString)),w,SLOT(getAlign(QString,QString)),Qt::AutoConnection);

        QObject::connect(queueInterface,SIGNAL(sendGetCar()),w,SLOT(receiveGetCar()),Qt::AutoConnection);

        QObject::connect(Wifi_Connect,SIGNAL(rfidConnected(bool)),w,SLOT(setRfidState(bool)),Qt::AutoConnection);
    }
    else if(factory=="BAIC")
    {
        Queuelock.lockForWrite();
        if(isQueue)
        {
            LeuzeRecv *leuzeRecv = new LeuzeRecv;              //data upload
            QObject::connect(leuzeRecv,SIGNAL(sendNewCode(QString)),w,SLOT(recvNewCode(QString)),Qt::AutoConnection);
            QTimer::singleShot(0,leuzeRecv,SLOT(init()));
        }
        Queuelock.unlock();

        //user login
        userManagement = new UserManagement;
        m.initUser(userManagement);
        userManagement->on_pushButton_toPassword_clicked();
#if 0
        CardRead *cardRead = new CardRead;
        QTimer::singleShot(0,cardRead,SLOT(comInit()));
        QObject::connect(cardRead,SIGNAL(sendCardNumber(int)),userManagement,SLOT(receiveCardNumber(int)),Qt::AutoConnection);
#endif

        sqlthreadBAIC = new SqlThreadBAIC;
        QTimer::singleShot(0,sqlthreadBAIC,SLOT(sqlinit()));       //Local Mysql

        MESUploadBAIC *mesUploadBAIC = new MESUploadBAIC;              //data upload
        QTimer::singleShot(0,mesUploadBAIC,SLOT(init()));

        if(ControlType_1 =="PFD")//Open protocol
        {
            TightenOpAtalsPF    *tighten_opnet = new TightenOpAtalsPF;
            tighten_opnet->vari1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadBAIC,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "PFC")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpAtlasPFC  *tighten_opnet = new TightenOpAtlasPFC;
//            Testalive *test_alive5s = new Testalive;//add 20171228
            tighten_opnet->vari_2 = variable2;
            tighten_opnet->vari_1 = main_variable1;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
//            QTimer::singleShot(0,test_alive5s,SLOT(start_timer5s()));//add 20171228
//            QObject::connect(test_alive5s,SIGNAL(sendalive5s()),tighten_opnet,SLOT(timerFunc5000ms()),Qt::AutoConnection);//add 20171228

            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadBAIC,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(QueueAlign(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
        }
        else if(ControlType_1 == "SB356")//FTP
        {
            ftpDataParse *ftpDatajson = new ftpDataParse;
//            if(Master_slave == "master")
//            {
//                w->setFTPMaster();
//                ftpDatajson->isMaster = true;
//                FtpClient *ftpClient = new FtpClient;
//                ftpClient->ftpServer = slave;
//                QTimer::singleShot(0,ftpClient,SLOT(initServer()));
//                QObject::connect(ftpClient,SIGNAL(sendFtpConnected(bool)),w,SLOT(setRfidState(bool)),Qt::AutoConnection);
//            }
//            else
//                ftpDatajson->isMaster = false;
            QTimer::singleShot(0,ftpDatajson,SLOT(ftpStart()));
            QObject::connect(w,SIGNAL(sendRunStatusToJson(QString)),ftpDatajson,SLOT(revRunStatus(QString)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),ftpDatajson,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(ftpDatajson,SIGNAL(sendfromjsonthread(QVariant)),sqlthreadBAIC,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(ftpDatajson,SIGNAL(send_mainwindow(QString,QString,QString,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int)),Qt::AutoConnection);
            QObject::connect(Wifi_Connect,SIGNAL(sb356connect(bool)),w,SLOT(SB356Connect(bool)),Qt::AutoConnection);
        }
    }

    else if(factory=="GZBAIC")
    {
        sqlthreadGZBAIC = new sqlThreadGZBAIC;
        QTimer::singleShot(0,sqlthreadGZBAIC,SLOT(sqlinit()));       //Local Mysql

        if(ControlType_1 == "PM")
        {
            alatsPM4000    *tighten_opnet = new alatsPM4000;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);

            QObject::connect(tighten_opnet,SIGNAL(sendfromOpThread(QVariant)),
                             sqlthreadGZBAIC,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);

            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QByteArray,QString,QString,QString,int)),
                             w,SLOT(slotShowBolt(QByteArray,QString,QString,QString,int)));

            QObject::connect(w,SIGNAL(skipBolt()),tighten_opnet,SLOT(slotSkipBolt()));
            QObject::connect(w,SIGNAL(signalSendBoltAmount(int)),tighten_opnet,SLOT(slotBoltAmount(int))) ;

        }
        else
        {
            DTdebug() << "control do not match,it should PM" ;
        }
    }
    else if(factory == "NOVAT" || factory == "KFCHERY")
    {
        sqlthreadBAIC = new SqlThreadBAIC;
        QTimer::singleShot(0,sqlthreadBAIC,SLOT(sqlinit()));       //Local Mysql

        if(factory == "NOVAT")
        {
            MESUploadBAIC *mesUploadBAIC = new MESUploadBAIC;              //data upload
            QTimer::singleShot(0,mesUploadBAIC,SLOT(init()));
        }

        if(ControlType_1 == "Nexo_OP")
        {
            FtpHandle *ftp_handle = new FtpHandle;
            QTimer::singleShot(0,ftp_handle,SLOT(ftpStart()));

            TightenOpNet  *tighten_opnet = new TightenOpNet;
            QTimer::singleShot(0,tighten_opnet,SLOT(tightenStart()));

            QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QVariant)),sqlthreadBAIC,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(sendTightenID(QString)),w,SLOT(slot_getTightenID(QString)));
            QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(signals_sendVINSuccess(bool)),w,SLOT(slots_getSendVINResult(bool)));
            QObject::connect(tighten_opnet,SIGNAL(signals_deleteCachedVIN(QByteArray)),serial,SLOT(slots_deleteCachedVIN(QByteArray)));
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnet,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnet,SIGNAL(QueueAlign(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
        }

    }
    else if(factory == "AQCHERY")
    {
        sqlthreadBAIC = new SqlThreadBAIC;
        QTimer::singleShot(0,sqlthreadBAIC,SLOT(sqlinit()));       //Local Mysql

        MESUploadAQCHERY *mesuploadAQCHERY = new MESUploadAQCHERY;
        QTimer::singleShot(0,mesuploadAQCHERY,SLOT(init()));
        queueThread    *queueHaima    = new queueThread ;

        if(ControlType_1 == "APEX_OP")
        {
            TightenOpNetCurve  *tighten_opnetCurve = new TightenOpNetCurve;
            QTimer::singleShot(0,tighten_opnetCurve,SLOT(tightenStart()));

            QObject::connect(w,SIGNAL(signalUpdateQueueSql()),queueHaima, SLOT(getData())) ;
            QObject::connect(w,SIGNAL(signalGetCarInfo(int,QString)),queueHaima,SLOT(getData(int,QString)));
            QObject::connect(queueHaima,SIGNAL(FisSendSerial(QString,QString)),w,SLOT(slotGetQueueNum(QString,QString)));
            QObject::connect(tighten_opnetCurve,SIGNAL(sendfromworkthread(QVariant)),sqlthreadBAIC,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCurve,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCurve,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_opnetCurve,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(tighten_opnetCurve,SIGNAL(QueueAlign(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
            QObject::connect(mesuploadAQCHERY,SIGNAL(serialCom(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);

            QObject::connect(w,SIGNAL(signalSendHttp(int)),mesuploadAQCHERY,SLOT(SendHttp(int)),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(signalDetectCard(QString)),mesuploadAQCHERY,SLOT(slot_DetectCard(QString)),Qt::AutoConnection);
            QObject::connect(&m,SIGNAL(signalDetectCard(QString)),mesuploadAQCHERY,SLOT(slot_DetectCard(QString)),Qt::AutoConnection);
            QObject::connect(mesuploadAQCHERY,SIGNAL(sendResult(bool)),&m,SLOT(receiveResult(bool)));
            QObject::connect(mesuploadAQCHERY,SIGNAL(signalPLCHeartbeat(bool)),w,SLOT(slot_PLCHeartbeat(bool)),Qt::AutoConnection);
        }
    }



    if(factory=="SVW2")
    {
        qDebug() << "mMESUploadSVW2";
        serial->mMESUploadSVW2 = mesUploadSVW2;
    }
    if((factory != "Haima") && (!StationName.contains("分装线")))
    {
        QTimer::singleShot(0,serial,SLOT(comInit()));
    }


    QObject::connect(serial,SIGNAL(serialCom(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
    QObject::connect(serial,SIGNAL(sendDebug(QString)),newconfiginfo,SLOT(receiveDebug(QString)),Qt::AutoConnection);

    if(factory != "Dongfeng")
    {
        rfid = new RfidControler;
        rfid->rfidip = RFID_IP;
        rfid->rfidport = RFID_Port;
        rfid->vari_1 =  main_variable1;
        rfid->RFID_Enable_Flag = isRFID;
        QTimer::singleShot(0,rfid,SLOT(rfidInit()));
        QObject::connect(rfid,SIGNAL(sendPinToUi(QString,bool,QString)),w,SLOT(getSerialNum(QString,bool,QString)),Qt::AutoConnection);
        QObject::connect(rfid,SIGNAL(rfidConnected(bool)),w,SLOT(setRfidState(bool)),Qt::AutoConnection);
        QObject::connect(rfid,SIGNAL(sendGetCar()),w,SLOT(receiveGetCar()),Qt::AutoConnection);
        QObject::connect(Wifi_Connect,SIGNAL(rfidconnect(bool)),rfid,SLOT(rfidDisconnectDo(bool)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendRunStatusToJson(QString)),rfid,SLOT(receiveSystemStatus(QString)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendControllerStatus(bool)),rfid,SLOT(revControllerStatus(bool)),Qt::AutoConnection);
        QObject::connect(w, SIGNAL(signalDeleteLastRfid()), rfid, SLOT(slotDeletArray())) ;
    }

    /*
      * masterTCP:master to salve控制线程
      *
    */
    MasterTCP *master = NULL;

    if(main_variable1 == "master")
    {
        master = new MasterTCP;
        QTimer::singleShot(0,master,SLOT(masterServerStart()));
        QObject::connect(rfid,SIGNAL(sendPinToMasterTcp(QString,bool,QString)),master,SLOT(revRFID_PIN(QString,bool,QString)),Qt::AutoConnection);
        QObject::connect(master,SIGNAL(sendCmdClearPinCode()),rfid,SLOT(clearCodeBuf()),Qt::AutoConnection);
    }



    /*
      * 下面都是信号槽的连接 具体看代码
      *
    */

    //QObject::connect(&change,SIGNAL(sendSerial(QString)),w.newconfiginfo,SLOT(receiveDebug(QString)),Qt::AutoConnection);
    //    QObject::connect(&w,SIGNAL(clo()),work,SLOT(fromNew1thread()),Qt::AutoConnection);
    //    QObject::connect(&communication,SIGNAL(update()),w,SLOT(init()),Qt::AutoConnection);

    //QObject::connect(&communication,SIGNAL(xmlcreate()),&XML_Create,SLOT(updateXML()),Qt::AutoConnection);
    //QObject::connect(w.newconfiginfo,SIGNAL(xmlcreate()),&XML_Create,SLOT(updateXML()),Qt::AutoConnection);
    QObject::connect(Wifi_Connect,SIGNAL(wifi_connect(bool)),w,SLOT(wifishow(bool)),Qt::AutoConnection);
    QObject::connect(Wifi_Connect,SIGNAL(wifi_connect(bool)),&m,SLOT(wifishow(bool)),Qt::AutoConnection);
    QObject::connect(Wifi_Connect,SIGNAL(data_connect(bool)),w,SLOT(datashow(bool)),Qt::AutoConnection);

    QObject::connect(&inputevents,SIGNAL(sendconfigwarning(bool)),w,SLOT(configwarning(bool)),Qt::AutoConnection);
    QObject::connect(&inputevents,SIGNAL(sendbatterysta(bool)),w,SLOT(batteryshow2(bool)),Qt::AutoConnection);
    QObject::connect(&inputevents,SIGNAL(sendbatterysta(bool)),&m,SLOT(batteryshow2(bool)),Qt::AutoConnection);
    QObject::connect(&taotong,SIGNAL(sendbattery(QString)),w,SLOT(batteryshow1(QString)),Qt::AutoConnection);
    QObject::connect(&taotong,SIGNAL(sendbattery(QString)),&m,SLOT(batteryshow1(QString)),Qt::AutoConnection);
    QObject::connect(&taotong,SIGNAL(taotong_num(int)),w,SLOT(taotong_main(int)),Qt::AutoConnection);
    QObject::connect(w,SIGNAL(sDebugSkipTaotong(int)),&taotong,SLOT(DebugSkipTaotong(int)),Qt::AutoConnection);
    QObject::connect(w,SIGNAL(signalSendTaotongNum()),&taotong,SLOT(sendTaotongNum()));

    QObject::connect(w,SIGNAL(sendInfo(bool,bool,int,int)),&communication,SLOT(receiveInfo(bool,bool,int,int)),Qt::AutoConnection);
    QObject::connect(w,SIGNAL(sendCmdToCs351FromMW(int)),tighten_xml,SLOT(sendCmdMessage(int))) ;


    sleep(1);
    if(factory == "SVW3" || factory=="Ningbo")
    {
        //        QObject::connect(&w,SIGNAL(closeThread()),tighten_xml,SLOT(restartDisconnect()),Qt::AutoConnection);
        if(ControlType_1 != "SB356"&&ControlType_1 != "SB356_PLC"&&ControlType_1 != "OFF")
        {
            QObject::connect(work,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(ReconnectSendOperate()),w,SLOT(receiveOperate()),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_xml,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(havedconnect(bool)),tighten_xml,SLOT(receivehavedconnect(bool)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(disconnectTellPortA()),tighten_xml,SLOT(portBSendPortA()),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(disconnectTellPortB()),work,SLOT(portAsendPortB()),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(sendfromworkthread(QVariant)),sqlthread,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
        }


        QObject::connect(fisupdate,SIGNAL(time_error(bool)),w,SLOT(time_warning(bool)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthread,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendOneGroupNok(QString ,QString *,QString *)),sqlthread,SLOT(configOneGroup(QString,QString *,QString *)),Qt::AutoConnection);
        QObject::connect(sqlthread,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(newconfiginfo,SIGNAL(sendGetTime()),fisupdate,SLOT(QueryTime()),Qt::AutoConnection);
        QObject::connect(fisupdate,SIGNAL(sendTime(QString)),newconfiginfo,SLOT(receivetime(QString)),Qt::AutoConnection);
        QObject::connect(newconfiginfo,SIGNAL(column_update(QString)),fisupdate,SLOT(update_column(QString)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthread,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }
    else if(factory=="BYDSZ" || factory=="BYDXA")
    {
        //        QObject::connect(tighten_opnet,SIGNAL(sendfromworkthread(QString *)),sqlthreadbyd,SLOT(sqlinsert(QString *)),Qt::AutoConnection);
        //        QObject::connect(tighten_opnet,SIGNAL(send_mainwindow(QString,QString,QString)),&w,SLOT(fromsecondthreaddata(QString,QString,QString)),Qt::AutoConnection);
        //        QObject::connect(tighten_opnet,SIGNAL(IsTigntenReady(bool)),&w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
        //        QObject::connect(&w,SIGNAL(sendoperate()),tighten_opnet,SLOT(sendReadOperate()),Qt::AutoConnection);
        QObject::connect(newconfiginfo,SIGNAL(sendGetTime()),fisupdatequeue,SLOT(QueryTime()),Qt::AutoConnection);
        QObject::connect(fisupdatequeue,SIGNAL(sendTime(QString)),newconfiginfo,SLOT(receivetime(QString)),Qt::AutoConnection);
        QObject::connect(newconfiginfo,SIGNAL(column_update(QString)),fisupdatequeue,SLOT(update_column(QString)),Qt::AutoConnection);

        QObject::connect(fisupdatequeue,SIGNAL(time_error(bool)),w,SLOT(time_warning(bool)),Qt::AutoConnection);
        QObject::connect(fisupdatequeue,SIGNAL(FisSendSerial(QString)),w,SLOT(ReceFisSerial(QString)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadbyd,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(sqlthreadbyd,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadbyd,SLOT(receiveNokAll(int)),Qt::AutoConnection);

    }
    else if(factory == "Benz")
    {
        QObject::connect(PlusCom,SIGNAL(sendPlusFlag(bool)),rfid,SLOT(receivePlusFlag(bool)),Qt::AutoConnection);
        QObject::connect(userManagement,SIGNAL(sendLogin(bool)),rfid,SLOT(receiveLogin(bool)),Qt::AutoConnection);
        //        QObject::connect(userManagement,SIGNAL(sendLogin(bool)),serial,SLOT(receiveLogin(bool)),Qt::AutoConnection);
        QObject::connect(userManagement,SIGNAL(sendLogin(bool)),w,SLOT(receiveLogin(bool)),Qt::AutoConnection);
        QObject::connect(sqlthreadbenz,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadbenz,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadbenz,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }

    else if(factory == "Haima")
    {
        QObject::connect(userManagement,SIGNAL(sendLogin(bool)),rfid,SLOT(receiveLogin(bool)),Qt::AutoConnection);
        QObject::connect(userManagement,SIGNAL(sendLogin(bool)),w,SLOT(receiveLogin(bool)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadbyd,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(sqlthreadbyd,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadbyd,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }
    else if(factory=="GM" /*||factory == "Haima"*/)
    {
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadbyd,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(sqlthreadbyd,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadbyd,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }
    else if(factory == "SVW2")
    {
        //        QObject::connect(&w,SIGNAL(closeThread()),tighten_xml,SLOT(restartDisconnect()),Qt::AutoConnection);
        if(ControlType_1 != "SB356"&&ControlType_1 != "SB356_PLC"&&ControlType_1 != "OFF"&&ControlType_1 != "PFD")
        {
            QObject::connect(work,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(signalOutOfRange()),w,SLOT(slotOutOfRange())) ;
            QObject::connect(tighten_xml,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(ReconnectSendOperate()),w,SLOT(receiveOperate()),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_xml,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(havedconnect(bool)),tighten_xml,SLOT(receivehavedconnect(bool)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(disconnectTellPortA()),tighten_xml,SLOT(portBSendPortA()),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(disconnectTellPortB()),work,SLOT(portAsendPortB()),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(sendErrorfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinserterror(QVariant)),Qt::AutoConnection);
        }

        if(isQRVerify)
        {
            serialThread *serial = new serialThread;
            QTimer::singleShot(0,serial,SLOT(initSerial()));
            QObject::connect(serial,SIGNAL(signalQRCodeFromSerial(QString)),w,SLOT(slotGetQRCode(QString))) ;
        }


        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadsvw2,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        //        QObject::connect(w,SIGNAL(sendOneGroupNok(QString ,QString *,QString *)),sqlthreadsvw2,SLOT(configOneGroup(QString,QString *,QString *)),Qt::AutoConnection);
        QObject::connect(sqlthreadsvw2,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        //        QObject::connect(w,SIGNAL(sendConfigureAll(int,int,int,int)),sqlthreadsvw2,SLOT(receiveConfigureAll(int,int,int,int)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int,int)),sqlthreadsvw2,SLOT(receiveNokAll(int,int)),Qt::AutoConnection);
    }
    else if(factory == "Dongfeng")
    {
        QObject::connect(serial,SIGNAL(alignCar(QString)),queueInterface,SLOT(delete_car(QString)),Qt::AutoConnection);
        QObject::connect(serial,SIGNAL(deleteCar(QString)),w,SLOT(receiveDelete(QString)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendToInter(QString)),queueInterface,SLOT(delete_car(QString)),Qt::AutoConnection);
        QObject::connect(queueInterface,SIGNAL(sendQueueError(int)),w,SLOT(receiveQueueError(int)),Qt::AutoConnection);

        if(ControlType_1 == "CS351"||(ControlType_1 == "Desoutter"))
        {
            QObject::connect(work,SIGNAL(send_mainwindow(QString,QString,QString,int,int)),w,SLOT(fromsecondthreaddata(QString,QString,QString,int,int)),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(IsTigntenReady(bool)),w,SLOT(TightenIsReady(bool)),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(ReconnectSendOperate()),w,SLOT(receiveOperate()),Qt::AutoConnection);
            QObject::connect(w,SIGNAL(sendOperate1(bool,int)),tighten_xml,SLOT(sendReadOperate(bool,int)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(havedconnect(bool)),tighten_xml,SLOT(receivehavedconnect(bool)),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(disconnectTellPortA()),tighten_xml,SLOT(portBSendPortA()),Qt::AutoConnection);
            QObject::connect(tighten_xml,SIGNAL(disconnectTellPortB()),work,SLOT(portAsendPortB()),Qt::AutoConnection);
            QObject::connect(work,SIGNAL(sendfromworkthread(QVariant)),sqlthreadsvw2,SLOT(sqlinsert(QVariant)),Qt::AutoConnection);

            QObject::connect(tighten_xml,SIGNAL(IsTigntenReady(bool)),queueInterface,SLOT(IsTightenReady(bool)),Qt::AutoConnection);  //*************
        }
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadsvw2,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(sqlthreadsvw2,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadsvw2,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }
    else if(factory == "BAIC")
    {
        QObject::connect(userManagement,SIGNAL(sendLogin(bool)),w,SLOT(receiveLogin(bool)),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadBAIC,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(sqlthreadBAIC,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadBAIC,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }
    else if(factory == "NOVAT" || factory == "KFCHERY")
    {
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),sqlthreadBAIC,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);
        QObject::connect(sqlthreadBAIC,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);
        QObject::connect(w,SIGNAL(sendNokAll(int)),sqlthreadBAIC,SLOT(receiveNokAll(int)),Qt::AutoConnection);
    }

    else if(factory == "GZBAIC")
    {
        QObject::connect(w,SIGNAL(sendfromsecondthread(QString ,QString,QString,int)),
                         sqlthreadGZBAIC,SLOT(configOne(QString,QString,QString,int)),Qt::AutoConnection);

        QObject::connect(sqlthreadGZBAIC,SIGNAL(send_mysqlerror()),w,SLOT(signal_mysqlerror_do()),Qt::AutoConnection);

    }

    ioBox *mIoBox = new ioBox;
    QObject::connect(w,SIGNAL(signalSetIoBox(int,bool)), mIoBox, SLOT(slotSetIoBox(int,bool)),Qt::AutoConnection);
    QObject::connect(mIoBox,SIGNAL(signalIoBoxInput(int)), serial, SLOT(slot_IoBoxInput(int)));
    QObject::connect(serial,SIGNAL(signalSetIoBox(int,bool)), mIoBox, SLOT(slotSetIoBox(int,bool)),Qt::AutoConnection);

    /*
      * QInputContext 屏幕键盘类
      * 修改第三方文件
    */
    QInputContext *im = new InputMethod();
    a.setInputContext( im );

    LightOperate* tmpLight = new LightOperate;
    QObject::connect(w,SIGNAL(sLightLogic(LightLogic)),tmpLight,SLOT(slot_LightLogicOperate(LightLogic)));

    return a.exec();
}

void QtSleep(unsigned int msec, QEventLoop::ProcessEventsFlags flags)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents(flags, 1);
#if defined(Q_OS_WIN)
        Sleep(1);
#elif defined(Q_OS_LINUX)
        QEventLoop eventloop;
        QTimer::singleShot(msec, &eventloop, SLOT(quit()));
        eventloop.exec();
#endif
    }
}

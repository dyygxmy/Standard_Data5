#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GlobalVarible.h"
#include <QSettings>
#include "newconfiginfo.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include "save.h"
#include "vinreverse.h"
#include <QTextCodec>
//#include "datamodel.h"
#include "deletecarsql.h"
//#include <QSharedMemory>
//#include <QBuffer>
//#include "baicinfointerface.h"
#include "CurrentStatus.h"
#include "location.h"
#include "./json/parser.h"
#include "inputcode.h"
#include "inputcode_aq.h"
#include "qextserial/qextserialport.h"
#include <QMessageBox>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#define CHANNEL_AMOUNT (4)

namespace Ui {
class MainWindow;
}

typedef enum
{
    e_SingleOK,
    e_SingleFail,
    e_GroupOK,
    e_GroupFail,
    e_ING,
    e_NotING,
    e_Enable,
    e_Disable,
    e_WaitForScan,
    e_ScanFinishThenWait,
    e_TightenDisConnect,
    e_TightenConnect,
    e_NetWorkConnect,
    e_NetWorkDisConnect,
    e_SingleSkip,

    e_PlcConnect,
    e_PlcDisConnect,

    e_DeviceError,
    e_PlcConnectAndReadError,

    e_AutoSearchVinNum,
    e_HandleCodeRFIDAlign,
    e_HandleCodeRFIDDongFengAlign,

    e_cleanDisplay,

}LightLogic;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
//    void closeEvent(QCloseEvent *event);
    void Show();

    bool FuzzyMatch(QByteArray pValue1, QByteArray pValue2);


public:
    void emitOperate1(bool enable,int flag);
    void emitOperate2(bool enable,int flag);
    void emitOperate3(bool enable,int flag);
    void emitOperate4(bool enable,int flag);
    void searchVinNum();                          //匹配vin
    void searchG9Num();                           //匹配G9
    void searchQRCodeNum( QString ) ;
    void judge();
    int VIN_VerifyFunc(char *buf);                //vin码校验
    static bool pinCodeVerify(QByteArray ,int);          //pin码校验
    static bool pinCodeRightVerify(QString ,int);
    void GZBAICVinPinCodeView(int whichcars);//匹配PDM
    void Start(int interval, int maxValue);
    void Stop();
//    void red_led(int);
//    void green_led(int);
//    void yellow_led(int);
//    void white_led(int);
//    void nok_led(int);
    void mysqlopen();
    void mysqlclose();
    void sendWebValue(int states,QString namepdm);
    void showhome();
//	void restartProgram();
    void initButton();
    void newconfigInit(Newconfiginfo *newconfiginfo);
    void searchJob(QString);
    void JobOffShow();
    void handleCodeBarcode_PIN(QString vincode, QString pincode,bool invalidFlag);
    void handleCodeBarcode(QString,bool);
    void handleCodeBarcode_NoJobID(QString,QString,bool);
    void handleCodeRFID(QString,QString,bool);
    void handleCodeBarcode_JOBID(QString,QString,bool);
    void startLine();
    void lablebatteryshow(int);//add lw 2017/9/19
    void setFTPMaster();

    bool GetTaotongNumCondition(int pCarInfoNum, int pTtNum);

public slots:
    void slotTest();
    void readCom();

//    void fromsecondthreaddata(QString,QString,QString);
    void fromsecondthreaddataIRMM(QString,QString,QString,int);
    void fromsecondthreaddata(QString,QString,QString,int);
    void fromsecondthreaddata(QString,QString,QString,int,int);
    void fromsecondthreaddata(QString MI1,QString WI1,QString IsOk1,QString MI2,QString WI2,QString IsOk2);
	void fromFtpJsonData(QString MI,QString WI,QString STATE,int Channel, int Screwid, int Program);  //多轴
    void getSerialNum(QString,bool,QString);
    void connectMysql();
    void init();
    void initBack();
    void wifishow(bool);
    void datashow(bool);
    void batteryshow1(QString);
    void batteryshow2(bool);
    void time_warning(bool);
//    void ReceGunNotReady();
//    void battery15();
    void closeSave();
 	void SB356Connect(bool);
    void nokOneBolt(int);
    void nokWhichBolt(int);
    void receiveJobError(int);
    void JobTimeout();
    void searchJobNum(QString);
    void searchJobBYD(QString);
    void receivePlusFlag(bool);
    void receiveJob(QString);
    void receiveQueueError(int);
    void autoSearchVinNum(QString);
    void revTagStatus(bool,int,QPointF,QPointF,QPointF,QPointF,QPointF,bool);//add lw 2017/9/19
    void lable_cor_battey_display(bool);
    void recvNewCode(QString);
    void leuzeTimerStart();
    void slotReplaceBarcode( QVariantMap ) ;
    void slotOutOfRange() ;
    void slot_cardTimer();
    void slot_light();

signals:
    void sDebugSkipTaotong(int pNum);
    void sLightLogic(LightLogic pLightLogic);
    void closeThread();
//    void sendoperate();
    void vinSendPLC(int,int,QString);
    void sendOperate1(bool,int);
    void sendOperate2(bool,int);
    void sendOperate3(bool,int);
    void sendOperate4(bool,int);
    void sendOneGroupNok(QString,QString *,QString *);
    void sendfromsecondthread(QString,QString,QString,int);
    void sendNokAll(int);
    void sendNokAll(int,int);
    void sendnexo(QString);
    void sendDeleteCar_VIN(QString);
    void SQL_deleteCar(QString,QString);
    void sendInfo(bool,bool,int,int);
    void sendError(bool,bool,bool);
	void killProcess();
    void gwkInit();
//    void requestJob(QString);
    void sendRequestJob(QString);
    void sendToInter(QString);
    void sendRunStatusToJson(QString);
    void sendControllerStatus(bool);
    void DisplayLocation(bool);
    void Send_tagcoor(QPointF);//add lw 2017/9/6
    void tagconnect(bool);

    void signalRedTwinkle( bool ) ;
    void sendCmdToCs351FromMW( int ) ;
    void signalDeleteLastRfid( ) ;

    void signalUpdateQueueSql() ;
    void signalSendBoltAmount(int) ;
    void skipBolt() ;
    void signalGetCarInfo(int,QString ) ;
    void signalGetCarInfoFast(QString,bool,int); //当数据库不存在当前车辆信息时，快速请求信息
    void signalSendTaotongNum() ;

    void signalSendHttp(int pType);
    void signalDetectCard(QString);

    void signalUploadVinVerify(int pResult);  //0:ok,1:err
    void signalUploadBeginTightening();
    void signalUploadStopPassby();  //passby:0
    void signalUploadStopTightening(int);

    void signalSetIoBox(int pIndex,bool power);

public slots:
    void TightenIsReady(bool);
    void TightenIsReady(int,bool);
    void slots_getSendVINResult(bool);
    void PLCIsReady(int);
    void receiveOperate();
    void cleanDisplay();
    void cleanBAICDisplay() ;
    //void on_pushButton_11_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_14_clicked();
    void on_pushButton_15_clicked();
    void on_pushButton_0_clicked();
    void ShowTime();
    void UpdateSlot();
    void PdmFlicker();//pdm flicker
//    void on_pushButton_18_clicked();
    void on_pushButton_reset_clicked();
    void receiveCloseInput(bool);
    void configwarning(bool);
    void taotong_main(int);
//    void shutdown(int);
    void clocked();
    void setRfidState(bool);
    void delete_car(bool);
    void modeChange();
    void VIN_coming(QString);
    void Wrong_Match_VIN(QString tmp1,QString tmp2);
    void displayLimit(QString,QString,QString);
    void displayStatusIcon(QString);    
    void wirelessPositionToEnable(bool toolEnable);
    void closelocation();
    //void on_pushButton_location_clicked();
    void slotGetQRCode(QString) ;
    void slotShowBolt(QByteArray,QString,QString,QString,int) ;
    void VinPinCodeView(bool flags,int whichcars);//vin or pin匹配函数

    void SetLsInfo(QString pProNum, QString pLsNum, QString pTtNum);


    void slot_PLCHeartbeat(bool pIsHeartbeat);
    void slot_getTightenID(QString);

private slots:
    void on_pushButton_17_clicked();
    void signal_mysqlerror_do();
//    void on_pushButton_shutdown_clicked();
    void ReceFisSerial(QString);
    void QueueTimerDo();
    void FisTimerDo(); //当前条码打完 延时
    void leuzeTimerDo();
    void UpdateSqlFlag();
    void receiveGetCar();
    void getAlign(QString,QString);
    void resetUiDo();//RFID模式 ING 状态确认

    void on_pushButton_16_clicked();

    void receiveDelete(QString);
    void send_Info();

    void on_pushButton_tiaoma_clicked();

    void on_pushButton_18_clicked();

    void on_pushButton_11_clicked();
    void receiveLogin(bool);

    void on_pushButton_align_clicked();
    void risingEdge();    //上升沿
    void fallingEdge();   //下降沿
    void gunPower(bool);   //条码枪通电断电

    void on_pushButton_JobOff_clicked();
    void on_pushButton_location_clicked();
//    void slotRedTwinkle( bool ) ;
    void slotHideLabelStatus() ;
    void slotHideLabelQRVerify() ;
    void showPDM();
    void slotLedTimerOut();

    void on_btnAdjustQueue_clicked();
    void slotGetQueueNum(QString,QString);

    void on_btnSkipOne_clicked();

    void on_btnSkipAll_clicked();

    void recvClose();
    void recvCode(QString);

//    void on_btn_taotongskip_clicked();


    void on_senLabelValue_clicked();
    void slot_postLabelResult();//循环请求Label结果
    void slot_alarmFlashFunc();

private:
    Ui::MainWindow *ui;
    InputCode *inputCode;
    Inputcode_AQ *inputCode_AQ;
    QNetworkAccessManager *manager;
    QString message;
    QString serialNums;
    QString tightenID;
    QString tightenID_temp;
    //QString tempPin;
    QString tempG9;
    QByteArray Data_Xml_Txheart;
    int isFull;
    QString temp;
    QString PDM_PATH;
    int person;
    int optionOrNot;  //是否选配
    int whichar;  //匹配出来的是哪个车型
    int enableLsnumber;
    int whichpronumis;//当前程序号
    int whichoption;
    QSqlDatabase db;
    QSqlQuery query;
    QSqlQuery query1;
    QSqlRecord record;
    QSqlField field;
    int  equeloptionbnum;
    int  equeloptionknum;
    QString selectVin;//sql
    int bxuannumtmp;
    QString PDMCurrentState[4];
    bool pdmnowromisOk;
    int m_CurrentValue;         //当前值
    int m_UpdateInterval;       //更新间隔
    int m_MaxValue;             //最大值
    QTimer m_Timer;
    QTimer *postTimer;
    QStringList boltSNList;
    QTimer timerpdm;
    QTimer shutdown_timer;
//    QTimer timer_showdown;
    QTimer FisTimer;
    QTimer leuzeTimer;
    QTimer timer_Info;
    QTimer ledTimer ;   //灯闪烁
    QTimer alarmFlash;//高风险闪烁
    bool alarmFlashed;
    bool flashIni;
    int numpdm;
    QPushButton *butt[4][50];
    QLabel *label1[4][50];
    QLabel *label2[4][50];
    QWidget *widget_channel[CHANNEL_AMOUNT];
    QPushButton *pushButton_channel[CHANNEL_AMOUNT][10]; //机械手多通道
    QLabel *label1_channel[CHANNEL_AMOUNT][10]; //机械手多通道
    QLabel *label2_channel[CHANNEL_AMOUNT][10]; //机械手多通道

    int   mIoBox[CAR_TYPE_AMOUNT];
    QPushButton *btnCarType[CAR_TYPE_AMOUNT] ;  //用于广州北汽选择车型

    int Channel_num[CHANNEL_AMOUNT];
    int Channel_Screwid[CHANNEL_AMOUNT];
    int Channel_time[CHANNEL_AMOUNT];

    int tempnumdpm[4]; //螺丝个数
    bool pdmflicker; //闪烁状态
    int whichpdmnumnow[4]; //当前第几个闪烁
    int Tacktime;
    int ScrewWhichExit;
    int battry_num;
    int bolt_index;//一组螺栓的最后一个
    QTimer queue_timer;
    bool controlMode;   //false 自动  true 手动
    QString DeleteVIN;
    QString ButtonStatus[5];
    QString VINhead;
    int restart_times;

    int PDMBoltStute[5];
    int PDMBoltBuf[5];
    int PDMBoltNum;
    int currentBoltSum;
    int currentBoltNum;
    int currentFirstBolt;

    int groupAllBoltNumCh[4];
    int allChannel;
    bool sendEnableFlag;
    int systemStatus;
    int preSystemStatus;

    int vinAttributeBit;
    int vinAttributeLen;
    QString vinAttributeCode;
    int linkCount;
    QTimer JobTimer;
    QString Factory;
    bool isRFIDConnected;
    bool IsQueue;   //private
    bool m_BAICQueue ;
    int searchJobTimes;
    QString searchJobSerialNum ;
    bool isLogin;
	int pdmflickerNum;
    bool ttChangeFlag;
    QString StationName;
    bool isFirst;   //是否第一次连上
//    QSharedMemory sharedMemory;
    QString DeleteIDCode;
    bool isSaveShow;
//    BaicInfoInterface *baicInterface;
    int StartBolt;
    bool allTightenResultFlag;
    QByteArray BYDType;
    bool changeTemp;
    bool isAlign;
    bool TagTimeout;
    bool first_timeout_flag;
	QPointF POINT_A,POINT_B,POINT_C,POINT_D;
    QList<QString> codeList;

    bool redLedWorked ; //红灯亮
    bool progressBarStop ; //是否定时器结束触发的跳过全部
    bool progressBarAll ; //是否定时器结束触发的跳过全部
    bool socketSpannerFault ;  // 是/否是因为套筒选择功能导致非使能
    bool nextCarAll ;   //是否下一辆车进站触发跳过全部
    bool nextCarCatch;   //是否下一辆车进站触发跳过全部
//    QString strQRCode;
    bool rfidFlag ;   //RFID校验是否通过
    int RfidNum ;        //第几颗螺栓
    bool flagQR ;       //是否启用零件校验
    QString strQRCodeRule ; //配置的零件码校验规则
    int G9Len ;  //G9码长度
    int boltAmount; //螺栓个数 广州北汽
    QString carType; //车型名称

    int cleanBolt;  //需要清除（delete）的螺栓个数
    bool firstUse ; //首次使用（即无需清除）
    bool firstBolt; //本辆车第一颗螺栓
    int currBolt ;  //当前螺栓号
    int whichCarType ;  //当前车型
    bool carNOK ;   //当前车辆螺栓状态（一颗螺栓不合格即置false）
    QString boltStatus ;    //上一颗螺栓螺栓状态
    QString strExecCatType; //该工位可通过车型
    bool ledStatus ;
    QString carTypeHaima;
    int carTypeNum ; //车型总数
    int optionalConfigurationTime ; //选配失败次数
    bool cflags ;

    QString AQ_carType;
    QString AQ_JOBID;
    QString mVinjob;
    bool  mPartCode;
    int whichindex;
    QStringList seriallist;

private:
    void initCarType() ;
    void showLabelStatus(int,int,int);
    int boltIndex;
    int labelValue;

public:
    Newconfiginfo * newconfiginfo;
    VinReverse * vinreverse;
    DeleteCarSQL * deletecarsql;
    QGraphicsOpacityEffect *e3;
    QGraphicsOpacityEffect *e3_flash;
    Save * save;
    bool ConfigOneOrAll;
    QString variable_1;
    Location *location_temp;

    bool redTwinkle; //进度跳即将结束红灯是否闪烁
    int redTwinkleTime; //红灯闪烁持续时间
    bool wifiWarning ;  //开启断网警告，四灯闪烁
    bool manuslOperationAdjust;
    int vinLen;

    bool cutEnableFlag; //节拍时间到，是否切使能

    QString mDeviceNo;
    QTimer* cardTimer;
};


#endif // MAINWINDOW_H

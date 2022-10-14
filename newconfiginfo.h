#ifndef NEWCONFIGINFO_H
#define NEWCONFIGINFO_H

#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPushButton>
#include <QListWidgetItem>
#include <QDir>
#include <QStringList>
#include "optiondialog.h"
#include <QGraphicsDropShadowEffect>
#include "save.h"
#include "basicset.h"
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QHeaderView>
#include <QList>
#include "paintarea.h"
#include "basestation.h"
#include "locationparsejson.h"
#include "locationconfig.h"
#include <QComboBox>
#include "./json/parser.h"
#include "./json/serializer.h"
#include "unistd.h"
#include "GlobalVarible.h"

//#include <QSqlQuery>

#define INAStyle0  "background-color: rgb(237, 28, 36);color: rgb(248, 248, 248);font: 14pt \"黑体\";"

namespace Ui {
class Newconfiginfo;
}

class Newconfiginfo : public QDialog
{
    Q_OBJECT
    
public:
    explicit Newconfiginfo(QWidget *parent = 0);
    ~Newconfiginfo();

    void Show();

    void initui();
    void setinitUi(int);
    void initLight();
    void InitLightState(QString pLogicState,QComboBox* pRedCombo,QComboBox* pGreenCombo,QComboBox* pYellowCombo,QComboBox* pWhiteCombo,QComboBox* pkeyCombo);
    void updownReadOperate(int);  // 1 up   0 down
    void updownWriteOperate(int); // 1 up   0 down
    void moveDo();  //pushbutton_right do something
    void warnings();
    void buttclicked();
    void xuanchoux(int);
    void judge();
    void baseInfoIsChange();
    void advancedIsChange();
    void masterslaveIsChange();
    void locationIsChange();
    void wifi_connect();
    void backShow(); //取消之后的效果
    void pagechange();//历史查询翻页
    void historyclear();
    void savePDM();
    void clearCache();
    void bound_show();
    void bound_save();
    void boundIsChange();
    void show_bound();
    void bound_init();
    void bound_update();
    void history();
    void PDMEdit();
    void configList();
    void systemConfigure();
    void restartShow(bool);
    void queryResult(QString);
    void setRepair(bool);

    void OptionMoveDo();

    void ResetModel(void);

    void ReinitOptionState();

signals:
    void closeconfig();
    void sendGetTime();
    void xmlcreate();
    void column_update(QString);
    void sendTruncateResult(bool);
    void sendTruncateQueueResult(bool);
    void sendRepairVIN(QString);
    void sendfromworkthread(QVariant);
	void SendCalibration();

public slots:
    void receivetime(QString);
    void receiveoptioninfo(QString,QString,bool);
    void on_pushButton_clicked();
    void receiveSaveState(bool);
    void on_listWidget_currentRowChanged(int currentRow);
    void receivecancelpdm();
    void receivebaseinfocancel();
    void receiveBaseinfo(QString,QString,QString,QString);
    void receiveBaseinfoSave(bool);
    void pdmSelect(QListWidgetItem*);
    void pdminit();
    void receiveMasterSlaveState(bool);
    void receiveDebug(QString);


    void on_pushButton_59_clicked();

    void on_pushButton_60_clicked();

    void on_pushButton_63_clicked();

    void on_pushButton_64_clicked();

    void on_pushButton_65_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_butt1_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_xuan1_clicked();

    void on_pushButton_xuan2_clicked();

    void on_pushButton_xuan3_clicked();

    void on_pushButton_xuan4_clicked();

    void on_pushButton_xuan5_clicked();

    void on_pushButton_20_clicked();

    void on_pushButton_18_clicked();

    void on_pushButton_19_clicked();

    bool eventFilter(QObject *, QEvent *);

    void on_pushButton_28_clicked();

    void on_pushButton_29_clicked();

    void on_pushButton_30_clicked();

    void on_pushButton_31_clicked();

    void on_pushButton_32_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_33_clicked();


    void on_comboBox_2_currentIndexChanged(const QString &arg1);

    void on_pushButton_24_clicked();

    void on_pushButton_23_clicked();

    void on_pushButton_25_clicked();

    void on_pushButton_52_clicked();

    void on_pushButton_17_clicked();



    void on_pushButton_butt2_clicked();

    void on_pushButton_butt3_clicked();

    void on_pushButton_butt4_clicked();

    void on_pushButton_butt5_clicked();

    void on_pushButton_62_clicked();

    void on_pushButton_26_clicked();

    void on_pushButton_27_clicked();

    void on_pushButton_34_clicked();


    //    void on_pushButton_38_clicked();

    //    void on_pushButton_51_clicked();

    //    void on_pushButton_37_clicked();

    //    void on_pushButton_50_clicked();

    //    void on_pushButton_87_clicked();

    //    void on_pushButton_88_clicked();

    void on_pushButton_61_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();



    void ShowTime();

    void on_pushButton_68_clicked();

    void on_pushButton_69_clicked();

    void on_pushButton_80_clicked();

    void on_pushButton_85_clicked();

    void on_pushButton_81_clicked();

    void on_pushButton_92_clicked();

    void on_pushButton_82_clicked();

    void on_pushButton_93_clicked();

    void on_pushButton_83_clicked();

    void on_pushButton_94_clicked();

    void on_pushButton_84_clicked();

    void on_pushButton_95_clicked();

    void on_pushButton_58_clicked();

    void on_pushButton_66_clicked();

    void on_pushButton_num1_clicked();

    void on_pushButton_num2_clicked();

    void on_pushButton_num3_clicked();

    void on_pushButton_num4_clicked();

    void on_pushButton_num5_clicked();

    void on_pushButton_num6_clicked();

    void on_pushButton_num7_clicked();

    void on_pushButton_num8_clicked();

    void on_pushButton_num9_clicked();

    void on_pushButton_num0_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_98_clicked();

    void on_pushButton_97_clicked();

    void on_pushButton_100_clicked();




    void on_pushButton_bxuan1_clicked();

    void on_pushButton_bxuan2_clicked();

    void on_pushButton_bxuan3_clicked();

    void on_pushButton_bxuan4_clicked();

    void on_pushButton_bxuan5_clicked();

    void on_pushButton_kxuan1_clicked();

    void on_pushButton_kxuan2_clicked();

    void on_pushButton_kxuan3_clicked();

    void on_pushButton_kxuan4_clicked();

    void on_pushButton_kxuan5_clicked();

    void on_pushButton_89_clicked();

    void on_pushButton_90_clicked();

    void receiveDesignle(bool);

    void on_pushButton_search_clicked();

    void on_pushButton_first_clicked();

    void on_pushButton_last_clicked();

    void on_pushButton_previous_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_pronum_add_clicked();

    void on_pushButton_pronum_minus_clicked();

    void on_pushButton_number_add_clicked();

    void on_pushButton_number_minus_clicked();

    void on_pushButton_iobox_add_clicked();
    void on_pushButton_iobox_minus_clicked();

    void on_pushButton_ioBoxOption_add_clicked();
    void on_pushButton_ioBoxOption_minus_clicked();

    void on_pushButton_xuanpronum_add_clicked();

    void on_pushButton_xuanpronum_minus_clicked();

    void on_pushButton_xuannumber_add_clicked();

    void on_pushButton_xuannumber_minus_clicked();


    void on_pushButton_67_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_36_clicked();

    void on_pushButton_21_clicked();

    void on_pushButton_86_clicked();

    void on_radioButton_2_clicked();

    void on_radioButton_1_clicked();

    void on_pushButton_37_clicked();

    void on_pushButton_38_clicked();

    void receiveBound(bool);

    void mysqlTruncate();

    void mysqlTruncateQueue();

    void receiveRepairReply(int,QVariantMap);
    void closeBaseStation();
    void closeLocationConfig();
    void receiveAnchor(QVariantMap);
    void receiveDataConfig(QVariantMap);

public slots:
    void on_pushButton_49_clicked();

    void on_pushButton_taotong_add_clicked();

    void on_pushButton_taotong_minus_clicked();

    void on_pushButton_50_clicked();

    void on_Line_radioButton_1_clicked();

    void on_Line_radioButton_2_clicked();

    void on_pushButton_51_clicked();

    void on_pushButton_104_clicked();

    void on_pushButton_55_clicked();

    void on_pushButton_tens_add_clicked();

    void on_pushButton_tens_minus_clicked();

    void on_pushButton_ones_add_clicked();

    void on_pushButton_ones_minus_clicked();

    void on_pushButton_88_clicked();

    void on_pushButton_87_clicked();

    void on_pushButton_bound_prev_clicked();

    void on_pushButton_bound_next_clicked();

    void on_pushButton_bound_clicked();

    void on_pushButton_56_clicked();

    void on_pushButton_queue_clicked();

    void on_Led_red_clicked();

    void on_Led_green_clicked();

    void on_Led_yellow_clicked();

    void on_Led_white_clicked();


    void on_IO_Reset_clicked();

    void on_Led_nok_clicked();
	
	void on_pushButton_197_clicked();

    void on_pushButton_198_clicked();

    void on_pushButton_199_clicked();

    void on_pushButton_196_clicked();

    void on_pushButton_201_clicked();

    void on_pushButton_200_clicked();

    void on_pushButton_restart_clicked();

    void on_pushButton_channel_add_clicked();

    void on_pushButton_channel_minus_clicked();

    void on_pushButton_22_clicked();


    void on_pushButton_channel_add_2_clicked();

    void on_pushButton_channel_minus_2_clicked();

    void on_pushButton_taotong_add_2_clicked();

    void on_pushButton_taotong_minus_2_clicked();

    void on_pushButton_pronum_add_history_clicked();

    void on_pushButton_pronum_minus_history_clicked();

//    void on_pushButton_search_threeDays_clicked();

//    void on_pushButton_search_oneWeek_clicked();

//    void on_pushButton_search_oneMonth_clicked();

//    void on_pushButton_101_clicked();

//    void on_pushButton_96_clicked();

//    void on_pushButton_search_2_clicked();

    void on_pushButton_down_clicked();

    void on_pushButton_up_clicked();

    void on_pushButton_threeDays_clicked();

    void on_pushButton_oneWeek_clicked();

    void on_pushButton_oneMonth_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_pushButton_repair_clicked();

    void on_pushButton_history_clicked();

    void on_pushButton_historyReturn_clicked();

    void repairSearch();

    void on_checkBox_isRepair_clicked(bool checked);

    void on_pushButton_repairClear_clicked();

    void on_pushButton_repairUpload_clicked();

    void on_pushButton_wirelessLocation_clicked();

    void on_pushButton_Anchor_append_clicked();

    void on_pushButton_Anchor_update_clicked();

    void on_pushButton_Anchor_remove_clicked();

    void on_pushButton_Anchor_insert_clicked();

    void on_pushButton_saveLocation_clicked();

    void saveLocation(bool);
    void on_pushButton_dataConfig_append_clicked();

    void on_pushButton_dataConfig_insert_clicked();

    void on_pushButton_dataConfig_update_clicked();

    void on_pushButton_dataConfig_remove_clicked();

    void on_radioButton_locationMaster_clicked();

    void on_radioButton_locationSlave_clicked();

    void on_radioButton_locationOff_clicked();

	void on_pushButton_Calibration_clicked();

    void Cell_Calibration_status(int );

    void Receiveoffsetlist(QVariantList);

    void on_targetNumRadio_clicked();

    void on_maxNumRadio_clicked();

    void on_editQRCodeRule1_textChanged(const QString &arg1);

    void on_editQRCodeRule2_textChanged(const QString &arg1);

    void on_editQRCodeRule3_textChanged(const QString &arg1);

    void on_editQRCodeRule4_textChanged(const QString &arg1);

    void on_editQRCodeRule5_cursorPositionChanged(int arg1, int arg2);

    void on_btnIOOut5_clicked();

    void on_btnIOOut6_clicked();

    void on_btnIOOut7_clicked();

    void on_btnSerial1_clicked();

    void on_btnSerial2_clicked();

    void on_btn_testLight_pressed();

    void on_btn_testLight_released();

    void on_pushButton_restart_2_clicked(bool checked);

public:
    QString strUpInversion;

private slots:
    void on_pushButton_ledSetting_clicked();

    void on_pushButton_ledSetting_back_clicked();

    void on_Line_radioButton_0_clicked();

    void on_btn_option_left_clicked();

    void on_btn_option_right_clicked();

private:
    Ui::Newconfiginfo *ui;
    // Ui::OptionDialog * uioption;
    bool isbarcode;
    bool isrfid;
    bool isqueue;
    int line_ID;
    int pagenum;
    int optionpagenum;
    int whichcar; //哪个车型被选中
    int isedit;
    int partY1V;      //数据存储局部变量
    int currentpages;
    bool optionIscheck;
    int whichoption;
    QPushButton *butt1[100];
    int numpdm;
    int temppdm ;
    QString pathpdm;

    QString  filename;
    int ispdminit;
    bool  ifcansave;
    int nowpdmnum;
    int nowcomboxnum;
    QString whichpdmyinyong;
    QString bxuanname;
    QString bxuancode;
    QString kxuanname;
    QString kxuancode;
    int bxuanpeinum;
    int kxuanpeinum;
    bool isxuanlook;
    bool isSavedpdm;
    int previouswhichar;
    OptionDialog *od;
    QGraphicsOpacityEffect *e3;
    int bxuanwhich; //哪个必选装件被选中
    bool bxuanstate; //比选还是可选
    QString bxuannamelist[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString bxuancodelist[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString kxuannamelist[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString kxuancodelist[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString lsnumersxuanlist[D_CAR_OPTION_NUM][D_BOLTNUM];//数量
    QString luoxuanlist[D_CAR_OPTION_NUM][D_BOLTNUM];//暂存螺栓编号
    QString proxuanlist[D_CAR_OPTION_NUM][D_BOLTNUM];//程序号
    QString channelxuanlist[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString taotongxuanlist[D_CAR_OPTION_NUM][D_BOLTNUM];

    QString bxuannamelist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString bxuancodelist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString kxuannamelist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString kxuancodelist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString lsnumersxuanlist2[D_CAR_OPTION_NUM][D_BOLTNUM];//数量
    QString luoxuanlist2[D_CAR_OPTION_NUM][D_BOLTNUM];//暂存螺栓编号
    QString proxuanlist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString channelxuanlist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString taotongxuanlist2[D_CAR_OPTION_NUM][D_BOLTNUM];
    QString PDMxuan_Name[D_CAR_OPTION_NUM][4];
    QString PDMxuan_Name2[D_CAR_OPTION_NUM][4];
    QString QRCodeRuleStr[D_CAR_OPTION_NUM];
    QPushButton  *buttonbox[5];
    QLineEdit    *editBox[5] ;

    int ioBoxOption[D_CAR_OPTION_NUM];

    QString carStyle;
    QString mIoBox;
    QString G9tmp ;
    QString Vintmp;
    QString QRtmp ;
    QString strExecCarType ;
    bool Out[4];
    QString PDM_Name[4];
    QString PDM_Name2[4];
    QString luo[D_BOLTNUM];//暂存螺栓编号
    QString pro[D_BOLTNUM];//程序号
    QString lsnumers[D_BOLTNUM];//数量
    QString luo2[D_BOLTNUM];//暂存螺栓编号
    QString pro2[D_BOLTNUM];//程序号
    QString lsnumers2[D_BOLTNUM];//数量
    QString taotong[D_BOLTNUM];
    QString taotong2[D_BOLTNUM];
    QString channel[D_BOLTNUM];
    QString channel2[D_BOLTNUM];
//    QString pdmxuanlist[D_BOLTNUM];
//    QString pdmxuanlist2[D_BOLTNUM];
    QString pdmxuan;

    int isFull;
    QString temp;
    int person;
    int change;
    QString newpassword;
    Save * save;
    BasicSet *basicset;

    int year;
    int month;
    int date;
    int hour;
    int minute;
    int second;

    QString controllerIp_01;
    QString controllerIp_02;
    QString PortA;
    QString PortB;
    QString RfidIp;
    QString RfidPort;
    QString DataServerIp;
    QString CurveServerIp;
    QString AndonServerIp;
    QString ServerPort;
    QString BarcodeGun;
    bool isbaseinfochange;
    bool isadvancedchange;
    bool ismasterslavechange;
    bool islocationchange;
    QString  whichButtonClick;

    int pages;
    int thepage;
    QString thepages;
    QString affall;
    QString aff;
    QSqlQueryModel *model;
    QString MysqlMin;

    QSqlDatabase        mDataBase;
    QSqlQuery           *query;
    QSqlQuery           *query1;

    QString VIN;
    QString ScrewID;
    QString TighteningStatusNOK;
    QString TighteningStatusManual;
    QString Programno;

    QString condition;
    QString condition1;
    int new_pronum;
    int new_number;
    bool isoptionsaved;
    bool isoption;
    bool Selectsecond;
    int Totalnum;
    int numRows;
    float Nok_percent;

    QString bound[100][4];
    int PronumNow;
    bool isboundchange;
    QString bound_temp[100][4];
    int bound_pages;
    int bound_current_page;
    QList<int> list;
    QList<int>::iterator iter;
    int current;
    QStringList header_vertical;
    QTableWidgetItem *tableWidgetItem[10][5];
    bool bound_enabled_temp;
    bool restart_enabled_temp;

    PaintArea *area;
    QSqlQuery query_number;
    QSqlQuery query_datas;
    QSqlQuery query_bound;
    QString Factory;
    void initRepairTable();
    void setRepairData();
    QVariantList repairDataList;
    void setColor(QString);
    void initAnchorTable();
    void initDataConfigTable();
    int anchorInsertRow;
    int dataConfigInsertRow;
    void readLocation(QVariantMap);
    BaseStation *baseStation;
    LocationConfig *locationConfig;
    void newBaseStation();
    void newLocationConfig();
    QVariantList anchorList;
    QVariantList dataList;
    QVariantList offsetList;
    QVariantMap currentLocation;
    enum anchorOperate{anchorAdd,anchorInsert,anchorUpdate};
    enum dataConfigOperate{dataConfigAdd,dataConfigInsert,dataConfigUpdate};
    int currentAnchorOperate;
    int currentDataConfigOperate;
    void updateAnchorItem0();
    void updateDataConfigItem0();
    void newSaveLocation();

};

#endif // NEWCONFIGINFO_H

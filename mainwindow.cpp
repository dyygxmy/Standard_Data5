#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QTimer>
#include "serialComs.h"
#include "taotong.h"
#include "mesuploadAQCHERY.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    this->setGeometry(QRect(0, 0, 1366, 768));
    manager = new QNetworkAccessManager(this);
    ui->LabelResult_color->hide();
    ui->LabelResult_text->hide();
#if DYTEST4^1
    ui->lineEdit_labelValue->hide();
    ui->senLabelValue->hide();
#endif
    alarmFlashed = false;
    connect(&alarmFlash,SIGNAL(timeout()),this,SLOT(slot_alarmFlashFunc()));
    tightenID = "";

    postTimer = new QTimer(this);
    if(factory == "SVW2" && (Station == "076R-1" || Station == "080R" || Station == "085R-1" || Station == "093R-2")){
        connect(postTimer,SIGNAL(timeout()),this,SLOT(slot_postLabelResult()));
    }


    gFisVinIndex = 0;

    for(int i = 0;i<CAR_TYPE_AMOUNT; i++)
    {
        btnCarType[i] = new QPushButton(ui->stackedWidget_2) ;
        btnCarType[i] -> setVisible(false);
        btnCarType[i] -> setFixedWidth(80);
        btnCarType[i] -> setStyleSheet("font: 14pt \"黑体\";color: rgb(248, 248, 255);"
                                                     "border-style: solid;background-color: rgb(51,153,255);");

        connect(btnCarType[i],SIGNAL(clicked()),SLOT(showPDM()));
    }

    boltIndex = 0;//
    labelValue = 100;//100 null value

    whichindex = 0;
    progressBarStop = false ;
    progressBarAll  = false ;

    cutEnableFlag = false;
    AQ_carType = "";
    nextCarAll = true ;
    nextCarCatch = true ;
    redLedWorked = false ;
    redTwinkle   = false ;
    redTwinkleTime = 10 ;
    socketSpannerFault = false;
    ledStatus = false ;
    ui->stackedWidget_2->setCurrentIndex(0);
    ui->stackedWidget_6->setCurrentIndex(0);
    IsQueue = isQueue;
    m_BAICQueue = BAICQueue;
    Factory = factory;
    controlMode = false;
    TagTimeout  = false;
    first_timeout_flag = true;
    strQRCodeRule = "" ;
    currBolt = 0 ;
    G9Len = 4 ;
    carType = "" ;
    carNOK = false ;
    optionalConfigurationTime = 1 ;
    if((ControlType_1 == "SB356")||(variable1 == "op")||(variable1 == "xml") || (ControlType_1 == "SB356_XML"))
    {
        if(Factory != "BAIC")
        {
            initButton();
        }
    }

    init();
    showhome();
    this->setFixedSize(1366,768);
    //    this->setWindowTitle("B.I.W @Data2.1.0");
    connectMysql();
    ui->stackedWidget->setCurrentIndex(0);
    optionOrNot = 0;
    whichar = 0;
    whichpronumis = 1;
    equeloptionbnum= 0;
    equeloptionknum= 0;
    pdmnowromisOk = true;
    numpdm = 0;
    pdmflicker = true;
    battry_num= 0;
    enableLsnumber = 0;
    DeleteVIN = "";
    restart_times = 0;
    firstBolt = true ;
    ui->label_black->hide();
    ui->pushButton_reset->hide();
    ui->pushButton_17->hide();

    //    QTimer *timer=new QTimer(this);
    //    connect(timer,SIGNAL(timeout()),this,SLOT(ShowTime()));  //timer Reset
    gunPower(false);
    //    timer->start(100);
    ScrewWhichExit = 0;
    ui->progressBar->setValue(0);
    // ui->label_9->hide();
    // ui->label_ss2->hide();
    ui->label_wifi->hide();
    ui->label_wifi_2->hide();
    ui->labelReplaceStatus->hide();   //条码替换开启时用来显示条码替换状态
    ui->labelReplaceStatus->setAlignment(Qt::AlignCenter);
    ui->labelQRVerify->setVisible(false);       //宁波大众的零件校验功能，目前是安全带校验
    ui->labelError->setVisible(false);
    ui->btnAdjustQueue->setVisible(false);
    ui->editQueueVin->setVisible(false);
    ui->btnSkipAll->setVisible(false);
#if 1
    ui->btnSkipOne->setVisible(false);//跳过一个按钮
#endif

    if(!battery)
    {
        //        ui->label_battery->hide();
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2_white.png);background:transparent;");
    }
    lable_cor_battey_display(true);//add lw 2017/9/19

    m_CurrentValue = m_MaxValue = m_UpdateInterval = 0;
//    connect(this, SIGNAL(signalRedTwinkle(bool)), this, SLOT(slotRedTwinkle(bool))) ;
    connect(&m_Timer, SIGNAL(timeout()), this, SLOT(UpdateSlot()));
    connect(&timerpdm, SIGNAL(timeout()), this, SLOT(PdmFlicker()));
    connect(&ledTimer, SIGNAL(timeout()), this, SLOT(slotLedTimerOut()));
    connect(&shutdown_timer,SIGNAL(timeout()),this,SLOT(clocked()));
    //    connect(&timer_showdown,SIGNAL(timeout()),this,SLOT(battery15()));
    connect(&FisTimer,SIGNAL(timeout()),this,SLOT(FisTimerDo()));
    connect(&leuzeTimer,SIGNAL(timeout()),this,SLOT(leuzeTimerDo()));
    connect(&timer_Info,SIGNAL(timeout()),this,SLOT(send_Info()));
    timer_Info.start(500);
//    yellow_led(1);
//    red_led(0);
//    green_led(0);
//    white_led(0);
    if((ControlType_1 == "Desoutter"&& Factory!="BYDSZ" && Factory!="BYDXA")||(ControlType_1=="CS351" && Factory!="BYDSZ" && Factory!="BYDXA"))
    {
        vinreverse = new VinReverse;
        QTimer::singleShot(0,vinreverse,SLOT(newconnects()));
        connect(this,SIGNAL(sendnexo(QString)),vinreverse,SLOT(receiveVin(QString)));
    }

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);

    cutEnableFlag  = configIniRead->value("baseinfo/cutEnableFlag").toInt() ;
    mPartCode = configIniRead->value("baseinfo/PartCode").toBool();  //AQ two specialstation
    vinAttributeBit = configIniRead->value("baseinfo/vinAttributeBit").toInt();  //vin verify code start index
    vinAttributeLen = configIniRead->value("baseinfo/vinAttributeLen").toInt(); //vin verify code length
    StationName = configIniRead->value("baseinfo/StationName").toString();
    redTwinkle      = configIniRead->value("baseinfo/redTwinkle").toBool() ;
    redTwinkleTime  = configIniRead->value("baseinfo/redTwinkleTime").toInt() ;
    wifiWarning     = configIniRead->value("baseinfo/wifiWarning").toInt() ;
    manuslOperationAdjust = configIniRead->value("baseinfo/manuslOperationAdjust").toBool() ;
    vinLen = configIniRead->value("baseinfo/vinLen").toInt();
    QString vinHead = configIniRead->value("baseinfo/vinHead").toByteArray();
    bool manuallySkip = configIniRead->value("baseinfo/manuallySkip").toBool() ;
    mDeviceNo = configIniRead->value("baseinfo/DeviceNo").toString();
    for(int m=0;m<CAR_TYPE_AMOUNT;m++)
    {
        bool ok = false;
        int tmpIoBox = configIniRead->value(QString("carinfo%1/ioBox").arg(m+1)).toInt(&ok);
        if(ok)
            mIoBox[m] = tmpIoBox;
        else
            mIoBox[m] = 0xff;
    }
    delete configIniRead;

    vinAttributeCode = "";
    ui->pushButton_align->hide();
    ui->pushButton_JobOff->hide();

    if(manuallySkip)
    {
        ui->btnSkipAll->setVisible(true);
        ui->btnSkipOne->setVisible(true);
    }

    if(manuslOperationAdjust)
    {
        ui->btnAdjustQueue->setVisible(true);
        ui->editQueueVin->setVisible(true);
    }

    if((Factory == "Dongfeng")) // || (Factory == "SVW2")
    {
        ui->label_11->setText(tr("通信"));
        G9Len = 5 ;
    }

    if(Factory=="BYDSZ"||Factory=="BYDXA"||Factory=="Haima")
    {
        DTdebug() << Factory ;

        if( Factory == "Haima" && StationName.contains("分装线"))        //分装线条码长度18位
        {
            VINhead = vinHead ;
        }
        else {
            VINhead="LC0";
            vinAttributeBit = 3;
        }
        if(Factory == "BYDSZ" && ( StationName =="QFCJ"|| StationName =="FDJ"))
        {
            ui->pushButton_align->show();
            ui->pushButton_align->setGeometry(120,529,51,51);
        }
    }
    else if(Factory=="SVW3"||Factory=="Ningbo" ||Factory =="SVW2")
    {
        //        if(isReplaceBarcode)
        //            VINhead="551";
        //        else
        VINhead="LSV";
        vinAttributeBit = 3;
    }
    else if(Factory == "Haima")
    {
        if(vinLen == 18)        //分装线条码长度18位
        {
            VINhead = vinHead ;
        }
        else {
            VINhead="C";
            vinAttributeBit = 1;
        }
    }
    else if(Factory == "NOVAT")
    {
        VINhead  = "LTW";   //vinHead;
    }
    else if(Factory == "KFCHERY")
    {
        VINhead  = "LVU";   //vinHead;
    }
    else
    {
        VINhead = vinHead ;
    }

    if(Factory=="GM")
    {
        ui->label_8->setText("扳 手");
        ui->pushButton_12->show();
        ui->stackedWidget_4->setCurrentIndex(1);
        ui->widget_pro->setGeometry(ui->widget_pro->x(),ui->widget_pro->y()-2,ui->widget_pro->width(),ui->widget_pro->height());
        ui->label->setGeometry(0,1,81,29);
        ui->label_pronum->setGeometry(86,1,63,29);
        ui->label->setText(tr("设定程序："));
    }
    else
    {
        ui->pushButton_12->hide();
        ui->stackedWidget_4->setCurrentIndex(0);
    }

    if(Factory =="Benz")
    {
        ui->label_4->setText(tr("线    别："));
        ui->label_5->setText(tr("站    别："));
        ui->label_10->setText("PLUS");
        isLogin = false;
        ui->label_ss3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        ui->label_ss3_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
    }

    if(Factory == "BAIC" || Factory == "GZBAIC")
    {
        ui->progressBar->setGeometry(171,10,1023,151);
        ui->label_17->setGeometry(171,10,1023,151);
        ui->label_tiaomastate->move(178,130);
        ui->label_time->move(338,130);
        QLabel *lableLogo = new QLabel(this);
        lableLogo->setGeometry(10,10,151,151);
        lableLogo->setStyleSheet("image: url(:/LCD_CS351/LCD_CS351/logo/BAIC.png);");
        lableLogo->show();
    }
    else if(Factory == "NOVAT")
    {
        ui->progressBar->setGeometry(171,10,1023,151);
        ui->label_17->setGeometry(171,10,1023,151);
        ui->label_tiaomastate->move(178,130);
        ui->label_time->move(338,130);
        QLabel *lableLogo = new QLabel(this);
        lableLogo->setGeometry(10,10,151,151);
        lableLogo->setStyleSheet("image: url(:/LCD_CS351/LCD_CS351/logo/NOVAT.png);");
        lableLogo->show();
    }
    else if(Factory == "AQCHERY")
    {
        ui->progressBar->setGeometry(171,10,1023,151);
        ui->label_17->setGeometry(171,10,1023,151);
        ui->label_tiaomastate->move(178,130);
        ui->label_time->move(338,130);
        QLabel *lableLogo = new QLabel(this);
        lableLogo->setGeometry(10,10,151,151);
        lableLogo->setStyleSheet("image: url(:/LCD_CS351/LCD_CS351/logo/Zhenyi.png);");
        lableLogo->show();
    }
    else if(Factory == "KFCHERY")
    {
        ui->progressBar->setGeometry(171,10,1023,151);
        ui->label_17->setGeometry(171,10,1023,151);
        ui->label_tiaomastate->move(178,130);
        ui->label_time->move(338,130);
        QLabel *lableLogo = new QLabel(this);
        lableLogo->setGeometry(10,10,151,151);
        lableLogo->setStyleSheet("image: url(:/LCD_CS351/LCD_CS351/logo/KFCHERY.png);");
        lableLogo->show();
    }
    if(Factory == "GZBAIC")
    {
        firstUse = true ; //开机即为首次使用，无需清除任何btn
        initCarType();
    }

    if(ControlType_1 == "PM"|| ControlType_1 == "PF" || ControlType_1 == "PFC" || ControlType_1 == "PMM")
        ui->label_tighten->setText("Atlas");
    else
        ui->label_tighten->setText(ControlType_1);

    connect(&queue_timer,SIGNAL(timeout()),this,SLOT(QueueTimerDo()));

    if(ControlType_1 == "PMM")
    {
        PDMBoltNum = 5;
    }
    else
    {
        PDMBoltNum = 0;
    }
    variable_1 = variable1;
    if(variable_1 == "IR" || variable_1 == "5")    //2017.10.27
    {
        PDMBoltNum = 5;
    }
    else if(variable_1 == "4")
    {
        PDMBoltNum = 4;
    }
    else if(variable_1 == "3")
    {
        PDMBoltNum = 3;
    }
    StartBolt = 0;
    currentBoltSum = 0;
    currentFirstBolt = 0;
    pdmflickerNum=0;
    ttChangeFlag = false;
    for(int i=0;i<4;i++)
    {
        groupNumCh[i] = 0;
        groupAllBoltNumCh[i]=0;
        tempnumdpm[i] = 0;
        whichpdmnumnow[i] = 0;
        PDMCurrentState[i] ="OK";
    }
    allChannel = 0;
    systemStatus = 1;
    preSystemStatus = -1;

    sendEnableFlag = false;
    ui->pushButton_shutdown->hide();
    linkCount = 0;
    connect(&JobTimer,SIGNAL(timeout()),this,SLOT(JobTimeout()));

    isRFIDConnected = false;
    //    searchJobTimes = 0;
    //    searchJobSerialNum = "";
    isFirst = true;
    DeleteIDCode = "";
    isSaveShow = false;
    allTightenResultFlag = false;
    BYDType="";
    changeTemp = true;
    isAlign = false;

    //    if(Factory == "BAIC")
    //        baicInterface = new BaicInfoInterface;
    POINT_A.setX(1000);
    POINT_A.setY(2000);
    POINT_B.setX(5000);
    POINT_B.setY(2000);
    POINT_C.setX(5000);
    POINT_C.setY(7000);
    POINT_D.setX(1000);
    POINT_D.setY(7000);

    QTimer::singleShot(1000, this, SLOT(slotTest()));

    setTaotongLed(0);

   // ui->label_plcHeart->hide();
    //ui->label_plcHeart_ss5->hide();
    ui->label_card->hide();
    ui->label_card_ss6->hide();

    if(factory == "AQCHERY")
    {
        cardTimer = 0;
        cardTimer = new QTimer(this);
        connect(cardTimer,SIGNAL(timeout()),this,SLOT(slot_cardTimer()));

        //ui->label_plcHeart->show();
        //ui->label_plcHeart_ss5->show();
        ui->label_card->show();
        ui->label_card_ss6->show();
    }
    QTimer::singleShot(1000,this,SLOT(slot_light()));

}


void MainWindow::slot_light()
{
    emit sLightLogic(e_NetWorkDisConnect);
    emit sLightLogic(e_TightenDisConnect);
    //if(factory == "SVW2")
    //    emit signalSetIoBox(3);
}

void MainWindow::readCom()
{
    if(gExtSerialPort)
    {
        QByteArray tmpData = gExtSerialPort->readAll();
        if(factory == "AQCHERY")
        {
            if(tmpData.size() > 12)
            {
                tmpData = tmpData.mid(0, 12);
            }
            else if(tmpData.size() < 12)
            {
                DTdebug()<< "readAll error card" << tmpData;
                return;
            }


            QString tmpRevString;
            if(!tmpData.isEmpty())
            {
                int i=0;
                QString tmpStr;
                for(i=0;i<tmpData.count()-1;i++)
                {
                    tmpRevString.append(tmpStr.sprintf("%02x", tmpData.at(i)).right(2));
                    tmpRevString.append(" ");
                }
                tmpRevString.append(tmpStr.sprintf("%02x", tmpData.at(i)).right(2));
            }

            DTdebug()<< "readAll card" << tmpRevString;

            if(!tmpRevString.isEmpty())
            {
                emit signalDetectCard(tmpRevString);
                ui->label_card_ss6->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
                cardTimer->start(5000);
            }
        }
    }
}

void MainWindow::slot_cardTimer()
{
    ui->label_card_ss6->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
    if(cardTimer->isActive())
        cardTimer->stop();
}

void MainWindow::slotTest()
{
    //VinPinCodeView(true, 1);
    //emit sLightLogic(e_SingleOK);

    //getSerialNum(QString("LVVDB11B7MD600066T7161ZEBWCX0022"), false, QString("VIN+CarType"));
    //getSerialNum(QString("LVVDB11B7MD6000661234"), false, QString("VIN+JOBID"));
}

void MainWindow::initCarType()
{
    FUNC();
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    int num = 1 ;
    for(int i = 0;i<CAR_TYPE_AMOUNT; i++)
    {
        QString carType = config->value(QString("carinfo").append(QString::number(i+1)).append("/carcx")).toString();

        if(!carType.isEmpty())
        {
            btnCarType[i]->setVisible(true);
            btnCarType[i]->setGeometry(10,10+45*num,300,35);
            btnCarType[i]->setText(carType);
            num++ ;
        }
    }
    delete config ;
}
void MainWindow::slotOutOfRange()
{
    systemStatus = 24 ;
}

void MainWindow::showPDM()
{
    FUNC();

    DTdebug() <<whichar << firstUse;

    QPushButton* btn = (QPushButton*)sender();  // 获取到了发送该信号按钮的指针
    QString strBtnText = btn->text();
    if(factory=="Haima")
    {
        for(int i = 0;i<CAR_TYPE_AMOUNT; i++)
        {
            if(btnCarType[i]->isVisible())
            {
                btnCarType[i]->setVisible(false);
            }
            carTypeHaima = strBtnText;
        }
        getSerialNum(serialNums,false,"VIN");
        return;
    }

    firstBolt=true ;
    currBolt=0 ;

    if((whichar == 0) && firstUse)
    {
        DTdebug() << "fistuse";
    }
    else if ( (!firstUse) && (whichCarType == 0))
    {
        cleanBAICDisplay();
    }
    else
    {
        cleanDisplay();
    }

    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    for(int i = 0;i<CAR_TYPE_AMOUNT; i++)
    {
        carType = config->value(QString("carinfo").append(QString::number(i+1)).append("/carcx")).toString();

        if(carType == strBtnText)
        {
            whichar = i+1 ;
            DTdebug() << whichar << carType;
        }
    }
    boltAmount = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber1")).toInt();
    emit signalSendBoltAmount(boltAmount);
    delete config ;
//    cleanBAICDisplay();
    if(firstUse)
    {
        firstUse = false ;
    }
    GZBAICVinPinCodeView(whichar);
}

void MainWindow::GZBAICVinPinCodeView(int whichtmp)
{
    DTdebug()<< whichtmp ;
    if(ui->labelQRVerify->isVisible())
    {
        ui->labelQRVerify->setVisible(false);
    }

    int channel;
    int kxuannumtmp = 0;
    int whichPDM = 0;
    bool readProGroupFlag = false;
    QString outBuf[4];
    for(int i=0;i<4;i++)  //clear car buff information
    {
        outBuf[i] = "0";
        tempnumdpm[i] = 0;
        groupNumCh[i] = 0;
        groupAllBoltNumCh[i] = 0;
        carInfor[i].pdmName = "";
        for(int j=0;j<20;j++)
        {
            carInfor[i].proNo[j] = "";
            carInfor[i].boltNum[j] = 0;
            carInfor[i].ttNum[j] = 0;
        }
    }
    preChannel = -1;
    allChannel = 0;
    BoltOrder[0] = 1;
    BoltOrder[1] = 1;
    whichar = whichtmp;
    whichpronumis = 1;
    VIN_PIN_SQL = serialNums;
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    optionOrNot = config->value(QString("carinfo").append(QString::number(whichar)).append("/IsOptionalStation")).toInt();
    Type = config->value(QString("carinfo").append(QString::number(whichar)).append("/carcx")).toString();
    //匹配成功,再判断是否选配
    if(optionOrNot)
    {
        //选配匹配 数据库中的vin码
        //取出选配，查询FIS 数据库，比对 必选fis信息
        bool tmpFlag = false;
        if(Factory == "NOVAT")
        {
            tmpFlag = true;
        }
        else
        {
            QString  tmpserial;
            if(Factory == "BYDSZ")
            {
                selectVin = "select Style from "+tablePreview+" where VIN ='";
                tmpserial = serialNums;
            }
            else
            {
                if(serialNums.length()==14)
                {
                    if(Factory == "SVW2")
                        selectVin = "select FisMatch from "+tablePreview+" where KNR ='";
                    else
                        selectVin = "select * from "+tablePreview+" where KNR ='";
                    tmpserial = serialNums.right(8);
                }
                else if(serialNums.length()==17)
                {
                    if(Factory == "SVW2")
                        selectVin = "select FisMatch from "+tablePreview+" where VIN ='";
                    else
                        selectVin = "select * from "+tablePreview+" where VIN ='";
                    tmpserial = serialNums;
                }
            }
            query.exec(selectVin+tmpserial.append("'"));
            tmpFlag = query.next();
            DTdebug() << "****select KNR******" << selectVin + tmpserial;
        }
        if(tmpFlag)  //查到了匹配
        {
            QString Style ="";
            QByteArray jsonData;
            QJson::Parser parser;
            bool ok = false;
            QVariantMap FisMatch;
            if(Factory == "NOVAT")
            {
            }
            else
            {
                if(Factory == "SVW2")
                {
                    jsonData = query.value(0).toByteArray();
                    DTdebug()<< "jsonData" <<jsonData;
                    FisMatch = parser.parse(jsonData, &ok).toMap();
                    if (!ok) {
                        DTdebug()<<"An error occurred during parsing"<<jsonData;
                    }
                }
                else if(Factory == "BYDSZ")
                {
                    Style = query.value(0).toString();
                    QStringList list = Style.split("(",QString::SkipEmptyParts);
                    Style = list[0];
                    DTdebug()<<"Style"<<Style;
                }

                record = query.record();
            }
            int k =1;
            for(k=1;k<=D_CAR_OPTION_NUM;k++)//optional NO (1~5)    选配
            {
                equeloptionbnum = 0;
                equeloptionknum = 0;
                bxuannumtmp = 0;
                int boltNumber = 0; //bolt number
                for(int n = 1;n < 21;n++) //optional program NO and bolt number
                {
                    boltNumber = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(k)).append(QString::number(n))).toInt();
                    if(boltNumber)
                    {
                        break;
                    }
                }
                whichoption = k;
                DTdebug() << "****optiongal para******" << k << boltNumber ;
                strQRCodeRule  = config->value(QString("carinfo")
                                               .append(QString::number(whichar))
                                               .append("/QRCodeRule")
                                               .append(QString::number(whichoption))).toString();   //new do

                if(boltNumber) //bolt number > 0
                {
                    for(int t = 1;t < 6;t++)//optional       必选
                    {
                        QString bxname = config->value(QString("carinfo").append(QString::number(whichar)).append("/bxuanname").append(QString::number(whichoption)).append(QString::number(t))).toString();
                        QString bxcode = config->value(QString("carinfo").append(QString::number(whichar)).append("/bxuancode").append(QString::number(whichoption)).append(QString::number(t))).toString();
                        if(bxname!="" && bxcode!="")
                        {
                            DTdebug()<<"bxname"<<bxname<<"bxcode"<<bxcode;
                            bxuannumtmp++;
                            if(Factory == "NOVAT")
                            {
                                if(FuzzyMatch(optionNum.toAscii(), bxcode.toAscii()))
                                {
                                    equeloptionbnum++;
                                }
                                else
                                {
                                    DTdebug() << "bixuan match fail" ;
                                }
                            }
                            else if(Factory == "SVW2")
                            {
                                if(ok)
                                {
                                    if(FisMatch[bxname].toString() == bxcode)
                                    {
                                        equeloptionbnum++;
                                    }
                                    else
                                    {
                                        DTdebug() << "bixuan match fail" ;
                                    }
                                }
                            }
                            else if(Factory =="BYDSZ")      //Style
                            {
                                if(Style == bxcode)
                                {
                                    DTdebug()<<"Style == bxcode=="<<bxcode<<t;
                                    equeloptionbnum++;
                                }
                                else if(bxcode =="?????")
                                {
                                    DTdebug()<<"bxcode == ?????"<<t;
                                    equeloptionbnum++;
                                }
                                else
                                {
                                    DTdebug() << "bixuan match fail"<<t ;
                                }
                            }
                            else
                            {
                                for(int m = 0;m< record.count();m++)//FIS
                                {
                                    field = record.field(m);
                                    if(field.name() == bxname)
                                    {
                                        if(field.value().toString() == bxcode)
                                        {
                                            equeloptionbnum++;
                                            break;
                                        }
                                        else
                                        {
                                            DTdebug() << "bixuan match fail" ;
                                            break;
                                        }
                                    }
                                    else //bxname no equal
                                    {
                                        continue;
                                    }
                                }
                            }
                        }
                        else //parameter name NULL
                        {}
                    }
                    equeloptionknum = 0;
                    kxuannumtmp = 0;
                    if(bxuannumtmp == equeloptionbnum)//bixuan parameter equal
                    {
                        DTdebug()<<"bixuan equal";
                        for(int j = 1;j < 6;j++)     // 可选
                        {
                            QString kxname = config->value(QString("carinfo").append(QString::number(whichar)).append("/kxuanname").append(QString::number(whichoption)).append(QString::number(j))).toString();
                            QString kxcode = config->value(QString("carinfo").append(QString::number(whichar)).append("/kxuancode").append(QString::number(whichoption)).append(QString::number(j))).toString();
                            if(kxname != "" && kxcode != "")
                            {
                                DTdebug()<<"kxname"<<kxname<<"kxcode"<<kxcode;
                                kxuannumtmp ++;
                                if(Factory == "NOVAT")
                                {
                                    equeloptionknum = 5;
                                }
                                else if(Factory == "SVW2")
                                {
                                    if(ok)
                                    {
                                        if(FisMatch[kxname].toString() == kxcode)
                                        {
                                            DTdebug()<<"kexuan success";
                                            equeloptionknum = 5;
                                        }
                                        else
                                        {
                                            DTdebug() << "kexuan match fail" ;
                                            equeloptionknum = 0;
                                        }
                                    }
                                }
                                else if(Factory == "BYDSZ")
                                {
                                    equeloptionknum = 5;
                                }
                                else
                                {
                                    for(int m = 0;m< record.count();m++)
                                    {
                                        field = record.field(m);
                                        if(field.name() == kxname)
                                        {
                                            if(field.value().toString() == kxcode)
                                            {
                                                equeloptionknum = 5;
                                                break;
                                            }
                                            else
                                            {
                                                DTdebug() << "kexuan match fail" ;
                                                equeloptionknum = 0;
                                            }
                                        }
                                        else
                                        {}
                                    }
                                }
                                if(equeloptionknum == 5)//kexuan success jump out for(int j = 1;j < 6;j++)
                                {
                                    break;
                                }
                            }
                            else
                            {
                                equeloptionknum ++;
                            }
                        }
                        if( (bxuannumtmp == 0)&&(kxuannumtmp == 0) ) //bixuan and kexuan name all name NULL
                        {
                            equeloptionknum = 0;
                            DTdebug() << "bixuan and kexuan name all name NULL";
                        }
                        else if(equeloptionknum==5)
                        {
                            break; //jump out optional NO (1~5)
                        }
                    }
                    else //bixuan parameter optional error
                    {}
                }
                else
                {}  //bolt number = 0  next optional
            }
            if( (equeloptionbnum == bxuannumtmp) && (equeloptionknum == 5))
            {
                DTdebug() << "optional success" << whichar << whichoption<<equeloptionbnum;
                readProGroupFlag = true;
                if(Factory=="BYDSZ" || Factory=="BYDXA" || Factory == "Haima")
                {
                    if(query.exec("select AutoNO from "+tablePreview + " where VIN='"+VIN_PIN_SQL+"'"))
                    {
                        if(query.next())
                        {
                            AutoNO = query.value(0).toString();
                        }
                        else
                            DTdebug()<<"0 there is no VIN:"+VIN_PIN_SQL;
                    }
                    else
                        DTdebug()<<"select AutoNO fail 0"<<query.lastError();
                }
                else if(Factory == "Haima")
                {
                    if(query.exec("select count(RecordID) from TaskPreview where UseFlag=0"))
                    {
                        int num = 0 ;
                        if(query.next())
                        {
                            num = query.value(0).toInt();
                            if(num < 5)
                                Q_EMIT signalUpdateQueueSql();
                        }
                        else
                            DTdebug()<<""<<query.lastError();
                    }
                    else
                        DTdebug()<<"select AutoNO fail 0"<<query.lastError();


                    if(query.exec("select AutoNO from TaskPreview where VIN='"+VIN_PIN_SQL+"'"))
                    {
                        if(query.next())
                        {
                            AutoNO = query.value(0).toString();
                        }
                        else
                            DTdebug()<<"0 there is no VIN:"+VIN_PIN_SQL;
                    }
                    else
                        DTdebug()<<"select AutoNO fail 0"<<query.lastError();
                }
                carInfor[0].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong").append(QString::number(whichoption).append("1"))).toString();
                carInfor[1].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong").append(QString::number(whichoption).append("2"))).toString();
                for(int j = 1;j< 21;j++)
                {
                    channel = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPChannel").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                    if(channel == 1)
                    {
                        carInfor[0].proNo[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPProNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                        carInfor[0].boltNum[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                        if((carInfor[0].proNo[j-1].toInt())&&(carInfor[0].boltNum[j-1]))
                        {
                            carInfor[0].boltSN[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLuoSuanNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                            carInfor[0].ttNum[j-1]  = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPTaotong").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                            DTdebug()<<"000000000*"<<carInfor[0].boltSN[j-1];
                            DTdebug()<<"000000000**"<<carInfor[0].proNo[j-1];
                            DTdebug()<<"000000000***"<<carInfor[0].boltNum[j-1];
                            DTdebug()<<"000000000****"<<carInfor[0].ttNum[j-1];
                            groupAllBoltNumCh[0] += carInfor[0].boltNum[j-1];
                        }
                    }
                    else if(channel == 2)
                    {
                        carInfor[1].proNo[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPProNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                        carInfor[1].boltNum[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                        if((carInfor[1].proNo[j-1].toInt())&&(carInfor[0].boltNum[j-1]))
                        {
                            carInfor[1].boltSN[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLuoSuanNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                            carInfor[1].ttNum[j-1]  = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPTaotong").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                            DTdebug()<<"111111111*"<<carInfor[1].boltSN[j-1];
                            DTdebug()<<"111111111**"<<carInfor[1].proNo[j-1];
                            DTdebug()<<"111111111***"<<carInfor[1].boltNum[j-1];
                            DTdebug()<<"111111111****"<<carInfor[1].ttNum[j-1];
                            groupAllBoltNumCh[1] += carInfor[1].boltNum[j-1];
                        }
                    }
                }
                BoltTotalNum = groupAllBoltNumCh[0] + groupAllBoltNumCh[1] ;
                if(BoltTotalNum == 0)
                {
                    systemStatus = 14;
                }
            }
            else //optional fail
            {
                DTdebug() << "***********optional fail***************"<< equeloptionbnum << bxuannumtmp << equeloptionknum;
                equeloptionknum = 0;
                equeloptionbnum = 0;
                bxuannumtmp = 0;
                //                    ui->label_tiaomastate->setText(tr("选配匹配失败"));
                DTdebug()<<"optional match fail";
                systemStatus = 11;
                ISmaintenance = false;
                gunPower(false);
                if(IsQueue)
                {
                    UpdateSqlFlag();
                    if(queue_timer.isActive())
                        queue_timer.stop();
                    queue_timer.start(1000);
                }
            }
        }
        else//local fis fail
        {
            DTdebug() << "***********local fis fail***************";
            //                ui->label_tiaomastate->setText(tr("本地无此车信息"));
            systemStatus = 12;
            ISmaintenance = false;
            DTdebug()<<"local fis match fail";
            gunPower(false);
            if(IsQueue)
            {
                UpdateSqlFlag();
                if(queue_timer.isActive())
                    queue_timer.stop();
                queue_timer.start(1000);
            }
        }
    }
    else
    {
        //不选配
        //设置获取匹配成功后的 程序号
        //再判断是否有套筒
        readProGroupFlag = true;
        if(Factory=="BYDSZ" || Factory=="BYDXA")
        {
            if(query.exec("select AutoNO from "+tablePreview + " where VIN='"+VIN_PIN_SQL+"'"))
            {
                if(query.next())
                {
                    AutoNO = query.value(0).toString();
                }
                else
                    DTdebug()<<"there is no VIN:"+VIN_PIN_SQL;
            }
            else
                DTdebug()<<"select AutoNO fail"<<query.lastError();
        }
        outBuf[0] = config->value("carinfo"+QString::number(whichar)+"/Out1").toString();
        outBuf[1] = config->value("carinfo"+QString::number(whichar)+"/Out2").toString();
        outBuf[2] = config->value("carinfo"+QString::number(whichar)+"/Out3").toString();
        outBuf[3] = config->value("carinfo"+QString::number(whichar)+"/Out4").toString();

        carInfor[0].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong01")).toString();
        carInfor[1].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong02")).toString();
        carInfor[2].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong03")).toString();
        carInfor[3].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong04")).toString();
        for(int m=1;m<21;m++)
        {
            channel = config->value(QString("carinfo").append(QString::number(whichar)).append("/Channel").append(QString::number(m))).toInt();
            if(channel == 1)
            {
                carInfor[0].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                carInfor[0].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                if((carInfor[0].proNo[m-1].toInt())&&(carInfor[0].boltNum[m-1]))
                {
                    carInfor[0].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                    carInfor[0].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                    DTdebug()<<"000000000*"<<carInfor[0].boltSN[m-1];
                    DTdebug()<<"000000000**"<<carInfor[0].proNo[m-1];
                    DTdebug()<<"000000000***"<<carInfor[0].boltNum[m-1];
                    DTdebug()<<"000000000****"<<carInfor[0].ttNum[m-1];
                    groupAllBoltNumCh[0] += carInfor[0].boltNum[m-1];
                }
            }
            else if(channel == 2)
            {
                carInfor[1].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                carInfor[1].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                if((carInfor[1].proNo[m-1].toInt())&&(carInfor[1].boltNum[m-1]))
                {
                    carInfor[1].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                    carInfor[1].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                    DTdebug()<<"111111111*"<<carInfor[1].boltSN[m-1];
                    DTdebug()<<"111111111**"<<carInfor[1].proNo[m-1];
                    DTdebug()<<"111111111***"<<carInfor[1].boltNum[m-1];
                    DTdebug()<<"111111111****"<<carInfor[1].ttNum[m-1];
                    groupAllBoltNumCh[1] += carInfor[1].boltNum[m-1];
                }
            }
            else if(channel == 3)
            {
                carInfor[2].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                carInfor[2].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                if((carInfor[2].proNo[m-1].toInt())&&(carInfor[2].boltNum[m-1]))
                {
                    carInfor[2].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                    carInfor[2].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                    groupAllBoltNumCh[2] += carInfor[2].boltNum[m-1];
                }
            }
            // more channel
            else
            {
                carInfor[3].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                carInfor[3].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                if((carInfor[3].proNo[m-1].toInt())&&(carInfor[3].boltNum[m-1]))
                {
                    carInfor[3].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                    carInfor[3].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                    groupAllBoltNumCh[3] += carInfor[3].boltNum[m-1];
                }
            }
        }
        BoltTotalNum = groupAllBoltNumCh[0] + groupAllBoltNumCh[1] + groupAllBoltNumCh[2] + groupAllBoltNumCh[3] ;
        if(BoltTotalNum == 0)//error code
        {
            systemStatus = 14;
        }
    }
    if(readProGroupFlag) //read program group and system ING
    {
        DTdebug() << readProGroupFlag ;

        int n=0;
        for(n=0;n<4;n++) //4 channel send enable
        {
            if(groupAllBoltNumCh[n] != 0)
            {
                allChannel++;
            }
        }
        if(ControlType_1 == "SB356_PLC")
        {
            n=0;
            int j = 0;
            for(j = 0;j< 20;j++)
            {
                if(carInfor[n].boltNum[j]>0)//bolt number
                {
                    enableLsnumber = carInfor[n].boltNum[j];
                    QString pathpdm;
                    int k = 1;
                    for(k = 1;k<1000;k++)
                    {
                        if(carInfor[n].pdmName == config->value(QString("pdminfo").append(QString::number((k))).append("/pdmname")))
                        {
                            tempnumdpm[n] = config->value(QString("pdminfo").append(QString::number((k))).append("/num")).toInt();
                            //                                NOKflag = false;
                            pathpdm = config->value(QString("pdminfo").append(QString::number((k))).append("/pdmpath")).toString();
                            break;
                        }
                    }
                    ui->label_pdmarea->setStyleSheet(QString("border-image: url(/PDM/").append(pathpdm).append(")"));

                    /************发给web端空白螺栓****************/
                    sendWebValue(1,pathpdm);
                    lock.lockForWrite();
                    info[4] =QString::number(tempnumdpm[n]);
                    lock.unlock();
                    for(int i = 0;i < tempnumdpm[n];i++)
                    {
                        butt[n][i] = new QPushButton(ui->stackedWidget_2);
                        label1[n][i] = new QLabel(ui->stackedWidget_2);
                        label2[n][i] = new QLabel(ui->stackedWidget_2);

                        butt[n][i]->raise();
                        butt[n][i]->setFlat(true);
                        label1[n][i]->setAlignment(Qt::AlignLeft);
                        label2[n][i]->setAlignment(Qt::AlignLeft);
                        butt[n][i]->setFocusPolicy(Qt::NoFocus);
                        label1[n][i]->setFocusPolicy(Qt::NoFocus);
                        label2[n][i]->setFocusPolicy(Qt::NoFocus);
                        butt[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                        label1[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                        label2[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);

                        numpdm++;

                        butt[n][i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                        butt[n][i]->setText(QString::number((i+1)));

                        int tempx = config->value(QString("pdminfo").append(QString::number(k)).append("/tempx").append(QString::number((i+1)))).toInt();
                        int tempy =  config->value(QString("pdminfo").append(QString::number(k)).append("/tempy").append(QString::number((i+1)))).toInt();
                        lock.lockForWrite();
                        status[i][3] =QString::number(tempx);
                        status[i][4] =QString::number(tempy);
                        lock.unlock();
                        double x=0.0;
                        double y=0.0;
                        x = (double)tempx/1000 * 1166;
                        y = (double)tempy/1000 * 598;

                        //                                double x = (double)tempx/1000 * 1166 + 200;
                        //                                double y = (double)tempy/1000 * 598 + 170;
                        butt[n][i]->setGeometry(x,y,46,46);
                        label1[n][i]->setGeometry(x+43,y,130,23);
                        label2[n][i]->setGeometry(x+43,y+13,140,23);
                        label1[n][i]->setStyleSheet("background:transparent;");
                        label2[n][i]->setStyleSheet("background:transparent;");
                        butt[n][i]->show();
                        label1[n][i]->show();
                        label2[n][i]->show();
                    }
                    SYSS = "ING";
                    emit vinSendPLC(0,20,"\x11\x11"+VIN_PIN_SQL+"1");
                    break;
                }
                else
                {
                    whichpronumis++;
                    continue;
                }
            }
            if(j == 20)
            {
                //                    ui->label_tiaomastate->setText(tr(QString("channel" + QString::number(n+1) +"螺栓数量为0").toLatin1().data()));
                systemStatus = 14;
            }
        }
        else //CS351 ATLAS
        {
            if(allChannel==1)
                ui->stackedWidget_6->setCurrentIndex(0);
            else if(allChannel==2)
                ui->stackedWidget_6->setCurrentIndex(1);
            for(n=0;n<4;n++) //4 channel send enable
            {
                if(groupAllBoltNumCh[n] == 0)
                {
                    continue;
                }
                else
                {
                    whichPDM++;
                    int j = 0;
                    for(j = 0;j< D_BOLTNUM;j++)
                    {
                        if(carInfor[n].boltNum[j]>0)//bolt number
                        {
                            enableLsnumber = carInfor[n].boltNum[j];
                            QString pathpdm;
                            int k = 1;
                            for(k = 1;k<100;k++) //1000  modify
                            {
                                if(carInfor[n].pdmName == config->value(QString("pdminfo").append(QString::number((k))).append("/pdmname")))
                                {
                                    tempnumdpm[n] = config->value(QString("pdminfo").append(QString::number((k))).append("/num")).toInt();
                                    //                                        NOKflag = false;
                                    pathpdm = config->value(QString("pdminfo").append(QString::number((k))).append("/pdmpath")).toString();
                                    break;
                                }
                            }
                            if( allChannel == 1)
                                ui->label_pdmarea->setStyleSheet(QString("border-image: url(/PDM/").append(pathpdm).append(")"));
                            else if(allChannel == 2)
                            {
                                if(whichPDM == 1)
                                    ui->label_left->setStyleSheet(QString("image: url(/PDM/").append(pathpdm).append(")"));
                                else
                                    ui->label_right->setStyleSheet(QString("image: url(/PDM/").append(pathpdm).append(")"));
                            }

                            /************发给web端空白螺栓****************/
                            sendWebValue(1,pathpdm);
                            lock.lockForWrite();
                            info[4] =QString::number(tempnumdpm[n]);
                            lock.unlock();
                            for(int i = 0;i < tempnumdpm[n];i++)
                            {
                                if( allChannel == 1)
                                {
                                    butt[n][i] = new QPushButton(ui->stackedWidget_2);
                                    label1[n][i] = new QLabel(ui->stackedWidget_2);
                                    label2[n][i] = new QLabel(ui->stackedWidget_2);
                                }
                                else if(allChannel == 2)
                                {
                                    if(whichPDM == 1)
                                    {
                                        butt[n][i] = new QPushButton(ui->widget_left);
                                        label1[n][i] = new QLabel(ui->widget_left);
                                        label2[n][i] = new QLabel(ui->widget_left);
                                    }
                                    else if(whichPDM == 2)
                                    {
                                        butt[n][i] = new QPushButton(ui->widget_right);
                                        label1[n][i] = new QLabel(ui->widget_right);
                                        label2[n][i] = new QLabel(ui->widget_right);
                                    }
                                }
                                butt[n][i]->raise();
                                butt[n][i]->setFlat(true);
                                label1[n][i]->setAlignment(Qt::AlignLeft);
                                label2[n][i]->setAlignment(Qt::AlignLeft);
                                butt[n][i]->setFocusPolicy(Qt::NoFocus);
                                label1[n][i]->setFocusPolicy(Qt::NoFocus);
                                label2[n][i]->setFocusPolicy(Qt::NoFocus);
                                butt[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                                label1[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                                label2[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);

                                numpdm++;

                                butt[n][i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                                butt[n][i]->setText(QString::number((i+1)));

                                int tempx = config->value(QString("pdminfo").append(QString::number(k)).append("/tempx").append(QString::number((i+1)))).toInt();
                                int tempy =  config->value(QString("pdminfo").append(QString::number(k)).append("/tempy").append(QString::number((i+1)))).toInt();

                                double x=0.0;
                                double y=0.0;

                                if( allChannel == 1)
                                {
                                    x = (double)tempx/1000 * 1166;
                                    y = (double)tempy/1000 * 598;
                                }
                                else if(allChannel == 2)
                                {
                                    x = (double)tempx/1000 * 683;
                                    y = (double)tempy/1000 * 350;
                                }
                                //                                double x = (double)tempx/1000 * 1166 + 200;
                                //                                double y = (double)tempy/1000 * 598 + 170;
                                butt[n][i]->setGeometry(x,y,46,46);
                                label1[n][i]->setGeometry(x+43,y,130,23);
                                label2[n][i]->setGeometry(x+43,y+13,140,23);
                                label1[n][i]->setStyleSheet("background:transparent;");
                                label2[n][i]->setStyleSheet("background:transparent;");
                                butt[n][i]->show();
                                label1[n][i]->show();
                                label2[n][i]->show();
                            }
                            SYSS = "ING";
                            groupNumCh[n] = j;
                            bool condition = GetTaotongNumCondition(n,j);
                            if((condition)&&(allChannel==1))
                            {
                                DTdebug() << "here waitting taotong";
                                TaoTongState = true;
                                preChannel = n;       //taotong channel
                                SetLsInfo(carInfor[n].proNo[j], QString::number(carInfor[n].boltNum[j]), QString::number(carInfor[n].ttNum[j]));
                                sendEnableFlag = false;
                                break;
                            }
                            else
                            {
                                if(allChannel==1)
                                {
                                    SetLsInfo(carInfor[n].proNo[j], QString::number(carInfor[n].boltNum[j]), QString::number(carInfor[n].ttNum[j]));
                                }
                                if(n == 0)
                                {
                                    emitOperate1(true,j);
                                    DTdebug()<<"*******send enable ch1*********group:"<<j;
                                }
                                else if(n == 1)
                                {
                                    emitOperate2(true,j);
                                    DTdebug()<<"*******send enable ch2*********group:"<<j;
                                }
                                else if(n == 2)
                                {
                                    emitOperate3(true,j);
                                    DTdebug()<<"*******send enable ch3*********group:"<<j;
                                }
                                else
                                {
                                     emitOperate4(true,j);
                                    DTdebug()<<"*******send enable ch4*********group:"<<j;
                                }
                                break;
                            }
                        }
                        else
                        {
                            whichpronumis++;
                            continue;
                        }
                    }
                    if(j == 20)
                    {
                        //                            ui->label_tiaomastate->setText(tr(QString("channel" + QString::number(n+1) +"螺栓数量为0").toLatin1().data()));
                        systemStatus = 14;
                    }
                }
            }
        }
        if(SYSS == "ING")
        {
            if(PDMBoltNum>0)
            {
                if(tempnumdpm[0]>PDMBoltNum)
                {
                    pdmflickerNum = PDMBoltNum;
                }
                else
                {
                    pdmflickerNum = tempnumdpm[0];
                }
            }
            DTdebug() <<"*********** syss ing **********"<<pdmflickerNum<<tempnumdpm[0];
            emit sendRunStatusToJson("ING");
            emit vinSendPLC(0,6,"\x11\x11"+outBuf[0]+outBuf[1]+outBuf[2]+outBuf[3]);
            emit sendnexo(serialNums);//download VIN
            ui->progressBar->setValue(0);
            m_CurrentValue  = 0;
            Start(100, Tacktime*10);
            ISmaintenance = true;
            QualifiedNum = 0;


            if(!IOFlag)
            {
                Q_EMIT sendCmdToCs351FromMW(5);    //由于槽函数的参数定义为int类型，暂用。空闲时可改为枚举类型。
            }
            else {
                Q_EMIT signalSendTaotongNum();
            }
//                timerpdm.start(500);
            displayStatusIcon("ING");
            //white_led(1);
            whichpdmnumnow[0] = 0;
            whichpdmnumnow[1] = 0;
            whichpdmnumnow[2] = 0;
            whichpdmnumnow[3] = 0;
            PDMBoltBuf[0] = 1;
            PDMBoltBuf[1] = 1;
            PDMBoltBuf[2] = 1;
            PDMBoltBuf[3] = 1;
            PDMBoltBuf[4] = 1;
            PDMBoltStute[0] = 0;
            PDMBoltStute[1] = 0;
            PDMBoltStute[2] = 0;
            PDMBoltStute[3] = 0;
            PDMBoltStute[4] = 0;
            StartBolt = 0;
            currentBoltSum = 0;
            currentFirstBolt = 0;
            allTightenResultFlag = true;
        }
        else
        {
            DTdebug() << "*******vin invalid********";
            ISmaintenance = false;
            DTdebug()<<"vin invalid";
            gunPower(false);
            if(IsQueue)
            {
                UpdateSqlFlag();
                if(queue_timer.isActive())
                    queue_timer.stop();
                queue_timer.start(1000);
            }
        }
    }
    if(isRFID)
    {
        int k = 0;
        QString lsNumTemp="";
        for(k = 0;k <21;k++)
        {
            if(!optionOrNot)  //非选配
            {
                lsNumTemp = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(k))).toString();
            }
            else
            {
                lsNumTemp = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(whichoption)).append(QString::number(k))).toString();
            }
            if(lsNumTemp.toInt() >0)
            {
                bolt_index = k;
            }
        }
    }
    delete config;

}


void MainWindow::newconfigInit(Newconfiginfo *newconfig)
{
    newconfiginfo = newconfig;
}

/*****************断开重新连接*******************/
void MainWindow::receiveOperate()
{   
    TightenIsReady(true);
    if(SYSS == "ING")
    {
        fromsecondthreaddata("","","Reconnect",0,groupNumCh[0]);
    }
}

/*****************拧紧枪是否准备*******************/
void MainWindow::TightenIsReady(bool isready)
{
    DTdebug() << isready;
    if(isready)
    {
        linkCount = 1;
        ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        CsIsConnect = true;
        emit sendControllerStatus(true);
        if(SYSS == "Ready")
        {
            lock.lockForWrite();
            StationStatus = 2;
            lock.unlock();
            displayStatusIcon("Ready");
        }
        else if(SYSS == "ING")
        {
            lock.lockForWrite();
            StationStatus = 2;
            lock.unlock();
            displayStatusIcon("ING");
        }
        else if(SYSS == "OK")
        {
            lock.lockForWrite();
            StationStatus = 2;
            lock.unlock();
            displayStatusIcon("OK");
        }
        else if(SYSS == "NOK")
        {
            lock.lockForWrite();
            StationStatus = 1;
            lock.unlock();
            displayStatusIcon("NOK");
        }
        DTdebug() << "sLightLogic(e_TightenConnect)";
        emit sLightLogic(e_TightenConnect);
        //yellow_led(0);
        //        ui->label_tiaomastate->setText("");
        systemStatus = 0;
        Queuelock.lockForRead();

        DTdebug() <<IsQueue << ISmaintenance << QueueIsNull ;
        if(IsQueue && !ISmaintenance && QueueIsNull
           && ( Factory == "BYDSZ" || Factory == "BYDXA" || m_BAICQueue) )
        {
            if(!FisTimer.isActive())
            {
                FisTimer.start(5000);
            }
        }
        if(Factory == "BAIC" && isQueue)
            leuzeTimerStart();
        Queuelock.unlock();
    }
    else
    {
        linkCount = 0;
        lock.lockForWrite();
        StationStatus = 0;
        lock.unlock();
        CsIsConnect = false;
        emit sendControllerStatus(false);
        emit sLightLogic(e_TightenDisConnect);
        //yellow_led(1);
        systemStatus = 1;
        ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        displayStatusIcon("Not Ready");

        Queuelock.lockForRead();
        if(Factory == "BAIC" && isQueue)
        {
            if(leuzeTimer.isActive())
                leuzeTimer.stop();
        }
        Queuelock.unlock();
    }
}

/*****************拧紧枪是否准备*******************/
void MainWindow::TightenIsReady(int n,bool isready)
{
    DTdebug() << n << isready;
    if(isready)
    {
        if(n == 0)
            linkCount |= 0x01;
        else if(n == 1)
            linkCount |= 0x02;
        else if(n == 2)
            linkCount |= 0x04;
        else if(n == 3)
            linkCount |= 0x08;
        else if(n == 4)
            linkCount |= 0x10;
        DTdebug()<<"*****linkCount*******"<<n<<linkCount;
        if(linkCount == 0x1f)
        {
            if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
            {
                SYSS = "Ready";
            }
            CsIsConnect = true;
            emit sendControllerStatus(true);
            ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
            ui->label_ss11->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
            ui->label_ss12->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
            if(SYSS == "Ready")
            {
                lock.lockForWrite();
                StationStatus = 2;
                lock.unlock();
                displayStatusIcon("Ready");
            }
            else if(SYSS == "ING")
            {
                lock.lockForWrite();
                StationStatus = 2;
                lock.unlock();
                displayStatusIcon("ING");
            }
            else if(SYSS == "OK")
            {
                lock.lockForWrite();
                StationStatus = 2;
                lock.unlock();
                displayStatusIcon("OK");
            }
            else if(SYSS == "NOK")
            {
                lock.lockForWrite();
                StationStatus = 1;
                lock.unlock();
                displayStatusIcon("NOK");
            }
            emit sLightLogic(e_TightenConnect);
            //yellow_led(0);
            if(systemStatus == 1){
                systemStatus = 0;
            }
            Queuelock.lockForRead();
            if((m_BAICQueue||IsQueue) && !ISmaintenance && QueueIsNull)
            {
                Queuelock.unlock();
                if(!FisTimer.isActive())
                    FisTimer.start(5000);
            }
            else
                Queuelock.unlock();
        }
    }
    else
    {
        if(n == 0)
            linkCount &= ~0x01;
        else if(n == 1)
            linkCount &= ~0x02;
        else if(n == 2)
            linkCount &= ~0x04;
        else if(n == 3)
            linkCount &= ~0x08;
        else if(n == 4)
            linkCount &= ~0x10;
        DTdebug()<<" no linkCount: "<<n<<linkCount;
        CsIsConnect = false;
        emit sendControllerStatus(false);
        lock.lockForWrite();
        StationStatus = 0;
        lock.unlock();
        emit sLightLogic(e_TightenDisConnect);
        //yellow_led(1);
        systemStatus = 1;
        ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        if(n == 1)
            ui->label_ss11->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        if(n == 2)
            ui->label_ss12->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        displayStatusIcon("Not Ready");
    }
}


//确认给拧紧机发送VIN码成功
void MainWindow::slots_getSendVINResult(bool result)
{
    if(result){
        if(systemStatus == 25){
            systemStatus = 0;
        }
    }else{
        systemStatus = 25;
    }
}



/*****************PLC connect ok*******************/
void MainWindow::PLCIsReady(int isready)
{
    DTdebug() <<"********plc is ready*****" << isready;
    if(isready == 1)//connect success
    {
        if(systemStatus != 0)
        {
            if(SYSS == "Ready")
            {
                displayStatusIcon("Ready");
                //                ui->pushButton_16->setText("Ready");
                //                ui->pushButton_16->setStyleSheet("font: 30pt ; background-color: rgb(51, 153, 255); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
            }
            else if(SYSS == "ING")
            {
                displayStatusIcon("ING");
                //                ui->pushButton_16->setText("ING");
                //                ui->pushButton_16->setStyleSheet("font: 50pt ; background-color: rgb(250, 225, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
            }
            else if(SYSS == "OK")
            {
                displayStatusIcon("OK");
                //                ui->pushButton_16->setText("OK");
                //                ui->pushButton_16->setStyleSheet("font: 60pt ; background-color: rgb(25, 125, 44); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
            }
            else if(SYSS == "NOK")
            {
                displayStatusIcon("NOK");
                //                ui->pushButton_16->setText("NOK");
                //                ui->pushButton_16->setStyleSheet("font: 40pt ; background-color: rgb(255, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
            }
            emit sLightLogic(e_PlcConnect);
            //yellow_led(0);
            systemStatus = 0;
        }
    }
    else if(isready == 2)//connect fail
    {
         emit sLightLogic(e_PlcDisConnect);
        //yellow_led(1);
        systemStatus = 2;
        displayStatusIcon("Not Ready");
        //        ui->pushButton_16->setText("Not Ready");
        //        ui->pushButton_16->setStyleSheet("font: 19pt ; background-color: rgb(250, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
    }
    else //connect success and read error
    {
        if(systemStatus != 3)
        {
            emit sLightLogic(e_PlcConnectAndReadError);
            //yellow_led(1);
            systemStatus = 3;
            displayStatusIcon("Not Ready");
            //            ui->pushButton_16->setText("Not Ready");
            //            ui->pushButton_16->setStyleSheet("font: 19pt ; background-color: rgb(250, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
        }
    }
}

/**********************************连接mysql数据库*/
void ::MainWindow::connectMysql()
{
    db=QSqlDatabase::addDatabase("QMYSQL","mysqlconnectionmatch");
    db.setHostName("localhost");
    db.setDatabaseName("Tighten");
    db.setUserName("root");
    db.setPassword("123456");
    if(!db.open()){
        DTdebug()<< db.lastError().text();
        DTdebug()<< "mainwindow open mysql fail";
    }else{
        DTdebug()<< "mainwindow open mysql ok";
    }
    query = QSqlQuery(db);
    query1 =QSqlQuery(db);

    if(!query.exec("select count(*) from "+Localtable))
    {
        DTdebug()<<"Mysql start error";
        system(QString("/usr/local/mysql/bin/myisamchk -c -r -o -f /usr/local/mysql/var/Tighten/"+Localtable+".MYI").toLocal8Bit().data());
        //        signal_mysqlerror_do();
        DTdebug()<<"repair Mysql";
    }
    else
        DTdebug()<<"mysql normal";
}

void MainWindow::mysqlopen()
{
    if(QSqlDatabase::contains("mysqlconnectionmatch")){
        db = QSqlDatabase::database("mysqlconnectionmatch");
    }else{
        db = QSqlDatabase::addDatabase("QMYSQL","mysqlconnectionmatch");
        db.setHostName("localhost");
        db.setDatabaseName("Tighten");
        db.setUserName("root");
        db.setPassword("123456");
        query = QSqlQuery(db);
        query1 =QSqlQuery(db);
    }

    if(!db.open())
    {
        if(!db.open())
        {
            DTdebug()<< "mainwindow localmysql "<< db.lastError().text();
        }else
        {
            DTdebug()<< "mainwindow localmysql open ok 2";
        }
    }else
    {
        //DTdebug()<< "mainwindow localmysql open ok 1";
    }
}

void MainWindow::mysqlclose()
{
    if(db.isOpen())
        db.close();
}
void MainWindow::sendWebValue(int states,QString namepdm)
{
    //传给web端螺丝数据状态

    if(states == 1)
    {
        ScrewWhichExit = 0;
        lock.lockForWrite();
        status[ScrewWhichExit][0] = "3";  //螺栓状态
        status[ScrewWhichExit][1] = "";//螺栓扭矩
        status[ScrewWhichExit][2] = "";// 螺栓角度

        info[0] = "1";
        info[1] = RFIDIsConnect?"1":"0";
        info[2] = QString::number(TimeLeft);
        info[3] = namepdm;


        for(int j = 1;j < tempnumdpm[0];j++)
        {
            status[j][0] = "4";
            status[j][1] = "";//螺栓扭矩
            status[j][2] = "";// 螺栓角度
        }
        lock.unlock();
    }
}

bool MainWindow::GetTaotongNumCondition(int pCarInfoNum, int pTtNum)
{
    bool condition = false;
    if(Factory == "SVW3" ||Factory == "SVW2" || Factory == "KFCHERY")    //||Factory == "NOVAT"
    {
        condition = ((carInfor[pCarInfoNum].ttNum[pTtNum] >= 1) && (carInfor[pCarInfoNum].ttNum[pTtNum] <= 4));
    }
    else if (Factory == "Ningbo"||Factory == "BYDSZ"||Factory == "BYDXA" || Factory == "Haima" || Factory == "AQCHERY" )  // new haima todo
    {
        condition = ((carInfor[pCarInfoNum].ttNum[pTtNum] >= 1) && (carInfor[pCarInfoNum].ttNum[pTtNum] <= 8));
    }
    return condition;
}

bool MainWindow::FuzzyMatch(QByteArray pValue1, QByteArray pValue2)
{
    if(pValue1.count() != pValue2.count())
        return false;
    for(int i=0;i<pValue1.count();i++)
    {
        if((pValue1.at(i) == '?') || (pValue2.at(i) == '?'))
        {

        }
        else if(pValue1.at(i) != pValue2.at(i))
        {
            return false;
        }
    }
    return true;
}

void MainWindow::VinPinCodeView(bool flags, int whichtmp)
{
    DTdebug() << "VinPinCodeView---flags,whichtmp" <<flags << whichtmp ;
    boltSNList.clear();

    if(ui->labelQRVerify->isVisible())
    {
        ui->labelQRVerify->setVisible(false);
    }
    if(flags)
    {
        int channel;
        int kxuannumtmp = 0;
        int whichPDM = 0;
        bool readProGroupFlag = false;
        QString outBuf[4];
        for(int i=0;i<4;i++)  //clear car buff information
        {
            outBuf[i] = "0";
            tempnumdpm[i] = 0;
            groupNumCh[i] = 0;
            groupAllBoltNumCh[i] = 0;
            carInfor[i].pdmName = "";
            for(int j=0;j<D_BOLTNUM;j++)
            {
                carInfor[i].proNo[j] = "";
                carInfor[i].boltNum[j] = 0;
                carInfor[i].ttNum[j] = 0;
            }
        }
        preChannel = -1;
        allChannel = 0;
        BoltOrder[0] = 1;
        BoltOrder[1] = 1;
        whichar = whichtmp;
        whichpronumis = 1;
        if(Factory == "NOVAT"||Factory == "AQCHERY"||Factory == "KFCHERY")
        {
            if(Factory == "KFCHERY"){
                VIN_PIN_SQL = serialNums.right(17);
            }else{
                VIN_PIN_SQL = serialNums.left(17);
            }
            serialNums = VIN_PIN_SQL;
            DTdebug() << "VIN_PIN_SQL" << VIN_PIN_SQL;
        }
        else
            VIN_PIN_SQL = serialNums;
        QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
        optionOrNot = config->value(QString("carinfo").append(QString::number(whichar)).append("/IsOptionalStation")).toInt();
        Type = config->value(QString("carinfo").append(QString::number(whichar)).append("/carcx")).toString();

        DTdebug() << "****ScanFinishThenWait******" ;
        emit sLightLogic(e_ScanFinishThenWait);

        //匹配成功,再判断是否选配
        if(optionOrNot)
        {
            //选配匹配 数据库中的vin码
            //取出选配，查询FIS 数据库，比对 必选fis信息
            QString  tmpserial;
            bool tmpFlag = false;
            if( Factory == "NOVAT")
            {
                tmpFlag = true;
            }
            else
            {
                if(Factory == "BYDSZ")
                {
                    selectVin = "select Style from "+tablePreview+" where VIN ='";
                    tmpserial = serialNums;
                }
                else
                {
                    if(serialNums.length()==14)
                    {
                        if(Factory == "SVW2")
                            selectVin = "select FisMatch from "+tablePreview+" where KNR ='";
                        else
                            selectVin = "select * from "+tablePreview+" where KNR ='";
                        tmpserial = serialNums.right(8);
                    }
                    else if(serialNums.length()==17)
                    {
                        if(Factory == "SVW2")
                            selectVin = "select FisMatch from "+tablePreview+" where VIN ='";
                        else
                            selectVin = "select * from "+tablePreview+" where VIN ='";
                        tmpserial = serialNums;
                    }
                }
                query.exec(selectVin+tmpserial.append("'"));
                tmpFlag = query.next();
                //tmpFlag = true;
                DTdebug() << "****select KNR******" << selectVin + tmpserial;
            }
            if(tmpFlag)  //查到了匹配
            {
                QString Style ="";
                QByteArray jsonData;
                QJson::Parser parser;
                bool ok = false;
                QVariantMap FisMatch;
                if( Factory == "NOVAT")
                {

                }
                else
                {
                    if(Factory == "SVW2")
                    {
                        jsonData = query.value(0).toByteArray();
                        DTdebug()<< "jsonData" <<jsonData;
                        FisMatch = parser.parse(jsonData, &ok).toMap();
                        if (!ok) {
                            DTdebug()<<"An error occurred during parsing"<<jsonData;
                        }
                    }
                    else if(Factory == "BYDSZ")
                    {
                        Style = query.value(0).toString();
                        QStringList list = Style.split("(",QString::SkipEmptyParts);
                        Style = list[0];
                        DTdebug()<<"Style"<<Style;
                    }
                    record = query.record();
                }
                int k =1;
                for(k=1;k<=D_CAR_OPTION_NUM;k++)//optional NO (1~5)    选配
                {
                    equeloptionbnum = 0;
                    equeloptionknum = 0;
                    bxuannumtmp = 0;
                    int boltNumber = 0; //bolt number
                    for(int n = 1;n < D_BOLTNUM+1;n++) //optional program NO and bolt number
                    {
                        boltNumber = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(k)).append(QString::number(n))).toInt();
                        if(boltNumber)
                        {
                            break;
                        }
                    }
                    whichoption = k;
                    DTdebug() << "****optiongal para******" << k << boltNumber ;
                    strQRCodeRule  = config->value(QString("carinfo")
                                                   .append(QString::number(whichar))
                                                   .append("/QRCodeRule")
                                                   .append(QString::number(whichoption))).toString();   //new do

                    if(boltNumber) //bolt number > 0
                    {
                        for(int t = 1;t < 6;t++)//optional       必选
                        {
                            QString bxname = config->value(QString("carinfo").append(QString::number(whichar)).append("/bxuanname").append(QString::number(whichoption)).append(QString::number(t))).toString();
                            QString bxcode = config->value(QString("carinfo").append(QString::number(whichar)).append("/bxuancode").append(QString::number(whichoption)).append(QString::number(t))).toString();
                            if(bxname!="" && bxcode!="")
                            {
                                DTdebug()<<"bxname"<<bxname<<"bxcode"<<bxcode;
                                bxuannumtmp++;
                                if( Factory == "NOVAT")
                                {
                                    if(FuzzyMatch(optionNum.toAscii(), bxcode.toAscii()))
                                    {
                                        equeloptionbnum++;
                                    }
                                    else
                                    {
                                        DTdebug() << "bixuan match fail" ;
                                    }
                                }
                                else if(Factory == "SVW2")
                                {
                                    if(ok)
                                    {
                                        if(FisMatch[bxname].toString() == bxcode)
                                        {
                                            equeloptionbnum++;

                                        }
                                        else
                                        {
                                            DTdebug() << "bixuan match fail" ;
                                        }
                                    }
                                }
                                else if(Factory =="BYDSZ")      //Style
                                {
                                    if(Style == bxcode)
                                    {
                                        DTdebug()<<"Style == bxcode=="<<bxcode<<t;
                                        equeloptionbnum++;
                                    }
                                    else if(bxcode =="?????")
                                    {
                                        DTdebug()<<"bxcode == ?????"<<t;
                                        equeloptionbnum++;
                                    }
                                    else
                                    {
                                        DTdebug() << "bixuan match fail"<<t ;
                                    }
                                }
                                else
                                {
                                    for(int m = 0;m< record.count();m++)//FIS
                                    {
                                        field = record.field(m);
                                        if(field.name() == bxname)
                                        {
                                            if(field.value().toString() == bxcode)
                                            {
                                                equeloptionbnum++;
                                                break;
                                            }
                                            else
                                            {
                                                DTdebug() << "bixuan match fail" ;
                                                break;
                                            }
                                        }
                                        else //bxname no equal
                                        {
                                            continue;
                                        }
                                    }
                                }
                            }
                            else //parameter name NULL
                            {}
                        }
                        equeloptionknum = 0;
                        kxuannumtmp = 0;
                        if(bxuannumtmp == equeloptionbnum)//bixuan parameter equal
                        {
                            DTdebug()<<"bixuan equal";
                            for(int j = 1;j < 6;j++)     // 可选
                            {
                                QString kxname = config->value(QString("carinfo").append(QString::number(whichar)).append("/kxuanname").append(QString::number(whichoption)).append(QString::number(j))).toString();
                                QString kxcode = config->value(QString("carinfo").append(QString::number(whichar)).append("/kxuancode").append(QString::number(whichoption)).append(QString::number(j))).toString();
                                if(kxname != "" && kxcode != "")
                                {
                                    DTdebug()<<"kxname"<<kxname<<"kxcode"<<kxcode;
                                    kxuannumtmp ++;
                                    if(Factory == "NOVAT")
                                    {
                                        equeloptionknum = 5;
                                    }
                                    else if(Factory == "SVW2")
                                    {
                                        if(ok)
                                        {
                                            if(FisMatch[kxname].toString() == kxcode)
                                            {
                                                DTdebug()<<"kexuan success";
                                                equeloptionknum = 5;
                                            }
                                            else
                                            {
                                                DTdebug() << "kexuan match fail" ;
                                                equeloptionknum = 0;
                                            }
                                        }
                                    }
                                    else if(Factory == "BYDSZ")
                                    {
                                        equeloptionknum = 5;
                                    }
                                    else
                                    {
                                        for(int m = 0;m< record.count();m++)
                                        {
                                            field = record.field(m);
                                            if(field.name() == kxname)
                                            {
                                                if(field.value().toString() == kxcode)
                                                {
                                                    equeloptionknum = 5;
                                                    break;
                                                }
                                                else
                                                {
                                                    DTdebug() << "kexuan match fail" ;
                                                    equeloptionknum = 0;
                                                }
                                            }
                                            else
                                            {}
                                        }
                                    }
                                    if(equeloptionknum == 5)//kexuan success jump out for(int j = 1;j < 6;j++)
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    equeloptionknum ++;
                                }
                            }
                            if( (bxuannumtmp == 0)&&(kxuannumtmp == 0) ) //bixuan and kexuan name all name NULL
                            {
                                equeloptionknum = 0;
                                DTdebug() << "bixuan and kexuan name all name NULL";
                            }
                            else if(equeloptionknum==5)
                            {
                                break; //jump out optional NO (1~5)  // mark by wxm 2021-6-11   find the option item
                            }
                        }
                        else //bixuan parameter optional error
                        {}
                    }
                    //else
                    //{}  //bolt number = 0  next optional
                }
                if( (equeloptionbnum == bxuannumtmp) && (equeloptionknum == 5))
                {
                    DTdebug() << "optional success" << whichar << whichoption<<equeloptionbnum;
                    readProGroupFlag = true;

                    int tmpOptionIobox =  config->value(QString("carinfo").append(QString::number(whichar)).append("/ioBoxOption").append(QString::number(whichoption))).toInt();
                    if(factory == "SVW2")
                        emit signalSetIoBox(tmpOptionIobox,true);
                    DTdebug() << "optional iobox" << tmpOptionIobox;

                    if(Factory=="BYDSZ" || Factory=="BYDXA" || Factory == "Haima")
                    {
                        if(query.exec("select AutoNO from "+tablePreview + " where VIN='"+VIN_PIN_SQL+"'"))
                        {
                            if(query.next())
                            {
                                AutoNO = query.value(0).toString();
                            }
                            else
                                DTdebug()<<"0 there is no VIN:"+VIN_PIN_SQL;
                        }
                        else
                            DTdebug()<<"select AutoNO fail 0"<<query.lastError();
                    }
                    else if(Factory == "Haima")
                    {
                        if(query.exec("select count(RecordID) from TaskPreview where UseFlag=0"))
                        {
                            int num = 0 ;
                            if(query.next())
                            {
                                num = query.value(0).toInt();
                                if(num < 5)
                                    Q_EMIT signalUpdateQueueSql();
                            }
                            else
                                DTdebug()<<""<<query.lastError();
                        }
                        else
                            DTdebug()<<"select AutoNO fail 0"<<query.lastError();


                        if(query.exec("select AutoNO from TaskPreview where VIN='"+VIN_PIN_SQL+"'"))
                        {
                            if(query.next())
                            {
                                AutoNO = query.value(0).toString();
                            }
                            else
                                DTdebug()<<"0 there is no VIN:"+VIN_PIN_SQL;
                        }
                        else
                            DTdebug()<<"select AutoNO fail 0"<<query.lastError();
                    }
                    carInfor[0].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong").append(QString::number(whichoption).append("1"))).toString();
                    carInfor[1].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong").append(QString::number(whichoption).append("2"))).toString();
                    for(int j = 1;j< D_BOLTNUM+1;j++)
                    {
                        channel = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPChannel").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                        if(channel == 1)
                        {
                            carInfor[0].proNo[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPProNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                            carInfor[0].boltNum[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                            if((carInfor[0].proNo[j-1].toInt())&&(carInfor[0].boltNum[j-1]))
                            {
                                carInfor[0].boltSN[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLuoSuanNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                                carInfor[0].ttNum[j-1]  = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPTaotong").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                                DTdebug()<<"000000000*"<<carInfor[0].boltSN[j-1];
                                boltSNList<<carInfor[0].boltSN[j-1];
                                DTdebug()<<"000000000**"<<carInfor[0].proNo[j-1];
                                DTdebug()<<"000000000***"<<carInfor[0].boltNum[j-1];
                                DTdebug()<<"000000000****"<<carInfor[0].ttNum[j-1];
                                groupAllBoltNumCh[0] += carInfor[0].boltNum[j-1];
                            }
                        }
                        else if(channel == 2)
                        {
                            carInfor[1].proNo[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPProNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                            carInfor[1].boltNum[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                            if((carInfor[1].proNo[j-1].toInt())&&(carInfor[0].boltNum[j-1]))
                            {
                                carInfor[1].boltSN[j-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLuoSuanNum").append(QString::number(whichoption)).append(QString::number(j))).toString();
                                carInfor[1].ttNum[j-1]  = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPTaotong").append(QString::number(whichoption)).append(QString::number(j))).toInt();
                                DTdebug()<<"111111111*"<<carInfor[1].boltSN[j-1];
                                DTdebug()<<"111111111**"<<carInfor[1].proNo[j-1];
                                DTdebug()<<"111111111***"<<carInfor[1].boltNum[j-1];
                                DTdebug()<<"111111111****"<<carInfor[1].ttNum[j-1];
                                groupAllBoltNumCh[1] += carInfor[1].boltNum[j-1];
                            }
                        }
                    }
                    BoltTotalNum = groupAllBoltNumCh[0] + groupAllBoltNumCh[1] ;
                    if(BoltTotalNum == 0)
                    {
                        systemStatus = 14;
                    }
                }
                else //optional fail
                {
                    DTdebug() << "***********optional fail***************"<< equeloptionbnum << bxuannumtmp << equeloptionknum;
                    equeloptionknum = 0;
                    equeloptionbnum = 0;
                    bxuannumtmp = 0;
                    //ui->label_tiaomastate->setText(tr("选配匹配失败"));
                    DTdebug()<<"optional match fail";
                    systemStatus = 11;
                    ISmaintenance = false;
                    gunPower(false);
                    if(IsQueue)
                    {
                        UpdateSqlFlag();
                        if(queue_timer.isActive())
                            queue_timer.stop();
                        queue_timer.start(1000);
                    }
                }
            }
            else//local fis fail
            {
                DTdebug() << "***********local fis fail***************";
                //                ui->label_tiaomastate->setText(tr("本地无此车信息"));
                if(optionalConfigurationTime == 1)
                {
                    Q_EMIT signalGetCarInfoFast(serialNums,flags,whichtmp);
                    optionalConfigurationTime ++ ;
                }
                else if (optionalConfigurationTime == 2) {
                    optionalConfigurationTime = 1 ;
                    systemStatus = 12;
                }

                ISmaintenance = false;
                DTdebug()<<"local fis match fail";
                gunPower(false);

                if(IsQueue)
                {
                    UpdateSqlFlag();
                    if(queue_timer.isActive())
                        queue_timer.stop();
                    queue_timer.start(1000);
                }
            }
        }
        else
        {
            if((whichtmp > 0) && (whichtmp <= CAR_TYPE_AMOUNT))
            {
                if(factory == "SVW2")
                    emit signalSetIoBox(mIoBox[whichtmp-1],true);
            }

            //不选配
            //设置获取匹配成功后的 程序号
            //再判断是否有套筒
            //判断是否有套筒
            readProGroupFlag = true;
            if(Factory=="BYDSZ" || Factory=="BYDXA")
            {
                if(query.exec("select AutoNO from "+tablePreview + " where VIN='"+VIN_PIN_SQL+"'"))
                {
                    if(query.next())
                    {
                        AutoNO = query.value(0).toString();
                    }
                    else
                        DTdebug()<<"there is no VIN:"+VIN_PIN_SQL;
                }
                else
                    DTdebug()<<"select AutoNO fail"<<query.lastError();
            }
            outBuf[0] = config->value("carinfo"+QString::number(whichar)+"/Out1").toString();
            outBuf[1] = config->value("carinfo"+QString::number(whichar)+"/Out2").toString();
            outBuf[2] = config->value("carinfo"+QString::number(whichar)+"/Out3").toString();
            outBuf[3] = config->value("carinfo"+QString::number(whichar)+"/Out4").toString();

            carInfor[0].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong01")).toString();
            carInfor[1].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong02")).toString();
            carInfor[2].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong03")).toString();
            carInfor[3].pdmName =  config->value(QString("carinfo").append(QString::number(whichar)).append("/pdmyinyong04")).toString();
            for(int m=1;m<D_BOLTNUM+1;m++)
            {
                channel = config->value(QString("carinfo").append(QString::number(whichar)).append("/Channel").append(QString::number(m))).toInt();
                if(channel == 1)
                {
                    carInfor[0].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                    carInfor[0].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                    if((carInfor[0].proNo[m-1].toInt())&&(carInfor[0].boltNum[m-1]))
                    {
                        carInfor[0].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                        carInfor[0].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                        DTdebug()<<"000000000*"<<carInfor[0].boltSN[m-1];
                        DTdebug()<<"000000000**"<<carInfor[0].proNo[m-1];
                        DTdebug()<<"000000000***"<<carInfor[0].boltNum[m-1];
                        DTdebug()<<"000000000****"<<carInfor[0].ttNum[m-1];
                        groupAllBoltNumCh[0] += carInfor[0].boltNum[m-1];
                    }
                }
                else if(channel == 2)
                {
                    carInfor[1].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                    carInfor[1].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                    if((carInfor[1].proNo[m-1].toInt())&&(carInfor[1].boltNum[m-1]))
                    {
                        carInfor[1].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                        carInfor[1].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                        DTdebug()<<"111111111*"<<carInfor[1].boltSN[m-1];
                        DTdebug()<<"111111111**"<<carInfor[1].proNo[m-1];
                        DTdebug()<<"111111111***"<<carInfor[1].boltNum[m-1];
                        DTdebug()<<"111111111****"<<carInfor[1].ttNum[m-1];
                        groupAllBoltNumCh[1] += carInfor[1].boltNum[m-1];
                    }
                }
                else if(channel == 3)
                {
                    carInfor[2].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                    carInfor[2].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                    if((carInfor[2].proNo[m-1].toInt())&&(carInfor[2].boltNum[m-1]))
                    {
                        carInfor[2].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                        carInfor[2].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                        groupAllBoltNumCh[2] += carInfor[2].boltNum[m-1];
                    }
                }
                else
                {
                    carInfor[3].proNo[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/ProNum").append(QString::number(m))).toString();
                    carInfor[3].boltNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(m))).toInt();
                    if((carInfor[3].proNo[m-1].toInt())&&(carInfor[3].boltNum[m-1]))
                    {
                        carInfor[3].boltSN[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/LuoSuanNum").append(QString::number(m))).toString();
                        carInfor[3].ttNum[m-1] = config->value(QString("carinfo").append(QString::number(whichar)).append("/Taotong").append(QString::number(m))).toInt();
                        groupAllBoltNumCh[3] += carInfor[3].boltNum[m-1];
                    }
                }
            }
            BoltTotalNum = groupAllBoltNumCh[0] + groupAllBoltNumCh[1] + groupAllBoltNumCh[2] + groupAllBoltNumCh[3] ;
            if(BoltTotalNum == 0)//error code
            {
                systemStatus = 14;
            }
        }
        DTdebug() << readProGroupFlag ;
        if(readProGroupFlag) //read program group and system ING
        {

            int n=0;
            for(n=0;n<4;n++) //4 channel send enable
            {
                if(groupAllBoltNumCh[n] != 0)
                {
                    allChannel++;
                }
            }
            if(ControlType_1 == "SB356_PLC")
            {
                n=0;
                int j = 0;
                for(j = 0;j< D_BOLTNUM;j++)
                {
                    if(carInfor[n].boltNum[j]>0)//bolt number
                    {
                        enableLsnumber = carInfor[n].boltNum[j];
                        QString pathpdm;
                        int k = 1;
                        for(k = 1;k<1000;k++)
                        {
                            if(carInfor[n].pdmName == config->value(QString("pdminfo").append(QString::number((k))).append("/pdmname")))
                            {
                                tempnumdpm[n] = config->value(QString("pdminfo").append(QString::number((k))).append("/num")).toInt();
                                //                                NOKflag = false;
                                pathpdm = config->value(QString("pdminfo").append(QString::number((k))).append("/pdmpath")).toString();
                                break;
                            }
                        }
                        ui->label_pdmarea->setStyleSheet(QString("border-image: url(/PDM/").append(pathpdm).append(")"));

                        /************发给web端空白螺栓****************/
                        sendWebValue(1,pathpdm);
                        lock.lockForWrite();
                        info[4] =QString::number(tempnumdpm[n]);
                        lock.unlock();
                        for(int i = 0;i < tempnumdpm[n];i++)
                        {
                            butt[n][i] = new QPushButton(ui->stackedWidget_2);
                            label1[n][i] = new QLabel(ui->stackedWidget_2);
                            label2[n][i] = new QLabel(ui->stackedWidget_2);

                            butt[n][i]->raise();
                            butt[n][i]->setFlat(true);
                            label1[n][i]->setAlignment(Qt::AlignLeft);
                            label2[n][i]->setAlignment(Qt::AlignLeft);
                            butt[n][i]->setFocusPolicy(Qt::NoFocus);
                            label1[n][i]->setFocusPolicy(Qt::NoFocus);
                            label2[n][i]->setFocusPolicy(Qt::NoFocus);
                            butt[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                            label1[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                            label2[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);

                            numpdm++;

                            butt[n][i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                            butt[n][i]->setText(QString::number((i+1)));

                            int tempx = config->value(QString("pdminfo").append(QString::number(k)).append("/tempx").append(QString::number((i+1)))).toInt();
                            int tempy =  config->value(QString("pdminfo").append(QString::number(k)).append("/tempy").append(QString::number((i+1)))).toInt();
                            lock.lockForWrite();
                            status[i][3] =QString::number(tempx);
                            status[i][4] =QString::number(tempy);
                            lock.unlock();
                            double x=0.0;
                            double y=0.0;
                            x = (double)tempx/1000 * 1166;
                            y = (double)tempy/1000 * 598;

                            //                                double x = (double)tempx/1000 * 1166 + 200;
                            //                                double y = (double)tempy/1000 * 598 + 170;
                            butt[n][i]->setGeometry(x,y,46,46);
                            label1[n][i]->setGeometry(x+23,y,140,23);
                            label2[n][i]->setGeometry(x+23,y+23,140,23);
                            label1[n][i]->setStyleSheet("background:transparent;");
                            label2[n][i]->setStyleSheet("background:transparent;");
                            butt[n][i]->show();
                            label1[n][i]->show();
                            label2[n][i]->show();
                        }
                        SYSS = "ING";
                        emit vinSendPLC(0,20,"\x11\x11"+VIN_PIN_SQL+"1");
                        break;
                    }
                    else
                    {
                        whichpronumis++;
                        continue;
                    }
                }
                if(j == D_BOLTNUM)
                {
                    //                    ui->label_tiaomastate->setText(tr(QString("channel" + QString::number(n+1) +"螺栓数量为0").toLatin1().data()));
                    systemStatus = 14;
                }
            }
            else //CS351 ATLAS
            {
                if(allChannel==1)
                    ui->stackedWidget_6->setCurrentIndex(0);
                else if(allChannel==2)
                    ui->stackedWidget_6->setCurrentIndex(1);
                for(n=0;n<4;n++) //4 channel send enable
                {
                    if(groupAllBoltNumCh[n] == 0)
                    {
                        continue;
                    }
                    else
                    {
                        whichPDM++;
                        int j = 0;
                        for(j = 0;j< D_BOLTNUM;j++)
                        {
                            if(carInfor[n].boltNum[j]>0)//bolt number
                            {
                                enableLsnumber = carInfor[n].boltNum[j];
                                QString pathpdm;
                                int k = 1;
                                for(k = 1;k<1000;k++)
                                {
                                    if(carInfor[n].pdmName == config->value(QString("pdminfo").append(QString::number((k))).append("/pdmname")))
                                    {
                                        tempnumdpm[n] = config->value(QString("pdminfo").append(QString::number((k))).append("/num")).toInt();
                                        //                                        NOKflag = false;
                                        pathpdm = config->value(QString("pdminfo").append(QString::number((k))).append("/pdmpath")).toString();
                                        break;
                                    }
                                }
                                if( allChannel == 1)
                                    ui->label_pdmarea->setStyleSheet(QString("border-image: url(/PDM/").append(pathpdm).append(")"));
                                else if(allChannel == 2)
                                {
                                    if(whichPDM == 1)
                                        ui->label_left->setStyleSheet(QString("image: url(/PDM/").append(pathpdm).append(")"));
                                    else
                                        ui->label_right->setStyleSheet(QString("image: url(/PDM/").append(pathpdm).append(")"));
                                }

                                /************发给web端空白螺栓****************/
                                sendWebValue(1,pathpdm);
                                lock.lockForWrite();
                                info[4] =QString::number(tempnumdpm[n]);
                                lock.unlock();
                                for(int i = 0;i < tempnumdpm[n];i++)
                                {
                                    if( allChannel == 1)
                                    {
                                        butt[n][i] = new QPushButton(ui->stackedWidget_2);
                                        label1[n][i] = new QLabel(ui->stackedWidget_2);
                                        label2[n][i] = new QLabel(ui->stackedWidget_2);
                                    }
                                    else if(allChannel == 2)
                                    {
                                        if(whichPDM == 1)
                                        {
                                            butt[n][i] = new QPushButton(ui->widget_left);
                                            label1[n][i] = new QLabel(ui->widget_left);
                                            label2[n][i] = new QLabel(ui->widget_left);
                                        }
                                        else if(whichPDM == 2)
                                        {
                                            butt[n][i] = new QPushButton(ui->widget_right);
                                            label1[n][i] = new QLabel(ui->widget_right);
                                            label2[n][i] = new QLabel(ui->widget_right);
                                        }
                                    }
                                    butt[n][i]->raise();
                                    butt[n][i]->setFlat(true);
                                    label1[n][i]->setAlignment(Qt::AlignLeft);
                                    label2[n][i]->setAlignment(Qt::AlignLeft);
                                    butt[n][i]->setFocusPolicy(Qt::NoFocus);
                                    label1[n][i]->setFocusPolicy(Qt::NoFocus);
                                    label2[n][i]->setFocusPolicy(Qt::NoFocus);
                                    butt[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                                    label1[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);
                                    label2[n][i]->setAttribute(Qt::WA_TransparentForMouseEvents);

                                    numpdm++;

                                    butt[n][i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                                    butt[n][i]->setText(QString::number((i+1)));

                                    int tempx = config->value(QString("pdminfo").append(QString::number(k)).append("/tempx").append(QString::number((i+1)))).toInt();
                                    int tempy =  config->value(QString("pdminfo").append(QString::number(k)).append("/tempy").append(QString::number((i+1)))).toInt();
                                    lock.lockForWrite();
                                    status[i][3] =QString::number(tempx);
                                    status[i][4] =QString::number(tempy);
                                    lock.unlock();
                                    double x=0.0;
                                    double y=0.0;

                                    if( allChannel == 1)
                                    {
                                        x = (double)tempx/1000 * 1166;
                                        y = (double)tempy/1000 * 598;
                                    }
                                    else if(allChannel == 2)
                                    {
                                        x = (double)tempx/1000 * 683;
                                        y = (double)tempy/1000 * 350;
                                    }
                                    //                                double x = (double)tempx/1000 * 1166 + 200;
                                    //                                double y = (double)tempy/1000 * 598 + 170;
                                    butt[n][i]->setGeometry(x,y,46,46);
                                    label1[n][i]->setGeometry(x+33,y,140,23);
                                    label2[n][i]->setGeometry(x+33,y+23,140,23);
                                    label1[n][i]->setStyleSheet("background:transparent;");
                                    label2[n][i]->setStyleSheet("background:transparent;");
                                    butt[n][i]->show();
                                    label1[n][i]->show();
                                    label2[n][i]->show();
                                }
                                SYSS = "ING";

                                groupNumCh[n] = j;
                                preChannel = n;
                                bool condition = GetTaotongNumCondition(n, j);
                                if((condition)&&(allChannel==1))
                                {
                                    DTdebug() << "here waitting taotong";
                                    preChannel = n;       //taotong channel
                                    sendEnableFlag = false;
                                    TaoTongState = true;
                                    SetLsInfo(carInfor[n].proNo[j], QString::number(carInfor[n].boltNum[j]), QString::number(carInfor[n].ttNum[j]));
                                    break;
                                }
#if 0
                                else if((isQRVerify) && (carInfor[n].boltNum[j]==1) )
                                {
                                    DTdebug() << "here waitting QRVerify" << j << allChannel;
                                    sendEnableFlag = false ;
                                    preChannel = n ;
                                    break ;

                                }
#endif
                                else
                                {
                                    if(allChannel==1)
                                    {
                                        SetLsInfo(carInfor[n].proNo[j], QString::number(carInfor[n].boltNum[j]), QString::number(carInfor[n].ttNum[j]));
                                    }
                                    if(n == 0)
                                    {
                                        emitOperate1(true,j);
                                        DTdebug()<<"*******send enable ch1*********group:"<<j;
                                    }
                                    else if(n == 1)
                                    {
                                        emitOperate2(true,j);
                                        DTdebug()<<"*******send enable ch2*********group:"<<j;
                                    }
                                    else if(n == 2)
                                    {
                                        emitOperate3(true,j);
                                        DTdebug()<<"*******send enable ch3*********group:"<<j;
                                    }
                                    else
                                    {
                                        emitOperate4(true,j);
                                        DTdebug()<<"*******send enable ch4*********group:"<<j;
                                    }
                                    break;
                                }
                            }
                            else
                            {
                                whichpronumis++;
                                continue;
                            }
                        }
                        if(j == D_BOLTNUM)
                        {
                            //                            ui->label_tiaomastate->setText(tr(QString("channel" + QString::number(n+1) +"螺栓数量为0").toLatin1().data()));
                            systemStatus = 14;
                        }
                    }
                }
            }
            if(SYSS == "ING")
            {
                if(PDMBoltNum>0)
                {
                    if(tempnumdpm[0]>PDMBoltNum)
                    {
                        pdmflickerNum = PDMBoltNum;
                    }
                    else
                    {
                        pdmflickerNum = tempnumdpm[0];
                    }
                }
                DTdebug() <<"*********** syss ing **********"<<pdmflickerNum<<tempnumdpm[0];
                emit sendRunStatusToJson("ING");
                emit vinSendPLC(0,6,"\x11\x11"+outBuf[0]+outBuf[1]+outBuf[2]+outBuf[3]);
                emit sendnexo(serialNums);//download VIN
                ui->progressBar->setValue(0);
                m_CurrentValue  = 0;
                Start(100, Tacktime*10);
                ISmaintenance = true;
                QualifiedNum = 0;


                if(!IOFlag)
                {
                    Q_EMIT sendCmdToCs351FromMW(5);    //由于槽函数的参数定义为int类型，暂用。空闲时可改为枚举类型。
                }
                else {
                    Q_EMIT signalSendTaotongNum();
                }
                timerpdm.start(500);
                displayStatusIcon("ING");
                //white_led(1);
                whichpdmnumnow[0] = 0;
                whichpdmnumnow[1] = 0;
                whichpdmnumnow[2] = 0;
                whichpdmnumnow[3] = 0;
                PDMBoltBuf[0] = 1;
                PDMBoltBuf[1] = 1;
                PDMBoltBuf[2] = 1;
                PDMBoltBuf[3] = 1;
                PDMBoltBuf[4] = 1;
                PDMBoltStute[0] = 0;
                PDMBoltStute[1] = 0;
                PDMBoltStute[2] = 0;
                PDMBoltStute[3] = 0;
                PDMBoltStute[4] = 0;
                StartBolt = 0;
                currentBoltSum = 0;
                currentFirstBolt = 0;
                allTightenResultFlag = true;
            }
            else
            {
                DTdebug() << "*******vin invalid********";
                ISmaintenance = false;
                DTdebug()<<"vin invalid";
                gunPower(false);
                if(IsQueue)
                {
                    UpdateSqlFlag();
                    if(queue_timer.isActive())
                        queue_timer.stop();
                    queue_timer.start(1000);
                }
            }
        }
        if(isRFID)
        {
            int k = 0;
            QString lsNumTemp="";
            for(k = 0;k <=D_BOLTNUM;k++)
            {
                if(!optionOrNot)  //非选配
                {
                    lsNumTemp = config->value(QString("carinfo").append(QString::number(whichar)).append("/LSNumber").append(QString::number(k))).toString();
                }
                else
                {
                    lsNumTemp = config->value(QString("carinfo").append(QString::number(whichar)).append("/OPLSNumber").append(QString::number(whichoption)).append(QString::number(k))).toString();
                }
                if(lsNumTemp.toInt() >0)
                {
                    bolt_index = k;
                }
            }
        }
        delete config;
    }
    else//vin match fail
    {
        //        ui->label_tiaomastate->setText(tr("特征码匹配失败"));
        if(Factory == "Benz"||Factory == "BYDXA")
        {
            systemStatus = 21;
            ISmaintenance = false;
            DTdebug()<<"################Job match fail";
            if(IsQueue)
            {
                UpdateSqlFlag();
                if(queue_timer.isActive())
                    queue_timer.stop();
                queue_timer.start(1000);
            }
        }
        else
        {
            if(isAlign)
            {
                //yellow_led(0);
                //red_led(0);
                isAlign = false;
            }
            systemStatus = 13;
            ISmaintenance = false;
            DTdebug()<<"################vin match fail";
            gunPower(false);
            if(IsQueue && (factory == "BYDSZ" || factory == "BYDXA" || m_BAICQueue) )
            {
                UpdateSqlFlag();
                if(queue_timer.isActive())
                    queue_timer.stop();
                queue_timer.start(1000);
            }
            else if (IsQueue && (factory=="Haima"))
            {
                Q_EMIT signalGetCarInfo(0,serialNums);

            }
            else if (factory == "Haima")
            {
                Q_EMIT signalGetCarInfo(0,VIN_PIN_SQL);

            }
            else if(IsQueue && factory == "BAIC")
            {
                QTimer::singleShot(5000,this,SLOT(leuzeTimerStart()));
            }
        }
    }
    DTdebug() << "VIN_PIN_SQL   serialNums" << VIN_PIN_SQL << serialNums;
}

void MainWindow::SetLsInfo(QString pProNum, QString pLsNum, QString pTtNum)
{
    DTdebug() << "SetLsInfo---pTtNum" << pTtNum;
    ui->label_pronum->setText(pProNum);
    ui->label_lsnum->setText(pLsNum);
    ui->label_taotong->setText(pTtNum);

    if(Factory =="AQCHERY")
    {
        setTaotongLed(pTtNum.toInt());
    }
    else
    {
        Q_EMIT signalSendTaotongNum();
    }
}


void MainWindow::slot_PLCHeartbeat(bool pIsHeartbeat)
{
    //if(pIsHeartbeat)
    //    ui->label_plcHeart_ss5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
    //else
    //    ui->label_plcHeart_ss5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
}

/*****************匹配G9码*********************/
void MainWindow::searchG9Num()
{
    FUNC() ;
    //匹配G9
    rfidFlag = false ;
    RfidNum  = 0;
    int i = 0;
    QString temp;
    bool flagss = false;
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
#if 0
    for(i = 1;i < (CAR_TYPE_AMOUNT+1);i++)     //100个车型
#else
    for(i = (CAR_TYPE_AMOUNT+1);i > 0;i--)    //100个车型
#endif
    {
        temp = config->value(QString("carinfo").append(QString::number(i)).append("/G9")).toString();
        //DTdebug() << temp ;

        if(temp.length()!=G9Len)
        {
//            DTdebug()<<temp.length();
            continue;
        }
        flagss = true;
        for(int j=0;j<G9Len;j++)
        {
            if(temp.at(j) == '?')
                continue;
            else
            {
                if(temp.at(j) == tempG9.at(j))
                {

                    flagss = true;
                    continue;
                }
                else
                {
                    DTdebug() << "false---carinfo/G9 is not equal PIN's G9 " ;
                    flagss = false;
                    break;
                }
            }
        }
        if(flagss)
        {
            break;
        }

    }
    delete config;

#if 0
    if(isQRVerify)
    {
        if(flagss)
        {
            rfidFlag = flagss ;
            RfidNum  = i ;
        }
        else {
            VinPinCodeView(flagss,i);
            ui->label_tiaomastate->setText("RFID校验失败");
        }
    }
    else
    {
        VinPinCodeView(flagss,i);
    }
#else
    VinPinCodeView(flagss,i);
#endif
}

//匹配QRCode
void MainWindow::searchQRCodeNum(QString strQRCode)
{
    DTdebug() << strQRCode << strQRCodeRule ;
    DTdebug() << strQRCode.length() << strQRCodeRule.length();

    QStringList strList = strQRCode.split(",");

    flagQR = true;

    for(int i =0;i<strList.size();i++)
    {
        if(strList.at(i).length() == strQRCode.length())
        {
            for(int j=0;j<strQRCodeRule.length();j++)
            {
                if(strQRCodeRule.at(j) == '?')
                    continue;

                else
                {
                    DTdebug() << strQRCodeRule.at(j) << strQRCode.at(j) ;
                    if(strQRCodeRule.at(j) == strQRCode.at(j))           //to do
                    {
                        flagQR = true;
                        continue;
                    }
                    else
                    {
                        DTdebug() << "false" ;
                        flagQR = false;
                        break;
                    }
                }
            }
        }
        else
        {
            flagQR = false ;
        }
    }

    if(flagQR)
    {
        ui->labelQRVerify->setText("零件规则校验成功");
        ui->labelQRVerify->setStyleSheet("background-color:green;color: white;font: 20pt \"黑体\";");
        if(!ui->labelQRVerify->isVisible())
        {
            ui->labelQRVerify->setVisible(true);
        }

        strQRCode = "" ; //成功清除校验规则，不成功继续校验
        strQRCodeRule = "" ;
#if 0
        //校验成功发送使能
        if(allChannel==1)
        {
            SetLsInfo(carInfor[preChannel].proNo[groupNumCh[preChannel]]), (QString::number(carInfor[preChannel].boltNum[groupNumCh[preChannel]]), (QString::number(carInfor[preChannel].ttNum[groupNumCh[preChannel]]));
        }
        if(preChannel == 0) //enable
        {
            emitOperate1(true,groupNumCh[preChannel]);
        }
        else if(preChannel == 1)
        {
            emit emitOperate2(true,groupNumCh[preChannel]);
        }
        else if(preChannel == 2)
        {
            emit emitOperate3(true,groupNumCh[preChannel]);
        }
        else
        {
            emit emitOperate4(true,groupNumCh[preChannel]);
        }
#endif
    }
    else {
        ui->labelQRVerify->setText("零件规则校验失败");
        ui->labelQRVerify->setStyleSheet("background-color:red;color: white;font: 20pt \"黑体\";");
        if(!ui->labelQRVerify->isVisible())
        {
            ui->labelQRVerify->setVisible(true);
        }

    }
    
//    QTimer::singleShot(10000, this, SLOT(slotHideLabelQRVerify()));       //校验状态 不 定时消失



}

/*******************自动匹配VIN码***************************/
void MainWindow::autoSearchVinNum(QString vinCode)
{
    serialNums = vinCode;
    ui->label_17->setText(vinCode);
    SetLsInfo("", "", "");
    ui->label_hege->setText("");
    if(timerpdm.isActive())
    {
        timerpdm.stop();
    }
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<tempnumdpm[i];j++)
        {
            delete butt[i][j];
            delete label1[i][j];
            delete label2[i][j];
        }
        tempnumdpm[i] = 0;
        PDMCurrentState[i] = "OK";
    }
    pdmnowromisOk = true;
    systemStatus = 0;//clear error status
    BoltOrder[0] = 1;
    BoltOrder[1] = 1;
    QualifiedNum = 0;
    //red_led(0);
    //green_led(0);
    emit sLightLogic(e_AutoSearchVinNum);
    searchVinNum();

}

/*******************匹配VIN码***************************/
void MainWindow::searchVinNum()
{
    FUNC() ;
    cflags = false ;
    bool tempFlag = false ;
    //匹配VIN码
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    bool flags = false;
    int i = 1;
    int len = 0;
    if(Factory == "Dongfeng")
    {
        vinAttributeCode = serialNums.mid(0,3) + serialNums[4] +serialNums[6];
        len = 5;
    }
    else if(Factory == "BYDXA")     //扫零件码工线
    {
        vinAttributeCode = serialNums.mid(6,3);
        len = 3;
    }
    else if(Factory == "NOVAT")
    {
        vinAttributeCode = serialNums.right(8);
        DTdebug() << serialNums << vinAttributeCode;
        len = 8;
    }
    else if(Factory == "AQCHERY"||Factory == "KFCHERY")
    {
        if(mVinjob == "VIN+JOBID")
        {
            vinAttributeCode = AQ_JOBID;
            DTdebug() << AQ_JOBID << vinAttributeCode;
            len = AQ_JOBID.size();
        }
        else if(mVinjob == "VIN+CarType")
        {
            vinAttributeCode = AQ_carType;
            DTdebug() << AQ_carType << vinAttributeCode;
            len = 2;
        }
        else if(mVinjob == "BarCode")
        {
            vinAttributeCode = serialNums.mid(0,2);
            DTdebug() << serialNums << vinAttributeCode;
            len = 2;    //len = vinAttributeLen;
        }
        else
        {

        }
    }
    else
    {
        vinAttributeCode = serialNums.mid(vinAttributeBit,5);
        DTdebug() << serialNums << vinAttributeCode << vinAttributeBit ;
        len = 5;
    }
#if 0
    for(i = 1;i < (CAR_TYPE_AMOUNT+1);i++)    //100个车型
#else
    for(i = (CAR_TYPE_AMOUNT+1);i > 0;i-- )    //230个车型
#endif
    {
        QString temp;
        if(mVinjob == "VIN+JOBID")
            temp = config->value(QString("carinfo").append(QString::number(i)).append("/carcx")).toString();
        else
            temp = config->value(QString("carinfo").append(QString::number(i)).append("/VIN")).toString();

        if(Factory == "KFCHERY"){
            len = temp.size();
        }
//        qDebug()<<"temp.size():"<<temp.size()<<temp.length();
        //海马分装线校验码长度不定
        if(Factory == "Haima" && StationName.contains("分装线"))
        {
            len = temp.length() ;
            vinAttributeCode = serialNums.mid(vinAttributeBit,len);
            if(len >serialNums.length())
            {
                continue ;
            }
        }
        if(temp.size()!=len || temp.size() == 0)
        {
            continue;
        }
        flags = true;
        for(int k = 0;k < len;k++)
        {
//            qDebug()<<"vinAttributeCode.at(k) != temp.at(k)"<<vinAttributeCode.at(k) << temp.at(k);
            if(temp.at(k) == '?')
            {
                continue;
            }
            else
            {
                if(vinAttributeCode.at(k) != temp.at(k))
                {
                    flags = false;
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
//        qDebug()<<"whicharresult:"<<i<<flags;
        if(flags)
        {
            cflags = true ;
            whichar = i ;
            break;
        }
    }
#if 1
    if(Factory == "BAIC")
    {
        QString strVin = serialNums ;
        strExecCatType = config->value(QString("carinfo").append(QString::number(i)).append("/execCarType")).toString();
        if(strExecCatType == "")
        {

        }
        else
        {
            QString selectCarType = "" ;
            QString carType = "" ;
            selectCarType = "select carType from "+tablePreview+" where VIN ='" + strVin + "'";
            DTdebug()<<strVin;

            if(!db.isOpen()||!QSqlDatabase::contains("mysqlconnectionmatch"))
                mysqlopen();
            if(db.isOpen() && QSqlDatabase::contains("mysqlconnectionmatch"))
            {
                query.exec(selectCarType/*+seri.append("'")*/);

                if(query.next())  //查到了匹配
                {
                    DTdebug()<<"select cartype success";
                    carType = query.value(0).toString() ;
                    if(strExecCatType == carType)
                    {
                        flags =true ;
                    }
                    else {
                        flags = false;
                    }

                }
                else
                {

                    flags = false ;
                    DTdebug() << "select car type fail" << query.lastError() ;
                }
            }
        }
    }
#endif
    if(manuslOperationAdjust)
    {
        DTdebug()<<factory;
        for(int i = 0;i<CAR_TYPE_AMOUNT; i++)
        {
            carType = config->value(QString("carinfo").append(QString::number(i+1)).append("/carcx")).toString();

            if(carType == carTypeHaima)
            {
                whichar = i+1 ;
                flags = true ;
                DTdebug() << whichar << carType;
                break;
            }
            else {
                flags = false;
            }
        }
    }
    delete config;
    rfidFlag = flags ;

    emit signalSendHttp(E_SendBeginTightening);
    VinPinCodeView(flags,whichar);
}

/*******************校验PIN码*************************/
bool MainWindow::pinCodeVerify(QByteArray pinBuf,int len)
{    
    int sum,sumOdd,sumEven,result,i;
    QByteArray PinTempBuf;
    sum = 0;
    sumOdd = 0;
    sumEven = 0;

    for(i=0;i<12;i++)//right 12 bit range(0~9)
    {
        if((pinBuf[i+2]>'9')||(pinBuf[i+2]<'0'))
        {
            return 0;
        }
    }

    DTdebug()<<"pinCodeVerify_pinBuf:"<< pinBuf <<"pinlen:"<<len;

    for(i=0;i<len;i++)
    {
        switch(pinBuf[i])
        {
        case '0':
            PinTempBuf[i] = 0;
            break;
        case '1':
        case 'A':
        case 'J':
        case 'S':
            PinTempBuf[i] = 1;
            break;
        case '2':
        case 'B':
        case 'K':
        case 'T':
            PinTempBuf[i] = 2;
            break;
        case '3':
        case 'C':
        case 'L':
        case 'U':
            PinTempBuf[i] = 3;
            break;
        case '4':
        case 'D':
        case 'M':
        case 'V':
            PinTempBuf[i] = 4;
            break;
        case '5':
        case 'E':
        case 'N':
        case 'W':
            PinTempBuf[i] = 5;
            break;
        case '6':
        case 'F':
        case 'O':
        case 'X':
            PinTempBuf[i] = 6;
            break;
        case '7':
        case 'G':
        case 'P':
        case 'Y':
            PinTempBuf[i] = 7;
            break;
        case '8':
        case 'H':
        case 'Q':
        case 'Z':
            PinTempBuf[i] = 8;
            break;
        case '9':
        case 'R':
        case 'I':
            PinTempBuf[i] = 9;
            break;
        default:
            return false ;
            break;
        }
    }
    for(i=0;i<len-1;i+=2)
    {
        sumOdd += PinTempBuf[i];
    }
    for(i=1;i<len-1;i+=2)
    {
        sumEven += PinTempBuf[i];
    }
    sum = sumOdd*3 + sumEven;
    result = (10-sum%10)%10;
    if(result == PinTempBuf[len-1])
    {
//        DTdebug()<<"Rfid pinCodeVerify success";
        return true;
    }
    else
    {
//        DTdebug()<<"Rfid pinCodeVerify fail";
        return false;
    }
}
bool MainWindow::pinCodeRightVerify(QString pinBuf1,int len)
{
    int sum,sumOdd,sumEven,result,i;
    QByteArray pinTemp;
    QString pinTempBuf="87654321",pinBuf = "12345678";
    pinBuf = pinBuf1.mid(6,len);
    sum = 0;
    sumOdd = 0;
    sumEven = 0;

    for(i=0;i<8;i++)//right 8 bit range(0~9)
    {
        if((pinBuf[i]>'9')||(pinBuf[i]<'0'))
        {
            return 0;
        }
    }

    DTdebug()<<"pinCodeRightVerify  pinBuf1:"<< pinBuf1 <<"len:"<<len;

    for(i=0;i<len;i++)
    {
        pinTemp = pinBuf.mid(i,1).toLatin1();
        switch(pinTemp[0])
        {
        case '0':
            pinTempBuf.replace(i,1,"0");
            break;
        case '1':
        case 'A':
        case 'J':
        case 'S':
            pinTempBuf.replace(i,1,"1");
            break;
        case '2':
        case 'B':
        case 'K':
        case 'T':
            pinTempBuf.replace(i,1,"2");
            break;
        case '3':
        case 'C':
        case 'L':
        case 'U':
            pinTempBuf.replace(i,1,"3");
            break;
        case '4':
        case 'D':
        case 'M':
        case 'V':
            pinTempBuf.replace(i,1,"4");
            break;
        case '5':
        case 'E':
        case 'N':
        case 'W':
            pinTempBuf.replace(i,1,"5");
            break;
        case '6':
        case 'F':
        case 'O':
        case 'X':
            pinTempBuf.replace(i,1,"6");
            break;
        case '7':
        case 'G':
        case 'P':
        case 'Y':
            pinTempBuf.replace(i,1,"7");
            break;
        case '8':
        case 'H':
        case 'Q':
        case 'Z':
            pinTempBuf.replace(i,1,"8");
            break;
        case '9':
        case 'R':
        case 'I':
            pinTempBuf.replace(i,1,"9");
            break;
        default:
            break;
        }
    }
    for(i=0;i<len-1;i+=2)
    {
        sumOdd += pinTempBuf.mid(i,1).toInt();
    }
    for(i=1;i<len-1;i+=2)
    {
        sumEven += pinTempBuf.mid(i,1).toInt();
    }
    sum = sumOdd*3 + sumEven;
    result = (10-sum%10)%10;
    if(result == pinTempBuf.mid(len-1,1).toInt())
    {
//         DTdebug()<<"Rfid pinCodeRightVerify success";
        return true;
    }
    else
    {
//         DTdebug()<<"Rfid pinCodeRightVerify fail";
        return false;
    }
}
/*******************校验VIN码*************************/
int MainWindow::VIN_VerifyFunc( char *buf)
{
    //校验VIN码
    unsigned char i;
    unsigned char VinTempBuf[17];
    unsigned short  Sum_Num;

    if(Factory=="NOVAT"||Factory == "AQCHERY"||Factory=="BAIC"||Factory == "GZBAIC"||Factory == "GM" ||Factory == "Haima" ||isReplaceBarcode ||(Factory == "Dongfeng" && manualMode))   //GM VIN not need verify
    {
        return 1;
    }

    for(i=0;i<VINhead.size();i++)
    {
        if(buf[i]!=VINhead[i])
            return 0;
    }

    for(i=0;i<6;i++)//right 6 bit range(0~9)
    {
        if((buf[i+11]>'9')||(buf[i+11]<'0'))
        {
            return 0;
        }
    }

    for(i=0;i<17;i++)
    {
        if(i == 8)
        {
            if(buf[i]=='X') //校验位不需要
            {
                VinTempBuf[i] = 'X';
                continue;
            }
        }
        switch(buf[i])
        {
        case '0':
            VinTempBuf[i] = 0;
            break;
        case '1':
        case 'A':
        case 'J':
            VinTempBuf[i] = 1;
            break;
        case '2':
        case 'B':
        case 'K':
        case 'S':
            VinTempBuf[i] = 2;
            break;
        case '3':
        case 'C':
        case 'L':
        case 'T':
            VinTempBuf[i] = 3;
            break;
        case '4':
        case 'D':
        case 'M':
        case 'U':
            VinTempBuf[i] = 4;
            break;
        case '5':
        case 'E':
        case 'N':
        case 'V':
            VinTempBuf[i] = 5;
            break;
        case '6':
        case 'F':
        case 'W':
            VinTempBuf[i] = 6;
            break;
        case '7':
        case 'G':
        case 'P':
        case 'X':
            VinTempBuf[i] = 7;
            break;
        case '8':
        case 'H':
        case 'Y':
            VinTempBuf[i] = 8;
            break;
        case '9':
        case 'R':
        case 'Z':
            VinTempBuf[i] = 9;
            break;
        default:
            return 0;//Vin code valid
            //				break;
        }
    }
    Sum_Num  = VinTempBuf[7]*10;
    Sum_Num += VinTempBuf[9]*9;
    Sum_Num += (VinTempBuf[0]+VinTempBuf[10])*8;
    Sum_Num += (VinTempBuf[1]+VinTempBuf[11])*7;
    Sum_Num += (VinTempBuf[2]+VinTempBuf[12])*6;
    Sum_Num += (VinTempBuf[3]+VinTempBuf[13])*5;
    Sum_Num += (VinTempBuf[4]+VinTempBuf[14])*4;
    Sum_Num += (VinTempBuf[5]+VinTempBuf[15])*3;
    Sum_Num += (VinTempBuf[6]+VinTempBuf[16])*2;
    Sum_Num %=11; //求余数
    DTdebug()<<"***********sum"<<Sum_Num<<VinTempBuf[8];
    if(Sum_Num == 10)
    {
        if(VinTempBuf[8] == 'X')
            return 1;//Vin code valid
        else
            return 0;//Vin code invalid
    }
    else
    {
        if(VinTempBuf[8] == Sum_Num)
            return 1;//Vin code valid
        else
            return 0;//Vin code invalid
    }
    return 0;
}

void MainWindow::receiveLogin(bool tmp)
{
    isLogin = tmp;
}

/********************获取扫描枪的条码**************************/
void MainWindow::handleCodeBarcode(QString code,bool invalidFlag)
{
    QString tmpSerialnums;
    if(Factory == "NOVAT")
    {
        tmpSerialnums = code.left(17);
        ui->label_17->setText(tmpSerialnums);
    }
    else if(Factory == "AQCHERY")
    {
        AQ_carType = code.mid(17);
        code = code.mid(0,17);
        ui->label_17->setText(code);
        DTdebug()<<"show"<<code;
    }
    else
        ui->label_17->setText(code);
    if(invalidFlag)
    {
        systemStatus = 9;
        gunPower(false);
        ISmaintenance = false;
    }
    else
    {
        if(SYSS == "OK" || SYSS == "NOK")
        {
            cleanDisplay();
        }
        //不重复 校验
        if(Factory == "Benz")    //to do
        {
            if(!isLogin)
                return;
            if(!controlMode)
            {
                searchJob(code);
            }
            else
            {
                tempG9 = code;
                searchG9Num();
            }
        }
        else if(Factory == "NOVAT")
        {
            //if(VIN_VerifyFunc(tmpSerialnums.toLatin1().data()))//VIN code verify
            {
                searchVinNum();
            }
        }
        else
        {
            bool tmpFlag = VIN_VerifyFunc(code.toLatin1().data());
            emit signalSendHttp(tmpFlag?E_SendVinVerifyOK:E_SendVinVerifyFail);
            if(tmpFlag)//VIN code verify
            {
                searchVinNum();
            }
        }
    }

}

void MainWindow::handleCodeBarcode_PIN(QString vincode, QString pincode,bool invalidFlag)
{
    QString code = vincode;
    ui->label_17->setText(pincode);
    if(invalidFlag)
    {
        systemStatus = 9;
        gunPower(false);
        ISmaintenance = false;
    }
    else
    {
        if(SYSS == "OK" || SYSS == "NOK")
        {
            cleanDisplay();
        }
        //不重复 校验
        if(Factory == "Benz")    //to do
        {
            if(!isLogin)
                return;
            if(!controlMode)
            {
                searchJob(code);
            }
            else
            {
                tempG9 = code;
                searchG9Num();
            }
        }
        else if(Factory == "NOVAT")
        {
            //if(VIN_VerifyFunc(tmpSerialnums.toLatin1().data()))//VIN code verify
            {
                searchVinNum();
            }
//            else
//            {
//                systemStatus = 10;
//                gunPower(false);
//                ISmaintenance = false;
//            }
        }
        else
        {
            //if(VIN_VerifyFunc(code.toLatin1().data()))//VIN code verify
            {
                searchVinNum();
            }
//            else
//            {
//                systemStatus = 10;
//                gunPower(false);
//                ISmaintenance = false;
//                if(IsQueue && factory == "BAIC")
//                {
//                    QTimer::singleShot(5000,this,SLOT(leuzeTimerStart()));
//                }
//            }
        }
    }

}
/******************************************************/
//RFID mode receive PIN code
/******************************************************/
void MainWindow::handleCodeRFID(QString code,QString G9,bool invalidFlag)
{
    if(code =="0"||code =="-1")   //东风柳汽
    {
        if(isAlign)
        {
            //yellow_led(0);
            //red_led(0);
            emit sLightLogic(e_HandleCodeRFIDAlign);
            isAlign = false;
        }
        if(code == "0")
            ui->label_17->setText(tr("没有获取到VIN码"));
        else if(code == "-1")
            ui->label_17->setText(tr("当前车辆为空"));
        return;
    }
    tempG9  = G9;
    DTdebug() << G9;
    ui->label_17->setText(code);
    if(invalidFlag)//pin码重复
    {
        systemStatus = 9;
    }
    else //不重复匹配
    {
        if(SYSS == "OK" || SYSS == "NOK")
        {
            cleanDisplay();
        }
        if(Factory == "Benz")
        {
            if(!isLogin)
                return;
            if(!controlMode)
            {
                searchJob(code);
            }
            else
            {
                tempG9 = code;
                searchG9Num();
            }
        }
        else if(Factory == "BAIC")
        {
            if(!isLogin)
                return;
            //用条码规则校验
            handleCodeBarcode(code,false);
        }
        else if(Factory == "Dongfeng")
        {
            if(isAlign)
            {
                //red_led(1);
                emit LightLogic(e_HandleCodeRFIDDongFengAlign);
            }
            tempG9 = tempG9.mid(0,3)+ tempG9[4] +tempG9[6];
            searchG9Num();
        }
        else
        {
            DTdebug() << "1";
            if(pinCodeRightVerify(code.toLocal8Bit(),8))//PIN code verify
            {
                searchG9Num();
            }
            else
            {
                if(pinCodeVerify(code.toLocal8Bit(),14))
                {
                    searchG9Num();
                }
                else
                {
                    systemStatus = 10;
                }
            }
        }
    }
}

void MainWindow::handleCodeBarcode_NoJobID(QString code,QString vinCarType, bool invalidFlag)
{
    mVinjob = vinCarType;

    QString tmpSerialnums;
    if(Factory == "NOVAT")
    {
        tmpSerialnums = code.left(17);
        ui->label_17->setText(tmpSerialnums);
    }
    else if(Factory == "AQCHERY"||Factory == "KFCHERY")
    {
        if(mPartCode == 1)
        {
            AQ_carType = code; // 特殊工位只有一个码，直接拿来匹配工艺
            DTdebug()<<"show PartCode"<<code;
        }
        else
        {
            if(Factory == "KFCHERY"){
                AQ_carType = code.mid(0,17); // 普通工位取后面的物料码来匹配工艺
                code = code.mid(17);//从17位开始取到底
            }else{
                AQ_carType = code.mid(17); // 普通工位取后面的物料码来匹配工艺
                code = code.mid(0,17);
            }
            DTdebug()<<"show code"<<code;
        }
        ui->label_17->setText(code);

    }
    else
        ui->label_17->setText(code);
    if(invalidFlag)
    {
        systemStatus = 9;
        gunPower(false);
        ISmaintenance = false;
    }
    else
    {
        if(SYSS == "OK" || SYSS == "NOK")
        {
            cleanDisplay();
        }
        //不重复 校验
        if(Factory == "Benz")    //to do
        {
            if(!isLogin)
                return;
            if(!controlMode)
            {
                searchJob(code);
            }
            else
            {
                tempG9 = code;
                searchG9Num();
            }
        }
        else if(Factory == "NOVAT")//不校验VIN码
        {
            searchVinNum();
        }
        else
        {
            if(mPartCode == 1)
            {
                searchVinNum();
            }
            else
            {
                bool tmpFlag = VIN_VerifyFunc(code.toLatin1().data());
                emit signalSendHttp(tmpFlag?E_SendVinVerifyOK:E_SendVinVerifyFail);
                if(tmpFlag || Factory == "KFCHERY")//VIN code verify
                {
                    searchVinNum();
                }
            }

        }
    }

}

void MainWindow::handleCodeBarcode_JOBID(QString code, QString vinjob, bool invalidFlag)
{
    mVinjob = vinjob;

    if(Factory == "AQCHERY"||Factory == "KFCHERY")
    {
        AQ_JOBID = code.mid(17);
        code = code.mid(0,17);
        ui->label_17->setText(code);
        DTdebug()<<"show"<<code;
    }
    else
        ui->label_17->setText(code);
    if(invalidFlag)
    {
        systemStatus = 9;
        gunPower(false);
        ISmaintenance = false;
    }
    else
    {
        if(SYSS == "OK" || SYSS == "NOK")
        {
            cleanDisplay();
        }
        {
            bool tmpFlag = VIN_VerifyFunc(code.toLatin1().data());
            emit signalSendHttp(tmpFlag?E_SendVinVerifyOK:E_SendVinVerifyFail);
            if(tmpFlag)//VIN code verify
            {
                searchVinNum();
            }
        }
    }
}

/********************获取扫描枪的条码**************************/
#include "serialComs.h"
#include "rfidcontroler.h"

void MainWindow::getSerialNum(QString serialNum,bool equeled,QString tempp)
{
    FUNC() ;
    ui->LabelResult_color->hide();
    ui->LabelResult_text->hide();
    DTdebug()<<serialNum<<tempp;
    if( (!controlMode && Factory =="BYDSZ") || Factory != "BYDSZ")
    {
        DTdebug()<<isRFID<<tempp;
        systemStatus = 0;//clear error status
        serialNums = serialNum;

        if(Special_Station)
        {
            DTdebug()<< "Special_Station";
            if(isBarCode)     //条码枪
            {
                DTdebug()<< "isBarCode";
                if(tempp =="VIN+G9")
                {
                    //设置条码
                    if(SYSS != "ING")
                    {
                        serialNums = serialNum.mid(0, vinLen);
                        handleCodeRFID(serialNums,serialNum.mid(vinLen,5),equeled);
                    }
                }
                else if(tempp =="VIN+FIS")
                {
                    DTdebug()<< "VIN+FIS";
                    if(SYSS != "ING")
                    {
                        DTdebug()<< "serialNum";
                        QString tmpVin = serialNums.mid(14);
                        QString tmpPin = serialNums.mid(0,14);
                        serialNums = tmpVin;
                        VIN_PIN_SQL_of_SpecialStation = tmpPin;
                        //handleCodeBarcode(serialNums.mid(14),equeled);
                        handleCodeBarcode_PIN(tmpVin,tmpPin,equeled);
                    }
                }
            }
        }
        else
        {
            DTdebug() << "MEBFisMatch" << MEBFisMatch;
            if(MEBFisMatch)
            {
                if(gFisVinIndex >= gFisVins.count())
                {
                    gFisVinIndex = 0;
                }
            }

            if(isRFID && tempp !="BarCode")//RFID
            {
                // 接收RFID G9 pin
                if(SYSS != "ING")
                {
                    if(factory == "SVW2")
                    {
                        if(MEBFisMatch && !equeled)
                        {
                            gFisPin = serialNums.right(8);
                            int tmpNewIndex = gFisPins.indexOf(gFisPin, gFisVinIndex);
                            DTdebug() << "Search:" << serialNums << gFisPin << gFisVinIndex << tmpNewIndex;
                            if((tmpNewIndex<0) || (gFisVinIndex != tmpNewIndex))
                            {
                                DTdebug() << "QMessageBox show";
                                system("echo 1 > /root/gpio/OUT5 &");
                                systemStatus = 5;
                                gMessageboxShow = true;
                                QMessageBox tmpMsgBox;
                                tmpMsgBox.setWindowTitle(tr("Confrim"));
                                tmpMsgBox.setText(tr("条码队列错乱，请确认?\n  %1").arg(serialNums));
                                QWidget  *textField = tmpMsgBox.findChild<QWidget*>("qt_msgbox_label");
                                if(textField  != NULL)
                                {
                                    textField->setMinimumSize(650, 160);
                                    textField->setStyleSheet("QLabel{font:28pt \"黑体\"; font-size:28pt;}");
                                }
                                QPushButton *tmpBtn = tmpMsgBox.addButton(QMessageBox::Ok);
                                tmpBtn->setText("拧紧");
                                tmpBtn->setFixedSize(120, 50);
                                tmpBtn = tmpMsgBox.addButton(QMessageBox::Discard);
                                tmpBtn->setText("放弃");
                                tmpBtn->setFixedSize(120, 50);
                                tmpMsgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
                                tmpMsgBox.move((width()-700)/2, (height()-300)/2);
                                tmpMsgBox.setStyleSheet("QPushButton { border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);"
                                                                   "background:transparent;"
                                                                   "font: 12pt \"黑体\";"
                                                                   "color: rgb(248, 248, 255); };"
                                                     "QLabel { font: 20pt \"黑体\";}");
                                int ret = tmpMsgBox.exec();
                                if(ret == QMessageBox::Discard)
                                {
                                    DTdebug() << "MessageBox::Discard";
                                    system("echo 0 > /root/gpio/OUT5 &");
                                    RfidControler *tmpRfid= qobject_cast<RfidControler*>(sender());
                                    if(tmpRfid) // if discard, will not be code repeat.
                                    {
                                        tmpRfid->pinCodeRepeatRemove(serialNum);
                                    }
                                    systemStatus = 0;
                                    gRfidParseDone = true;
                                    gMessageboxShow = false;
                                    return;
                                }
                                system("echo 0 > /root/gpio/OUT5 &");
                                gMessageboxShow = false;
                                DTdebug() << "MessageBox::Yes"<< serialNums << gFisPin << gFisVinIndex << tmpNewIndex;
                                gFisVinIndex = tmpNewIndex+1;
                                if(gFisVinIndex < 0)
                                    gFisVinIndex = 0;
                                systemStatus = 0;
                            }
                            else
                            {
                                gFisVinIndex++;
                            }
                        }
                    }
                    handleCodeRFID(serialNums,tempp,equeled);
                }
            }
            else if(isBarCode) //条码枪
            {
                DTdebug()<<"barcode";
                //设置条码
                if(SYSS != "ING")
                {
                    if(Factory == "AQCHERY"|| Factory == "KFCHERY")
                    {
                        if(tempp == "VIN+JOBID")
                        {
                            handleCodeBarcode_JOBID(serialNums,tempp,equeled);
                        }
                        else
                        {
                            handleCodeBarcode_NoJobID(serialNums,tempp,equeled);
                        }
                    }
                    else if(Factory == "SVW2")
                    {
                        if(MEBFisMatch && !equeled)
                        {
                            gFisVin = serialNums;
                            int tmpNewIndex = gFisVins.indexOf(gFisVin, gFisVinIndex);
                            DTdebug() << "Search:" << gFisVin << gFisVinIndex << tmpNewIndex;
                            if((tmpNewIndex<0) || (gFisVinIndex != tmpNewIndex))
                            {
                                DTdebug() << "QMessageBox show";
                                system("echo 1 > /root/gpio/OUT5 &");
                                systemStatus = 5;
                                gMessageboxShow = true;
                                QMessageBox tmpMsgBox;
                                tmpMsgBox.setWindowTitle(tr("Confrim"));
                                tmpMsgBox.setText(tr("条码队列错乱，请确认?\n  %1").arg(serialNums));

                                QWidget  *textField = tmpMsgBox.findChild<QWidget*>("qt_msgbox_label");
                                if(textField  != NULL)
                                {
                                    textField->setMinimumSize(650, 160);
                                    textField->setStyleSheet("QLabel{font:28pt \"黑体\"; font-size:28pt;}");
                                }
                                QPushButton *tmpBtn = tmpMsgBox.addButton(QMessageBox::Ok);
                                tmpBtn->setText("拧紧");
                                tmpBtn->setFixedSize(120, 50);
                                tmpBtn = tmpMsgBox.addButton(QMessageBox::Discard);
                                tmpBtn->setText("放弃");
                                tmpBtn->setFixedSize(120, 50);
                                tmpMsgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
                                tmpMsgBox.move((width()-700)/2, (height()-300)/2);
                                tmpMsgBox.setStyleSheet("QPushButton { border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);"
                                                                   "background:transparent;"
                                                                   "font: 12pt \"黑体\";"
                                                                   "color: rgb(248, 248, 255); };"
                                                     "QLabel { font: 20pt \"黑体\";}");
                                int ret = tmpMsgBox.exec();
                                if(ret == QMessageBox::Discard)
                                {
                                    DTdebug() << "MessageBox::Discard";
                                    system("echo 0 > /root/gpio/OUT5 &");
                                    SerialComs *tmpCom = qobject_cast<SerialComs*>(sender());
                                    if(tmpCom)
                                    {
                                        tmpCom->seriallist.removeAll(serialNum);
                                    }
                                    systemStatus = 0;
                                    gRfidParseDone = true;
                                    gMessageboxShow = true;
                                    return;
                                }
                                system("echo 1 > /root/gpio/OUT5 &");
                                DTdebug() << "MessageBox::Yes"<< serialNums << gFisPin << gFisVinIndex << tmpNewIndex;
                                gMessageboxShow = true;
                                gFisVinIndex = tmpNewIndex+1;
                                if(gFisVinIndex < 0)
                                    gFisVinIndex = 0;
                                systemStatus = 0;
                            }
                            else
                            {
                                gFisVinIndex++;
                            }
                        }
                        handleCodeBarcode(serialNums,equeled);
                    }
                    else
                    {
                        handleCodeBarcode(serialNums,equeled);
                    }
                }
            }
            else if(IsQueue || m_BAICQueue)  //队列
            {
                if(!m_BAICQueue && Factory == "BAIC")  //自动条码枪
                {
                   DTdebug()<<"autocode";
                    //设置条码
                    if(SYSS != "ING")
                    {
                        handleCodeBarcode(serialNums,equeled);
                    }
                    gRfidParseDone = true;
                    return;
                }
                if(tempp=="VIN")
                {
                    DTdebug()<<"delete";
                    //重新定位队列，匹配到条码以前没有使用过的条码更新为已使用，确认当前车辆，重新采集队列的条码；
                    if(SaveWhat == "delete_car")     //删除车型
                    {
                        if(serialNums == VIN_PIN_SQL)
                        {
                            //切使能
                            if(SYSS == "ING")
                            {
                                DTdebug() << "ING" ;
                                emitOperate1(false,groupNumCh[0]);
                            }
                        }
                        emit sendDeleteCar_VIN(serialNums);
                        query.exec("select RecordID, AutoNO from "+tablePreview+" where VIN = '"+ serialNums+"'");
                        if(query.next())
                        {
                            if(query1.exec("update "+tablePreview+" set UseFlag = 2 where RecordID = "+query.value(0).toString()))
                            {
                                DTdebug()<< "local update UseFlag=2 success "+serialNums;
                            }
                            else
                            {
                                DTdebug()<< "local update UseFlag=2 fail "+serialNums <<query1.lastError();
                            }
                            deletelock.lockForWrite();
                            QVector<QString> vector_temp;
                            vector_temp.push_back(query.value(1).toString());
                            vector_temp.push_back(serialNums);
                            queue.push_back(vector_temp);
                            deletelock.unlock();
                        }
                        else
                        {
                            DTdebug()<<"there is no VIN: "+serialNums+" in "+tablePreview;
                            deletelock.lockForWrite();
                            QVector<QString> vector_temp;
                            vector_temp.push_back("");
                            vector_temp.push_back(serialNums);
                            queue.push_back(vector_temp);
                            deletelock.unlock();
                        }
                    }
                    else
                    {
                        //                ui->label_17->setText(serialNums);
                        if(serialNums != VIN_PIN_SQL)
                        {
                            if(Factory == "BAIC")
                            {
                                query.exec("select RecordID from "+tablePreview+" where VIN = '"+serialNums+"' ORDER BY RecordID DESC LIMIT 1");

                            }
                            else if(factory == "Haima")
                            {
                                DTdebug()<<"this VIN UseFlag=0"<<serialNum;
                                if(SYSS == "ING")
                                {
                                    Stop();
                                    emitOperate1(false,groupNumCh[0]);
                                    ISmaintenance=false;
                                }
                                cleanDisplay();
                                ReceFisSerial(serialNums);
                                gRfidParseDone = true;
                                return;
                            }
                            else {
                                query.exec("select RecordID, UseFlag, Type from "+tablePreview+" where VIN = '"+serialNums+"' ORDER BY RecordID DESC LIMIT 1");

                            }
                            if(query.next())
                            {
                                if(query.value(1).toInt() == 0)
                                {
                                    //update queue
                                    DTdebug()<<"this VIN UseFlag=0"<<serialNums;
                                    if(!query1.exec("update "+tablePreview+" set UseFlag=1 where UseFlag =0 and RecordID <"+query.value(0).toString()))
                                        DTdebug()<<"1331"<<query1.lastError();
                                    if(SYSS == "ING")
                                    {
                                        Stop();
                                        emitOperate1(false,groupNumCh[0]);
                                        ISmaintenance=false;
                                    }
                                    cleanDisplay();
                                    FisTimerDo();
                                }
                                else if(query.value(1).toInt() == 1)
                                {
                                    DTdebug()<<"this VIN UseFlag=1"<<serialNums;
                                    if(!query1.exec("update "+tablePreview+" set UseFlag=0 where UseFlag != 2 and RecordID >="+query.value(0).toString()))
                                        DTdebug()<<"1363"<<query1.lastError();
                                    if(SYSS == "ING")
                                    {
                                        Stop();
                                        emitOperate1(false,groupNumCh[0]);
                                        ISmaintenance=false;
                                    }
                                    cleanDisplay();
                                    FisTimerDo();
                                }
                                else if(query.value(1).toInt() == 2)
                                {
                                    DTdebug()<<"this VIN UseFlag=2"<<serialNums;
                                    DeleteVIN = serialNums;
                                    if(SYSS == "ING")
                                    {
                                        Stop();
                                        emitOperate1(false,groupNumCh[0]);
                                        ISmaintenance=false;
                                    }
                                    cleanDisplay();
                                    BYDType = query.value(2).toByteArray();
                                    ReceFisSerial(serialNums);
                                }
                            }
                            else
                            {
                                DTdebug()<<"There is no VIN:"<<serialNums;
                                if(!isSaveShow)
                                {
                                    if(SYSS == "ING")
                                    {
                                        emitOperate1(false,groupNumCh[0]);
                                    }
                                    e3 = new QGraphicsOpacityEffect(this);
                                    e3->setOpacity(0.5);
                                    ui->label_black->setGraphicsEffect(e3);
                                    ui->label_black->show();
                                    ui->label_black->setGeometry(0,0,1366,768);
                                    isSaveShow = true ;
                                    SaveWhat = "queue_no"+serialNums;
                                    save = new Save(this);
                                    connect(save,SIGNAL(change_mode()),this,SLOT(modeChange()));
                                    save->show();
                                }
                            }
                        }
                    }
                }
                else if(tempp=="AutoNO")
                {
                    //重新定位队列，匹配到条码以前没有使用过的条码更新为已使用，确认当前车辆，重新采集队列的条码；
                    if(SaveWhat == "delete_car")     //删除车型
                    {
                        emit sendDeleteCar_VIN(serialNums);
                        query.exec("select RecordID, VIN from "+tablePreview+" where AutoNO = '"+ serialNums+"'");
                        if(query.next())
                        {
                            if(query.value(1).toString() == VIN_PIN_SQL)
                            {
                                //切使能
                                if(SYSS == "ING")
                                {
                                    emitOperate1(false,groupNumCh[0]);
                                }
                            }

                            if(query1.exec("update "+tablePreview+" set UseFlag = 2 where RecordID = "+query.value(0).toString()))
                            {
                                DTdebug()<< "local update UseFlag=2 success "+serialNums;
                            }
                            else
                            {
                                DTdebug()<< "local update UseFlag=2 fail "+serialNums <<query1.lastError();
                            }
                            deletelock.lockForWrite();
                            QVector<QString> vector_temp;
                            vector_temp.push_back(serialNums);
                            vector_temp.push_back(query.value(1).toString());
                            queue.push_back(vector_temp);
                            deletelock.unlock();
                        }
                        else
                        {
                            DTdebug()<<"there is no AutoNO: "+serialNums+" in "+tablePreview;
                            deletelock.lockForWrite();
                            QVector<QString> vector_temp;
                            vector_temp.push_back(serialNums);
                            vector_temp.push_back("");
                            queue.push_back(vector_temp);
                            deletelock.unlock();
                        }
                    }
                    else   //ING状态
                    {
                        query.exec("select RecordID, VIN, UseFlag from "+tablePreview+" where AutoNO = '"+ serialNums+"' ORDER BY RecordID DESC LIMIT 1");
                        if(query.next())
                        {
                            QString Auto=serialNums;
                            serialNums=query.value(1).toString();
                            if(serialNums != VIN_PIN_SQL)
                            {
                                if(query.value(2).toInt()==0)
                                {
                                    //update queue
                                    DTdebug()<<"this Auto UseFlag=0"<<serialNums;
                                    if(!query1.exec("update "+tablePreview+" set UseFlag=1 where UseFlag =0 and RecordID <"+query.value(0).toString()))
                                        DTdebug()<<"1481"<<query1.lastError();

                                    if(SYSS == "ING")
                                    {
                                        Stop();
                                        emitOperate1(false,groupNumCh[0]);
                                        ISmaintenance=false;
                                    }
                                    cleanDisplay();
                                    FisTimerDo();
                                }
                                else if(query.value(2).toInt()==1)
                                {
                                    DTdebug()<<"this Auto UseFlag=1"<<serialNums;
                                    if(!query1.exec("update "+tablePreview+" set UseFlag=0 where UseFlag !=2 and RecordID >="+query.value(0).toString()))
                                        DTdebug()<<"1497"<<query1.lastError();

                                    if(SYSS == "ING")
                                    {
                                        Stop();
                                        emitOperate1(false,groupNumCh[0]);
                                        ISmaintenance=false;
                                    }
                                    cleanDisplay();
                                    FisTimerDo();
                                }
                                else if(query.value(2).toInt()==2)
                                {
                                    DTdebug()<<"this Auto UseFlag=2"<<serialNums;
                                    DeleteVIN = serialNums;
                                    if(SYSS == "ING")
                                    {
                                        Stop();
                                        emitOperate1(false,groupNumCh[0]);
                                        ISmaintenance=false;
                                    }
                                    cleanDisplay();
                                    BYDType = query.value(2).toByteArray();
                                    ReceFisSerial(serialNums);
                                }
                            }
                        }
                        else
                        {
                            DTdebug()<<"There is no Auto:"<<serialNums;
                            if(SYSS == "ING")
                            {
                                emitOperate1(false,groupNumCh[0]);
                            }
                            if(!isSaveShow)
                            {
                                e3 = new QGraphicsOpacityEffect(this);
                                e3->setOpacity(0.5);
                                ui->label_black->setGraphicsEffect(e3);
                                ui->label_black->show();
                                ui->label_black->setGeometry(0,0,1366,768);
                                isSaveShow = true;
                                SaveWhat = "queue_no"+serialNums;
                                save = new Save(this);
                                connect(save,SIGNAL(change_mode()),this,SLOT(modeChange()));
                                save->show();
                            }
                        }
                    }
                }
                else if(tempp == "ready")
                {
                    DTdebug()<<"serialNums"<<serialNums;
                    DTdebug()<<serialNums<<serialNums.toAscii()+" "+BYDType;
                    ui->label_17->setText(tr(serialNums.toAscii()+" "+BYDType));
                    if(CsIsConnect) //351没连接 不匹配
                    {
                        if(SYSS == "OK" || SYSS == "NOK")
                        {
                            cleanDisplay();
                            ui->label_17->setText(tr(serialNums.toAscii()+" "+BYDType));
                        }
                        if(Factory == "BYDSZ"||Factory == "BAIC" || Factory == "Haima")
                            searchVinNum();
                        else if(Factory == "BYDXA")
                            searchJobBYD(serialNums);

                    }
                }
            }
        }
    }
    DTdebug()<<"gRfidParseDone = true";
    gRfidParseDone = true;
}
/*********************************************/
void MainWindow::slotGetQueueNum(QString vin, QString car)
{
    if(vin.isEmpty())
    {
        ui->labelError->setVisible(true);
    }
    else if ((car == "timeout") && (serialNums != vin))
    {
        serialNums = vin ;
        initCarType();
    }
    else {
        carTypeHaima = car ;
        serialNums = vin ;
    //    handleCodeBarcode(vin,false);
        getSerialNum(vin,false,"VIN");
    }

}

//串口二维码
void MainWindow::slotGetQRCode(QString strQRCodeSerial)
{
    FUNC() ;

    if(strQRCodeSerial.contains("#"))
    {
        QStringList  strList = strQRCodeSerial.split("#") ;
        if(strList.size() > 2)
        {
            QString strQRCode = strList.at(1);
            strQRCode = strQRCode.replace(" ","") ;  //去空格
            if(!strQRCode.isEmpty())
            {
                searchQRCodeNum(strQRCode);
            }
            else
            {
                DTdebug() << "QRcode is empty" ;
                ui->labelQRVerify->setText("零件码错误");
                ui->labelQRVerify->setStyleSheet("background-color:red;color: white;font: 20pt \"黑体\";");
                if(!ui->labelQRVerify->isVisible())
                {
                    ui->labelQRVerify->setVisible(true);
                }
            }
        }
    }
    else {
#if 0
        //一开始只有安全带二维码都是带#的，现在加了座椅二维码 2019年3月29日
        DTdebug() << "QRcode is empty" ;
        ui->labelQRVerify->setText("零件码错误");
        ui->labelQRVerify->setStyleSheet("background-color:red;color: white;font: 20pt \"黑体\";");
        if(!ui->labelQRVerify->isVisible())
        {
            ui->labelQRVerify->setVisible(true);
        }
#else
       QString strQRCode = strQRCodeSerial.mid(1,1)+strQRCodeSerial.mid(5,2);
       if(strQRCode.size()==3)
       {
           searchQRCodeNum(strQRCode);
       }
       else {
           DTdebug() << "QRcode is wrong" << strQRCodeSerial<<strQRCode;
           ui->labelQRVerify->setText("零件码错误");
           ui->labelQRVerify->setStyleSheet("background-color:red;color: white;font: 20pt \"黑体\";");
           if(!ui->labelQRVerify->isVisible())
           {
               ui->labelQRVerify->setVisible(true);
           }
       }
#endif
    }
}

void MainWindow::searchJob(QString serialNum)
{
    emit sendRequestJob(serialNum);
    JobTimer.start(3000);
}


void MainWindow::receiveJob(QString Job)
{
    if(JobTimer.isActive())
        JobTimer.stop();
    searchJobNum(Job);
}

void MainWindow::searchJobNum(QString Job)
{
    //匹配Job
    DTdebug()<<"****************job************ "<<Job;
    //    if(JobTimer.isActive())
    //        JobTimer.stop();
    QString job = Job.mid(0,8);
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    bool flags = false;
    int i;

    for(i = 1;i < (CAR_TYPE_AMOUNT+1);i++)
    {
        QString temp = config->value(QString("carinfo").append(QString::number(i)).append("/VIN")).toString();
        if(temp == job || temp =="????????")
        {
            flags = true;
            break;
        }
    }
    delete config;
    VinPinCodeView(flags,i);
}

void MainWindow::searchJobBYD(QString serialNum)
{
    FUNC() ;
    if(!db.isOpen()||!QSqlDatabase::contains("mysqlconnectionmatch"))
        mysqlopen();
    if(db.isOpen() && QSqlDatabase::contains("mysqlconnectionmatch"))
    {
        if(!query.exec("SELECT Job FROM TaskPreview WHERE VIN = '"+serialNum+"'"))
            DTdebug()<<"select Job fail "<<query.lastError();
        else
        {
            if(query.next())
            {
                QString Job =query.value(0).toString();
                //匹配Job
                DTdebug()<<"*********byd*job************ "<<Job;
                QString job = Job;
                QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
                bool flags = false;
                int i;

                for(i = 1;i < (CAR_TYPE_AMOUNT+1);i++)
                {
                    QString temp = config->value(QString("carinfo").append(QString::number(i)).append("/VIN")).toString();
                    if(temp == job)
                    {
                        flags = true;
                        break;
                    }
                }
                delete config;
                VinPinCodeView(flags,i);
                //                DTdebug() << "******query****"<<query.value(0).toString();
            }
            else
            {
                DTdebug() << "******no serialNum****"<<serialNum.size()<<serialNum;
                //                emit requestJob(serialNum);
                //                JobTimer.start(4000);
            }
        }
    }
    else
    {
        DTdebug()<<"db not open or unconnected";
    }
}

void MainWindow::receivePlusFlag(bool PlusFlag)
{
    if(PlusFlag)
    {
        ui->label_ss3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        ui->label_ss3_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
    }
    else
    {
        ui->label_ss3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        ui->label_ss3_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
    }
}

void MainWindow::modeChange()
{
    ui->label_black->hide();
    delete e3;
    delete save;
    SaveWhat = "";
    isSaveShow = false;
    on_pushButton_tiaoma_clicked();
}

void MainWindow::closeSave()
{
    ui->label_black->hide();
    delete e3;
    delete save;
    SaveWhat = "";
    isSaveShow = false;
    //    ISmaintenance = false;
    //    workmode = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Show()
{
    disconnect(gExtSerialPort, SIGNAL(readyRead()), 0, 0);
    connect(gExtSerialPort, SIGNAL(readyRead()), this, SLOT(readCom()));
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->show();
}

void MainWindow::emitOperate1(bool enable, int flag)
{
    if(enable)
        emit sLightLogic(e_Enable);
    else
        emit sLightLogic(e_Disable);

    emit sendOperate1(enable,flag);
}
void MainWindow::emitOperate2(bool enable,int flag)
{
    if(enable)
        emit sLightLogic(e_Enable);
    else
        emit sLightLogic(e_Disable);

    emit sendOperate2(enable,flag);
}

void MainWindow::emitOperate3(bool enable,int flag)
{
    if(enable)
        emit sLightLogic(e_Enable);
    else
        emit sLightLogic(e_Disable);

    emit sendOperate3(enable,flag);
}

void MainWindow::emitOperate4(bool enable,int flag)
{
    if(enable)
        emit sLightLogic(e_Enable);
    else
        emit sLightLogic(e_Disable);

    emit sendOperate4(enable,flag);
}

//void MainWindow::closeEvent(QCloseEvent *event)
//{
//    emit clo();
//    event->accept();
//}


/**********************PDM闪烁*****************************/
//PDMBoltBuf[i]=0,stop;=1,w and y;=2,r and y;=4,g and y
/**********************PDM闪烁*****************************/
void MainWindow::PdmFlicker()
{
    if(pdmflicker)
    {
        pdmflicker = false;
    }
    else
    {
        pdmflicker = true;
    }
    if(ControlType_1 == "SB356_PLC")
    {
        if(tempnumdpm[0]>10)
        {
            tempnumdpm[0] = 10;
        }
        if(whichpdmnumnow[0]<tempnumdpm[0])
        {
            if(PDMBoltBuf[0]>0)
            {
                if(pdmflicker)
                {
                    if(PDMBoltBuf[0] == 1)//ING
                    {
                        butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                    }
                    else if(PDMBoltBuf[0] == 2)//NOK
                    {
                        butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                    }
                    else if(PDMBoltBuf[0] == 4)//OK
                    {
                        butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                    }
                }
                else
                {
                    butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/y01.png);font: 18pt;");
                }
            }
        }
        if((whichpdmnumnow[0]+5)<tempnumdpm[0])
        {
            if(PDMBoltBuf[1]>0)
            {
                if(pdmflicker)
                {
                    if(PDMBoltBuf[1] == 1)//ING
                    {
                        butt[0][whichpdmnumnow[0]+5]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                    }
                    else if(PDMBoltBuf[1] == 2)//NOK
                    {
                        butt[0][whichpdmnumnow[0]+5]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                    }
                    else if(PDMBoltBuf[1] == 4)//OK
                    {
                        butt[0][whichpdmnumnow[0]+5]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                    }
                }
                else
                {
                    butt[0][whichpdmnumnow[0]+5]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/y01.png);font: 18pt;");
                }
            }
        }
    }
    else
    {
        if(PDMBoltNum > 0) //bolt number
        {
            for(int i=0;i<pdmflickerNum;i++)
            {
                if(PDMBoltBuf[i]>0)
                {
                    if(pdmflicker)
                    {
                        if(PDMBoltBuf[i] == 1)//ING
                        {
                            butt[0][i+currentFirstBolt]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                        }
                        else if(PDMBoltBuf[i] == 2)//NOK
                        {
                            butt[0][i+currentFirstBolt]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                        }
                        else if(PDMBoltBuf[i] == 4)//OK
                        {
                            butt[0][i+currentFirstBolt]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                        }
                    }
                    else
                    {
                        butt[0][i+currentFirstBolt]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/y01.png);font: 18pt;");
                    }
                }
            }
        }
        else
        {
            if((whichpdmnumnow[0] == tempnumdpm[0])&&(whichpdmnumnow[1] == tempnumdpm[1])&&(whichpdmnumnow[2] == tempnumdpm[2])&&(whichpdmnumnow[3] == tempnumdpm[3]))
            {
                timerpdm.stop();
            }
            else
            {
                for(int i=0;i<4;i++)
                {
                    if(tempnumdpm[i] == 0)
                    {
                        continue;
                    }
                    if(whichpdmnumnow[i] != tempnumdpm[i])
                    {
                        if(PDMCurrentState[i] == "NOK")
                        {
                            if(pdmflicker)
                            {
                                butt[i][whichpdmnumnow[i]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                            }
                            else
                            {
                                butt[i][whichpdmnumnow[i]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/y01.png);font: 18pt;");
                            }
                        }
                        else
                        {
                            if(pdmflicker)
                            {
                                butt[i][whichpdmnumnow[i]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/y01.png);font: 18pt;");
                            }
                            else
                            {
                                butt[i][whichpdmnumnow[i]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/w01.png);font: 18pt;");
                            }
                        }
                    }
                }
            }
        }
    }
}
/**********************清空界面*****************************/
void MainWindow::cleanDisplay()
{
    showhome();
    SetLsInfo("", "", "");
    ui->label_hege->setText("");
    if(timerpdm.isActive())
    {
        timerpdm.stop();
    }
    for(int i=0;i<4;i++)
    {
        DTdebug() << tempnumdpm[i];
        for(int j=0;j<tempnumdpm[i];j++)
        {
            delete butt[i][j];
            delete label1[i][j];
            delete label2[i][j];
        }
        tempnumdpm[i] = 0;
        PDMCurrentState[i] = "OK";
    }
    pdmnowromisOk = true;
    BoltOrder[0] = 1;
    BoltOrder[1] = 1;
    SYSS = "Ready";
    lock.lockForWrite();
    StationStatus = 2;
    lock.unlock();
    if(CsIsConnect)
    {
        displayStatusIcon("Ready");
    }
    QualifiedNum = 0;
    TaoTongState = false;

    DTdebug() << "sLightLogic(e_cleanDisplay)";
    emit sLightLogic(e_cleanDisplay);
    //white_led(0);
    //red_led(0);
    //green_led(0);
}

void MainWindow::slot_postLabelResult()
{
    QVariantMap dataMap;
    dataMap.insert("BoltID",boltSNList.at(boltIndex-1));
    QJson::Serializer serializer;
    bool ok;
    QByteArray json = serializer.serialize(dataMap, &ok);
    QNetworkRequest reqFis;
    reqFis.setUrl(QUrl(QString("http://%1:%2/%3").arg("172.21.12.110").arg("51566").arg("GetBoltBaseInfoLable")));
    reqFis.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    reqFis.setHeader(QNetworkRequest::ContentLengthHeader, json.length());

    if (ok)
    {
        qDebug() << "post get addPostLabel" << json;   //<<json;
        QNetworkReply *reply = manager->post(reqFis,json);
        reply->ignoreSslErrors();
        QTime t_time;
        t_time.start();

        bool timeout = false;

        while (!reply->isFinished())
        {
            QApplication::processEvents();
            if (t_time.elapsed() >= 5000) {
                timeout = true;
                qDebug()<<"addPostLabel reply timeout";
                break;
            }
        }
        if (!timeout && reply->error() == QNetworkReply::NoError){
            QByteArray bytes = reply->readAll();
            DTdebug()<<bytes;

            QJson::Parser parser;
            bool ok;

            QVariantMap result = parser.parse(bytes, &ok).toMap();
            if (!ok) {
                qDebug()<<"get addPostLabel An error occurred during parsing"<<bytes;
            }
            else{
                qDebug()<<"get addPostLabel";
                if(result["Result"].toBool()){
                    postTimer->stop();
                    qDebug()<<"get addPostLabel Result==1";
//                    QVariant Value = result["VINS"];
//                    QVariantMap value = Value.toMap();
                    QString BoltID = result["BoltID"].toString();
                    QString Label = result["Label"].toString();

                    if(BoltID == boltSNList.at(boltIndex-1) && (!Label.isEmpty() || !Label.contains("null",Qt::CaseInsensitive))){
                        showLabelStatus(1,boltIndex,Label.toInt());
                    }
                }
                else{
                    qDebug()<<"get addLabel Result==wrong"<<bytes;
                }
            }
        }
        else if(reply->error() != QNetworkReply::NoError){
            qDebug()<<"addPostLabel handle errors here";
            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
            qDebug( )<<"addPostLabel found error ....code:"<< statusCodeV.toInt()<<(int)reply->error();
            qDebug()<<reply->errorString();
        }
        reply->deleteLater();
    }
    else{
        qDebug() << "addPostLabel errorMessage:" << serializer.errorMessage();
    }

//    showLabelStatus(1,boltIndex,2);
//    postTimer->stop();
}

//测试手动发送Label数据
void MainWindow::on_senLabelValue_clicked()
{
    ui->LabelResult_color->show();
    ui->LabelResult_text->show();
    showLabelStatus(1,boltIndex,ui->lineEdit_labelValue->text().toInt());
}

//实现高风险时红灯闪烁
void MainWindow::slot_alarmFlashFunc()
{
    if(flashIni){
        flashIni = false;
        ui->LabelResult_color->setStyleSheet("background-color:red;");
    }else{
        flashIni = true;
        ui->LabelResult_color->setStyleSheet("");
    }
}


void MainWindow::showLabelStatus(int have, int index, int value)
{
    QStringList list;
    list<<"无风险"<<"高风险"<<"低风险"<<"低风险"<<"异常";
    if(value < 0 || value > 3){
        value = 4;
    }
    //        inherits("e3_flash")
    //        qobject_cast<QGraphicsOpacityEffect*>(e3_flash)
    if(alarmFlashed){
        alarmFlashed = false;
//        qDebug()<<"delete ui->LabelResult_color";
        e3_flash->deleteLater();
    }
    if(have == 1 && value != 100){//收到风险结果
        ui->LabelResult_text->setText(QString("第%1颗螺栓 %2").arg(index).arg(list.at(value)));
        alarmFlash.stop();
        ui->LabelResult_color->setGeometry(10,10,56,56);
        if(value == 0){
            ui->LabelResult_color->setStyleSheet("background-color:green;");
        }else if(value == 1){
            flashIni = true;
            alarmFlash.start(300);
            ui->LabelResult_color->setStyleSheet("background-color:red;");
            e3_flash = new QGraphicsOpacityEffect(this);
            alarmFlashed = true;
            e3_flash->setOpacity(0.5);
            ui->LabelResult_color->setGraphicsEffect(e3_flash);
            ui->LabelResult_color->setGeometry(10,10,1146,578);
        }else if(value == 2 || value == 3){
            ui->LabelResult_color->setStyleSheet("background-color:yellow;");
        }else{
            ui->LabelResult_color->setStyleSheet("");
        }
    }else{//刚拧紧完一颗螺栓，等待风险结果生成中
        ui->LabelResult_color->setStyleSheet("");
        ui->LabelResult_text->setText(QString("等待第%1颗螺栓的结果中。。。").arg(index));
    }
}

/*******************************************/
//more channel 1,2,3,4
//ch 0,1,2,3
/*******************************************/
void MainWindow::fromsecondthreaddata(QString MI,QString WI,QString IsOk,int ch,int groupNum)
{
    if(factory == "NOVAT"){//天际汽车添加循环号检测功能
        //检测循环号有没重复的，有重复的不处理
        query1.exec("SELECT Cycle FROM "+Localtable+" where Cycle = '" + tightenID + "'");//database存储的Cycle是Int类型
        int numRows_cycle = query1.numRowsAffected();
        //    mysqlclose();//整个项目对sql的管理相当混乱，这里不敢关，怕其他地方在用导致程序崩溃
        if(numRows_cycle > 0 || tightenID == tightenID_temp){//duplicate
            //        query1.next();
            qDebug()<<"duplicateCycle:"<<tightenID;
            return;
        }
        tightenID_temp = tightenID;
    }
    //0 dat  1 time   2 state   3  扭矩 4 角度 5 螺栓编号   6 vin pin 码   7 循环号  8 曲线
//    FUNC() ;
    qDebug()<<"whichpdmnumnow:"<<whichpdmnumnow[0]<<whichpdmnumnow[1]<<whichpdmnumnow[2]<<whichpdmnumnow[3]<<"tempnumdpm:"<<tempnumdpm[0]<<tempnumdpm[1]<<tempnumdpm[2]<<tempnumdpm[3];
    if(factory == "SVW2" && (Station == "076R-1" || Station == "080R" || Station == "085R-1" || Station == "093R-2")){
        boltIndex = whichpdmnumnow[ch]+1;
        ui->LabelResult_color->show();
        ui->LabelResult_text->show();
        postTimer->start(500);
        showLabelStatus(0,boltIndex,100);
    }
    qDebug()<<"fromsecondthreaddata:"<<MI<<WI<<IsOk<<ch<<groupNum<<whichpdmnumnow[ch] << tempnumdpm[ch];


    if(factory == "KFCHERY")
    {
        if((whichpdmnumnow[ch] + 1 == tempnumdpm[ch]) && (IsOk == "OK")){//拧紧完成
            qDebug()<<"Abnormal outbound not stop line";//最后一颗拧紧完成的，不会停线
            GAbnormalOut_KF = false;
        }
    }


    RFIDlock.lockForRead();
    if(rfidNextCom && Factory != "Dongfeng")
    {
        RFIDlock.unlock();
        DTdebug()<<"After nextCar data";
        if(whichpronumis == bolt_index && IsOk == "OK") //  判断是否是最后一组螺栓
        {
            DTdebug()<<"After nextCar the last data OK";
            on_pushButton_reset_clicked();
            return;
        }
        else if(whichpronumis == bolt_index && IsOk == "NOK")
        {
            DTdebug()<<"After nextCar the last data NOK";
            lock.lockForWrite();
            status[whichpdmnumnow[ch]][1] = MI; // 螺栓扭矩
            status[whichpdmnumnow[ch]][2] = WI; // 螺栓角度
            status[whichpdmnumnow[ch]][0] = "5";  //螺栓状态
            info[0] = "1"; //拧紧状态
            lock.unlock();
            if(whichpdmnumnow[ch] < tempnumdpm[ch])
            {
                qDebug()<<"showTightenInfo0:"<<ch<<MI<<WI;
                label1[ch][whichpdmnumnow[ch]]->setText(QString("  T:"+MI+"Nm"));
                label2[ch][whichpdmnumnow[ch]]->setText(QString("  A:"+WI+"Deg"));
                label1[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                label2[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                butt[ch][whichpdmnumnow[ch]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
            }
            PDMCurrentState[ch] = "NOK";
            return;
        }
        else
        {
            return;
        }
    }
    else
    {
        RFIDlock.unlock();
        if(IsOk== "Reconnect")  //重新连接
        {
            if(carInfor[0].boltNum[groupNumCh[0]]>0)
            {
                emit sendnexo(serialNums);
                emitOperate1(true,groupNumCh[0]);
                return;
            }
            else if(!carInfor[0].boltNum[groupNumCh[0]])
            {
                if(groupAllBoltNumCh[ch]>0)
                {
                    for(int i=groupNumCh[0]+1;i<20;i++)
                    {
                        if(carInfor[0].boltNum[i]>0)
                        {
                            emitOperate1(true,i);
                            return;
                        }
                    }
                }
            }
        }
    }
    lock.lockForWrite();
    status[whichpdmnumnow[ch]][1] = MI; // 螺栓扭矩
    status[whichpdmnumnow[ch]][2] = WI; // 螺栓角度
    lock.unlock();
    //status[whichpdmnumnow[0]][4] = data_model.DATE_b+" "+data_model.TIME_b;//螺栓拧紧时间
    if(IsOk == "NOK" || IsOk == "LSN") // 本次螺栓不合格nok
    {
        lock.lockForWrite();
        status[whichpdmnumnow[ch]][0] = "5";  //螺栓状态
        info[0] = "1"; //拧紧状态
        lock.unlock();
        if(whichpdmnumnow[ch] < tempnumdpm[ch])
        {
            qDebug()<<"showTightenInfo0:"<<ch<<MI<<WI;
            label1[ch][whichpdmnumnow[ch]]->setText(QString("  T:"+MI+"Nm"));
            label2[ch][whichpdmnumnow[ch]]->setText(QString("  A:"+WI+"Deg"));
            label1[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            label2[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            if(IsOk == "NOK"){
            butt[ch][whichpdmnumnow[ch]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
            }
        }

        DTdebug() << "sLightLogic(e_SingleFail)";
        emit sLightLogic(e_SingleFail);
            //nok_led(1);

        if(IsOk == "NOK")
            PDMCurrentState[ch] = "NOK";
        else {
            DTdebug() << "LSN can not red twinke" ;
        }
    }
    else  //本次螺栓合格ok
    {
        DTdebug() << "sLightLogic(e_SingleOK)";
        emit sLightLogic(e_SingleOK);

        DTdebug()<<"ok"<<PDMCurrentState[ch];
        if(MI!= "-1" && WI != "-1")
        {
            QualifiedNum++;//合格数加1
            ui->label_hege->setText(QString::number(QualifiedNum));
        }
        DTdebug() << QString("ch[%1], groupNum[%2], boltNum[%3], groupAllBoltNumCh[%4]").arg(ch).arg(groupNum).arg(carInfor[ch].boltNum[groupNum]).arg(groupAllBoltNumCh[ch]);
        if(carInfor[ch].boltNum[groupNum] > 0)
        {
            carInfor[ch].boltNum[groupNum]--;   //结果ok 数量减1
            enableLsnumber = carInfor[ch].boltNum[groupNum];
        }
        if(groupAllBoltNumCh[ch]>0)
        {
            groupAllBoltNumCh[ch]--;
        }
        if(whichpdmnumnow[ch] < tempnumdpm[ch])
        {
            if(MI == "-1" && WI == "-1")
            { //nok确认1 个
                lock.lockForWrite();
                status[whichpdmnumnow[ch]][0] = "2";  //螺栓状态
                lock.unlock();
                pdmnowromisOk = false;
                butt[ch][whichpdmnumnow[ch]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                if(PDMCurrentState[ch] != "NOK")
                {
                    // DTdebug() << "psmdfpsdmfsdfsdfsfsdf";
                    qDebug()<<"showTightenInfo-1:"<<ch<<MI<<WI;
                    label1[ch][whichpdmnumnow[ch]]->setText(QString("  T:-1Nm"));
                    label2[ch][whichpdmnumnow[ch]]->setText(QString("  A:-1Deg"));
                    label1[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    label2[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    //存入数据库
                }
                //nok_led(0);
                whichpdmnumnow[ch]++;

                timerpdm.start(500);
            }
            else
            {
                //正常
                //nok_led(0);
                qDebug()<<"showTightenInfo1:"<<ch<<MI<<WI;
                butt[ch][whichpdmnumnow[ch]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                label1[ch][whichpdmnumnow[ch]]->setText(QString("  T:"+MI+"Nm"));
                label2[ch][whichpdmnumnow[ch]]->setText(QString("  A:"+WI+"Deg"));
                label1[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                label2[ch][whichpdmnumnow[ch]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                lock.lockForWrite();
                status[whichpdmnumnow[ch]][0] = "1";  //螺栓状态
                whichpdmnumnow[ch]++;
                status[whichpdmnumnow[ch]][0] = "3";
                lock.unlock();
            }
        }
        PDMCurrentState[ch] = "OK";
        DTdebug() << "******carInfor[ch].boltNum[n]******"<<carInfor[ch].boltNum[groupNum]<<ch<<groupNum;
        if(carInfor[ch].boltNum[groupNum] == 0)
        {
            QualifiedNum = 0;
            groupNum++;
            DTdebug() << "********groupNum*****" << groupNum <<groupAllBoltNumCh[ch];
            int k = 0;
            if((groupNum == 21)||(groupAllBoltNumCh[ch]==0))
            {
                k = 20;
            }
            else
            {
                for( k = groupNum;k <20;k++)
                {
                    if(!IOFlag)
                    {
                        Q_EMIT sendCmdToCs351FromMW(5);
                    }
                    else {
                        //IO taotong
                        Q_EMIT signalSendTaotongNum();
                    }
                    DTdebug() << "****send next****" <<carInfor[ch].boltNum[k]<<carInfor[ch].ttNum[k]<<k;
                    if(carInfor[ch].boltNum[k])
                    {
                        bool condition = GetTaotongNumCondition(ch, k);
                        if(condition)
                        {
                            TaoTongState = true;
                            sendEnableFlag = false;
                            ttChangeFlag = true;
                            preChannel = ch;
                            groupNumCh[ch] = k;
                            SetLsInfo(carInfor[ch].proNo[k], QString::number(carInfor[ch].boltNum[k]), QString::number(carInfor[ch].ttNum[k]));
                            DTdebug()<< "&&&&&&&" << ch <<groupNumCh[ch];
                            break;
                        }
                        else
                        {
                            TaoTongState = false;
                        }
                    }
                    if(carInfor[ch].boltNum[k])
                    {
                        if(allChannel==1)
                        {
                            ui->label_pronum->setText(carInfor[ch].proNo[k]);
                            ui->label_lsnum->setText(QString::number(carInfor[ch].boltNum[k]));
                        }
                        groupNumCh[ch] = k;
                        if(ch == 0)
                        {
                            emitOperate1(true,k);
                        }
                        else if(ch == 1)
                        {
                            emit emitOperate2(true,k);
                        }
                        else if(ch == 2)
                        {
                            emit emitOperate3(true,k);
                        }
                        else
                        {
                            emit emitOperate4(true,k);
                        }
                        break;
                    }
                }
            }
            if(k == 20)
            {
                if(ch == 0)
                {
                    emitOperate1(false,k);
                }
                else if(ch == 1)
                {
                    emit emitOperate2(false,k);
                }
                else if(ch == 2)
                {
                    emit emitOperate3(false,k);
                }
                else
                {
                    emit emitOperate4(false,k);
                }
                DTdebug() << groupAllBoltNumCh[0] << groupAllBoltNumCh[1] << groupAllBoltNumCh[2] << groupAllBoltNumCh[3];
                if((groupAllBoltNumCh[0]==0)&&(groupAllBoltNumCh[1]==0)&&(groupAllBoltNumCh[2]==0)&&(groupAllBoltNumCh[3]==0))
                {
                    if(isAlign)
                    {
                        //yellow_led(0);
                        //red_led(0);
                        isAlign = false;
                    }
                    if(MI!= "-1" && WI != "-1")
                    {
                        lock.lockForWrite();
                        StationStatus =2;
                        lock.unlock();
                        if(!pdmnowromisOk)
                        {
                            DTdebug() << "sLightLogic(e_GroupFail)";
                           // emit sLightLogic(e_GroupFail);
                            displayStatusIcon("NOK");
                            lock.lockForWrite();
                            status[whichpdmnumnow[ch]][0] = "2";  //螺栓状态
                            info[0] = "3"; //拧紧状态
                            StationStatus =1;
                            lock.unlock();
                            SYSS = "NOK";
                            //red_led(1);

                        }
                        else
                        {
                            DTdebug() << "sLightLogic(e_GroupOK)";
                            //emit sLightLogic(e_GroupOK);
                            displayStatusIcon("OK");
                            lock.lockForWrite();
                            status[whichpdmnumnow[ch]][0] = "1";  //螺栓状态
                            info[0] = "2"; //拧紧状态
                            lock.unlock();
                            SYSS = "OK";
                            //green_led(1);
                            if(factory == "AQCHERY")
                            {
                                DTdebug() << "AQCHERY : signalSetIoBox 0";
                                emit signalSetIoBox(0,true);
                            }
                        }
                    }
                    else
                    {
                        DTdebug() << "sLightLogic(e_GroupFail)";
                        // emit sLightLogic(e_GroupFail);
                        displayStatusIcon("NOK");
                        lock.lockForWrite();
                        info[0] = "3"; //拧紧状态
                        StationStatus = 1;
                        lock.unlock();
                        SYSS = "NOK";
                        //red_led(1);
                    }
                    emit sendRunStatusToJson(SYSS);
                    TaoTongState = false;
                    ISmaintenance = false;
                    //white_led(0);
                    Stop();
                    ui->progressBar->setValue(0);
                    m_CurrentValue  = 0;
                    gunPower(false);

                    if(IsQueue && (factory == "BYDSZ" || factory == "BYDXA" || m_BAICQueue) )
                    {
                        //结果打完  队列标志位置1
                        UpdateSqlFlag();
                        //当前条码打完 去队列找下一条 发使能
                        FisTimer.start(5000);
                    }
                    else if(IsQueue && factory == "BAIC")
                    {
                        QTimer::singleShot(5000,this,SLOT(leuzeTimerStart()));
                    }
                    else if (IsQueue && factory=="Haima")
                    {
                        Q_EMIT signalGetCarInfo(0,serialNums);
                    }

                    startLine();
                }
            }
        }
        else
        {
            if(MI == "-1" && WI == "-1") //more bolt number
            {
                if(ch == 0)
                {
                    emitOperate1(true,groupNum);
                }
                else if(ch == 1)
                {
                    emit emitOperate2(true,groupNum);
                }
                else if(ch == 2)
                {
                    emit emitOperate3(true,groupNum);
                }
                else
                {
                    emit emitOperate4(true,groupNum);
                }
            }
        }
    }
}
/*******************************************/
//5 channel=1,2,3,4,5
//PDMBoltBuf[i] == 1)//ING
//PDMBoltBuf[i] == 2)//NOK
//PDMBoltBuf[i] == 4)//OK
//PDMBoltBuf[i] == 0)//
/*******************************************/
void MainWindow::fromsecondthreaddata(QString MI,QString WI,QString IsOk,int channel)
{
    DTdebug()<< "&&&&&&&&&&&&channel&&&&&&&&&&&" << channel;
    lock.lockForWrite();
    status[whichpdmnumnow[0]][1] = MI; // 螺栓扭矩
    status[whichpdmnumnow[0]][2] = WI; // 螺栓角度
    lock.unlock();
    if(channel == 0)
    {
        currentBoltSum = 1;
        pdmnowromisOk = true;
    }
    else
    {
        currentBoltSum ++;
    }
    //status[whichpdmnumnow[0]][4] = data_model.DATE_b+" "+data_model.TIME_b;//螺栓拧紧时间
    if(IsOk == "NOK" || IsOk == "LSN") // 本次螺栓不合格nok
    {
        DTdebug() << whichpdmnumnow[0]<<tempnumdpm[0];
        lock.lockForWrite();
        status[whichpdmnumnow[0]][0] = "5";  //螺栓状态
        info[0] = "1"; //拧紧状态
        lock.unlock();
        if(whichpdmnumnow[0] < tempnumdpm[0])
        {
            DTdebug() << "1" ;
            QString style = "font: 14pt; border-style:solid; border-width:1px 1px 0px 1px; border-color:rgb(255, 0, 0); background-color: rgb(248, 248, 255);";
            label1[0][whichpdmnumnow[0]]->setStyleSheet(style);
            label1[0][whichpdmnumnow[0]]->setText(QString("  T:"+MI+"Nm"));
            butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt; color:rgb(248, 248, 255)");
            style = "font: 14pt; border-style:solid; border-width:0px 1px 1px 1px; border-color:rgb(255, 0, 0); background-color: rgb(248, 248, 255);";
            label2[0][whichpdmnumnow[0]]->setStyleSheet(style);
            label2[0][whichpdmnumnow[0]]->setText(QString("  A:"+WI+"Deg"));
        }
        //nok_led(1);
        //        PDMCurrentState[0] = "NOK";
//        if(IsOk == "NOK")
                PDMBoltBuf[channel] = 2; //NOK
//        else if(IsOk == "LSN")
//                PDMBoltBuf[channel] == 5;  //LSN

        pdmnowromisOk = false;
        whichpdmnumnow[0] ++;
        //        DTdebug() << "*****PDMBoltBuf[0]*****" << PDMBoltBuf[0];
        //        DTdebug() << "*****PDMBoltBuf[1]*****" << PDMBoltBuf[1];
        //        DTdebug() << "*****PDMBoltBuf[2]*****" << PDMBoltBuf[2];
        //        DTdebug() << "*****PDMBoltBuf[3]*****" << PDMBoltBuf[3];
        //        DTdebug() << "*****PDMBoltBuf[4]*****" << PDMBoltBuf[4];
    }
    else  //本次螺栓合格ok
    {
        if(whichpdmnumnow[0] < tempnumdpm[0])
        {
            if(MI == "-1" && WI == "-1") //nok确认1 个
            {
                DTdebug() << "1" ;
                butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,248,255)");
                if(label1[0][whichpdmnumnow[0]]->text() == "")
                {
                    label1[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 255);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    label1[0][whichpdmnumnow[0]]->setText(QString("   T:-1Nm"));
                    label2[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 255);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    label2[0][whichpdmnumnow[0]]->setText(QString("   A:-1Deg"));
                }
                allTightenResultFlag = false;
            }
            else
            {
                QString style = "font: 14pt; border-style:solid; border-width:1px 1px 0 1px; border-color:rgb(25, 125, 44);background-color: rgb(248, 248, 255);";
                label1[0][whichpdmnumnow[0]]->setStyleSheet(style);
                label1[0][whichpdmnumnow[0]]->setText(QString("  T:"+MI+"Nm"));
                butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                style = "font: 14pt; border-style:solid; border-width:0 1px 1px 1px; border-color:rgb(25, 125, 44);background-color: rgb(248, 248, 255);";
                label2[0][whichpdmnumnow[0]]->setStyleSheet(style);
                label2[0][whichpdmnumnow[0]]->setText(QString("  A:"+WI+"Deg"));
            }
            lock.lockForWrite();
            status[whichpdmnumnow[0]][0] = "1";  //螺栓状态
            whichpdmnumnow[0]++;  //OK
            status[whichpdmnumnow[0]][0] = "3";
            lock.unlock();
            //nok_led(0);
            PDMBoltBuf[channel] = 0; //OK
            DTdebug() << "*****PDMBoltBuf[0]*****" << PDMBoltBuf[0]<<whichpdmnumnow[0];
            DTdebug() << "*****PDMBoltBuf[1]*****" << PDMBoltBuf[1];
            DTdebug() << "*****PDMBoltBuf[2]*****" << PDMBoltBuf[2];
            DTdebug() << "*****PDMBoltBuf[3]*****" << PDMBoltBuf[3];
            DTdebug() << "*****PDMBoltBuf[4]*****" << PDMBoltBuf[4];
        }
        //        PDMCurrentState[0] = "OK";
    }

    if(PDMBoltNum == currentBoltSum)   //bolt number of group OK
    {
        if((pdmnowromisOk))
        {
            StartBolt += currentBoltSum;
            DTdebug()<<"111111111111111"<<whichpdmnumnow[0]<<tempnumdpm[0];
            QualifiedNum = whichpdmnumnow[0]+1;//合格数加1
            ui->label_hege->setText(QString::number(QualifiedNum));
            int i;
            currentBoltNum = 0;
            for(i = whichpronumis-1;i<20;i++) //clear finished number
            {
                if(carInfor[0].boltNum[i])
                {
                    carInfor[0].boltNum[i]--;
                    currentBoltNum++;
                }
                whichpronumis++;
                if(currentBoltNum == PDMBoltNum)
                {
                    break;
                }
            }
            if(groupAllBoltNumCh[0]>PDMBoltNum)
            {
                groupAllBoltNumCh[0] -= PDMBoltNum;
                currentFirstBolt += PDMBoltNum;
                if(tempnumdpm[0]>(currentFirstBolt+PDMBoltNum))
                {
                    pdmflickerNum = PDMBoltNum;
                }
                else
                {
                    pdmflickerNum = tempnumdpm[0] - currentFirstBolt;
                }
                int i;
                for(i=0;i<5;i++)
                {
                    if(PDMBoltBuf[i] == 0)
                    {
                        PDMBoltBuf[i] = 1;
                    }
                }
                for( i = whichpronumis-1;i<20;i++)
                {
                    DTdebug() << "*****i******" << i<<carInfor[0].boltNum[i];
                    if(carInfor[0].boltNum[i])
                    {
                        ui->label_pronum->setText(carInfor[0].proNo[i]);
                        ui->label_lsnum->setText(QString::number(PDMBoltNum));
                        emitOperate1(true,i);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else //finished groupAllBoltNumCh[0] = 0
            {
                if(isAlign)
                {
                    //yellow_led(0);
                    //red_led(0);
                    isAlign = false;
                }
                if(allTightenResultFlag)
                {
                    displayStatusIcon("OK");
                    lock.lockForWrite();
                    status[whichpdmnumnow[0]][0] = "1";  //螺栓状态ok
                    info[0] = "2"; //拧紧状态
                    lock.unlock();
                    SYSS = "OK";
                    //green_led(1);
                }
                else
                {
                    displayStatusIcon("NOK");
                    lock.lockForWrite();
                    status[whichpdmnumnow[0]][0] = "2";  //螺栓状态
                    info[0] = "3"; //拧紧状态
                    lock.unlock();
                    SYSS = "NOK";
                    //red_led(1);
                }
                emit sendRunStatusToJson(SYSS);
                DTdebug()<<"2222222 tightening finish 222222";
                if(timerpdm.isActive())
                {
                    timerpdm.stop();
                }
                groupAllBoltNumCh[0] = 0;
                emitOperate1(false,0);
                TaoTongState = false;
                ISmaintenance = false;
                gunPower(false);
                //white_led(0);
                Stop();
                if(IsQueue && (factory == "BYDSZ" || factory == "BYDXA" || m_BAICQueue))
                {
                    //结果打完  队列标志位置1
                    UpdateSqlFlag();
                    //当前条码打完 去队列找下一条 发使能
                    FisTimer.start(5000);
                }
                else if(IsQueue && factory == "BAIC")
                {
                    QTimer::singleShot(5000,this,SLOT(leuzeTimerStart()));
                }
                else if (IsQueue && factory=="Haima")
                {
                    Q_EMIT signalGetCarInfo(0,serialNums);
                }

                startLine();
            }
        }
        else
        {
            whichpdmnumnow[0] -= currentBoltSum;
            for(int n=0;n<5;n++)
            {
                if(PDMBoltBuf[n] == 0)
                {
                    PDMBoltBuf[n] = 4;
                }
            }
        }
        currentBoltSum = 0;
        PDMBoltStute[0] = 0;
        PDMBoltStute[1] = 0;
        PDMBoltStute[2] = 0;
        PDMBoltStute[3] = 0;
        PDMBoltStute[4] = 0;
    }
}
/*******************************************/
//IR control 5 channel=1,2,3,4,5
//PDMBoltBuf[i] == 1)//ING
//PDMBoltBuf[i] == 2)//NOK
//PDMBoltBuf[i] == 4)//OK
//PDMBoltBuf[i] == 0)//
/*******************************************/
void MainWindow::fromsecondthreaddataIRMM(QString MI,QString WI,QString IsOk,int channel)
{
    DTdebug()<< "&&&&&&&&&&&&channel&&&&&&&&&&&" << channel << currentBoltSum<<PDMBoltStute[channel]<<StartBolt;
    int boltNum = StartBolt+channel;
    if(PDMBoltStute[channel] == 0)
    {
        currentBoltSum ++;
    }
    if(IsOk == "NOK" || IsOk == "LSN") // 本次螺栓不合格nok
    {
        if(boltNum < tempnumdpm[0])
        {
            DTdebug() << "1" ;
            label1[0][boltNum]->setText(QString("  T:"+MI+"Nm"));
            label2[0][boltNum]->setText(QString("  A:"+WI+"Deg"));
            label1[0][boltNum]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            label2[0][boltNum]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            if(IsOk == "NOK")
                butt[0][boltNum]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
        }
        pdmnowromisOk = false;
        //nok_led(1);
        PDMBoltBuf[channel] = 2; //NOK
        if(PDMBoltStute[channel] == 0)
        {
            whichpdmnumnow[0] ++;
        }
        DTdebug() << "**NOK***PDMBoltBuf[0]*****" << PDMBoltBuf[0];
        DTdebug() << "**NOK***PDMBoltBuf[1]*****" << PDMBoltBuf[1];
        DTdebug() << "**NOK***PDMBoltBuf[2]*****" << PDMBoltBuf[2];
        DTdebug() << "**NOK***PDMBoltBuf[3]*****" << PDMBoltBuf[3];
        DTdebug() << "**NOK***PDMBoltBuf[4]*****" << PDMBoltBuf[4];
    }
    else  //本次螺栓合格ok
    {
        if(boltNum < tempnumdpm[0])
        {
            if(MI == "-1" && WI == "-1") //nok确认1 个
            {
                butt[0][boltNum]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                if(label1[0][boltNum]->text() == "")
                {
                    label1[0][boltNum]->setText(QString("   T:-1Nm"));
                    label2[0][boltNum]->setText(QString("   A:-1Deg"));
                    label1[0][boltNum]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    label2[0][boltNum]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                }
                allTightenResultFlag = false;
            }
            else
            {
                butt[0][boltNum]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                label1[0][boltNum]->setText(QString("  T:"+MI+"Nm"));
                label2[0][boltNum]->setText(QString("  A:"+WI+"Deg"));
                label1[0][boltNum]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                label2[0][boltNum]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(25, 125, 44);");
            }
            //nok_led(0);
            PDMBoltBuf[channel] = 0; //OK
            if(PDMBoltStute[channel] == 0)
            {
                whichpdmnumnow[0]++;  //OK
            }
            DTdebug() << "**OK***PDMBoltBuf[0]*****" << PDMBoltBuf[0];
            DTdebug() << "**OK***PDMBoltBuf[1]*****" << PDMBoltBuf[1];
            DTdebug() << "**OK***PDMBoltBuf[2]*****" << PDMBoltBuf[2];
            DTdebug() << "**OK***PDMBoltBuf[3]*****" << PDMBoltBuf[3];
            DTdebug() << "**OK***PDMBoltBuf[4]*****" << PDMBoltBuf[4];
        }
        //        PDMCurrentState[0] = "OK";
    }
    PDMBoltStute[channel] = 1;
    if(PDMBoltNum == currentBoltSum)   //bolt number of group OK
    {
        DTdebug()<<"******pdmnowromisOk *****"<<pdmnowromisOk<<PDMBoltNum<<groupAllBoltNumCh[0];
        if((pdmnowromisOk))
        {
            StartBolt += currentBoltSum;
            QualifiedNum = whichpdmnumnow[0]+1;//合格数加1
            ui->label_hege->setText(QString::number(QualifiedNum));
            int i;
            currentBoltNum = 0;
            for(i = whichpronumis-1;i<20;i++) //clear finished number
            {
                if(carInfor[0].boltNum[i])
                {
                    carInfor[0].boltNum[i]--;
                    currentBoltNum++;
                }
                whichpronumis++;
                if(currentBoltNum == PDMBoltNum)
                {
                    break;
                }
            }
            if(groupAllBoltNumCh[0]>PDMBoltNum)
            {
                groupAllBoltNumCh[0] -= PDMBoltNum;
                currentFirstBolt += PDMBoltNum;
                if(tempnumdpm[0]>(currentFirstBolt+PDMBoltNum))
                {
                    pdmflickerNum = PDMBoltNum;
                }
                else
                {
                    pdmflickerNum = tempnumdpm[0] - currentFirstBolt;
                }
                int i;
                for(i=0;i<5;i++)
                {
                    if(PDMBoltBuf[i] == 0)
                    {
                        PDMBoltBuf[i] = 1;
                    }
                }
                for( i = whichpronumis-1;i<20;i++)
                {
                    DTdebug() << "*****i******" << i<<carInfor[0].boltNum[i];
                    if(carInfor[0].boltNum[i])
                    {
                        ui->label_pronum->setText(carInfor[0].proNo[i]);
                        ui->label_lsnum->setText(QString::number(PDMBoltNum));
                        emitOperate1(true,StartBolt);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else //finished groupAllBoltNumCh[0] = 0
            {
                if(isAlign)
                {
                    //yellow_led(0);
                    //red_led(0);
                    isAlign = false;
                }
                if(allTightenResultFlag)
                {
                    displayStatusIcon("OK");
                    SYSS = "OK";
                    //green_led(1);
                    DTdebug() << "******GroupOK********";
                }
                else
                {
                    displayStatusIcon("NOK");
                    SYSS = "NOK";
                    //red_led(1);
                    DTdebug() << "******GroupFail********";
                }
                if(timerpdm.isActive())
                {
                    timerpdm.stop();
                }
                groupAllBoltNumCh[0] = 0;
                emitOperate1(false,0);
                TaoTongState = false;
                ISmaintenance = false;
                //white_led(0);
                Stop();
                if(IsQueue)
                {
                    //结果打完  队列标志位置1
                    UpdateSqlFlag();
                    //当前条码打完 去队列找下一条 发使能
                    FisTimer.start(5000);
                }
                if (IsQueue && factory=="Haima")
                {
                    Q_EMIT signalGetCarInfo(0,serialNums);
                }

                startLine();
            }
        }
        else
        {
            whichpdmnumnow[0] -= currentBoltSum;
            for(int n=0;n<5;n++)
            {
                if(PDMBoltBuf[n] == 0)
                {
                    PDMBoltBuf[n] = 4;
                }
            }
        }
        pdmnowromisOk = true;
        currentBoltSum = 0;
        PDMBoltStute[0] = 0;
        PDMBoltStute[1] = 0;
        PDMBoltStute[2] = 0;
        PDMBoltStute[3] = 0;
        PDMBoltStute[4] = 0;
    }
}

/*******************************************/
//2 channel=1,2
/*******************************************/
void MainWindow::fromsecondthreaddata(QString MI1,QString WI1,QString IsOk1,QString MI2,QString WI2,QString IsOk2)
{
    DTdebug()<<"*********************************************11111111111111111111111*****************************************************************************";
    if((IsOk1 == "NOK")||(IsOk2 == "NOK"))//one nok
    {
        if(whichpdmnumnow[0] < tempnumdpm[0])
        {
            label1[0][whichpdmnumnow[0]]->setText(QString("  T:"+MI1+"Nm"));
            label2[0][whichpdmnumnow[0]]->setText(QString("  A:"+WI1+"Deg"));
            label1[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            label2[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            if(IsOk1 == "NOK")
            {
                PDMBoltBuf[0] = 2; //NOK
                PDMCurrentState[0] = "NOK";
                //on_pushButton_17_clicked();
            }
            else
            {
                PDMBoltBuf[0] = 4; //OK
            }
        }
        if((whichpdmnumnow[0]+5) < tempnumdpm[0])
        {
            label1[0][whichpdmnumnow[0]+5]->setText(QString("  T:"+MI2+"Nm"));
            label2[0][whichpdmnumnow[0]+5]->setText(QString("  A:"+WI2+"Deg"));
            label1[0][whichpdmnumnow[0]+5]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            label2[0][whichpdmnumnow[0]+5]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
            if(IsOk2 == "NOK")
            {
                PDMBoltBuf[1] = 2; //NOK
                PDMCurrentState[1] = "NOK";
                //on_pushButton_17_clicked();
            }
            else
            {
                PDMBoltBuf[1] = 4; //OK
            }
        }
        //nok_led(1);
        ui->label_pronum->setText(carInfor[0].proNo[whichpdmnumnow[0]]);

       // on_pushButton_17_clicked();
    }
    else  //本次螺栓合格ok
    {
        if(whichpdmnumnow[0] < tempnumdpm[0])
        {
            if(MI1 == "-1" && WI1 == "-1")
            {
                pdmnowromisOk = false;
                if(PDMCurrentState[0] != "NOK")
                {
                    label1[0][whichpdmnumnow[0]]->setText(QString("  T:-1Nm"));
                    label2[0][whichpdmnumnow[0]]->setText(QString("  A:-1Deg"));
                    label1[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    label2[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                }
                butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
            }
            else
            {
                label1[0][whichpdmnumnow[0]]->setText(QString("  T:"+MI1+"Nm"));
                label2[0][whichpdmnumnow[0]]->setText(QString("  A:"+WI1+"Deg"));
                label1[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                label2[0][whichpdmnumnow[0]]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                butt[0][whichpdmnumnow[0]]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                ui->label_pronum->setText(carInfor[0].proNo[whichpdmnumnow[0]]);
                QualifiedNum++;//合格数加1
            }
            PDMCurrentState[0] = "OK";
        }
        if((whichpdmnumnow[0]+5) < tempnumdpm[0])
        {
            if(MI2 == "-1" && WI2 == "-1")
            {
                pdmnowromisOk = false;
                if(PDMCurrentState[1] != "NOK")
                {
                    label1[0][whichpdmnumnow[0]+5]->setText(QString("  T:-1Nm"));
                    label2[0][whichpdmnumnow[0]+5]->setText(QString("  A:-1Deg"));
                    label1[0][whichpdmnumnow[0]+5]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                    label2[0][whichpdmnumnow[0]+5]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                }
                butt[0][whichpdmnumnow[0]+5]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
            }
            else
            {
                label1[0][whichpdmnumnow[0]+5]->setText(QString("  T:"+MI2+"Nm"));
                label2[0][whichpdmnumnow[0]+5]->setText(QString("  A:"+WI2+"Deg"));
                label1[0][whichpdmnumnow[0]+5]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                label2[0][whichpdmnumnow[0]+5]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(25, 125, 44);");
                butt[0][whichpdmnumnow[0]+5]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt;color:rgb(248,255,255)");
                QualifiedNum++;//合格数加1
            }
            PDMCurrentState[1] = "OK";
        }
        ui->label_hege->setText(QString::number(QualifiedNum));
        SetLsInfo(carInfor[0].proNo[whichpdmnumnow[0]], QString::number(carInfor[0].boltNum[whichpdmnumnow[0]]), QString::number(carInfor[0].ttNum[whichpdmnumnow[0]]));
        //nok_led(0);
        whichpdmnumnow[0]++;  //OK
        PDMBoltBuf[0] = 1; //ING
        PDMBoltBuf[1] = 1; //ING
        if((carInfor[0].boltNum[whichpdmnumnow[0]] == 0)||(carInfor[1].boltNum[whichpdmnumnow[0]] == 0))
        {
            if(!pdmnowromisOk)
            {
                displayStatusIcon("NOK");
                //                ui->pushButton_16->setStyleSheet("font: 40pt ; background-color: rgb(255, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
                //                ui->pushButton_16->setText("NOK");
                SYSS = "NOK";
                //red_led(1);
                DTdebug() << "******GroupFail********";
            }
            else
            {
                displayStatusIcon("OK");
                //                ui->pushButton_16->setText("OK");
                //                ui->pushButton_16->setStyleSheet("font: 60pt ; background-color: rgb(25, 125, 44); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
                SYSS = "OK";
                //green_led(1);
                DTdebug() << "******GroupOK********";
            }
            TaoTongState = false;
            ISmaintenance = false;
            //white_led(0);
            Stop();
            if(timerpdm.isActive())
            {
                timerpdm.stop();
            }
            ui->progressBar->setValue(0);
            m_CurrentValue  = 0;
            gunPower(false);
            whichpdmnumnow[0] = 0;
            pdmnowromisOk = true;
            PDMBoltBuf[0] = 0;
            PDMBoltBuf[1] = 0;
        }
    }
}



/**********************GWK Init*****************************/
void MainWindow::on_pushButton_12_clicked()
{
    emit gwkInit();
}

void MainWindow::on_pushButton_1_clicked()
{
    temp.append("1");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_2_clicked()
{
    temp.append("2");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_3_clicked()
{
    temp.append("3");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_4_clicked()
{
    temp.append("4");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_5_clicked()
{
    temp.append("5");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_6_clicked()
{
    temp.append("6");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_7_clicked()
{
    temp.append("7");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_8_clicked()
{
    temp.append("8");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_9_clicked()
{
    temp.append("9");
    isFull++;
    judge();
}

void MainWindow::on_pushButton_0_clicked()
{
    temp.append("0");
    isFull++;
    judge();
}


void MainWindow::on_pushButton_delete_clicked()
{
    if(isFull == 1)
    {
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        isFull--;
        temp.resize(temp.size() - 1);
    }
    else if(isFull == 2)
    {
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/95.bmp)");
        isFull--;
        temp.resize(temp.size() - 1);
    }
    else if(isFull == 3)
    {
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/96.bmp)");
        isFull--;
        temp.resize(temp.size() - 1);
    }
}

void MainWindow::judge()
{
    if(isFull == 1)
    {
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/95.bmp)");
        ui->pushButton_error->setStyleSheet("border-image: url(:/re/142.bmp)");
    }
    else if(isFull == 2)
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/96.bmp)");
    else if(isFull == 3)
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/97.bmp)");
    else if(isFull == 4)
    {
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/98.bmp)");
        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        if(configIniRead->value(QString("baseinfo/GCpassword")).toString()==""||
                configIniRead->value(QString("baseinfo/ControllerIp_1")).toString()=="" )
        {
            system("cp /config1.ini /config.ini &");
        }
        delete configIniRead;
        QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
        if (person ==0)
        {
            if(temp == config->value(QString("baseinfo/GCpassword")).toString()||
                    temp == "5027")
            {
                isJS = false;
                ui->stackedWidget->setCurrentIndex(0);
                newconfiginfo->initui();
                //newconfiginfo->clearCache();
                newconfiginfo->show();
                ui->pushButton_13->setStyleSheet("border-image: url(:/re/99.png)");
                ui->pushButton_password->setStyleSheet("border-image : url(:)");
                ui->pushButton_14->setStyleSheet("border-image : url(:/re/100.png)");
                ui->pushButton_error->setStyleSheet("border-image : url(:)");
                //ui->stackedWidget->setCurrentIndex(0);
                //usleep(10000);
            }
            //            else if(temp == "0852")
            //            {
            //                e3 = new QGraphicsOpacityEffect(this);
            //                e3->setOpacity(0.5);
            //                ui->label_black->setGraphicsEffect(e3);
            //                ui->label_black->show();
            //                ui->label_black->setGeometry(0,0,1366,768);
            //                SaveWhat = "config_init";
            //                save = new Save;
            //                connect(save,SIGNAL(configinit(bool)),this,SLOT(config_init(bool)));
            //                save->show();
            //            }
            else
            {
                ui->pushButton_error->setStyleSheet("border-image : url(:/re/124.bmp)");
                ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
                isFull = 0;
                temp = "";
            }
        }
        else if (person ==1)
        {
            if(temp == config->value(QString("baseinfo/JSpassword")).toString())
            {
                // DTdebug() << "SSSSSSSSSSSSSSSS";
                isJS = true;
                ui->stackedWidget->setCurrentIndex(0);
                newconfiginfo->initui();
                newconfiginfo->show();
                ui->pushButton_14->setStyleSheet("border-image: url(:/re/100.png)");
                ui->pushButton_13->setStyleSheet("border-image : url(:/re/99.png)");
                ui->pushButton_error->setStyleSheet("border-image : url(:)");
                ui->pushButton_password->setStyleSheet("border-image : url(:)");
                // ui->stackedWidget->setCurrentIndex(0);
                //usleep(10000);

            }
            else
            {
                ui->pushButton_error->setStyleSheet("border-image : url(:/re/124.bmp)");
                ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
                isFull = 0;
                temp = "";
            }
        }
        delete config;
    }
}


void MainWindow::on_pushButton_13_clicked()
{
    if(person != 0)
    {
        ui->pushButton_13->setStyleSheet("border-image : url(:/re/93.bmp)");
        ui->pushButton_14->setStyleSheet("border-image: url(:/re/100.png)");

        ui->pushButton_error->setStyleSheet("border-image : url(:)");
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        isFull = 0;
        temp = "";
        person = 0;
    }
}

void MainWindow::on_pushButton_14_clicked()
{
    if(person != 1)
    {
        ui->pushButton_14->setStyleSheet("border-image : url(:/re/92.bmp)");
        ui->pushButton_13->setStyleSheet("border-image: url(:/re/99.png)");

        ui->pushButton_error->setStyleSheet("border-image : url(:)");
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        isFull = 0;
        temp = "";
        person = 1;
    }
}

void MainWindow::on_pushButton_15_clicked()
{
    ui->pushButton_13->setStyleSheet("border-image : url(:/re/93.png)");
    ui->pushButton_14->setStyleSheet("border-image : url(:/re/100.png)");
    ui->pushButton_error->setStyleSheet("border-image : url(:)");
    ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget_6->setCurrentIndex(0);
    //QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    //config->setValue("baseinfo/workmode","0");
    //delete config;
    ISmaintenance = false;
    workmode = false;
}

void MainWindow::ShowTime()
{
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    QString str = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
    ui->label_12->setText(str);
    ui->label_clock_2->setText(str);
    //    ui->label_14->setText(str);
    if(restart_enabled && Factory !="Haima" && Factory != "SVW2" && Factory != "Dongfeng" && Factory != "BAIC")
    {
        int currentHour=str.mid(11,2).toInt();
        int currentMinute=str.mid(14,2).toInt();
        int currentSecond=str.mid(17,2).toInt();
        QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
        QString restartTime = config->value(QString("baseinfo/restartTime")).toString();
        delete config;
        if(currentHour==restartTime.mid(0,2).toInt() &&
                currentMinute==restartTime.mid(2,2).toInt() &&
                currentSecond==restartTime.mid(4).toInt())
        {
            restart_times++;
            DTdebug()<<"restart_times"<<restart_times;
            if(restart_times == 1)
            {
                restart_times = 0;
                DTdebug()<<"=======================================restart time";
                system("killall -9 Data2_UpLoad");
                //                system("/etc/data2/Data2_UpLoad -qws &");
                //                emit closeThread();
                //                restartProgram();
                //                //sleep(3);
                //                pid_t pid = getpid();
                //                DTdebug()<<"kill pid"<<pid;
                //                DTdebug()<<QString("kill ")+QString::number(pid);
                //                system((QString("kill ")+QString::number(pid)).toLocal8Bit().data());
                //                emit killProcess();
            }
        }
    }
}

//void MainWindow::restartProgram()
//{
//    QString fileName = "/etc/profile";
//    QFile file(fileName);
//    if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
//        DTdebug()   << "Mainwindow Cannot open profile file for Reading";
//        return;
//    }
//    QString str(file.readAll());
//    file.close();
//    if(str.contains("/etc/data2/", Qt::CaseInsensitive)){
//        QRegExp rxLine("/etc/data2/\\S*");
//        if(rxLine.indexIn(str) != -1)
//        {
//            DTdebug()<<"run "<<rxLine.cap(0)+" -qws &";
//            system((rxLine.cap(0)+" -qws &").toLocal8Bit().data());
//        }
//    }
//}

//void MainWindow::on_pushButton_18_clicked()
//{
//    ui->stackedWidget_6->setCurrentIndex(0);
//}
/*************************************************/
//progressBar decrease of timer
/*************************************************/
void MainWindow::UpdateSlot()
{
    m_CurrentValue++;
    TimeLeft = Tacktime-m_CurrentValue/10;

    if(redTwinkleTime > Tacktime)
    {
        redTwinkleTime = 10 ;
    }

    else if(redTwinkle && TimeLeft <= redTwinkleTime && m_CurrentValue%2 == 0)
    {
        //红灯闪烁
        if(redLedWorked)
        {
            redLedWorked = false ;
        }
        else
        {
            redLedWorked = true ;
        }
        Q_EMIT signalRedTwinkle(redLedWorked);
    }

    lock.lockForWrite();
    info[2] = QString::number(TimeLeft);
    lock.unlock();
    if( m_CurrentValue == m_MaxValue )
    {
        Stop();
        if( (variable_1 == "master")||(variable_1 == "slave") ) //
        {
            on_pushButton_reset_clicked();//all nok
        }
    }
    ui->progressBar->setValue(m_CurrentValue);
}

/*************************************************
 * progressBar decrease start
*************************************************/
void MainWindow::Start(int interval/* =100 */, int maxValue/* =600 */)
{
    progressBarStop = false ;
    progressBarAll  = false ;
    if(unCacheRfidFlag)
    {
        nextCarCatch = true ;
        nextCarAll = true ;
    }
    TimeLeft=Tacktime;
    lock.lockForWrite();
    info[2] = QString::number(TimeLeft);
    lock.unlock();
    m_UpdateInterval = interval;
    m_MaxValue = maxValue;
    m_Timer.start(m_UpdateInterval);
    ui->progressBar->setRange(0, m_MaxValue);
    ui->progressBar->setValue(0);
}

/*************************************************
 * progressBar stop
*************************************************/
void MainWindow::Stop()
{
    if(redTwinkle)
    {
        emit signalRedTwinkle(false);
    }

    if(cutEnableFlag)
    {
        emit sendOperate1(false,1) ;
        progressBarStop = true;
        progressBarAll  = true;
    }

    ui->progressBar->setValue(0);
    m_Timer.stop();
    m_CurrentValue  = 0;
    TimeLeft=0;
    lock.lockForWrite();
    info[2] = QString::number(TimeLeft);
    lock.unlock();
}

//rfid Ui
void MainWindow::setRfidState(bool istrue)
{
    if(istrue)
    {
        isRFIDConnected = true;
        ui->label_ss4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        ui->label_ss4_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        systemStatus = 0;
    }
    else
    {
        isRFIDConnected = false;
        ui->label_ss4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        ui->label_ss4_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        systemStatus = 4;
    }
}
/*******************************************/
//NOK all bolt
/*******************************************/
void MainWindow::on_pushButton_reset_clicked()
{
    if(factory == "GZBAIC")
    {
        return ;
    }
    //no ok 确认全部
    RFIDlock.lockForWrite();
    if(rfidNextCom && Factory != "Dongfeng")    //数据库不记录全部确认的数据
    {
        //RFID模式 ING 状态确认
        rfidNextCom = false;
        RFIDlock.unlock();
        QString pintmpp,G9tmpp;
        if(Factory == "Benz")
        {
            pintmpp = VIN_PIN_SQL_RFID.mid(0,17);
            G9tmpp = "";
        }
        else
        {
            if(VIN_PIN_SQL_RFID_previous != "")
            {
                pintmpp = VIN_PIN_SQL_RFID_previous.mid(0,14);
                G9tmpp = VIN_PIN_SQL_RFID_previous.right(4);
                resetUiDo();
                DTdebug()<<"no ok VIN_PIN_SQL_RFID_previous"<<VIN_PIN_SQL_RFID_previous;
                getSerialNum(pintmpp,false,G9tmpp);
                VIN_PIN_SQL_RFID_previous = "";
            }
            emit sendNokAll(2,2);
            emit sendNokAll(2);
            resetUiDo();
            pintmpp = VIN_PIN_SQL_RFID.mid(0,14);
            G9tmpp = VIN_PIN_SQL_RFID.right(4);
        }
        DTdebug()<<"rfdiNextCom reset";
        getSerialNum(pintmpp,false,G9tmpp);
    }
    else
    {
        RFIDlock.unlock();
        DTdebug()<<"normal reset";
        if(SYSS == "ING")
        {
            for(int i=0;i<2;i++)
            {
                for(int j=whichpdmnumnow[i];j<tempnumdpm[i];j++)
                {
                    DTdebug() << "******nok all*******"<<i<<j;
                    butt[i][j]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt;color:rgb(248,255,255)");
                    if(label1[i][j]->text() == "")
                    {
                        label1[i][j]->setText(QString("   T:-1Nm"));
                        label2[i][j]->setText(QString("   A:-1Deg"));
                        label1[i][j]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:1px 1px 0 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                        label2[i][j]->setStyleSheet("font: 14pt;background-color: rgb(248, 248, 248);border-width:0 1px 1px 1px; border-style:solid; border-color:rgb(255, 0, 0);");
                        lock.lockForWrite();
                        status[j][0]="2";
                        status[j][1]="-1";
                        status[j][2]="-1";
                        lock.unlock();
                    }
                }
            }
            ISRESET = true;  //nok all reset
            if(groupAllBoltNumCh[0]>0)
            {
                emitOperate1(false,1);
            }
            if(groupAllBoltNumCh[1]>0)
            {
                emit emitOperate2(false,1);
            }
            emit sendNokAll(2,1);
            emit sendNokAll(2);
            DTdebug() << "******nok all*******";
            gunPower(false);
            resetUiDo();
//            DTdebug() << "******SingleFail*******"; //add
            //red_led(1);
            TaoTongState = false; //确认全部关闭套筒
            emit sendRunStatusToJson("NOK");
        }

        if(IsQueue && (factory == "BYDSZ" || factory == "BYDXA" || m_BAICQueue) )
        {
            //结果打完  队列标志位置1
            UpdateSqlFlag();
            //当前条码打完 去队列找下一条 发使能
            FisTimer.start(5000);
        }
        else if(IsQueue && factory == "BAIC")
        {
            QTimer::singleShot(5000,this,SLOT(leuzeTimerStart()));
        }
        else if (IsQueue && factory=="Haima")
        {
            Q_EMIT signalGetCarInfo(0,serialNums);
        }

        startLine();
    }
}
// ING 状态确认
void MainWindow::resetUiDo()
{
    lock.lockForWrite();
    info[0] = "3";
    lock.unlock();
    SYSS = "NOK";
    Stop();
    displayStatusIcon("NOK");
    //    ui->pushButton_16->setStyleSheet("font: 40pt ; background-color: rgb(255, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
    //    ui->pushButton_16->setText("NOK");
    //    workmode = false;
    ISmaintenance = false;
    //white_led(0);
    //nok_led(0);
    enableLsnumber = 0;
    timerpdm.stop();
}
/*******************************************/
//NOK one bolt -1 -1 -1
/*******************************************/
void MainWindow::on_pushButton_17_clicked()
{
    //nok确认1个
    DTdebug() << PDMBoltBuf << allChannel;
    int channel;
    RFIDlock.lockForRead();
    if(rfidNextCom && Factory !="Dongfeng")//
    {
        RFIDlock.unlock();
        on_pushButton_reset_clicked(); //NOK all bolt
    }
    else
    {
        RFIDlock.unlock();
        if(PDMBoltNum > 0)
        {
            //            on_pushButton_reset_clicked(); //NOK all bolt
            for(int i=0;i<PDMBoltNum;i++)
            {
                emit  sendfromsecondthread(carInfor[0].boltSN[StartBolt+i],VIN_PIN_SQL,carInfor[0].proNo[StartBolt+i],0);
                if(variable_1 == "IR")//IRMM
                {
                    fromsecondthreaddataIRMM("-1","-1","OK",i);
                }
                else//PFMM
                {
                    fromsecondthreaddata("-1","-1","OK",i);

                }
            }
        }
        else
        {
            if((factory == "GZBAIC") && (whichar != 0) && (currBolt < boltAmount))
            {
                DTdebug() << "baic jump one bolt" ;
                if(boltStatus == "OK")
                {
                    currBolt ++ ;
                }
                else {
                    DTdebug() << "last bolt NOK ,do not add" ;
                }

                emit sendfromsecondthread( QString::number(currBolt) ,VIN_PIN_SQL,"0",1);
                emit skipBolt() ;
                slotShowBolt(VIN_PIN_SQL.toUtf8(),"-1","-1","OK",currBolt);
                return;
            }
            else if((factory == "GZBAIC") && (whichar != 0) && (currBolt == boltAmount) && boltStatus=="NOK")
            {
                DTdebug() << "baic jump one bolt NOK last" ;

                emit sendfromsecondthread( QString::number(currBolt) ,VIN_PIN_SQL,"0",1);
                emit skipBolt() ;
                slotShowBolt(VIN_PIN_SQL.toUtf8(),"-1","-1","OK",currBolt);
                return;
            }
            else if ((factory == "GZBAIC") && (whichar == 0))
            {
                DTdebug() << "can not jump" << whichar ;
                return ;
            }
            if(allChannel == 1)//single channel
            {
                if(carInfor[0].boltNum[groupNumCh[0]]>0)
                {
                    channel = 1;
                }
                else if(carInfor[1].boltNum[groupNumCh[1]]>0)
                {
                    channel = 2;
                }
                else
                {
                    return;
                }
                emitOperate1(false,98);//job batch incerment
                emit sendfromsecondthread(carInfor[channel-1].boltSN[groupNumCh[channel-1]],VIN_PIN_SQL,carInfor[channel-1].proNo[groupNumCh[channel-1]],channel);
                //                ISRESET = true; //nok one
                fromsecondthreaddata("-1","-1","OK",channel-1,groupNumCh[channel-1]);
            }
            else
            {
                if(!isSaveShow)
                {
                    e3 = new QGraphicsOpacityEffect(this);
                    e3->setOpacity(0.5);
                    ui->label_black->setGraphicsEffect(e3);
                    ui->label_black->show();
                    ui->label_black->setGeometry(0,0,1366,768);
                    isSaveShow = true;
                    SaveWhat = "which_one";
                    save = new Save(this);
                    connect(save,SIGNAL(choiceChannel(int)),this,SLOT(nokOneBolt(int)));
                    save->show();
                }
            }
        }
    }
    //
}
/***********************************************/
//NOK one bolt
/***********************************************/
void MainWindow::nokOneBolt(int tmp)
{
    if((tmp == 1)||(tmp == 2))
    {
        if(carInfor[tmp-1].boltNum[groupNumCh[tmp-1]]>0)
        {
            if(SYSS == "ING")
            {
                emit  sendfromsecondthread(carInfor[tmp-1].boltSN[groupNumCh[tmp-1]],VIN_PIN_SQL,carInfor[tmp-1].proNo[groupNumCh[tmp-1]],tmp);
                fromsecondthreaddata("-1","-1","OK",tmp-1,groupNumCh[tmp-1]);
            }
        }
    }
    ui->label_black->hide();
    delete e3;
    delete save;
    SaveWhat = "";
    isSaveShow = false;
}
/***********************************************/
//SB356_PLC NOK two bolt
/***********************************************/
void MainWindow::nokWhichBolt(int tmp)
{
    int i=whichpdmnumnow[0];
    QString boltSN[2],proNO[2];
    if(SYSS == "ING")
    {
        if(tmp < 10) //one bolt group NOK
        {
            if(tmp == whichpdmnumnow[0])//current bolt group= NOK bolt group
            {
                boltSN[0] = carInfor[0].boltSN[whichpdmnumnow[0]];
                boltSN[1] = carInfor[1].boltSN[whichpdmnumnow[0]];
                proNO[0]  = carInfor[0].proNo[whichpdmnumnow[0]];
                proNO[1]  = carInfor[1].proNo[whichpdmnumnow[0]];
                emit sendOneGroupNok(VIN_PIN_SQL,boltSN,proNO);
                fromsecondthreaddata("-1","-1","OK","-1","-1","OK");
            }
        }
        else//all bolt group NOK
        {
            while((carInfor[0].boltNum[i]!=0)&&(carInfor[1].boltNum[i]!=0))
            {
                boltSN[0] = carInfor[0].boltSN[i];
                boltSN[1] = carInfor[1].boltSN[i];
                proNO[0]  = carInfor[0].proNo[i];
                proNO[1]  = carInfor[1].proNo[i];
                emit sendOneGroupNok(VIN_PIN_SQL,boltSN,proNO);
                fromsecondthreaddata("-1","-1","OK","-1","-1","OK");
                i++;
            }
        }
    }
}

//重启系统
void MainWindow::signal_mysqlerror_do()
{
    if(!isSaveShow)
    {
        emitOperate1(false,groupNumCh[0]);
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);
        isSaveShow = true;
        SaveWhat = "sqlerror";
        save = new Save(this);
        //    connect(save,SIGNAL(sendShutDown(int)),this,SLOT(shutdown(int)));
        save->show();
    }
}

void MainWindow::init()
{
    ui->stackedWidget_6->setCurrentIndex(0);
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    ui->label_gongduan->setText(config->value(QString("baseinfo/StationName")).toString());
    ui->label_gongwei->setText(config->value(QString("baseinfo/StationId")).toString());
    //ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/mainlogo.png);");
    workmode = false;
    //config->setValue("baseinfo/workmode","0");
    //ui->stackedWidget->setCurrentIndex(0);
    if(!controlMode)
    {
        ui->label_17->setText(tr("等待车辆进站..."));
        emit sLightLogic(e_WaitForScan);

        //    ui->label_tighten->setText("Atlas");
        if(isRFID)
        {
            if(Factory == "Dongfeng")
                ui->pushButton_tiaoma->setText(tr("自动"));
            else
                ui->pushButton_tiaoma->setText("RFID");
            ui->label_11->show();
            ui->label_ss4->show();
            ui->label_ss4_2->show();
            ui->label_13->show();
        }
        else if(isBarCode)
        {
            ui->pushButton_tiaoma->setText(tr("条码枪"));
            ui->label_11->hide();
            ui->label_ss4->hide();
            ui->label_ss4_2->hide();
            ui->label_13->hide();
        }
        else if(IsQueue)
        {
            if(!m_BAICQueue && Factory == "BAIC")
                ui->pushButton_tiaoma->setText(tr("自动条码枪"));
            else
                ui->pushButton_tiaoma->setText(tr("队列"));
            ui->label_11->hide();
            ui->label_ss4->hide();
            ui->label_ss4_2->hide();
            ui->label_13->hide();
        }
    }
    else
    {
        ui->pushButton_tiaoma->setText(tr("手动"));
        ui->label_17->setText(tr("当前手动模式..."));
    }
    if(ControlType_1=="SB356_PLC")
    {
        ui->label_20->hide();
        ui->label_ss11->hide();
    }
    else if(ControlType_2!="OFF")
    {
        ui->label_20->show();
        ui->label_ss12->show();
    }
    if(Factory != "Benz")
        ui->label_ss3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");

    Tacktime = config->value("baseinfo/TackTime").toInt();
    if(Tacktime == 0)
        Tacktime = 60;

    lock.lockForWrite();
    info[5] = QString::number(Tacktime);
    info[6] = config->value(QString("baseinfo/StationId")).toString();
    lock.unlock();

    delete config;

    QDateTime dateTime = QDateTime::currentDateTime();
    int year=dateTime.date().year();
    if(year<2017)
        ui->label_time->setText(tr("系统时间错误"));
    else
    {
        ui->label_time->clear();
    }
}

void MainWindow::initBack()
{
    //    ui->stackedWidget_6->setCurrentIndex(0);
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    ui->label_gongduan->setText(config->value(QString("baseinfo/StationName")).toString());
    ui->label_gongwei->setText(config->value(QString("baseinfo/StationId")).toString());

    if(ControlType_1=="SB356_PLC")
    {
        ui->label_20->hide();
        ui->label_ss11->hide();
    }
    else if(ControlType_2!="OFF")
    {
        ui->label_20->show();
        ui->label_ss12->show();
    }
    if(Factory != "Benz")
        ui->label_ss3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");

    Tacktime = config->value("baseinfo/TackTime").toInt();
    if(Tacktime == 0)
        Tacktime = 60;
    lock.lockForWrite();
    info[5] = QString::number(Tacktime);
    info[6] = config->value(QString("baseinfo/StationId")).toString();
    lock.unlock();

    delete config;

    QDateTime dateTime = QDateTime::currentDateTime();
    int year=dateTime.date().year();
    if(year<2017)
        ui->label_time->setText(tr("系统时间错误"));
    else
    {
        ui->label_time->clear();
    }
}

//处理钥匙盒信号
void MainWindow::configwarning(bool isconfigone)
{
//    FUNC() ;

    if(isconfigone)  //skip one
        emit sLightLogic(e_SingleSkip);

    qDebug()<< "configwarning:" << isconfigone << nextCarCatch<< progressBarStop << isSaveShow ;
    ConfigOneOrAll = isconfigone;


    if(!isconfigone || !nextCarCatch || progressBarStop)//skip all
    {
        if(factory == "GZBAIC")
        {
            return ;
        }else if(factory == "KFCHERY"){
            emit signalSetIoBox(0,false);//恢复停线
        }else{
            if(!isSaveShow)
            {
                ISWARNING = true;
                e3 = new QGraphicsOpacityEffect(this);
                e3->setOpacity(0.5);
                ui->label_black->setGraphicsEffect(e3);
                ui->label_black->show();
                ui->label_black->setGeometry(0,0,1366,768);
                isSaveShow = true;
                SaveWhat = "input";
                save = new Save(this);
                connect(save,SIGNAL(sendCloseInput(bool)),this,SLOT(receiveCloseInput(bool)));
                save->show();
            }
        }
    }
    else
    {
        on_pushButton_17_clicked();
    }

}



/****************************************************
 * save弹框 关闭/保存
 * 修改时间：20180614
 * 修改功能：判断条件增加progressBarAll，即进度条结束
*****************************************************/
void MainWindow::receiveCloseInput(bool tmp)
{
    FUNC();
    if(tmp)
    {
        if(!ConfigOneOrAll || !nextCarAll || progressBarAll)
        {
            on_pushButton_reset_clicked();
            if(unCacheRfidFlag)
            {
                ui->label_17->setText(tr("等待车辆进站..."));
                Q_EMIT signalDeleteLastRfid();
            }

            ui->label_black->hide();
            progressBarAll = false;
            nextCarAll = true;
            delete e3;
            delete save;

           // emit sLightLogic(e_WaitForScan);
        }
        else
        {
            ui->label_black->hide();
            delete e3;
            delete save;
        }

        ISWARNING = false;
    }
    else
    {
        ui->label_black->hide();
        delete e3;
        delete save;
        ISWARNING = false;
    }
    SaveWhat ="";
    isSaveShow = false;
}

//void MainWindow::red_led(int leds)
//{
//    if(leds)
//        system("echo 1 > /root/gpio/OUT4 &");
//    else
//        system("echo 0 > /root/gpio/OUT4 &");
//}
//void MainWindow::green_led(int leds)
//{
//    if(leds)
//    {
//        Q_EMIT signalRedTwinkle(false);
//        system("echo 1 > /root/gpio/OUT3 &");
//    }
//    else
//        system("echo 0 > /root/gpio/OUT3 &");
//}
//void MainWindow::yellow_led(int leds)
//{
//    if(leds)
//        system("echo 1 > /root/gpio/OUT2 &");
//    else
//        system("echo 0 > /root/gpio/OUT2 &");
//}
//void MainWindow::white_led(int leds)
//{
//    if(leds)
//        system("echo 1 > /root/gpio/OUT1 &");
//    else
//        system("echo 0 >  /root/gpio/OUT1 &");
//}
//void MainWindow::nok_led(int leds)
//{
//    if(!TaoTongState)
//    {
//        if(leds)
//            system("echo 1 > /root/gpio/OUT0 &");
//        else
//            system("echo 0 > /root/gpio/OUT0 &");
//    }

//}

//void MainWindow::slotRedTwinkle(bool ledWorked)
//{
//    if(ledWorked)
//        system("echo 1 > /root/gpio/OUT4 &");
//    else
//        system("echo 0 > /root/gpio/OUT4 &");
//}


void MainWindow::wifishow(bool wificonnect)
{
    if(wificonnect)
    {
        ui->label_wifi->show();
        ui->label_wifi_2->show();
    }
    else
    {
        ui->label_wifi->hide();
        ui->label_wifi_2->hide();
    }
}
void MainWindow::datashow(bool dataconnect)
{
    DTdebug() << "wifi status" << dataconnect ;
    if(dataconnect)
    {
        emit sLightLogic(e_NetWorkConnect);  //emit sLightLogic(e_NetWorkConnect);
        ui->label_ss2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        ui->label_ss2_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        if(ledTimer.isActive())
        {
            ledTimer.stop();
        }
    }
    else
    {
        emit sLightLogic(e_NetWorkDisConnect);
        ui->label_ss2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        ui->label_ss2_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        if((!ledTimer.isActive()) && wifiWarning)
        {
            ledTimer.start(300);
        }

    }
}

//add lw  2017/9/19 定位标签电量显示
void MainWindow::lablebatteryshow(int power)
{
    switch(power)
    {
    case 0:
    case 1:
    case 2:
    case 3://20%
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_5.png);background:transparent;");
        break;
    case 4:
    case 5://40%
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_4.png);background:transparent;");
        break;
    case 6:
    case 7://60%
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_3.png);background:transparent;");
        break;
    case 8:
    case 9://80%
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_2.png);background:transparent;");
        break;
    case 10:
    case 11://100%
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_1.png);background:transparent;");
        break;
    case 12://充电
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_6.png);background:transparent;");
        break;
    case 13://充电充满
        ui->label_tagPower->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/lable_7.png);background:transparent;");
        break;
    case 14://隐藏
        ui->label_tagPower->hide();
        ui->pushButton_location->setText("error");//hide();
        break;
    default: break;
    }
}

void MainWindow::batteryshow1(QString power)
{
    if(power == "17")
    {
        //        if(timer_showdown.isActive())
        //        {
        //            DTdebug()<<"timer_showdown stop";
        //            timer_showdown.stop();
        //        }
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2_white.png);background:transparent;");
    }
    else if(power == "16")
    {
        //        if(timer_showdown.isActive())
        //        {
        //            DTdebug()<<"timer_showdown stop";
        //            timer_showdown.stop();
        //        }
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery1.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery1_white.png);background:transparent;");
    }
    else if(power == "05")
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_white.png);background:transparent;");
    }
    else if(power == "04")
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_2.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_2_white.png);background:transparent;");
    }
    else if(power == "03")
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
    }
    else if(power == "02"||power == "01")
    {
        //15 分钟后关机
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
        //        timer_showdown.start(900000);
    }
}

//void MainWindow::battery15()
//{
//    system("echo 0 > /sys/class/leds/control_lvds/brightness");  //关背光
//    DTdebug("power down 1");
//    system("poweroff");
//}

void MainWindow::batteryshow2(bool balive)
{
    if(balive)
    {
        shutdown_timer.start(300000);
        //        ui->label_battery->show();
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_white.png);background:transparent;");
    }
    else
    {
        shutdown_timer.stop();
        battry_num = 0;
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2_white.png);background:transparent;");
        //        ui->label_battery->show();
    }
}


void MainWindow::clocked()
{
    battry_num++;
    if(battry_num == 1)
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_2.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_2_white.png);background:transparent;");
    }
    else if(battry_num == 2)
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
        ui->label_battery_2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
    }
    else if(battry_num == 3)
    {
        //        system("echo 0 > /sys/class/leds/control_lvds/brightness");
        //        DTdebug("power down 2");
        //        system("poweroff");
    }
}

void MainWindow::time_warning(bool time_warn)
{
    if(time_warn)
        ui->label_time->setText(tr("系统时间错误"));
    else
        ui->label_time->clear();
}

//void MainWindow::ReceGunNotReady()
//{
//    //yellow_led(1);
//    //    ui->label_tiaomastate->setText(tr("拧紧枪没有准备"));
//    ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
//    displayStatusIcon("Not Ready");
//    //    ui->pushButton_16->setText("Not Ready");
//    //    ui->pushButton_16->setStyleSheet("font: 19pt ; background-color: rgb(250, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
//}

//void MainWindow::on_pushButton_shutdown_clicked()
//{
//    //    if(!ISmaintenance)
//    //    {
//    //        ISmaintenance = true;
//    //        workmode = true;
//    e3 = new QGraphicsOpacityEffect(this);
//    e3->setOpacity(0.5);
//    ui->label_black->setGraphicsEffect(e3);
//    ui->label_black->show();
//    ui->label_black->setGeometry(0,0,1366,768);
//    SaveWhat = "shutdown";
//    save = new Save;
//    connect(save,SIGNAL(sendShutDown(int)),this,SLOT(shutdown(int)));
//    save->show();
//    //    }
//}

void MainWindow::ReceFisSerial(QString serials)
{
    //接收fis队列发送条码
    if(SYSS!="ING")
        getSerialNum(serials,false,"ready");
}

void MainWindow::QueueTimerDo()
{
    DTdebug()<<"!!!!!!!!!!!!!!!!!QueueTimerDo";
    if(queue_timer.isActive())
        queue_timer.stop();
    //    UpdateSqlFlag();
    FisTimer.start(5000);
}

void MainWindow::FisTimerDo()
{
    FUNC() ;
    //当前条码打完 延时
    if( (!ISmaintenance)&&(CsIsConnect) )
    {
        if(FisTimer.isActive())
            FisTimer.stop();
        if(Factory == "Haima")
        {
            Q_EMIT signalGetCarInfo(0,serialNums);
        }

        else if(Factory == "BAIC")
        {
#if 1
            if(!query.exec("select VIN, carType from "+tablePreview+" where RecordID =(select min(RecordID) from "+tablePreview+" where UseFlag = 0)"))
                DTdebug() <<"mainwindow  "  << query.lastError();
#else
            if(!query.exec("select VIN, carType from "+tablePreview+" where UseFlag = 0 ORDER BY RecordID ASC LIMIT 1)"))
                DTdebug() <<"select next error "  << query.lastError();
#endif
            else
            {
                if(query.next())
                {
                    DTdebug()<<"QueueisNull= false";
                    Queuelock.lockForWrite();
                    QueueIsNull = false;
                    Queuelock.unlock();
                    BYDType = "";
                    ReceFisSerial(query.value(0).toString());
                }
                else
                {
                    DTdebug()<<"QueueisNull= true";
                    Queuelock.lockForWrite();
                    QueueIsNull = true;
                    Queuelock.unlock();
                }
            }
        }

        else {
            if(!query.exec("select VIN, Type from "+tablePreview+" where RecordID =(select min(RecordID) from "+tablePreview+" where UseFlag = 0)"))
                DTdebug() <<"mainwindow  "  << query.lastError();
            else
            {
                if(query.next())
                {
                    DTdebug()<<"QueueisNull= false";
                    Queuelock.lockForWrite();
                    QueueIsNull = false;
                    Queuelock.unlock();
                    BYDType = query.value(1).toByteArray();
                    ReceFisSerial(query.value(0).toString());
                }
                else
                {
                    DTdebug()<<"QueueisNull= true";
                    Queuelock.lockForWrite();
                    QueueIsNull = true;
                    Queuelock.unlock();
                }
            }
        }

    }
    else
    {
        if(!FisTimer.isActive())
            FisTimer.start(5000);
    }
}

void MainWindow::UpdateSqlFlag()
{
    //数据库命名方式太傻了，写个独立的sql线程吧，一个工厂不定只有一个数据库

    FUNC( ) ;
    // 更新数据库队列标志位
    int flag = 0;
    QString querytmp;
    if(DeleteVIN=="")
    {
        if(factory == "Haima")
            querytmp = "select min(RecordID) from TaskPreview where UseFlag = 0";
#if 0
        else if (Factory == "BAIC") {
            querytmp = "select min(34) from TaskPreview where UseFlag = 0";
        }
#endif
        else
            querytmp = "select min(RecordID) from "+tablePreview+" where UseFlag = 0";
    }

    else
    {
        if(factory == "Haima")
            querytmp = "select RecordID from TaskPreview where VIN = '"+DeleteVIN+"'";
#if 0
        else if (Factory == "BAIC") {
            querytmp = "select min(ID) from TaskPreview where UseFlag = 0";
        }
#endif
        else
            querytmp = "select RecordID from "+tablePreview+" where VIN = '"+DeleteVIN+"'";
    }
    DTdebug()<<"querytmp"<<querytmp;

    if(!query.exec(querytmp))
        DTdebug() <<"mainwindow "  << query.lastError();
    else
    {
        if(query.next())
        {
            flag = query.value(0).toInt();
            DTdebug()<<""<<flag << factory ;
            if(factory == "Haima")
            {
#if 0
                if(!query.exec("update TaskPreview set UseFlag = 1 where RecordID ="+QString::number(flag)))
                    DTdebug()<<"update UseFlag fail "<<query.lastError();
                else
                    DTdebug()<<"update UseFlag success RecordID:"<<flag;
#endif
            }
#if 0
            else if (Factory == "BAIC") {
                if(!query.exec("update "+tablePreview+" set UseFlag = 1 where ID ="+QString::number(flag)))
                    DTdebug()<<"update UseFlag fail "<<query.lastError();
                else
                    DTdebug()<<"update UseFlag success RecordID:"<<flag;
            }
#endif
            else {
                if(!query.exec("update "+tablePreview+" set UseFlag = 1 where RecordID ="+QString::number(flag)))
                    DTdebug()<<"update UseFlag fail "<<query.lastError();
                else
                    DTdebug()<<"update UseFlag success RecordID:"<<flag;
            }

        }
        else
        {
            DTdebug()<<"update tablePreview fail";
        }
    }
    DeleteVIN="";
}


//RFID新来车型显示
void MainWindow::receiveGetCar()
{
    if(SYSS=="ING")
    {
        if(Factory != "Dongfeng")
        {
            ISRESET = true; //next car nok all bolt
            emitOperate1(false,groupNumCh[0]);
            emit emitOperate2(false,groupNumCh[1]);
        }
        else
        {
            risingEdge();
            DTdebug()<<" Stop the production line ";
        }
        //red_led(1);
        //white_led(0);
        ui->label_17->setText(tr("下辆车已进站"));

        if(unCacheRfidFlag)
        {
            nextCarAll = false;
            nextCarCatch = false ;
        }
    }
    else
    {
        DTdebug()<<"ING change";
        //        resetUiDo();
        RFIDlock.lockForWrite();
        if(rfidNextCom)
        {
            VIN_PIN_SQL_RFID_previous = "";
            rfidNextCom = false;
            DTdebug()<<"rfidNextCom unlock";
        }
        RFIDlock.unlock();
        QString pintmpp;
        QString G9tmpp;
        if(Factory == "Benz")
        {
            pintmpp = VIN_PIN_SQL_RFID.mid(0,8);
            G9tmpp = "";
        }
        else if(Factory == "Dongfeng")
        {
            if(VIN_PIN_SQL_RFID != "-1-1" && VIN_PIN_SQL_RFID != "00")
            {
                pintmpp = VIN_PIN_SQL_RFID.mid(0,17);
                G9tmpp = VIN_PIN_SQL_RFID.mid(17,17);
            }
            else if(VIN_PIN_SQL_RFID == "-1-1")
            {
                pintmpp ="-1";
                G9tmpp = "-1";
            }
            else if(VIN_PIN_SQL_RFID == "00")
            {
                pintmpp ="0";
                G9tmpp = "0";
            }
        }
        else
        {
            pintmpp = VIN_PIN_SQL_RFID.mid(0,14);
            G9tmpp = VIN_PIN_SQL_RFID.right(4);
        }
        DTdebug()<<"receiveGetCar not ING";
        getSerialNum(pintmpp,false,G9tmpp);
    }
}

void MainWindow::getAlign(QString pintmpp, QString G9tmpp)    //柳汽对齐
{
    //red_led(1);
    //yellow_led(1);
    SYSS = "NOK";
    isAlign = true;
    DTdebug()<<"get align";
    RFIDlock.lockForWrite();
    rfidNextCom = false;
    RFIDlock.unlock();
    getSerialNum(pintmpp,false,G9tmpp);
}

void MainWindow::on_pushButton_16_clicked()
{
//    fromsecondthreaddata("0.47","8739","NOK",0,0);
//    getSerialNum("F2007B44711165000HJRPBGJB5NF029382",false,"VIN+CarType"); //开封测试
#if DYTEST4
    getSerialNum("78202272516543",false,"GM14");//大众MEB测试
#endif

#if DYTEST5
    getSerialNum("78202273353857",false,"3M12");//大众MEB测试
#endif

    if((IsQueue && !controlMode && (Factory =="BYDSZ"||Factory =="BYDXA")) || (Factory == "Dongfeng" && !manualMode)||m_BAICQueue)
    {
        if(!isSaveShow)
        {
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_black->setGraphicsEffect(e3);
            ui->label_black->show();
            ui->label_black->setGeometry(0,0,1366,768);
            isSaveShow = true;
            SaveWhat = "delete_car";
            //        if(SerialGunMode)
            //        {
            //            system("echo 0 > /sys/class/leds/control_uart2/brightness &");
            //        }
            save = new Save(this);
            connect(save,SIGNAL(sendDelete_car(bool)),this,SLOT(delete_car(bool)));
            connect(this,SIGNAL(sendDeleteCar_VIN(QString)),save,SLOT(show_VIN(QString)));
            save->show();
        }
    }
    else if (  SYSS != "ING"  && CsIsConnect ) {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);

        if(Factory == "AQCHERY")
        {
            inputCode_AQ = new Inputcode_AQ(this);
            connect(inputCode_AQ,SIGNAL(sendCode(QString)),this,SLOT(recvCode(QString)));
            connect(inputCode_AQ,SIGNAL(sendClose()),this,SLOT(recvClose()));
            inputCode_AQ->show();
        }
        else
        {
            inputCode = new InputCode(this);
            connect(inputCode,SIGNAL(sendCode(QString)),this,SLOT(recvCode(QString)));
            connect(inputCode,SIGNAL(sendClose()),this,SLOT(recvClose()));
            inputCode->show();
        }
    }
    else {
        DTdebug()  << SYSS << CsIsConnect ;
    }
}

void MainWindow::recvClose()
{
    delete e3;
    ui->label_black->hide();
    if(Factory == "AQCHERY")
    {
        delete inputCode_AQ;
    }
    else
        delete inputCode;
}

void MainWindow::recvCode(QString code)
{
    recvClose();
//    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
//    if(code.size() == 32){
//        mPartCode = 0;
//    }else if(code.size() == 15){
//        mPartCode = 1;
//    }
//    configIniRead->setValue("baseinfo/PartCode",mPartCode);
//    configIniRead->deleteLater();

    bool isEqual = false;
    if(seriallist.isEmpty())
    {
        seriallist.push_back(code);
        whichindex++;
    }
    else
    {
        QList<QString>::iterator i;
        for (i = seriallist.begin(); i != seriallist.end(); ++i)
        {
            if(!(QString::compare(code, *i, Qt::CaseSensitive)))
            {
                isEqual = true;
                break;
            }
        }
        if(!isEqual)
        {
            if(seriallist.size() == 10)
            {
                seriallist[whichindex] = code;
            }
            else
            {
                seriallist.push_back(code);
            }
            whichindex++;
            if(whichindex == 10)
                whichindex = 0;
        }
    }

    if(Factory == "AQCHERY")
    {
        DTdebug()<<"SerialNumbs" <<code;
        getSerialNum(code,isEqual,"VIN+CarType");
    }
    else
        getSerialNum(code,isEqual,"BarCode");
}

void MainWindow::receiveDelete(QString IDCode)
{
    DeleteIDCode= IDCode;
    emit sendDeleteCar_VIN(IDCode);
}


void MainWindow::delete_car(bool deleteCar)
{
    if(deleteCar)
    {
        ui->label_black->hide();
        delete e3;
        delete save;
        SaveWhat = "";
        if(Factory == "Dongfeng")
        {
            SaveWhat = "delete_car";
            emit sendToInter(DeleteIDCode);
        }
    }
    else
    {
        ui->label_black->hide();
        delete e3;
        delete save;
        SaveWhat = "";
    }
    isSaveShow = false;
}

void MainWindow::showhome()
{
    if(Factory == "SVW3"|| Factory == "Ningbo")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_VW.bmp);");
    else if( Factory == "SVW2")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_SVW2.bmp);");
    else if(Factory == "BYDSZ" || Factory == "BYDXA")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_BYD.bmp);");
    else if(Factory == "Benz")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_Benz.bmp);");
    else if(Factory == "GM")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_GM.bmp);");
    else if(Factory == "Haima")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_Haima.bmp);");
    else if(Factory == "Dongfeng")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_Dongfeng.bmp);");
    else if(Factory == "BAIC" || Factory == "GZBAIC")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_BAIC.bmp);");
    else if(Factory == "NOVAT")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_NOVAT.bmp);");
    else if(Factory == "AQCHERY")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_Zhengyi.bmp);");
    else if(Factory == "KFCHERY")
        ui->label_pdmarea->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/DWIN_SET/home_KFCHERY.bmp);");
}

void MainWindow::send_Info()
{
    ShowTime();
    int OK;
    if(SYSS == "ING") //当前状态 ING or OK or NOK
        OK = 1;
    else if(SYSS == "OK")
        OK = 2;
    else if(SYSS == "NOK")
        OK = 3;
    else
        OK = 2;

    emit sendInfo(linkCount&0x01,linkCount&0x02,TimeLeft,OK);
    emit sendError(linkCount&0x01,linkCount&0x02,isRFIDConnected);
    if(preSystemStatus != systemStatus)
    {
        preSystemStatus = systemStatus;
        DTdebug() << "*********systemStatus**********"<<systemStatus << preSystemStatus;
        switch(systemStatus)
        {
        case 1:
            ui->label_tiaomastate->setText(tr("拧紧控制器断开连接"));
            break;
        case 2:
            ui->label_tiaomastate->setText(tr("PLC断开连接"));
            break;
        case 3:
            ui->label_tiaomastate->setText(tr("PLC read error"));
            break;
        case 4:
            ui->label_tiaomastate->setText(tr("RFID断开连接"));
            break;
        case 5:
            ui->label_tiaomastate->setText(tr("条码队列错乱请确认"));
            break;
        case 9:
            ui->label_tiaomastate->setText(tr("条码重复"));
            break;
        case 10:
            ui->label_tiaomastate->setText(tr("条码校验失败"));
            break;
        case 11:
            ui->label_tiaomastate->setText(tr("选配匹配失败"));
            break;
        case 12:
            ui->label_tiaomastate->setText(tr("本地无此车信息"));
            break;
        case 13:
            ui->label_tiaomastate->setText(tr("特征码匹配失败"));
            break;
        case 14:
            ui->label_tiaomastate->setText(tr("螺栓参数设置异常"));
            break;
        case 15:
            ui->label_tiaomastate->setText(tr("请正确选择套筒"));
            break;
        case 16:
            ui->label_tiaomastate->setText(tr("VIN码匹配失败"));
            break;
        case 17:
            ui->label_tiaomastate->setText(tr("程序号或通道匹配失败"));
            break;
        case 18:
            ui->label_tiaomastate->setText(tr("获取Job信息失败"));
            break;
        case 19:
            ui->label_tiaomastate->setText(tr("不存在条码信息"));
            break;
        case 20:
            ui->label_tiaomastate->setText(tr("条码不属于本站"));
            break;
        case 21:
            ui->label_tiaomastate->setText(tr("此辆车不操作"));
            //            ui->label_tiaomastate->setText(tr("Job匹配失败"));
            break;
        case 22:
            ui->label_tiaomastate->setText(tr("拧紧枪不在工作区域"));
            //            ui->label_tiaomastate->setText(tr("Job匹配失败"));
            break;
        case 23:
            ui->label_tiaomastate->setText(tr("条码为空"));
            break;
        case 24:
            ui->label_tiaomastate->setText(tr("数据超出监控范围"));
            break;
        case 25:
            ui->label_tiaomastate->setText(tr("VIN码未发送成功"));
            break;
        default:
            ui->label_tiaomastate->setText("");
            break;
        }
    }
    //    //add lw test 2017/10/30
    //    int Control_power2 = 0;
    //    system("cat /sys/class/leds/control_power2/brightness") >> Control_power2;//读取
    //    DTdebug()<<"IO control_power2 value:"<< Control_power2;
}

void MainWindow::on_pushButton_tiaoma_clicked()
{
    if(Factory == "BYDSZ")
    {
        if(!controlMode)
        {
            DTdebug()<<"change manual";
            controlMode = true;
            if(SYSS == "ING")
            {
                Stop();
                ISmaintenance=false;
            }
            emitOperate1(false,99); //manual mode
            cleanDisplay();
            PDM_PATH="";
            ui->label_pdmarea->setStyleSheet("font: 100pt \"黑体\";color: rgb(0, 0, 0);");
            ui->label_pdmarea->setText(tr("手动"));
            ui->pushButton_tiaoma->setText(tr("手动"));
            ui->label_17->setText(tr("当前手动模式..."));
        }
        else
        {
            emitOperate1(false,1); //auto mode
            DTdebug()<<"change automatic";
            ui->label_pdmarea->setText("");
            ui->pushButton_tiaoma->setText(tr("队列"));
            ui->label_17->setText(tr("等待车辆进站..."));
            emit sLightLogic(e_WaitForScan);
            SYSS = "Ready";
            displayStatusIcon("Ready");
            showhome();
            controlMode = false;
            FisTimer.start(5000);
        }
    }
    else if(Factory == "Benz")
    {
        if(!controlMode)         //匹配特征码2
        {
            DTdebug()<<"change manual";
            controlMode = true;
            ui->pushButton_tiaoma->setText(tr("手动"));
        }
        else                     //匹配特征码1 :Job
        {
            DTdebug()<<"change automatic";
            controlMode = false;
            if(isRFID)
            {
                ui->pushButton_tiaoma->setText("RFID");
            }
            else if(isBarCode)
            {
                ui->pushButton_tiaoma->setText(tr("条码枪"));
            }
        }
    }
    else if(Factory == "Dongfeng")
    {
        if(!manualMode)         //匹配特征码2
        {
            DTdebug()<<"change manual";
            manualMode = true;
            ui->pushButton_tiaoma->setText(tr("手动"));
        }
        else                     //匹配特征码1 :Job
        {
            DTdebug()<<"change automatic";
            manualMode = false;
            ui->pushButton_tiaoma->setText(tr("自动"));
        }
    }
}

void MainWindow::initButton()
{
    FUNC() ;
    ui->pushButton_16->hide();
    ui->widget_2->setGeometry(0,0,1366,170);
    //    ui->progressBar->raise();
    //    ui->label_17->raise();
    ui->progressBar->setGeometry(10,10,1346,151);
    ui->label_17->setGeometry(10,10,1346,151);
    ui->label->setText("Ch0.1 P:");
    ui->label_2->setText("Ch0.2 P:");
    ui->label_3->setText("Ch0.3 P:");
    //ui->label_6->setText("Ch0.4 P:");
#if 1
    for(int i = 0 ;i<CHANNEL_AMOUNT ;i++)
    {
        widget_channel[i] = new QWidget(ui->page_6);
        widget_channel[i]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255);border-right-width:0.5px;border-top-width:0.5px;");
        widget_channel[i]->setGeometry(1168/4*i,0,292,600);
    }
    widget_channel[CHANNEL_AMOUNT-1]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255);border-top-width:0.5px;");

//    for(int i = 0 ;i<CHANNEL_AMOUNT ;i++)
//    {
//        QLabel *label_channel[5] = new QLabel(widget_channel[0]);
//        label_channel[i]->setStyleSheet("color: rgb(51, 153, 255);border-width:0px;font: 20pt \"黑体\";");
//        label_channel[i]->setText(QString("CH 0.%1").arg(QString::number(i+1)));
//        label_channel[i]->setGeometry(150,3,151,31);
//    }
#else
    widget_channel[0] = new QWidget(ui->page_6);
    widget_channel[1] = new QWidget(ui->page_6);
    widget_channel[2] = new QWidget(ui->page_6);
    widget_channel[0]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255);border-right-width:0.5px;border-top-width:0.5px;");
    widget_channel[1]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255);border-right-width:0.5px;border-top-width:0.5px;");
    widget_channel[2]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255);border-top-width:0.5px;");
    widget_channel[0]->setGeometry(0,0,389,600);
    widget_channel[1]->setGeometry(389,0,389,600);
    widget_channel[2]->setGeometry(778,0,388,600);
#endif
    //more channel
    QLabel *label_channel_1 = new QLabel(widget_channel[0]);
    QLabel *label_channel_2 = new QLabel(widget_channel[1]);
    QLabel *label_channel_3 = new QLabel(widget_channel[2]);
    QLabel *label_channel_4 = new QLabel(widget_channel[3]);
    label_channel_1->setStyleSheet("color: rgb(51, 153, 255);border-width:0px;font: 20pt \"黑体\";");
    label_channel_2->setStyleSheet("color: rgb(51, 153, 255);border-width:0px;font: 20pt \"黑体\";");
    label_channel_3->setStyleSheet("color: rgb(51, 153, 255);border-width:0px;font: 20pt \"黑体\";");
    label_channel_4->setStyleSheet("color: rgb(51, 153, 255);border-width:0px;font: 20pt \"黑体\";");
    label_channel_1->setText(tr("CH 0.1"));
    label_channel_2->setText(tr("CH 0.2"));
    label_channel_3->setText(tr("CH 0.3"));
    label_channel_4->setText(tr("CH 0.4"));
    label_channel_1->setGeometry(150,3,151,31);
    label_channel_2->setGeometry(150,3,151,31);
    label_channel_3->setGeometry(150,3,151,31);
    label_channel_4->setGeometry(150,3,151,31);

    for(int i=0;i<CHANNEL_AMOUNT;i++)    //机械手多通道
    {
        for(int j = 0;j<10;j++)
        {
            pushButton_channel[i][j] = new QPushButton(widget_channel[i]);
            label1_channel[i][j] = new QLabel(widget_channel[i]);
            label2_channel[i][j] = new QLabel(widget_channel[i]);
            pushButton_channel[i][j]->raise();
            pushButton_channel[i][j]->setFlat(true);
            pushButton_channel[i][j]->setFocusPolicy(Qt::NoFocus);
            label1_channel[i][j]->setFocusPolicy(Qt::NoFocus);
            label2_channel[i][j]->setFocusPolicy(Qt::NoFocus);
            pushButton_channel[i][j]->setAttribute(Qt::WA_TransparentForMouseEvents);

            pushButton_channel[i][j]->setText(QString::number(1));
            pushButton_channel[i][j]->setGeometry(30,35+56*j,40,40);
            label1_channel[i][j]->setGeometry(80,34+56*j,120,46);
            label2_channel[i][j]->setGeometry(200,34+56*j,150,46);
            label1_channel[i][j]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255); border-width:0px;font: 14pt \"黑体\";");
            label2_channel[i][j]->setStyleSheet("border-style:solid;border-color: rgb(51, 153, 255); border-width:0px;font: 14pt \"黑体\";");
            pushButton_channel[i][j]->hide();
            label1_channel[i][j]->hide();
            label2_channel[i][j]->hide();
        }
    }
}

//机械手三轴拧紧
void MainWindow::fromFtpJsonData(QString MI, QString WI, QString STATE, int Channel, int Screwid, int Program)
{
    FUNC();
    if(Channel==1)
        ui->label_pronum->setText(QString::number(Program));
    else if(Channel==2)
        ui->label_lsnum->setText(QString::number(Program));
    else if(Channel==3)
        ui->label_hege->setText(QString::number(Program));
//    else if(Channel==4)
//        ui->label_hege_2->setText(QString::number(Program));

    if(Screwid != Channel_Screwid[Channel-1])
    {
        Channel_num[Channel-1]++;
        Channel_time[Channel-1]=1;
    }
    else
    {
        Channel_time[Channel-1]++;
    }

    int num = Channel_num[Channel-1];

    pushButton_channel[Channel-1][num]->setText(QString::number(Channel_time[Channel-1]));
    if(STATE=="OK")
    {
        pushButton_channel[Channel-1][num]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 18pt \"黑体\";color: rgb(248, 248, 255);");
        //red_led(0);
        //green_led(1);
    }
    else if(STATE=="NOK")
    {
        pushButton_channel[Channel-1][num]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 18pt \"黑体\";color: rgb(248, 248, 255);");
        //red_led(1);
        //green_led(0);
    }
    label1_channel[Channel-1][num]->setText(QString(tr("T:").append(MI).append("Nm")));
    label2_channel[Channel-1][num]->setText(QString(tr("A:").append(WI).append("°")));
    pushButton_channel[Channel-1][num]->show();
    label1_channel[Channel-1][num]->show();
    label2_channel[Channel-1][num]->show();
    Channel_Screwid[Channel-1] = Screwid;
    //    ui->label_tiaomastate->setText("");
    DTdebug() << "xianshi" ;
    systemStatus = 0;//clear error status
}

void MainWindow::VIN_coming(QString VIN)
{
    FUNC() ;

    //next car com ,reset led
    //green_led(1);
    //red_led(0);

    BoltOrder[0] = 1;
    BoltOrder[1] = 1;
    ui->label_17->setText(VIN);
    ui->stackedWidget_2->setCurrentIndex(1);
    for(int i=0;i<4;i++)
    {
        Channel_num[i]=-1;
        Channel_Screwid[i]=-1;
        Channel_time[i]=1;
        for(int j = 0;j<10;j++)
        {
            pushButton_channel[i][j]->hide();
            label1_channel[i][j]->hide();
            label2_channel[i][j]->hide();
        }
    }
    SetLsInfo("", "", "");
    ui->label_hege->setText("");
    //    ui->label_tiaomastate->setText("");
    systemStatus = 0;//clear error status
}

void MainWindow::Wrong_Match_VIN(QString tmp1,QString tmp2)
{

    if(tmp1 == "VIN")
        systemStatus = 16;//VIN码匹配失败
    else if(tmp1 == "NULL")
        systemStatus = 23;//条码为空
    else if(tmp1 == "99"&& tmp2=="1")
        ui->label_pronum->setText("99");
    else if(tmp1 == "99"&& tmp2=="2")
        ui->label_lsnum->setText("99");
    else if(tmp1 == "99"&& tmp2=="3")
        ui->label_hege->setText("99");
    else
    {
        if(tmp1=="1")
            ui->label_pronum->setText(tmp2);
        else if(tmp1=="2")
            ui->label_lsnum->setText(tmp2);
        else if(tmp1=="3")
            ui->label_hege->setText(tmp2);
        //        ui->label_tiaomastate->setText("Ch0."+tmp1+tr(" 匹配失败"));
        systemStatus = 17;
    }
}
/****************************************/
//gwk display limit
/****************************************/
void MainWindow::displayLimit(QString programNO, QString upLimit, QString downLimit)
{
    ui->label_proNO->setText(programNO);
    ui->label_UpLimitValue->setText(upLimit+" Nm");
    ui->label_DownLimitValue->setText(downLimit+" Nm ");
}

void MainWindow::SB356Connect(bool sb356connect)
{
    if(ControlType_1 =="SB356" && Factory != "BAIC")
    {
        if(sb356connect)
        {
            systemStatus = 0;
            ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
            CsIsConnect = true;
            emit sendControllerStatus(true);
            //yellow_led(0);
            //white_led(1);
        }
        else
        {
            systemStatus = 1;
            ui->label_ss1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
            CsIsConnect = false;
            emit sendControllerStatus(false);
            //yellow_led(1);
            //white_led(0);
        }
    }
    else if(ControlType_1 =="SB356_PLC" ||(ControlType_1 =="SB356" && Factory =="BAIC"))
    {
        if(sb356connect)
        {
            if(SYSS != "ING" && SYSS != "OK" && SYSS != "NOK")
            {
                SYSS = "Ready";
            }
            TightenIsReady(true);
            ui->label_ss11->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/68.bmp);");
        }
        else
        {
            TightenIsReady(false);
            ui->label_ss11->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/69.bmp);");
        }
    }
}

void MainWindow::on_pushButton_18_clicked()
{
    this->close();
}

void MainWindow::on_pushButton_11_clicked()
{
    this->close();
}

void MainWindow::receiveJobError(int Error)
{
    DTdebug()<<"receive JobError "<<Error;
    if(Error == 1)
        systemStatus=19;
    else if(Error ==2)
        systemStatus=20;
    else if(Error == 3)
        systemStatus=18;
    //    else if(Error == 0)
    //        systemStatus=22;
    else
        systemStatus=18;
}

void MainWindow::JobTimeout()
{
    FUNC() ;
    if(JobTimer.isActive())
        JobTimer.stop();
    DTdebug()<<"Job time out";
    systemStatus = 18;
}

void MainWindow::on_pushButton_align_clicked()
{
    if(!controlMode)
    {
        if(!isSaveShow)
        {
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_black->setGraphicsEffect(e3);
            ui->label_black->show();
            ui->label_black->setGeometry(0,0,1366,768);
            isSaveShow = true;
            SaveWhat = "Align";
            //        if(SerialGunMode)
            //        {
            //            system("echo 0 > /sys/class/leds/control_uart2/brightness &");
            //        }
            save = new Save(this);
            connect(save,SIGNAL(sendClose()),this,SLOT(closeSave()));
            connect(save,SIGNAL(QueueAlign(QString,bool,QString)),this,SLOT(getSerialNum(QString,bool,QString)));
            save->show();
        }
    }
}

void MainWindow::receiveQueueError(int ErrorCode)
{
    if(!isSaveShow)
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);
        isSaveShow = true;
        SaveWhat = "QueueError_"+QString::number(ErrorCode);
        save = new Save(this);
        connect(save,SIGNAL(sendClose()),this,SLOT(closeSave()));
        save->show();
    }
}

void MainWindow::startLine()
{
    /******************dongfeng*******************/
    RFIDlock.lockForWrite();
    if(rfidNextCom)
    {
        risingEdge();
        DTdebug()<<"dongxian";
        //RFID模式 ING 状态确认
        rfidNextCom = false;
        RFIDlock.unlock();
        resetUiDo();
        QString pintmpp,G9tmpp;
        if(Factory == "Dongfeng")
        {
            if(VIN_PIN_SQL_RFID != "-1-1" && VIN_PIN_SQL_RFID != "00")
            {
                pintmpp = VIN_PIN_SQL_RFID.mid(0,17);
                G9tmpp = VIN_PIN_SQL_RFID.mid(17,17);
            }
            else if(VIN_PIN_SQL_RFID == "-1-1")
            {
                pintmpp ="-1";
                G9tmpp = "-1";
            }
            else if(VIN_PIN_SQL_RFID == "00")
            {
                pintmpp ="0";
                G9tmpp = "0";
            }
        }
        DTdebug()<<"rfdiNextCom reset";
        getSerialNum(pintmpp,false,G9tmpp);
    }
    else
    {
        RFIDlock.unlock();
    }
    /************************************************/
}

void MainWindow::risingEdge()
{
    //system("echo 1 > /sys/class/leds/control_led5/brightness &");
    //system("echo 0 > /sys/class/leds/control_led5/brightness &");
    //    QTimer::singleShot(1000,this,SLOT(fallingEdge()));
}

void MainWindow::fallingEdge()
{
    //system("echo 0 > /sys/class/leds/control_led5/brightness &");
}

//条码枪断电
void MainWindow::gunPower(bool isOn)
{
    if(isOn)
    {

    }
    else
    {
        if(SerialGunMode)
        {
            if(Factory == "Haima"||(Factory == "SVW2" && ControlType_1 == "GWK"))
                system("echo 0 > /sys/class/leds/control_uart1/brightness &");
            else
                system("echo 0 > /sys/class/leds/control_uart2/brightness &");
        }
    }
}


void MainWindow::displayStatusIcon(QString status)
{
    if(status == "NOK")
    {
        ui->pushButton_16->setText("NOK");
        ui->pushButton_16->setStyleSheet("font: 40pt ; background-color: rgb(255, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
        emit sLightLogic(e_GroupFail);
        emit sLightLogic(e_NotING);
        //emit signalSendHttp(E_SendSetPassby);
    }
    else if(status == "OK")
    {
        ui->pushButton_16->setText("OK");
        ui->pushButton_16->setStyleSheet("font: 60pt ; background-color: rgb(25, 125, 44); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
        emit sLightLogic(e_NotING);
        emit sLightLogic(e_GroupOK);
        emit signalSendHttp(E_SendEndTightening);
    }
    else if(status == "ING")
    {
        ui->pushButton_16->setText("ING");
        ui->pushButton_16->setStyleSheet("font: 50pt ; background-color: rgb(250, 225, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
        emit sLightLogic(e_ING);
    }
    else if(status == "Ready")
    {
        ui->pushButton_16->setText("Ready");
        ui->pushButton_16->setStyleSheet("font: 30pt ; background-color: rgb(51, 153, 255); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
        emit sLightLogic(e_NotING);
    }
    else
    {
        ui->pushButton_16->setText("Not Ready");
        ui->pushButton_16->setStyleSheet("font: 19pt ; background-color: rgb(250, 0, 0); color: rgb(248, 248, 255); border-width:0px;   border-style:solid; border-color: rgb(51, 153, 255);  ");
        emit sLightLogic(e_NotING);
    }
}

void MainWindow::JobOffShow()
{
    ui->pushButton_JobOff->show();
}

void MainWindow::on_pushButton_JobOff_clicked()
{
    emitOperate1(false,97);//job off
}

/*****************无线定位使能切换********************************/
void MainWindow::wirelessPositionToEnable(bool toolEnable)  //切换使能
{
    FUNC();
    if(SYSS == "ING")
    {
        if(toolEnable)
        {
            if(!changeTemp)
            {
                changeTemp = true;
                systemStatus = 0;
                emitOperate1(true,0);
                DTdebug()<<"++++Able wireless tool!";
            }
        }
        else//拧紧枪不在工作范围
        {
            if(changeTemp)//条码扫入发送使能时标志位置1
            {
                changeTemp = false;
                systemStatus = 22;
                emitOperate1(false,0);
                DTdebug()<<"++++Disable wireless tool!";
            }
        }
    }
}

/*******************************************************
 * new add
 *__________
< 无线定位显示界面 >  out in error off
----------
       \   ^__^
        \  (oo)\_______
           (__)\       )\/\
               ||----w |
               ||     ||
 ********************************************************
 */

void MainWindow::revTagStatus(bool isInside,int tagPower,QPointF a,QPointF b,QPointF c,QPointF d,QPointF P,bool Timeout)  //接收到标签信息
{
    //     DTdebug()<<"**********"<<TagTimeout<<Timeout;  //Timeout =true, dis,  =false ,hide
//    if(TagTimeout!=Timeout)
//    {
//          DTdebug()<<"revTagStatus TagTimeout,display red!";
//        TagTimeout = Timeout;
//        lable_cor_battey_display(!Timeout);//true ->hide  false ->display
//        ui->pushButton_location->setText("error");
//    }
    if(Timeout == false)//标签断开连接超时
    {
         //if(TagTimeout!=Timeout)//只执行一次  false false 没法执行
          if(first_timeout_flag)
         {
            ui->label_tagPower->hide();
            ui->pushButton_location->setText("error");
            ui->pushButton_location->setStyleSheet("color:red;");
            DTdebug()<<"revTagStatus TagTimeout,display red!";
            TagTimeout = Timeout;
            first_timeout_flag = false;
         }
    }
    else
    {
        if(TagTimeout!=Timeout)
        {
            TagTimeout = Timeout;
            first_timeout_flag = true;
            lable_cor_battey_display(false);
        }

        if(locationStatus != 0)
        {
            DTdebug()<<"revTagStatus"<<isInside<<tagPower;

           // wirelessPositionToEnable(isInside);
            wirelessPositionToEnable(true);//测试数据
            if((tagPower&0xFF)<0x1C)//add lw 2017/8/21 0x0C 0x0D 0x0E 0x0F ?
            {
                if(tagPower&0x10)
                {
                    if(tagPower<4)
                        lablebatteryshow(12);
                    else
                        lablebatteryshow(13);
                }
                else
                {
                    lablebatteryshow(tagPower);
                }
            }

            emit Send_tagcoor(P);
            //ui->pushButton_location->setText(QString("(%1,%2)").arg(P.x()).arg(P.y()));
            if(isInside)
            {
                ui->pushButton_location->setText("in");
                //ui->pushButton_location->setStyleSheet("color: rgb(51, 153, 255);");
                ui->pushButton_location->setStyleSheet("color:green;");
            }
            else
            {
                ui->pushButton_location->setText("out");
                ui->pushButton_location->setStyleSheet("color:red;");
            }
            POINT_A = a;
            POINT_B = b;
            POINT_C = c;
            POINT_D = d;
        }
    }
}

//add lw 2017/9/6 Display realtime location cor
void MainWindow::on_pushButton_location_clicked()
{
    if(locationStatus != 0)
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);


        location_temp = new Location(this);
        location_temp->Paintrectangle(POINT_A,POINT_B,POINT_C,POINT_D);
        connect(location_temp,SIGNAL(closeDiglag()),this,SLOT(closelocation()));
        connect(this,SIGNAL(Send_tagcoor(QPointF)),location_temp,SLOT(showPlot(QPointF)));
        location_temp->move((this->width() - location_temp->width())/2, (this->height() - location_temp->height())/2);
        location_temp->show();
    }
}

//add lw 2017/9/6 close realtime location cor
void MainWindow::closelocation()
{
    delete e3;
    ui->label_black->hide();
    delete location_temp;
}

void MainWindow::lable_cor_battey_display(bool hide_disaplay)
{
    if(hide_disaplay == true)
    {
        ui->label_tagPower->hide();//add lw 2017/9/19
        ui->pushButton_location->setText("off");
        ui->pushButton_location->setStyleSheet("color: rgb(51, 153, 255);");
        ui->pushButton_location->hide();//
    }
    else
    {
        ui->label_tagPower->show();//add lw 2017/9/19
        ui->pushButton_location->show();
        ui->pushButton_location->setText("on");
        ui->pushButton_location->setStyleSheet("color: rgb(51, 153, 255);");
    }
}

void MainWindow::setFTPMaster()
{
    ui->label_11->show();
    ui->label_ss4->show();
    ui->label_11->setText(tr("从机"));
    setRfidState(false);
}

void MainWindow::recvNewCode(QString newCode)
{
    FUNC();
    codeList.append(newCode);
    if(codeList.size()>1)
        codeList.removeFirst();
}

void MainWindow::leuzeTimerStart()
{
    if(!leuzeTimer.isActive())
        leuzeTimer.start(3000);
}

void MainWindow::leuzeTimerDo()
{
    if(!codeList.isEmpty() && CsIsConnect && SYSS != "ING")
    {
        if(leuzeTimer.isActive())
            leuzeTimer.stop();
        QString nextCode = codeList.takeFirst();
        DTdebug() << "next code" << nextCode;
        getSerialNum(nextCode,false,"ready");
    }
    else {
        DTdebug()<< "code is empty ;csisconnect;syss"<<codeList.isEmpty() << CsIsConnect << SYSS ;
    }
}

void MainWindow::slotReplaceBarcode(QVariantMap replaceStatus)
{
    FUNC() ;
    DTdebug() <<replaceStatus ;
//    background:transparent;
    ui->labelReplaceStatus->setVisible(true);
    QString strStatus = replaceStatus.value("status").toString() ;
    ui->labelReplaceStatus->setText(replaceStatus.value("text").toString());

    if(strStatus == "replaceSuccess")
        ui->labelReplaceStatus->setStyleSheet("font: 60pt \"黑体\";color:green;background-color:white;");

    else if(strStatus == "replaceFail")
        ui->labelReplaceStatus->setStyleSheet("font: 60pt \"黑体\";color:red;background-color:white;");

    else if(strStatus == "replaceCodeNull")
        ui->labelReplaceStatus->setStyleSheet("font: 60pt \"黑体\";color:red;background-color:white;");

    QTimer::singleShot(10000, this, SLOT(slotHideLabelStatus()));
}

void MainWindow::slotHideLabelStatus()
{
    ui->labelReplaceStatus->setText("");
    ui->labelReplaceStatus->hide();
}

void MainWindow::slotHideLabelQRVerify()
{
    FUNC() ;
    if(ui->labelQRVerify->isVisible())
    {
        ui->labelQRVerify->setVisible(false);
    }
}

void MainWindow::slotShowBolt(QByteArray strVin,QString strTorque ,QString strAngle,QString strStatus,int bolt)
{
    FUNC() ;
    currBolt = bolt ;
    boltStatus = strStatus ;
    if(firstUse)
    {
        firstUse = false ;
    }
    DTdebug() << "VIN"<<strVin << "torque"<<strTorque << "angle" << strAngle << "status" <<strStatus << bolt ;
    ui->stackedWidget_2->show();
    ui->label_17->setText(strVin);

    if(bolt==1&&whichar==0)  //新车进站
    {
        cleanDisplay();

        ui->label_pdmarea->setStyleSheet("border-image: url(/PDM/white.png)");
    }

    if(whichar == 0)
    {
        whichCarType = 0 ;
        DTdebug() << "do not choose car type" ;
        ui->label_17->setText("");

        if(bolt == 1&&firstBolt)
        {
            cleanDisplay();
            ui->label_pdmarea->setStyleSheet("border-image: url(/PDM/white.png)");
            firstBolt=false;
            //没有选择车型，数据在同一个位置显示，只需new一组控件
            butt[0][0] = new QPushButton(ui->stackedWidget_2);
            label1[0][0] = new QLabel(ui->stackedWidget_2);
            label2[0][0] = new QLabel(ui->stackedWidget_2);
        }


        if(strStatus=="OK")
        {
            butt[0][0]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 20pt \"黑体\";color: rgb(248, 248, 255);");
        }
        else if(strStatus=="NOK")
        {
            butt[0][0]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 20pt \"黑体\";color: rgb(248, 248, 255);");
        }

        ui->label_pronum->setText(QString::number(bolt));


        butt[0][0]->raise();
        butt[0][0]->setFlat(true);
        label1[0][0]->setAlignment(Qt::AlignLeft);
        label2[0][0]->setAlignment(Qt::AlignLeft);
        butt[0][0]->setFocusPolicy(Qt::NoFocus);
        label1[0][0]->setFocusPolicy(Qt::NoFocus);
        label2[0][0]->setFocusPolicy(Qt::NoFocus);
        butt[0][0]->setAttribute(Qt::WA_TransparentForMouseEvents);
        label1[0][0]->setAttribute(Qt::WA_TransparentForMouseEvents);
        label2[0][0]->setAttribute(Qt::WA_TransparentForMouseEvents);


        label1[0][0]->setText(QString(tr("T:").append(strTorque).append("Nm")));
        label2[0][0]->setText(QString(tr("A:").append(strAngle).append("°")));
        butt[0][0]->setGeometry(10,10,46,46);
        label1[0][0]->setGeometry(10+50,20,140,23);
        label2[0][0]->setGeometry(10+200,20,140,23);
        label1[0][0]->setStyleSheet("background:transparent;");
        label2[0][0]->setStyleSheet("background:transparent;");
        butt[0][0]->show();
        label1[0][0]->show();
        label2[0][0]->show();
        butt[0][0]->setText(QString::number(bolt));
    }

    else
    {

        DTdebug() << "choose car is :"<<whichar ;
#if 0
        butt[0][bolt] = new QPushButton(ui->stackedWidget_2);
        label1[0][bolt] = new QLabel(ui->stackedWidget_2);
        label2[0][bolt] = new QLabel(ui->stackedWidget_2);
#endif

        if(strStatus=="OK")
        {
            butt[0][bolt-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 20pt \"黑体\";color: rgb(248, 248, 255);");
            if((strAngle.toInt() == -1) && (strTorque.toInt() == -1))
            {
                carNOK = true ;
                butt[0][bolt-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 20pt \"黑体\";color: rgb(248, 248, 255);");
            }
        }
        else if(strStatus=="NOK")
        {
            DTdebug()<<"nok";
            butt[0][bolt-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/r01.png);font: 20pt \"黑体\";color: rgb(248, 248, 255);");
        }

        label1[0][bolt-1]->setText(QString(tr("T:").append(strTorque).append("Nm")));
        label2[0][bolt-1]->setText(QString(tr("A:").append(strAngle).append("°")));

        butt[0][bolt-1]->setText(QString::number(bolt));
        whichpdmnumnow[0]++ ;

        if(bolt == boltAmount)  //最后一颗
        {

            if(strStatus == "OK")
            {
                if(carNOK)
                {
                    SYSS = "NOK" ;
                    displayStatusIcon("NOK");
                }
                else
                {
                    SYSS = "OK" ;
                    displayStatusIcon("OK");
                }

                VIN_PIN_SQL = "" ;
                currBolt = 0 ;
                carNOK = false ;
                whichCarType = whichar ;
                whichar = 0;
                ui->label_17->setText("");
                Stop();
            }
        }
    }
}

void MainWindow::cleanBAICDisplay()
{
    showhome();
    SetLsInfo("", "", "");
    ui->label_hege->setText("");
    if(timerpdm.isActive())
    {
        timerpdm.stop();
    }

    delete butt[0][0];
    delete label1[0][0];
    delete label2[0][0];


    pdmnowromisOk = true;
    BoltOrder[0] = 1;
    BoltOrder[1] = 1;
    SYSS = "Ready";
    lock.lockForWrite();
    StationStatus = 2;
    lock.unlock();
    if(CsIsConnect)
    {
        displayStatusIcon("Ready");
    }
    QualifiedNum = 0;
    TaoTongState = false;
    //white_led(0);
    //red_led(0);
    //green_led(0);
}

void MainWindow::slotLedTimerOut()
{
    if(ledStatus)
    {
        //red_led(1);
        //green_led(1);
        //white_led(1);
        //yellow_led(1);
        ledStatus = false ;
    }
    else
    {
        //red_led(0);
        //green_led(0);
        //white_led(0);
        //yellow_led(0);
        ledStatus = true ;
    }
}

void MainWindow::on_btnAdjustQueue_clicked()
{
    ui->labelError->setVisible(false);
    QString strVin = ui->editQueueVin->text();
    if(strVin.length()==17)
    {
        Q_EMIT signalGetCarInfo(1,strVin);
    }
    else {
        ui->labelError->setVisible(true);
        DTdebug() << strVin ;
    }
}

//跳过一个按钮
void MainWindow::on_btnSkipOne_clicked()
{
    FUNC() ;
    if(SYSS == "ING")
    {
        configwarning(true);
    }
    else
    {
        DTdebug() << SYSS ;
    }

}

//跳过全部按钮
void MainWindow::on_btnSkipAll_clicked()
{
    FUNC() ;
    if(SYSS == "ING")
    {
        configwarning(false);
    }
    else
    {
        DTdebug() << SYSS ;
    }
}

/*****************套筒选择信号对应使能发送********************************/
void MainWindow::taotong_main(int  which_t)
{
    FUNC();
    DTdebug() << carInfor[preChannel].ttNum[groupNumCh[preChannel]] ;
    if(preChannel!=-1)
    {
        DTdebug() << carInfor[preChannel].ttNum[groupNumCh[preChannel]];
        DTdebug() << which_t ;
        DTdebug() << sendEnableFlag ;
        DTdebug() << carInfor[preChannel].boltNum[groupNumCh[preChannel]];
        DTdebug() << socketSpannerFault ;
        DTdebug() << preChannel ;

        int tmpWhich = carInfor[preChannel].ttNum[groupNumCh[preChannel]];
        if(tmpWhich == which_t || which_t == 50)
        {
            if(!sendEnableFlag || which_t == 50)
            {
                ttChangeFlag = false;
                DTdebug() << "************taotong send enable *************";
                //                ui->label_tiaomastate->setText("");
                systemStatus = 0;

                DTdebug() << "taotong_main VIN_PIN_SQL" << VIN_PIN_SQL << serialNums;

                if(Factory == "NOVAT"||Factory == "AQCHERY"||Factory == "KFCHERY")
                {
                    VIN_PIN_SQL = serialNums.left(17);
                    serialNums = VIN_PIN_SQL;
                    DTdebug() << "VIN_PIN_SQL" << VIN_PIN_SQL;
                }
                else
                {
                    VIN_PIN_SQL = serialNums;
                    DTdebug() << "VIN_PIN_SQL" << VIN_PIN_SQL;
                }


                if(Factory=="BYDSZ" || Factory=="BYDXA")
                {

                    DTdebug() << "taotong_main BYDSZ VIN_PIN_SQL" << VIN_PIN_SQL;
                    if(query.exec("select AutoNO from "+tablePreview + " where VIN='"+VIN_PIN_SQL+"'"))
                    {
                        if(query.next())
                        {
                            AutoNO = query.value(0).toString();
                        }
                        else
                            DTdebug()<<"there is no VIN:"+VIN_PIN_SQL;
                    }
                    else
                        DTdebug()<<"select AutoNO fail"<<query.lastError();
                }
                if(carInfor[preChannel].boltNum[groupNumCh[preChannel]] /*&& socketSpannerFault*/)
                {
                    socketSpannerFault = false ;
                    sendEnableFlag = true;
                    if(allChannel==1)
                    {
                        ui->label_pronum->setText(carInfor[preChannel].proNo[groupNumCh[preChannel]]);
                        ui->label_lsnum->setText(QString::number(carInfor[preChannel].boltNum[groupNumCh[preChannel]]));
                    }
                    if(preChannel == 0) //enable
                    {
                        int tmpNum = groupNumCh[preChannel];
                        emitOperate1(true,tmpNum);
                    }
                    else if(preChannel == 1)
                    {
                        emitOperate2(true,groupNumCh[preChannel]);
                    }
                    else if(preChannel == 2)
                    {
                        emitOperate3(true,groupNumCh[preChannel]);
                    }
                    else
                    {
                        emitOperate4(true,groupNumCh[preChannel]);
                    }
                }
            }
        }
        else
        {
            systemStatus = 15;
            socketSpannerFault = true;
            TaoTongState = true;
            SetLsInfo(carInfor[preChannel].proNo[groupNumCh[preChannel]],
                    QString::number(carInfor[preChannel].boltNum[groupNumCh[preChannel]]),
                    QString::number(carInfor[preChannel].ttNum[groupNumCh[preChannel]]));
            if((sendEnableFlag)||(ttChangeFlag))
            {
                ttChangeFlag = false;
                DTdebug() << "************taotong send disable";
                if(carInfor[preChannel].boltNum[groupNumCh[preChannel]])
                {
                    socketSpannerFault = true ;
                    sendEnableFlag = false;
                    if(preChannel == 0) //reset
                    {
                        int tmpNum = groupNumCh[preChannel];
                        emitOperate1(false,tmpNum);
                    }
                    else if(preChannel == 1)
                    {
                        emitOperate2(false,groupNumCh[preChannel]);
                    }
                    else if(preChannel == 2)
                    {
                        emitOperate3(false,groupNumCh[preChannel]);
                    }
                    else
                    {
                        emitOperate4(false,groupNumCh[preChannel]);
                    }
                }
            }
        }
    }
}

void MainWindow::slot_getTightenID(QString id)
{
    tightenID = id;
}


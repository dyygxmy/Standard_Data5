#include "newconfiginfo.h"
#include "ui_newconfiginfo.h"
#include <QSettings>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QSqlError>
#include <QTableView>
#include <QSqlQuery>
#include <QProcess>
#include <QScrollBar>

void ModifySSID(QString pSSID, QString pPasswd)
{
#ifdef Q_OS_WIN32
    QFile tmpFile("D:/WXM/ap-hotspot.conf");
#else
    QFile tmpFile("/etc/ap-hotspot.conf");
#endif
    if(tmpFile.open(QFile::ReadOnly))
    {
        QString tmpContent = tmpFile.readAll();
        tmpFile.close();

        int index1 = 0;
        int index2 = 0;
        index1 = tmpContent.indexOf("ssid=");
        if(index1 > 0)
        {
            index2 = tmpContent.indexOf("\n", index1);
            if(index2 > 0)
            {
                tmpContent.replace(index1+5, (index2-index1-5), pSSID);
            }
        }
        index1 = tmpContent.indexOf("wpa_passphrase=");
        if(index1 > 0)
        {
            index2 = tmpContent.indexOf("\n", index1);
            if(index2 > 0)
            {
                tmpContent.replace(index1+15, (index2-index1-15), pPasswd);
            }
        }
        if(tmpFile.open(QFile::WriteOnly))
        {
            tmpFile.write(tmpContent.toLatin1());
            tmpFile.close();
        }
    }
}

//void ModifyDHCP(QString pIP1, QString pIP2)
//{
//    QString tmpRange = QString("%1,%2,12h").arg(pIP1).arg(pIP2);
//#ifdef Q_OS_WIN32
//    QFile tmpFile("D:/WXM/ap-hotspot");
//#else
//    QFile tmpFile("/usr/bin/ap-hotspot");
//#endif
//    if(tmpFile.open(QFile::ReadOnly))
//    {
//        QString tmpContent = tmpFile.readAll();
//        tmpFile.close();

//        int index1 = 0;
//        int index2 = 0;
//        index1 = tmpContent.indexOf("dhcp-range=");
//        if(index1 > 0)
//        {
//            index2 = tmpContent.indexOf("\n", index1);
//            if(index2 > 0)
//            {
//                tmpContent.replace(index1+11, (index2-index1-11), tmpRange);
//            }
//        }
//        if(tmpFile.open(QFile::WriteOnly))
//        {
//            tmpFile.write(tmpContent.toLatin1());
//            tmpFile.close();
//        }
//    }

//#ifdef Q_OS_WIN32
//    tmpFile.setFileName("D:/WXM/ap-hotspot.rules");
//#else
//    tmpFile.setFileName("/etc/dnsmasq.d/ap-hotspot.rules");
//#endif
//    if(tmpFile.open(QFile::ReadOnly))
//    {
//        QString tmpContent = tmpFile.readAll();
//        tmpFile.close();

//        int index1 = 0;
//        int index2 = 0;
//        index1 = tmpContent.indexOf("dhcp-range=");
//        if(index1 > 0)
//        {
//            index2 = tmpContent.indexOf("\n", index1);
//            if(index2 > 0)
//            {
//                tmpContent.replace(index1+11, (index2-index1-11), tmpRange);
//            }
//        }
//        if(tmpFile.open(QFile::WriteOnly))
//        {
//            tmpFile.write(tmpContent.toLatin1());
//            tmpFile.close();
//        }
//    }
//}

void StartWifi()
{
    system("wpa_cli disable_network 0 &");

    system("sudo ap-hotspot start &");
}

void StopWifi()
{
    system("sudo ap-hotspot stop &");

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    system((QString("wpa_cli set_network 0 ssid '\"")+configIniRead->value("baseinfo/SSID").toString()+QString("\"'")+" &").toLatin1().data());
    system((QString("wpa_cli set_network 0 psk  '\"")+configIniRead->value("baseinfo/psk").toString()+QString("\"'")+" &").toLatin1().data());
    system("wpa_cli disable_network 0 &");
    system("wpa_cli enable_network 0 &");
    system((QString("route add default gw ")+configIniRead->value("baseinfo/gateway").toString()+" wlan0 &").toLatin1().data());
    configIniRead->deleteLater();
}

void ReStartWifi()
{
    system("sudo ap-hotspot restart &");
}

Newconfiginfo::Newconfiginfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Newconfiginfo)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏

    this->setGeometry(QRect(0, 0, 1366, 768));

//    if(factory == "AQCHERY")
//    {
//        ui->label_114->setText("JobID");
//    }

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    int tmpWifiHotSpot = configIniRead->value("baseinfo/WIFIHotSpot").toInt();
    if(tmpWifiHotSpot)
    {
        ui->pushButton_restart_2->setChecked(true);
        ModifySSID(configIniRead->value("baseinfo/APSSID").toString(), configIniRead->value("baseinfo/APpsk").toString());
        StartWifi();
    }
    else
    {
        StopWifi();
    }

    Factory = factory;
    if((Factory == "Ningbo" && isRFID)|| Factory=="Benz" || (Factory == "SVW2" /*&& isRFID*/) ||Factory =="Dongfeng")
        ui->lineEdit_G9->setEnabled(true);
    else
    {
        ui->lineEdit_G9->setEnabled(false);
        ui->lineEdit_G9->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid; border-color:rgb(150, 150, 150)");
    }
    //    if(Factory == "Benz" || Factory == "GM" || Factory == "Haima")
    //    {
    ui->pushButton_38->hide();
    ui->label_94->setGeometry(525,246,130,29);
    ui->label_160->setVisible(false);
    ui->pushButton_50->setVisible(false); //Data5 do not need
    //    }
    area = new PaintArea(ui->tab_2);
    initRepairTable();
    pagenum = 1;
    optionpagenum = 1;
    optionIscheck = false;
    numpdm = 0;
    temppdm = 0;
    isbarcode = isBarCode;
    isrfid    = isRFID;
    Queuelock.lockForRead();
    isqueue   = isQueue;
    Queuelock.unlock();
    line_ID = Line_ID;
    partY1V = Y1V ;
    ui->label_100->hide();
    whichcar = 0;
    previouswhichar = whichcar;
    isedit = -1;
    pages = 0;
    thepage = 0;
    currentpages = 0;
    ifcansave = false;
    ispdminit = 0;
    nowpdmnum = 0;
    nowcomboxnum = 0;
    whichoption = 0;
    isxuanlook = true;
    isbaseinfochange = false;
    isadvancedchange = false;
    ismasterslavechange = false;
    islocationchange = false;
    isoptionsaved = true; // option is saved or not
    isSavedpdm = true;//是否 pdm正常保存
    bxuanwhich = 0;
    isoption  = false;
    PronumNow = 0;
    isboundchange = false;
    Selectsecond = false;
    Totalnum = 0;
    numRows =0;
    strUpInversion = "false" ;
    buttonbox[0] = ui->pushButton_butt1;
    buttonbox[1] = ui->pushButton_butt2;
    buttonbox[2] = ui->pushButton_butt3;
    buttonbox[3] = ui->pushButton_butt4;
    buttonbox[4] = ui->pushButton_butt5;

    editBox[0]   = ui->editQRCodeRule1 ;
    editBox[1]   = ui->editQRCodeRule2 ;
    editBox[2]   = ui->editQRCodeRule3 ;
    editBox[3]   = ui->editQRCodeRule4 ;
    editBox[4]   = ui->editQRCodeRule5 ;

    //    ui->label_taotongg->hide();
    //    ui->pushButton_taotong_add->hide();
    //    ui->pushButton_taotong_minus->hide();
    //    ui->lineEdit_taotong->hide();
    ui->lineEdit_taotong->setText("OFF");
    ui->lineEdit_channel->setText("1");
    ui->pushButton_10->setEnabled(false);
    ui->pushButton_11->setEnabled(false);
    ui->label_version->setText(Version);
    ui->label_version_2->setText(Version);

    controllerIp_01 = configIniRead->value("baseinfo/ControllerIp_1").toString();
    controllerIp_02 = configIniRead->value("baseinfo/ControllerIp_2").toString();
    //    PortA = configIniRead->value("baseinfo/PortA").toString();
    //    PortB = configIniRead->value("baseinfo/PortB").toString();
    RfidIp = configIniRead->value("baseinfo/RfidIp").toString();
    RfidPort = configIniRead->value("baseinfo/RfidPort").toString();
    DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    CurveServerIp = configIniRead->value("baseinfo/CurveServerIp").toString();
    AndonServerIp = configIniRead->value("baseinfo/AndonServerIp").toString();
    ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
    BarcodeGun = configIniRead->value("baseinfo/BarcodeGun").toString();
    QString tmpDeivceNo = configIniRead->value("baseinfo/DeviceNo").toString();
    QString strUpInversionCon = configIniRead->value("baseinfo/upInversion").toString();
    QString tmpIoBoxSerial = configIniRead->value("baseinfo/IoBoxSerial").toString();
    if(tmpIoBoxSerial.isEmpty())
       tmpIoBoxSerial =  ui->comboBox_IoBoxSerial->currentText();
    delete configIniRead;

    DTdebug() << "IoBoxSerial" << tmpIoBoxSerial;
    for(int i=0;i<ui->comboBox_IoBoxSerial->count();i++)
    {
        if(tmpIoBoxSerial == ui->comboBox_IoBoxSerial->itemText(i))
        {
            ui->comboBox_IoBoxSerial->setCurrentIndex(i);
            break;
        }
    }

    DTdebug() << "strUpInversionCon"<<strUpInversionCon;
    if(strUpInversionCon == "true")
    {
        ui->upInversioncheck->setChecked(true);
        upInversion = true;
        strUpInversion = "true" ;
    }
    else if(strUpInversionCon == "false")
    {
        ui->upInversioncheck->setChecked(false);
        upInversion = false;
        strUpInversion = "false" ;
    }
    DTdebug() << "strUpInversion"<<strUpInversion;

    QRegExp rx("[0-9.]{7,15}");
    QValidator *validator = new QRegExpValidator(rx, this );
    ui->lineEdit_tacktime->setValidator(validator);
    ui->lineEdit_offset->setValidator(validator);
    ui->lineEdit_torque_max->setValidator(validator);
    ui->lineEdit_torque_min->setValidator(validator);
    ui->lineEdit_angle_max->setValidator(validator);
    ui->lineEdit_angle_min->setValidator(validator);
    //    if(!isRFID)
    //        ui->lineEdit_G9->setEnabled(false);
    QRegExp rx1("[0-9]{1,9}");
    QValidator *validator1 = new QRegExpValidator(rx1, this );
    ui->year->setValidator(validator1);
    ui->month->setValidator(validator1);
    ui->date->setValidator(validator1);
    ui->hour->setValidator(validator1);
    ui->minute->setValidator(validator1);
    ui->second->setValidator(validator1);
    ui->lineEdit_pronum->setValidator(validator1);
    ui->lineEdit_number->setValidator(validator1);
    ui->lineEdit_iobox->setValidator(validator1);
    ui->lineEdit_xuanpronum->setValidator(validator1);
    ui->lineEdit_xuannumber->setValidator(validator1);
    ui->lineEdit_leastNum->setValidator(validator1);
    ui->lineEdit_tag_timeout->setValidator(validator1);//add lw 2017/9/19
    //    QRegExp rx21("[0-9,A-Z,-,a-z]{1,21}");
    //    QValidator *validator21 = new QRegExpValidator(rx21, this );

    ui->lineEdit_deviceNo->setText(tmpDeivceNo);

    ui->label_repair->hide();
    ui->pushButton_repair->hide();

    ui->label_wirelessLocation->hide();
    ui->pushButton_wirelessLocation->hide();

    ui->labelExecCarType->setVisible(false);
    ui->editExecCarType->setVisible(false);

    if(Factory =="Benz")
    {
        ui->label_107->setText(tr("Job:"));
        ui->label_114->setText(tr("Job:"));
        //        ui->lineEdit_Lsnumber->setValidator(validator21);
        //        ui->lineEdit_xuanLsnum->setValidator(validator21);
        ui->lineEdit_Lsnumber->setStyleSheet("font: 12pt \"黑体\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255);");
        ui->lineEdit_xuanLsnum->setStyleSheet("font: 12pt \"黑体\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255);");

        ui->label_Lsnum->setStyleSheet("font: 12pt \"Arial\";");
        ui->lineEdit_Lsnumber->setGeometry(122,109,201,35);
        ui->lineEdit_xuanLsnum->setGeometry(145,68,201,35);
        ui->label_44->setText(tr("线      别："));
        ui->label_46->setText(tr("站      别："));
        ui->pushButton_67->hide();
        ui->pushButton_86->hide();
        //        ui->pushButton_55->hide();
        ui->pushButton_5->setEnabled(false);
        ui->label_offset->hide();
        ui->lineEdit_offset->hide();
        ui->label_173->hide();
    }
    else if(Factory == "BYDXA")
    {
        ui->label_107->setText(tr("Job:"));
        ui->label_114->setText(tr("Job:"));
        ui->lineEdit_Lsnumber->setValidator(validator1);
        ui->lineEdit_xuanLsnum->setValidator(validator1);
        ui->label_98->hide();
        ui->pushButton_67->hide();
        ui->label_135->hide();
        ui->checkBox_output1->hide();
        ui->checkBox_output2->hide();
        ui->checkBox_output3->hide();
        ui->checkBox_output4->hide();
        ui->label_offset->hide();
        ui->lineEdit_offset->hide();
        ui->label_173->hide();
        ui->label_158->setText(tr("变量1："));
        ui->label_159->setText(tr("变量2："));
    }
    else if(Factory == "BYDSZ")
    {
        ui->lineEdit_Lsnumber->setValidator(validator1);
        ui->lineEdit_xuanLsnum->setValidator(validator1);
        ui->label_98->hide();
        ui->pushButton_67->hide();
        ui->label_135->hide();
        ui->checkBox_output1->hide();
        ui->checkBox_output2->hide();
        ui->checkBox_output3->hide();
        ui->checkBox_output4->hide();
        ui->label_158->setText(tr("变量1："));
        ui->label_159->setText(tr("变量2："));
    }
    else if(Factory == "Haima")
    {
        ui->lineEdit_Lsnumber->setValidator(validator1);
        ui->lineEdit_xuanLsnum->setValidator(validator1);
        ui->pushButton_67->hide();
        ui->pushButton_86->hide();
        ui->label_135->hide();
        ui->checkBox_output1->hide();
        ui->checkBox_output2->hide();
        ui->checkBox_output3->hide();
        ui->checkBox_output4->hide();
        ui->label_taotongg->hide();
        ui->pushButton_taotong_add->hide();
        ui->pushButton_taotong_minus->hide();
        ui->lineEdit_taotong->hide();
        ui->label_113->hide();
        ui->lineEdit_G9->hide();
        ui->label_106->hide();
        ui->label_G9->hide();
        ui->label_107->setText(tr("AVI特征码："));
        ui->label_114->setText(tr("AVI特征码："));
        ui->lineEdit_VIN->setFixedWidth(350);
        ui->label_offset->hide();
        ui->lineEdit_offset->hide();
        ui->label_173->hide();
        ui->label_repair->show();
        ui->pushButton_repair->show();
    }
    else if(Factory == "SVW2"  || Factory == "NOVAT" || Factory == "AQCHERY"||Factory == "KFCHERY")
    {
//        ui->lineEdit_Lsnumber->setValidator(validator1);
//        ui->lineEdit_xuanLsnum->setValidator(validator1);
        ui->pushButton_67->hide();
        ui->pushButton_86->hide();
        ui->label_135->hide();
        ui->checkBox_output1->hide();
        ui->checkBox_output2->hide();
        ui->checkBox_output3->hide();
        ui->checkBox_output4->hide();

        //        ui->label_113->hide();
        //        ui->lineEdit_G9->hide();
        //        ui->label_106->hide();
        //        ui->label_G9->hide();

        ui->label_offset->hide();
        ui->lineEdit_offset->hide();
        ui->label_173->hide();
        ui->pushButton_wirelessLocation->show();
        ui->pushButton_wirelessLocation->move(35,375);
        ui->label_wirelessLocation->move(16,382);
    }
    else if(Factory == "Dongfeng")
    {
        ui->lineEdit_Lsnumber->setValidator(validator1);
        ui->lineEdit_xuanLsnum->setValidator(validator1);
        ui->label_135->hide();
        ui->pushButton_67->hide();
        ui->checkBox_output1->hide();
        ui->checkBox_output2->hide();
        ui->checkBox_output3->hide();
        ui->checkBox_output4->hide();
        ui->label_158->setText(tr("变量1："));
        ui->label_159->setText(tr("变量2："));
        ui->label_offset->setText(tr("使能百分比"));
        ui->label_173->setText("%");
    }
    else
    {
        if(Factory != "SVW3" && Factory != "Ningbo")
            ui->pushButton_67->hide();
        ui->lineEdit_Lsnumber->setValidator(validator1);
        ui->lineEdit_xuanLsnum->setValidator(validator1);
        ui->label_135->hide();
        ui->checkBox_output1->hide();
        ui->checkBox_output2->hide();
        ui->checkBox_output3->hide();
        ui->checkBox_output4->hide();
        ui->label_offset->hide();
        ui->lineEdit_offset->hide();
        ui->label_173->hide();

        if(Factory == "BAIC" || Factory == "GZBAIC")
        {
            ui->label_158->setText(tr("变量1："));
            ui->label_159->setText(tr("变量2："));
        }
        if(Factory == "BAIC")
        {
            ui->labelExecCarType->setVisible(true);
            ui->editExecCarType->setVisible(true);
        }
    }

    if(Factory =="Haima")
        ui->label_model->setText(tr("Dat@2L"));

    QRegExp rx2("^((([1-9]{0,1}|2[0-4]|1\\d)\\d|25[0-5])(\\.|$)){4}");
    QValidator *validator2 = new QRegExpValidator(rx2, this);
    ui->lineEdit_localip->setValidator(validator2);
    ui->lineEdit_localip2->setValidator(validator2);
    ui->lineEdit_wirelessip->setValidator(validator2);
    ui->lineEdit_netmask->setValidator(validator2);
    ui->lineEdit_gateway->setValidator(validator2);
    ui->lineEdit_slave_1->setValidator(validator2);
    ui->lineEdit_slave_2->setValidator(validator2);
    ui->lineEdit_slave_3->setValidator(validator2);
    //    ipLineEdit->setValidator(validator2);
    //    ui->lineEdit_localip->setInputMask("000.000.000.000;");
    //    ui->lineEdit_wirelessip->setInputMask("000.000.000.000;");
    //    ui->lineEdit_netmask->setInputMask("000.000.000.000;");
    //    ui->lineEdit_gateway->setInputMask("000.000.000.000;");
    //    ui->lineEdit_slave_1->setInputMask("000.000.000.000;");
    //    ui->lineEdit_slave_2->setInputMask("000.000.000.000;");
    //    ui->lineEdit_slave_3->setInputMask("000.000.000.000;");

//    ui->lineEdit_psk->setEchoMode(QLineEdit::Password);
    ui->label_bx1name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx1code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx2name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx2code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx3name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx3code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx4name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx4code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx5name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_bx5code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx1name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx1code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx2name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx2code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx3name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx3code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx4name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx4code->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx5name->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_kx5code->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->pushButton_70->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_71->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_72->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_73->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_74->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_75->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_76->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_77->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_78->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->pushButton_79->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_143->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_142->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_167->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_144->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_171->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_172->setAttribute(Qt::WA_TransparentForMouseEvents);

    //默认零件规则为不可编辑状态
    ui->editQRCodeRule1->setEnabled(false);
    ui->editQRCodeRule2->setEnabled(false);
    ui->editQRCodeRule3->setEnabled(false);
    ui->editQRCodeRule4->setEnabled(false);
    ui->editQRCodeRule5->setEnabled(false);
    ui->editQRCodeRule1->hide();
    ui->editQRCodeRule2->hide();
    ui->editQRCodeRule3->hide();
    ui->editQRCodeRule4->hide();
    ui->editQRCodeRule5->hide();

    ui->stackedWidget_6->setCurrentIndex(0);

    bound_init();
    // pdminit();
    ui->pushButton_13->hide();
    ui->pushButton_91->hide();
    ui->label_78->hide();
    ui->pushButton_97->hide();
    ui->pushButton_100->hide();
    ui->label_101->hide();

    ui->label_M->setText("0");
    ui->label_N->setText("0");

    thepages = "";

    initui();
    pdminit();
    initAnchorTable();
    initDataConfigTable();

    LocationParseJson * locationParseJson = new LocationParseJson;
    currentLocation = locationParseJson->parseJson();
    delete locationParseJson;
    if(currentLocation.value("errorCode")!=1)
        readLocation(currentLocation);

    //    ui->stackedWidget_history->setCurrentIndex(0);

    //    image = QImage(ui->page_25,QImage::Format_RGB32);
    ////    image = QImage(600,300,QImage::Format_RGB32);  //画布的初始化大小设为600*500，使用32位颜色
    //    QColor backColor = qRgb(248,248,255);    //画布初始化背景色使用白色
    //    image.fill(backColor);//对画布进行填充
    //    QPainter painter(ui->page_25);
    //    painter.setRenderHint(QPainter::Antialiasing, true);//设置反锯齿模式，好看一点


    initLight();

    mDataBase = mysql_open(QString("Newconfiginfo"));
    if(!mDataBase.isOpen())
    {
        DTdebug()<< "mDataBase is not open";
        model = 0;
    }
    else
    {
        DTdebug()<< "mDataBase is open, create query";
        query = new QSqlQuery(mDataBase);
        query1 = new QSqlQuery(mDataBase);
        query_number = QSqlQuery(mDataBase);
        query_datas = QSqlQuery(mDataBase);
        query_bound = QSqlQuery(mDataBase);

        model = new QSqlQueryModel(this);
        model->setQuery("select IDCode,ScrewID,Program,Torque,Angle,TighteningTime,TighteningStatus,UploadMark,Cycle from "+Localtable+" WHERE RecordID <0", mDataBase);
        ui->tableView->setModel(model);
        ResetModel();
    }
}

Newconfiginfo::~Newconfiginfo()
{
    delete ui;
}

void Newconfiginfo::Show()
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->show();
}

void SetComboxValue(QComboBox* pCombox, QString pLightState)
{
    if(pCombox)
    {
        if(pLightState == "OFF")
        {
            pCombox->setCurrentIndex(0);
        }
        else if(pLightState == "ON")
        {
            pCombox->setCurrentIndex(2);
        }
        else if(pLightState == "BlinkOnce")
        {
            pCombox->setCurrentIndex(3);
        }
        else if(pLightState == "Blink")
        {
            pCombox->setCurrentIndex(4);
        }
        else
        {
            pCombox->setCurrentIndex(1);
        }
    }
}


QString GetLightState(QComboBox* pRedCombo,QComboBox* pGreenCombo,QComboBox* pYellowCombo,QComboBox* pWhiteCombo,QComboBox* pkeyCombo)
{
    QString TightenConnect_red = pRedCombo ->currentText();
    QString TightenConnect_green= pGreenCombo->currentText();
    QString TightenConnect_yellow= pYellowCombo->currentText();
    QString TightenConnect_white = pWhiteCombo->currentText();
    QString TightenConnect_key = pkeyCombo->currentText();

    QString TightenConnect= (TightenConnect_red.isEmpty() ? QString("") : (QString("red_")+TightenConnect_red +"#"))+
                            (TightenConnect_green.isEmpty() ? QString("") : (QString("green_")+TightenConnect_green+"#"))+
                            (TightenConnect_yellow.isEmpty() ? QString("") : (QString("yellow_")+TightenConnect_yellow+"#"))+
                            (TightenConnect_white.isEmpty() ? QString("") : (QString("white_")+TightenConnect_white+"#"))+
                            (TightenConnect_key.isEmpty() ? QString("") : (QString("key_")+TightenConnect_key));
   return TightenConnect;
}


void Newconfiginfo::InitLightState(QString pLogicState,QComboBox* pRedCombo,QComboBox* pGreenCombo,QComboBox* pYellowCombo,QComboBox* pWhiteCombo,QComboBox* pkeyCombo)
{
    pRedCombo->setCurrentIndex(1);
    pGreenCombo->setCurrentIndex(1);
    pYellowCombo->setCurrentIndex(1);
    pWhiteCombo->setCurrentIndex(1);
    pkeyCombo->setCurrentIndex(1);

    QStringList tmpStateList = pLogicState.split("#");
    foreach (QString value, tmpStateList)
    {
        QStringList subValueList= value.split("_");
        if(subValueList.length() == 2)
        {
            QString tmpLightColor = subValueList.at(0);
            QString tmpLightState = subValueList.at(1);
            if(tmpLightColor == "red")
            {
                SetComboxValue(pRedCombo, tmpLightState);
            }
            else if(tmpLightColor == "green")
            {
                SetComboxValue(pGreenCombo, tmpLightState);
            }
            else if(tmpLightColor == "yellow")
            {
                SetComboxValue(pYellowCombo, tmpLightState);
            }
            else if(tmpLightColor == "white")
            {
                SetComboxValue(pWhiteCombo, tmpLightState);
            }
            else  if(tmpLightColor == "key")
            {
                SetComboxValue(pkeyCombo, tmpLightState);
            }
        }
    }
}

void Newconfiginfo::initLight()
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString SingleOK = configIniRead->value("baseinfo/LightLogic_SingleOK").toString();
    InitLightState(SingleOK,ui->comboBox_SingleOK_red,ui->comboBox_SingleOK_green,ui->comboBox_SingleOK_yellow,ui->comboBox_SingleOK_white,ui->comboBox_SingleOK_lockled);
    QString SingleFail = configIniRead->value("baseinfo/LightLogic_SingleFail").toString();
    InitLightState(SingleFail,ui->comboBox_SingleFail_red,ui->comboBox_SingleFail_green,ui->comboBox_SingleFail_yellow,ui->comboBox_SingleFail_white,ui->comboBox_SingleFail_lockled);
    QString GroupOK = configIniRead->value("baseinfo/LightLogic_GroupOK").toString();
    InitLightState(GroupOK,ui->comboBox_GroupOK_red,ui->comboBox_GroupOK_green,ui->comboBox_GroupOK_yellow,ui->comboBox_GroupOK_white,ui->comboBox_GroupOK_lockled);
    QString GroupFail = configIniRead->value("baseinfo/LightLogic_GroupFail").toString();    
    InitLightState(GroupFail,ui->comboBox_GroupFail_red,ui->comboBox_GroupFail_green,ui->comboBox_GroupFail_yellow,ui->comboBox_GroupFail_white,ui->comboBox_GroupFail_lockled);

    QString ING = configIniRead->value("baseinfo/LightLogic_ING").toString();
    InitLightState(ING,ui->comboBox_ING_red,ui->comboBox_ING_green,ui->comboBox_ING_yellow,ui->comboBox_ING_white,ui->comboBox_ING_lockled);
    QString NotING = configIniRead->value("baseinfo/LightLogic_NotING").toString();
    InitLightState(NotING,ui->comboBox_NotING_red,ui->comboBox_NotING_green,ui->comboBox_NotING_yellow,ui->comboBox_NotING_white,ui->comboBox_NotING_lockled);

    QString Enable = configIniRead->value("baseinfo/LightLogic_Enable").toString();
    InitLightState(Enable,ui->comboBox_Enable_red,ui->comboBox_Enable_green,ui->comboBox_Enable_yellow,ui->comboBox_Enable_white,ui->comboBox_Enable_lockled);
    QString Disable = configIniRead->value("baseinfo/LightLogic_Disable").toString();
    InitLightState(Disable,ui->comboBox_Disable_red,ui->comboBox_Disable_green,ui->comboBox_Disable_yellow,ui->comboBox_Disable_white,ui->comboBox_Disable_lockled);
    QString WaitForScan = configIniRead->value("baseinfo/LightLogic_WaitForScan").toString();
    InitLightState(WaitForScan,ui->comboBox_WaitForScan_red,ui->comboBox_WaitForScan_green,ui->comboBox_WaitForScan_yellow,ui->comboBox_WaitForScan_white,ui->comboBox_WaitForScan_lockled);
    QString ScanFinishThenWait = configIniRead->value("baseinfo/LightLogic_ScanFinishThenWait").toString();
    InitLightState(ScanFinishThenWait,ui->comboBox_ScanFinishThenWait_red,ui->comboBox_ScanFinishThenWait_green,ui->comboBox_ScanFinishThenWait_yellow,ui->comboBox_ScanFinishThenWait_white,ui->comboBox_ScanFinishThenWait_lockled);
    QString TightenConnect = configIniRead->value("baseinfo/LightLogic_TightenConnect").toString();
    InitLightState(TightenConnect,ui->comboBox_TightenConnect_red,ui->comboBox_TightenConnect_green,ui->comboBox_TightenConnect_yellow,ui->comboBox_TightenConnect_white,ui->comboBox_TightenConnect_lockled);
    QString TightenDisConnect = configIniRead->value("baseinfo/LightLogic_TightenDisConnect").toString();
    InitLightState(TightenDisConnect,ui->comboBox_TightenDisConnect_red,ui->comboBox_TightenDisConnect_green,ui->comboBox_TightenDisConnect_yellow,ui->comboBox_TightenDisConnect_white,ui->comboBox_TightenDisConnect_lockled);
    QString NetWorkConnect = configIniRead->value("baseinfo/LightLogic_NetWorkConnect").toString();
    InitLightState(NetWorkConnect,ui->comboBox_NetWorkConnect_red,ui->comboBox_NetWorkConnect_green,ui->comboBox_NetWorkConnect_yellow,ui->comboBox_NetWorkConnect_white,ui->comboBox_NetWorkConnect_lockled);
    QString NetWorkDisConnect = configIniRead->value("baseinfo/LightLogic_NetWorkDisConnect").toString();
    InitLightState(NetWorkDisConnect,ui->comboBox_NetWorkDisConnect_red,ui->comboBox_NetWorkDisConnect_green,ui->comboBox_NetWorkDisConnect_yellow,ui->comboBox_NetWorkDisConnect_white,ui->comboBox_NetWorkDisConnect_lockled);
    QString SingleSkip = configIniRead->value("baseinfo/LightLogic_SingleSkip").toString();
    InitLightState(SingleSkip,ui->comboBox_SingleSkip_red,ui->comboBox_SingleSkip_green,ui->comboBox_SingleSkip_yellow,ui->comboBox_SingleSkip_white,ui->comboBox_SingleSkip_lockled);

    configIniRead->deleteLater();
}


void Newconfiginfo::clearCache()
{
    FUNC() ;
    ui->label_carname->clear();
    ui->label_iobox->clear();
    ui->label_G9->clear();
    ui->label_VIN->clear();
    ui->label_isxuan->clear();
    ui->label_Lsnum->clear();
    ui->label_pronum->clear();
    ui->label_number->clear();
    ui->label_108->show();
    ui->label_pronum->show();
    ui->label_number->show();
    ui->label_isxuan->clear();
    ui->pushButton_10->show();
    ui->pushButton_11->show();
    ui->label_109->show();
    ui->label_110->show();
    ui->pushButton_10->setEnabled(false);
    ui->pushButton_11->setEnabled(false);
    ui->label_currentindex->setText("1");
    ui->label_currentindex->show();
    ui->label_pageum->setText("1");
    //    pagenum = 2;
    //    on_pushButton_8_clicked();
    ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan1->setEnabled(false);
    ui->pushButton_xuan2->setEnabled(false);
    ui->pushButton_xuan3->setEnabled(false);
    ui->pushButton_xuan4->setEnabled(false);
    ui->pushButton_xuan5->setEnabled(false);
    ui->editQRCodeRule1->setEnabled(false);
    ui->editQRCodeRule2->setEnabled(false);
    ui->editQRCodeRule3->setEnabled(false);
    ui->editQRCodeRule4->setEnabled(false);
    ui->editQRCodeRule5->setEnabled(false);
    ui->pushButton_70->setText("");
    ui->pushButton_71->setText("");
    ui->pushButton_72->setText("");
    ui->pushButton_73->setText("");
    ui->pushButton_74->setText("");
    ui->pushButton_75->setText("");
    ui->pushButton_76->setText("");
    ui->pushButton_77->setText("");
    ui->pushButton_78->setText("");
    ui->pushButton_79->setText("");
    ui->editQRCodeRule1->setText("");
    ui->editQRCodeRule2->setText("");
    ui->editQRCodeRule3->setText("");
    ui->editQRCodeRule4->setText("");
    ui->editQRCodeRule5->setText("");

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    for(int i = 1;i < 6;i++ )
    {
//        editBox[i-1] ->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/QRCodeRule").append(QString::number(i))).toString());
        if(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/IsOptional<KNR>1</KNR>")).toInt())
        {
            buttonbox[i-1]->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/carcx")).toString());
            buttonbox[i-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
        }
        else
        {
            for(int k = 1;k < 6;k++)
            {
                if(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/LSNumber").append(QString::number(k))).toInt())
                {
                    buttonbox[i-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
                    buttonbox[i-1]->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/carcx")).toString());
                    break;
                }
            }
        }

    }
    delete configIniRead;
}
void Newconfiginfo::initui()
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    ui->stackedWidget->setCurrentIndex(1);
    ui->stackedWidget_3->setCurrentIndex(0);
    ui->pushButton_12->hide();
    ui->pushButton_15->hide();
    ui->pushButton_16->hide();
    ui->pushButton_14->hide();
    ui->label_83->show();
    ui->label_84->hide();
    ui->label_85->hide();
    ui->label_86->hide();
    //管理员   技术员
    //isJS = false;

    //读取车型配置信息

    for(int i = 1;i < 6;i++ )
    {
//        editBox[i-1] ->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/QRCodeRule")).toString());
        if(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/IsOptionalStation")).toInt())
        {
            buttonbox[i-1]->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/carcx")).toString());
            buttonbox[i-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
        }
        else
        {
            for(int k = 1;k < 6;k++)
            {
                if(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/LSNumber").append(QString::number(k))).toInt())
                {
                    buttonbox[i-1]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
                    buttonbox[i-1]->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/carcx")).toString());
                    break;
                }
            }
        }

    }

    if(isJS)
    {
        //        //技术
        //        ui->label_86->hide();
        //        ui->pushButton_28->hide();
        //        ui->stackedWidget_2->setCurrentIndex(0);
        //        ui->label_stationname->setText(configIniRead->value("baseinfo/StationName").toString());
        //        ui->label_stationid->setText(configIniRead->value("baseinfo/StationId").toString());
        //        ui->label_localip->setText(configIniRead->value("baseinfo/LocalIp").toString());
        //        ui->label_4->setText(configIniRead->value("baseinfo/Operator").toString());
        //        ui->label_6->setText(configIniRead->value("baseinfo/WirelessIp").toString());
        //        ui->label_csip->setText(configIniRead->value("baseinfo/cs351Ip").toString());
        //        ui->label_csporta->setText("4700");
        //        ui->label_csportb->setText("4710");
        //        ui->label_rfip->setText(configIniRead->value("baseinfo/RfidIp").toString());
        //        ui->label_rfport->setText(configIniRead->value("baseinfo/RfidPort").toString());
        //        ui->label_dataserverip->setText(configIniRead->value("baseinfo/DataServerIp").toString());
        //        ui->label_curveserverip->setText(configIniRead->value("baseinfo/CurveServerIp").toString());
        //        ui->label_BarcodeGun_2->setText(configIniRead->value("baseinfo/BarcodeGun").toString());
        //        ui->label_14->setText(configIniRead->value("baseinfo/SSID").toString());
        //        ui->label_5->setText(configIniRead->value("baseinfo/TackTime").toString());   //节拍时间


        //        if(isRFID == 1)   //RFID 条码抢模式的 界面初始化
        //        {
        //            ui->stackedWidget_7->setCurrentIndex(0);
        //        }
        //        else
        //        {
        //            ui->stackedWidget_7->setCurrentIndex(1);
        //        }
    }
    else
    {
        //管理
        //        if(isRFID == 1)   //RFID 条码抢模式的 界面初始化
        //        {
        //            ui->stackedWidget_4->setCurrentIndex(0);
        //        }
        //        else
        //        {
        //            ui->stackedWidget_4->setCurrentIndex(1);
        //        }
        ui->pushButton_28->show();
        ui->stackedWidget_2->setCurrentIndex(2);

        // ui->stackedWidget_4->setCurrentIndex(0);
        ui->stackedWidget_5->setCurrentIndex(0);

        ui->lineEdit_staname->setText(configIniRead->value("baseinfo/StationName").toString());
        ui->lineEdit_lineName->setText(configIniRead->value("baseinfo/LineName").toString());
        LineName = ui->lineEdit_lineName->text();
        //        ui->lineEdit_Operator->setText(configIniRead->value("baseinfo/Operator").toString());
        ui->lineEdit_localip->setText(configIniRead->value("baseinfo/LocalIp").toString());
        ui->lineEdit_localip2->setText(configIniRead->value("baseinfo/LocalIp2").toString());
        ui->lineEdit_wirelessip->setText(configIniRead->value("baseinfo/WirelessIp").toString());
        ui->lineEdit_staid->setText(configIniRead->value("baseinfo/StationId").toString());

        ui->label_97->setText(configIniRead->value("baseinfo/ControllerIp_1").toString());
        ui->label_155->setText(configIniRead->value("baseinfo/RfidIp").toString());
        ui->label_154->setText(configIniRead->value("baseinfo/DataServerIp").toString());
        if(isRFID)
            ui->label_153->setText(RfidIp);
        else if(isBarCode)
            ui->label_153->setText(BarcodeGun);
        else
            ui->label_153->setText("");
        //        ui->label_153->setText(configIniRead->value("baseinfo/BarcodeGun").toString());
        //        ui->lineEdit_csip->setText(configIniRead->value("baseinfo/cs351Ip").toString());
        //        ui->lineEdit_csporta->setText(configIniRead->value("baseinfo/PortA").toString());
        //        ui->lineEdit_csportb->setText(configIniRead->value("baseinfo/PortB").toString());
        //        ui->lineEdit_serip->setText(configIniRead->value("baseinfo/ServerIp").toString());
        //        ui->lineEdit_serport->setText(configIniRead->value("baseinfo/ServerPort").toString());


    }

    delete configIniRead;
}
void Newconfiginfo::on_pushButton_clicked()
{
    //QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    ui->pushButton_butt1->setEnabled(true);
    ui->pushButton_butt2->setEnabled(true);
    ui->pushButton_butt3->setEnabled(true);
    ui->pushButton_butt4->setEnabled(true);
    ui->pushButton_butt5->setEnabled(true);
    if(SaveWhat == "pdm")
    {
        ui->label_69->show();
        ui->comboBox->show();
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        SaveWhat = "pdmoutmain";
        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();
    }
    else
    {
        if(isJS)
        {
            //QSqlDatabase::removeDatabase("QMYSQL");
            //QSqlDatabase::removeDatabase("qt_sql_default_connection");
            //            historyclear();
            //workmode = true;
            //config->setValue("baseinfo/workmode","0");
            workmode = false;
            ISmaintenance = false;
            int tempdata = numpdm;
            for(int i = 0;i<tempdata;i++)
            {
                delete butt1[i];
                numpdm--;
            }
            ui->listWidget->setCurrentRow(1);
            this->close();
            if(ControlType_1 !="SB356")
                emit closeconfig();
            //            emit xmlcreate();
        }
        else
        {
            whichButtonClick = "baseback";
            if(!isbaseinfochange)
            {
                baseInfoIsChange();
            }
            else
            {
                //QSqlDatabase::removeDatabase("QMYSQL");
                //QSqlDatabase::removeDatabase("qt_sql_default_connection");
                historyclear();
                ui->tabWidget->setCurrentIndex(0);
                //workmode = true;
                //config->setValue("baseinfo/workmode","0");
                workmode = false;
                ISmaintenance = false;
                int tempdata = numpdm;
                for(int i = 0;i<tempdata;i++)
                {
                    delete butt1[i];
                    numpdm--;
                }
                ui->listWidget->setCurrentRow(1);
                DebugMode = false;
                this->close();
                if(ControlType_1 !="SB356")
                    emit closeconfig();
                //                emit xmlcreate();
                system("cp /config.ini /config1.ini &");
            }
        }

    }

    // on_pushButton_15_clicked();
    //delete config;
}

void Newconfiginfo::baseInfoIsChange()
{
    // 监听
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    if (controllerIp_01 != configIniRead->value("baseinfo/ControllerIp_1").toString()||
            controllerIp_02 != configIniRead->value("baseinfo/ControllerIp_2").toString()||
            //PortA != configIniRead->value("baseinfo/PortA").toString()||
            //PortB != configIniRead->value("baseinfo/PortB").toString()||
            RfidIp != configIniRead->value("baseinfo/RfidIp").toString()||
            RfidPort != configIniRead->value("baseinfo/RfidPort").toString()||
            DataServerIp != configIniRead->value("baseinfo/DataServerIp").toString()||
            CurveServerIp != configIniRead->value("baseinfo/CurveServerIp").toString()||
            AndonServerIp != configIniRead->value("baseinfo/AndonServerIp").toString()||
            ServerPort != configIniRead->value("baseinfo/ServerPort").toString()||
            BarcodeGun != configIniRead->value("baseinfo/BarcodeGun").toString()||
            ui->lineEdit_staname->text() != configIniRead->value("baseinfo/StationName").toString()||
            //            ui->lineEdit_Operator->text() != configIniRead->value("baseinfo/Operator").toString()||
            ui->lineEdit_localip->text() != configIniRead->value("baseinfo/LocalIp").toString()||
            ui->lineEdit_localip2->text() != configIniRead->value("baseinfo/LocalIp2").toString()||
            ui->lineEdit_wirelessip->text() != configIniRead->value("baseinfo/WirelessIp").toString()||
            ui->lineEdit_staid->text() != configIniRead->value("baseinfo/StationId").toString()||
            ui->lineEdit_lineName->text()!= configIniRead->value("baseinfo/LineName").toString())
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        SaveWhat = "baseinfo";
        save = new Save(this);
        connect(save,SIGNAL(sendSaveBaseinfo(bool)),this,SLOT(receiveBaseinfoSave(bool)));
        save->show();
        isbaseinfochange = true;
        delete configIniRead;
    }
    else
    {
        delete configIniRead;
        isbaseinfochange = true;
        if (whichButtonClick == "baseback")
        {
            on_pushButton_22_clicked();
            isbaseinfochange = false;
        }
        //        else if (whichButtonClick == "about")
        //        {
        //            on_pushButton_13_clicked();
        //            isbaseinfochange = false;
        //        }
        //        else if (whichButtonClick == "PDMEdit")
        //        {
        //            on_pushButton_28_clicked();
        //            isbaseinfochange = false;
        //        }
        //        else if (whichButtonClick == "config")
        //        {
        //            ui->stackedWidget_2->setCurrentIndex(4);
        //            ui->label_83->hide();
        //            ui->label_84->show();
        //            ui->label_85->hide();
        //            ui->label_86->hide();
        //            //            ui->pushButton_13->hide();
        //            //            ui->label_52->setFocus();
        //            isbaseinfochange = false;
        //        }
        //        else if (whichButtonClick == "history")
        //        {
        //            ui->stackedWidget_2->setCurrentIndex(3);
        //            ui->label_83->hide();
        //            ui->label_84->hide();
        //            ui->label_85->show();
        //            ui->label_86->hide();
        //            //            ui->pushButton_13->hide();
        //            //            ui->label_52->setFocus();
        //            isbaseinfochange = false;
        //        }
        else if (whichButtonClick == "advanced")
        {
            on_pushButton_62_clicked();
            isbaseinfochange = false;
        }
        //        else if (whichButtonClick == "base")
        //        {
        //            isbaseinfochange = false;
        //        }

    }
}

void Newconfiginfo::on_listWidget_currentRowChanged(int currentRow)
{
    //ui->label_22->setText(QString::number(currentRow));

    if(SaveWhat == "pdm")
    {
        nowcomboxnum =  currentRow;
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        SaveWhat = "pdmout";
        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();
    }
    else
    {
        int tempdata = numpdm;
        for(int i = 0;i<tempdata;i++)
        {
            delete butt1[i];
            numpdm--;
        }
        ui->listWidget->setStyleSheet("QListWidget::item:seleted{ border-color: rgb(51, 153, 255);} QListWidget{font: 18pt ;color: rgb(248, 248, 255);background:transparent;}");
        SaveWhat = "";
        if(isJS)
        {
            if(currentRow == 0)
            {
                historyclear();
                ui->stackedWidget_2->setCurrentIndex(0);
                ui->label_83->show();
                ui->label_84->hide();
                ui->label_85->hide();
                //            ui->pushButton_13->show();
            }
            else if(currentRow == 3)
            {
                historyclear();
                ui->stackedWidget_2->setCurrentIndex(4);
                ui->stackedWidget_5->setCurrentIndex(0);
                clearCache();
                ui->label_83->hide();
                ui->label_84->show();
                ui->label_85->hide();
                //            ui->pushButton_13->hide();
                //            ui->label_52->setFocus();

            }else if(currentRow == 6)
            {
                ui->stackedWidget_2->setCurrentIndex(3);
                ui->label_83->hide();
                ui->label_84->hide();
                ui->label_85->show();
                //            ui->pushButton_13->hide();
                //            ui->label_52->setFocus();

            }

        }else
        {
            if(currentRow == 0)
            {
                historyclear();
                ui->stackedWidget_2->setCurrentIndex(2);
                ui->label_83->show();
                ui->label_84->hide();
                ui->label_85->hide();
                ui->label_86->hide();
                //            ui->pushButton_13->show();
                //            ui->label_52->setFocus();

            }
            else if(currentRow == 3)
            {
                historyclear();
                if (ui->stackedWidget_2->currentIndex()==2)
                {
                    whichButtonClick = "config";
                    if(!isbaseinfochange)
                        baseInfoIsChange();
                }
                else
                {
                    ui->stackedWidget_2->setCurrentIndex(4);
                    ui->label_83->hide();
                    ui->label_84->show();
                    ui->label_85->hide();
                    ui->label_86->hide();
                }
            }
            else if(currentRow == 6)
            {
                if (ui->stackedWidget_2->currentIndex()==2)
                {
                    whichButtonClick = "history";
                    if(!isbaseinfochange)
                        baseInfoIsChange();
                }
                else
                {
                    ui->stackedWidget_2->setCurrentIndex(3);
                    ui->label_83->hide();
                    ui->label_84->hide();
                    ui->label_85->show();
                    ui->label_86->hide();
                }
            }
        }
    }
}
// cancel
void Newconfiginfo::receivebaseinfocancel()
{
    delete e3;
    ui->label_100->hide();
    delete basicset;
    SaveWhat = "";
}
void Newconfiginfo::receiveBaseinfo(QString a , QString b, QString c, QString d)
{
    if (SaveWhat == "Controller")
    {
        controllerIp_01 = a;
        controllerIp_02 = b;
        //        PortB = c;
        ui->label_97->setText(controllerIp_01);
    }
    //    else if(SaveWhat=="RFID")
    //    {
    //        RfidIp = a;
    //        RfidPort = b;
    //        ui->label_155->setText(RfidIp);

    //    }
    else if(SaveWhat=="server")
    {
        DataServerIp = a;
        CurveServerIp = b;
        AndonServerIp = c;
        ServerPort = d;
        ui->label_154->setText(DataServerIp);
    }
    else if(SaveWhat=="Barcode")
    {
        if(isBarCode)
        {
            BarcodeGun = c;
            ui->label_153->setText(BarcodeGun);
        }
        if(isRFID)
        {
            RfidIp = a;
            RfidPort = b;
            ui->label_153->setText(RfidIp);
        }
    }
    delete e3;
    ui->label_100->hide();
    delete basicset;
    SaveWhat = "";
}
void Newconfiginfo::on_pushButton_59_clicked()
{
    //cs351设置 按钮
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);
    SaveWhat = "Controller";
    basicset = new BasicSet(this);
    basicset->setConfigValue351(controllerIp_01,controllerIp_02);
    connect(basicset,SIGNAL(sendBaseinfo(QString,QString,QString,QString)),this,SLOT(receiveBaseinfo(QString,QString,QString,QString)));
    connect(basicset,SIGNAL(sendbaseinfocancel()),this,SLOT(receivebaseinfocancel()));
    connect(basicset,SIGNAL(sendTruncate()),this,SLOT(mysqlTruncate()));
    connect(basicset,SIGNAL(sendTruncateQueue()),this,SLOT(mysqlTruncateQueue()));
    connect(this,SIGNAL(sendTruncateResult(bool)),basicset,SLOT(receiveResult(bool)));
    connect(this,SIGNAL(sendTruncateQueueResult(bool)),basicset,SLOT(receiveQueueResult(bool)));
    basicset->show();

}
void Newconfiginfo::mysqlTruncate()
{
    DTdebug()<<"truncate table "+Localtable;
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
        return;
    }
    if(!query->exec("truncate table "+Localtable))
    {
        DTdebug()<<"truncate Databases fail"<<query->lastError();
        emit sendTruncateResult(false);
    }
    else
    {
        DTdebug()<<"truncate Databases success";
        emit sendTruncateResult(true);
    }
}

void Newconfiginfo::mysqlTruncateQueue()
{
    DTdebug()<<"truncate table "+tablePreview;
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
        return;
    }
    if(!query->exec("truncate table "+tablePreview))
    {
        DTdebug()<<"truncate queue Databases fail"<<query->lastError();
        emit sendTruncateQueueResult(false);
    }
    else
    {
        DTdebug()<<"truncate queue Databases success";
        emit sendTruncateQueueResult(true);
    }
}

void Newconfiginfo::on_pushButton_60_clicked()
{
    //RFID 设置按钮

    //    e3 = new QGraphicsOpacityEffect(this);
    //    e3->setOpacity(0.5);
    //    ui->label_100->setGraphicsEffect(e3);
    //    ui->label_100->show();
    //    ui->label_100->setGeometry(0,0,1366,768);
    //    SaveWhat = "RFID";
    //    basicset = new BasicSet(this);
    //    basicset->setSerialOrRfidMode(RfidIp,RfidPort);
    //    connect(basicset,SIGNAL(sendBaseinfo(QString,QString,QString)),this,SLOT(receiveBaseinfo(QString,QString,QString)));
    //    connect(basicset,SIGNAL(sendbaseinfocancel()),this,SLOT(receivebaseinfocancel()));
    //    basicset->show();

}
void Newconfiginfo::on_pushButton_61_clicked()
{
    //server
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);
    SaveWhat = "server";
    basicset = new BasicSet(this);
    basicset->setServerValue(DataServerIp,CurveServerIp,AndonServerIp,ServerPort);
    connect(basicset,SIGNAL(sendBaseinfo(QString,QString,QString,QString)),this,SLOT(receiveBaseinfo(QString,QString,QString,QString)));
    connect(basicset,SIGNAL(sendbaseinfocancel()),this,SLOT(receivebaseinfocancel()));
    basicset->show();
}

void Newconfiginfo::on_pushButton_65_clicked()
{
    //条码枪

    //ui->stackedWidget_4->setCurrentIndex(1);
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);
    SaveWhat = "Barcode";
    basicset = new BasicSet(this);
    basicset->setSerialOrRfidMode(RfidIp,RfidPort,BarcodeGun);
    connect(basicset,SIGNAL(sendBaseinfo(QString,QString,QString,QString)),this,SLOT(receiveBaseinfo(QString,QString,QString,QString)));
    connect(basicset,SIGNAL(sendbaseinfocancel()),this,SLOT(receivebaseinfocancel()));
    basicset->show();
}


void Newconfiginfo::on_pushButton_63_clicked()
{
    //取消
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    ui->lineEdit_staname->setText(configIniRead->value("baseinfo/StationName").toString());
    //    ui->lineEdit_Operator->setText(configIniRead->value("baseinfo/Operator").toString());
    ui->lineEdit_localip->setText(configIniRead->value("baseinfo/LocalIp").toString());
    ui->lineEdit_localip2->setText(configIniRead->value("baseinfo/LocalIp2").toString());
    ui->lineEdit_wirelessip->setText(configIniRead->value("baseinfo/WirelessIp").toString());
    ui->lineEdit_staid->setText(configIniRead->value("baseinfo/StationId").toString());
    ui->lineEdit_lineName->setText(configIniRead->value("baseinfo/LineName").toString());
    LineName = ui->lineEdit_lineName->text();

    controllerIp_01 = configIniRead->value("baseinfo/ControllerIp_1").toString();
    controllerIp_02 = configIniRead->value("baseinfo/ControllerIp_2").toString();
    //    PortA = configIniRead->value("baseinfo/PortA").toString();
    //    PortB = configIniRead->value("baseinfo/PortB").toString();
    RfidIp = configIniRead->value("baseinfo/RfidIp").toString();
    RfidPort = configIniRead->value("baseinfo/RfidPort").toString();
    DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
    CurveServerIp= configIniRead->value("baseinfo/CurveServerIp").toString();
    AndonServerIp= configIniRead->value("baseinfo/AndonServerIp").toString();
    ServerPort= configIniRead->value("baseinfo/ServerPort").toString();
    BarcodeGun = configIniRead->value("baseinfo/BarcodeGun").toString();

    ui->label_97->setText(controllerIp_01);
    //    ui->label_155->setText(RfidIp);
    ui->label_154->setText(DataServerIp);
    if(isRFID)
        ui->label_153->setText(RfidIp);
    else if(isBarCode)
        ui->label_153->setText(BarcodeGun);
    else
        ui->label_153->setText("");

    ui->label_100->hide();

    delete configIniRead;
}

void Newconfiginfo::receiveBaseinfoSave(bool statetmp)
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    if(statetmp)
    {
        //true        
        configIniRead->setValue("baseinfo/StationName",ui->lineEdit_staname->text());
        //        configIniRead->setValue("baseinfo/Operator",ui->lineEdit_Operator->text());
        //        Operator=ui->lineEdit_Operator->text();
        configIniRead->setValue("baseinfo/StationId",ui->lineEdit_staid->text());
        Station =ui->lineEdit_staid->text();
        configIniRead->setValue("baseinfo/LineName",ui->lineEdit_lineName->text());
        LineName = ui->lineEdit_lineName->text();

        configIniRead->setValue("baseinfo/ControllerIp_1",controllerIp_01);
        configIniRead->setValue("baseinfo/ControllerIp_2",controllerIp_02);
        //        configIniRead->setValue("baseinfo/PortA",PortA);
        //        configIniRead->setValue("baseinfo/PortB",PortB);
        configIniRead->setValue("baseinfo/RfidIp",RfidIp);
        configIniRead->setValue("baseinfo/RfidPort",RfidPort);
        configIniRead->setValue("baseinfo/AndonServerIp",AndonServerIp);
        configIniRead->setValue("baseinfo/BarcodeGun",BarcodeGun);
        configIniRead->setValue("baseinfo/ServerPort",ServerPort);

        configIniRead->setValue("baseinfo/DeviceNo",ui->lineEdit_deviceNo->text());

        if(CurveServerIp != configIniRead->value("baseinfo/CurveServerIp").toString()||
                DataServerIp != configIniRead->value("baseinfo/DataServerIp").toString()||
                AndonServerIp != configIniRead->value("baseinfo/AndonServerIp").toString())
        {
            configIniRead->setValue("baseinfo/CurveServerIp",CurveServerIp);
            configIniRead->setValue("baseinfo/DataServerIp",DataServerIp);
            configIniRead->setValue("baseinfo/AndonServerIp",AndonServerIp);

            QString fileName = "/usr/local/arm/freetds/etc/freetds.conf";
            QFile file(fileName);
            if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
                DTdebug()   << "Cannot open testdsn file for Reading";
            }
            QString str (file.readAll());
            if(str.contains("[testdsn1]", Qt::CaseInsensitive)&&str.contains("[testdsn2]", Qt::CaseInsensitive)){
                str.replace(QRegExp("\\[testdsn1\\]\\s*host = \\S*"),QString("[testdsn1]\n\thost = ")+CurveServerIp);
                str.replace(QRegExp("\\[testdsn2\\]\\s*host = \\S*"),QString("[testdsn2]\n\thost = ")+DataServerIp);
                str.replace(QRegExp("\\[testdsn3\\]\\s*host = \\S*"),QString("[testdsn3]\n\thost = ")+AndonServerIp);
            }
            file.close();
            if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                DTdebug()   << "Cannot open testdsn file for Writing";
            }
            file.write(str.toUtf8());
            file.close();
        }

        //        configIniRead->setValue("baseinfo/DataServerIp",DataServerIp);

        if(ui->lineEdit_localip->text() != configIniRead->value("baseinfo/LocalIp").toString()||
                ui->lineEdit_localip2->text() != configIniRead->value("baseinfo/LocalIp2").toString()||
                ui->lineEdit_wirelessip->text() != configIniRead->value("baseinfo/WirelessIp").toString())
        {
            //            QString fileName = "/etc/profile";
            //            QFile file(fileName);
            //            if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
            //                DTdebug()   << "Cannot open profile file for Reading";
            //            }
            //            QString str (file.readAll());
            int connectNet = configIniRead->value("baseinfo/connectNet").toInt();
            if(ui->lineEdit_localip->text() != configIniRead->value("baseinfo/LocalIp").toString())
            {
                configIniRead->setValue("baseinfo/LocalIp",ui->lineEdit_localip->text());
#if 0
                //20190725
                if(connectNet == 1)
                    system(QString("ifconfig eth0 "+ui->lineEdit_localip->text()+" netmask "+configIniRead->value("baseinfo/netmask").toString()+"  &").toLatin1().data());
                else
                    system(QString("ifconfig eth0 "+ui->lineEdit_localip->text()+" netmask 255.255.255.0 &").toLatin1().data());
#endif
                //                system(QString("ifconfig eth0 ").append(ui->lineEdit_localip->text()+" netmask 255.255.255.0 &").toLatin1().data());
                //                if(str.contains("ifconfig eth0", Qt::CaseInsensitive)){
                //                    str.replace(QRegExp("ifconfig eth0 \\S*"),QString("ifconfig eth0 ")+ui->lineEdit_localip->text());
                //                }
            }
            if(ui->lineEdit_localip2->text() != configIniRead->value("baseinfo/LocalIp2").toString())
            {
                configIniRead->setValue("baseinfo/LocalIp2",ui->lineEdit_localip2->text());
#if 0
                //20190725
                if(connectNet == 2)
                    system(QString("ifconfig eth1 "+ui->lineEdit_localip2->text()+" netmask "+configIniRead->value("baseinfo/netmask").toString()+"  &").toLatin1().data());
                else
                    system(QString("ifconfig eth1 "+ui->lineEdit_localip2->text()+" netmask 255.255.255.0 &").toLatin1().data());
                //                system(QString("ifconfig eth1 "+ui->lineEdit_localip2->text()+" netmask 255.255.255.0 &").toLatin1().data());
#endif
            }
            if(ui->lineEdit_wirelessip->text() != configIniRead->value("baseinfo/WirelessIp").toString())
            {
                configIniRead->setValue("baseinfo/WirelessIp",ui->lineEdit_wirelessip->text());
#if 0
                //20190725
                WirelessIp = ui->lineEdit_wirelessip->text();
                if(connectNet == 0)
                    system((QString("ifconfig wlan0 ")+ui->lineEdit_wirelessip->text()+QString(" netmask ")+configIniRead->value("baseinfo/netmask").toString()+" &").toLatin1().data());
                else
                    system(QString("ifconfig wlan0 "+ui->lineEdit_wirelessip->text()+" netmask 255.255.255.0 &").toLatin1().data());
                //                if(str.contains("ifconfig wlan0", Qt::CaseInsensitive)){
                //                    str.replace(QRegExp("ifconfig wlan0 \\S*"),QString("ifconfig wlan0 ")+ui->lineEdit_wirelessip->text());
                //                }
#endif
            }

            //            file.close();
            //            if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            //                DTdebug()   << "Cannot open profile file for Writing";
            //            }
            //            file.write(str.toUtf8());
            //            file.close();
        }

        ui->label_100->hide();
        delete e3;
        delete save;

    }else
    {
        //false
        ui->lineEdit_staname->setText(configIniRead->value("baseinfo/StationName").toString());
        //        ui->lineEdit_Operator->setText(configIniRead->value("baseinfo/Operator").toString());
        ui->lineEdit_localip->setText(configIniRead->value("baseinfo/LocalIp").toString());
        ui->lineEdit_localip2->setText(configIniRead->value("baseinfo/LocalIp2").toString());
        ui->lineEdit_wirelessip->setText(configIniRead->value("baseinfo/WirelessIp").toString());
        ui->lineEdit_staid->setText(configIniRead->value("baseinfo/StationId").toString());
        ui->lineEdit_lineName->setText(configIniRead->value("baseinfo/LineName").toString());
        LineName = ui->lineEdit_lineName->text();

        controllerIp_01 = configIniRead->value("baseinfo/ControllerIp_1").toString();
        controllerIp_02 = configIniRead->value("baseinfo/ControllerIp_2").toString();
        //        PortA = configIniRead->value("baseinfo/PortA").toString();
        //        PortB = configIniRead->value("baseinfo/PortB").toString();
        RfidIp = configIniRead->value("baseinfo/RfidIp").toString();
        RfidPort = configIniRead->value("baseinfo/RfidPort").toString();
        DataServerIp = configIniRead->value("baseinfo/DataServerIp").toString();
        CurveServerIp = configIniRead->value("baseinfo/CurveServerIp").toString();
        AndonServerIp = configIniRead->value("baseinfo/AndonServerIp").toString();
        ServerPort = configIniRead->value("baseinfo/ServerPort").toString();
        BarcodeGun = configIniRead->value("baseinfo/BarcodeGun").toString();

        ui->label_97->setText(controllerIp_01);
        ui->label_155->setText(RfidIp);
        ui->label_154->setText(DataServerIp);
        ui->label_153->setText(BarcodeGun);
        ui->label_100->hide();
        delete e3;
        delete save;
    }
    delete configIniRead;
    isbaseinfochange = true;

    if(whichButtonClick == "baseback")
    {
        on_pushButton_22_clicked();
        isbaseinfochange = false;
    }
    else if (whichButtonClick == "about")
    {
        on_pushButton_13_clicked();
        isbaseinfochange = false;
    }
    else if (whichButtonClick == "PDMEdit")
    {
        on_pushButton_28_clicked();
        isbaseinfochange = false;
    }
    else if (whichButtonClick == "config")
    {
        ui->stackedWidget_2->setCurrentIndex(4);
        ui->label_83->hide();
        ui->label_84->show();
        ui->label_85->hide();
        ui->label_86->hide();
        //        ui->pushButton_13->hide();
        ui->label_52->setFocus();
        isbaseinfochange = false;
    }
    else if (whichButtonClick == "history")
    {
        ui->stackedWidget_2->setCurrentIndex(3);
        ui->label_83->hide();
        ui->label_84->hide();
        ui->label_85->show();
        ui->label_86->hide();
        //        ui->pushButton_13->hide();
        ui->label_52->setFocus();
        isbaseinfochange = false;
    }
    else if (whichButtonClick == "advanced")
    {
        on_pushButton_62_clicked();
        isbaseinfochange = false;
    }
    else if (whichButtonClick == "save")
    {
        isbaseinfochange = false;
    }
    //    else if (whichButtonClick == "base")
    //    {
    //        isbaseinfochange = false;
    //    }

}

void Newconfiginfo::on_pushButton_64_clicked()
{
    //保存 参数按钮
    //保存
    whichButtonClick = "save";
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);

    SaveWhat = "baseinfo";
    save = new Save(this);
    connect(save,SIGNAL(sendSaveBaseinfo(bool)),this,SLOT(receiveBaseinfoSave(bool)));
    save->show();
}


void Newconfiginfo::moveDo()
{
    //左右 滑动 逻辑

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    //DTdebug() << "pagenum is is is " << pagenum;
    ui->label_carname->clear();
    ui->label_iobox->clear();
    ui->label_G9->clear();
    ui->label_VIN->clear();
    ui->label_isxuan->clear();
    ui->label_Lsnum->clear();
    ui->label_pronum->clear();
    ui->label_number->clear();
    ui->label_108->show();
    ui->label_pronum->show();
    ui->label_number->show();
    ui->label_isxuan->clear();
    ui->pushButton_10->show();
    ui->pushButton_11->show();
    ui->label_109->show();
    ui->label_110->show();
    ui->pushButton_10->setEnabled(false);
    ui->pushButton_11->setEnabled(false);
    ui->label_currentindex->setText("1");
    ui->label_currentindex->show();
    //    pagenum = 2;
    //    on_pushButton_8_clicked();
    ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan1->setEnabled(false);
    ui->pushButton_xuan2->setEnabled(false);
    ui->pushButton_xuan3->setEnabled(false);
    ui->pushButton_xuan4->setEnabled(false);
    ui->pushButton_xuan5->setEnabled(false);

    ui->editQRCodeRule1->setEnabled(false);
    ui->editQRCodeRule2->setEnabled(false);
    ui->editQRCodeRule3->setEnabled(false);
    ui->editQRCodeRule4->setEnabled(false);
    ui->editQRCodeRule5->setEnabled(false);

    ui->pushButton_70->setText("");
    ui->pushButton_71->setText("");
    ui->pushButton_72->setText("");
    ui->pushButton_73->setText("");
    ui->pushButton_74->setText("");
    ui->pushButton_75->setText("");
    ui->pushButton_76->setText("");
    ui->pushButton_77->setText("");
    ui->pushButton_78->setText("");
    ui->pushButton_79->setText("");

    ui->editQRCodeRule1->setText("");
    ui->editQRCodeRule2->setText("");
    ui->editQRCodeRule3->setText("");
    ui->editQRCodeRule4->setText("");
    ui->editQRCodeRule5->setText("");


    //    ui->label_carname->clear();
    //    ui->label_G9->clear();
    //    ui->label_VIN->clear();
    //    ui->label_isxuan->clear();
    //    ui->label_Lsnum->clear();
    //    ui->label_pronum->clear();
    //    ui->label_number->clear();
    previouswhichar = 0;
    int buttnumber = 0;
    buttnumber = (pagenum -1)*5+1;
    int t = 0;
    for(int j = buttnumber;j < (buttnumber+5);j++)
    {
        if(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/IsOptionalStation")).toInt()) //有选配
        {
            buttonbox[t]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
            buttonbox[t]->setText(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/carcx")).toString());
//            editBox[t]  ->setText(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/QRCodeRule")).toString());
        }
        else //非选配
        {
            int i = 1;
            for(i = 1;i < 6;i++)
            {
                if(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/LSNumber").append(QString::number(i))).toInt())
                {
                    buttonbox[t]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
                    buttonbox[t]->setText(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/carcx")).toString());
//                    editBox[t]  ->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/QRCodeRule")).toString());
                    break;
                }
            }
            if(i == 6)
            {
                buttonbox[t]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                buttonbox[t]->setText("");
                editBox[t]  ->setText("");            }
        }
        t++;
    }

    delete configIniRead;
}
void Newconfiginfo::on_pushButton_8_clicked()
{
    //left
    if(pagenum == 1)
        pagenum = 2;
    else
    {
        ui->pushButton_12->hide();
    }
    pagenum--;
    ui->label_pageum->setText(QString::number(pagenum));
    moveDo();

}

void Newconfiginfo::on_pushButton_9_clicked()
{
    int tmpPageNum = (CAR_TYPE_AMOUNT/5);
    //right
    if(pagenum == tmpPageNum)    //车型页数
        pagenum = tmpPageNum-1;
    else
    {
        ui->pushButton_12->hide();
    }
    pagenum++;
    ui->label_pageum->setText(QString::number(pagenum));
    moveDo();
}
void Newconfiginfo::buttclicked()
{
    //点击清空
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    ui->comboBox_2->clear();
    ui->comboBox_3->clear();
    ui->comboBox_2->addItem("");
    ui->pushButton_10->setEnabled(true);
    ui->pushButton_11->setEnabled(true);
    for(int i= 0;i < 1000 ;i++)
    {
        if(configIniRead->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString()!= "")
        {
            ui->comboBox_2->addItem(configIniRead->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString());
        }
    }
    ui->comboBox_3->addItem("");
    for(int i= 0;i < 1000 ;i++)
    {

        if(configIniRead->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString()!= "")
        {
            ui->comboBox_3->addItem(configIniRead->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString());
        }
    }
    ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
    ui->pushButton_xuan1->setEnabled(false);
    ui->pushButton_xuan2->setEnabled(false);
    ui->pushButton_xuan3->setEnabled(false);
    ui->pushButton_xuan4->setEnabled(false);
    ui->pushButton_xuan5->setEnabled(false);

    ui->editQRCodeRule1->setEnabled(false);
    ui->editQRCodeRule2->setEnabled(false);
    ui->editQRCodeRule3->setEnabled(false);
    ui->editQRCodeRule4->setEnabled(false);
    ui->editQRCodeRule5->setEnabled(false);

    ui->pushButton_70->setText("");
    ui->pushButton_71->setText("");
    ui->pushButton_72->setText("");
    ui->pushButton_73->setText("");
    ui->pushButton_74->setText("");
    ui->pushButton_75->setText("");
    ui->pushButton_76->setText("");
    ui->pushButton_77->setText("");
    ui->pushButton_78->setText("");
    ui->pushButton_79->setText("");

    ui->editQRCodeRule1->setText("");
    ui->editQRCodeRule2->setText("");
    ui->editQRCodeRule3->setText("");
    ui->editQRCodeRule4->setText("");
    ui->editQRCodeRule5->setText("");

    ui->lineEdit_carname->clear();
    ui->lineEdit_iobox->clear();
    ui->lineEdit_G9->clear();
    ui->lineEdit_VIN->clear();
    ui->editExecCarType->clear();
    ui->lineEdit_Lsnumber->clear();
    ui->lineEdit_pronum->setText("0");
    ui->lineEdit_number->setText("0");
    ui->lineEdit_iobox->setText("255");
    ui->lineEdit_xuanLsnum->clear();
    ui->lineEdit_xuanpronum->setText("0");
    ui->lineEdit_xuannumber->setText("0");
    ui->lineEdit_channel_2->setText("1");
    ui->lineEdit_taotong_2->setText("OFF");
    ui->label_126->hide();
    ui->comboBox_2->hide();
    isedit = 0;
    carStyle = "";
    mIoBox = "";
    G9tmp = "";
    for(int i=0;i<4;i++)
    {
        Out[i] = false;
        PDM_Name[i] = "";
        PDM_Name2[i] = "";
    }
    Vintmp = "";
    pdmxuan = "";
    for(int m = 0;m<20;m++)
    {
        luo[m] = "0";
        pro[m] = "0";
        lsnumers[m] = "0";
        taotong[m] = "0";
        channel[m] = "1";


        luo2[m] = "0";
        pro2[m] = "0";
        lsnumers2[m] = "0";
        taotong2[m] = "0";
        channel2[m] = "1";
    }
    for(int i = 0;i<D_CAR_OPTION_NUM;i++)
    {
        ioBoxOption[i] = 0;
        for(int k =0;k < 20;k++)
        {
            luoxuanlist[i][k]="0";
            proxuanlist[i][k]="0";
            lsnumersxuanlist[i][k]="0";
            bxuannamelist[i][k]="";
            bxuancodelist[i][k]="";
            kxuannamelist[i][k]="";
            kxuancodelist[i][k]="";
            channelxuanlist[i][k]="1";
            taotongxuanlist[i][k]="0";

            luoxuanlist2[i][k]="0";
            proxuanlist2[i][k]="0";
            lsnumersxuanlist2[i][k]="0";
            bxuannamelist2[i][k]="";
            bxuancodelist2[i][k]="";
            kxuannamelist2[i][k]="";
            kxuancodelist2[i][k]="";
            channelxuanlist2[i][k]="1";
            taotongxuanlist2[i][k]="0";

        }
        for(int k =0;k < 4;k++)
        {
            PDMxuan_Name[i][k]="";
            PDMxuan_Name2[i][k]="";
        }
        //        pdmxuanlist[i]="";
        //        pdmxuanlist2[i]="";
    }

    ui->label_currentindex->setText("1");
    ui->label_119->setText("1");
    ui->label_51->setText("1");
    // DTdebug() <<  "priouswhich car is   sjkdjf ji " << previouswhichar;

    // currentpages = ui->label_51->text().toInt();
    int tmpisselect = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/IsOptionalStation")).toInt();
    int tmpisselect1 = configIniRead->value(QString("carinfo").append(QString::number(previouswhichar)).append("/IsOptionalStation")).toInt();
    if(previouswhichar <= 5*pagenum && previouswhichar >0) //点击别的 回复之前button效果
    {
        if(previouswhichar == 4)
        {
            //DTdebug() << "aaa";
        }
        if(previouswhichar != whichcar)
        {
            int buttnumber = 0;
            buttnumber = (pagenum -1)*5+1;
            if(tmpisselect1) //有选配
            {
                buttonbox[previouswhichar-buttnumber]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
                buttonbox[previouswhichar-buttnumber]->setText(configIniRead->value(QString("carinfo").append(QString::number(previouswhichar)).append("/carcx")).toString());
              }
            else //非选配
            {
                int i = 1;
                for(i = 1;i < 6;i++)
                {
                    if(configIniRead->value(QString("carinfo").append(QString::number(previouswhichar)).append("/LSNumber").append(QString::number(i))).toInt())
                    {
                        buttonbox[previouswhichar-buttnumber]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
                        buttonbox[previouswhichar-buttnumber]->setText(configIniRead->value(QString("carinfo").append(QString::number(previouswhichar)).append("/carcx")).toString());
//                        editBox[previouswhichar-buttnumber]  ->setText(configIniRead->value(QString("carinfo").append(QString::number(i)).append("/QRCodeRule")).toString());
                        break;
                    }
                }
                if(i == 6)
                {
                    buttonbox[previouswhichar-buttnumber]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    buttonbox[previouswhichar-buttnumber]->setText("");
                    editBox[previouswhichar-buttnumber]  ->setText("");
                }
            }
        }

    }
    if(tmpisselect)  ////第一组有选配车型信息
    {
        //有选配
        carStyle = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString();
        mIoBox = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ioBox")).toString();
        G9tmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/G9")).toString();
        Vintmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/VIN")).toString();
        for(int i=0;i<4;i++)
            Out[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/Out"+QString::number(i+1))).toInt();

        ui->pushButton_15->hide();
        //for()
        ui->pushButton_70->show();
        ui->pushButton_71->show();
        ui->pushButton_72->show();
        ui->pushButton_73->show();
        ui->pushButton_74->show();
        ui->pushButton_75->show();
        ui->pushButton_76->show();
        ui->pushButton_77->show();
        ui->pushButton_78->show();
        ui->pushButton_79->show();

        isedit = 1;
        isoption = true;
        //ui->label_isxuan->setText("yes");
        ui->stackedWidget_5->setCurrentIndex(0);
        ui->label_carname->setText(carStyle);
        ui->label_iobox->setText(mIoBox);
        ui->label_G9->setText(G9tmp);
        ui->label_VIN->setText(Vintmp);
        ui->checkBox_output1->setChecked(Out[0]);
        ui->checkBox_output2->setChecked(Out[1]);
        ui->checkBox_output3->setChecked(Out[2]);
        ui->checkBox_output4->setChecked(Out[3]);
        ui->label_isxuan->setText("Yes");
        ui->label_Lsnum->hide();
        ui->label_pronum->hide();
        ui->label_number->hide();
        ui->label_108->hide();
        ui->label_109->hide();
        ui->label_110->hide();
        ui->pushButton_10->hide();
        ui->pushButton_11->hide();
        ui->label_currentindex->hide();
        ui->label_52->hide();
        ui->pushButton_12->show();
        ui->pushButton_xuan1->setEnabled(false);
        ui->pushButton_xuan2->setEnabled(false);
        ui->pushButton_xuan3->setEnabled(false);
        ui->pushButton_xuan4->setEnabled(false);
        ui->pushButton_xuan5->setEnabled(false);

        ui->editQRCodeRule1->setEnabled(false);
        ui->editQRCodeRule2->setEnabled(false);
        ui->editQRCodeRule3->setEnabled(false);
        ui->editQRCodeRule4->setEnabled(false);
        ui->editQRCodeRule5->setEnabled(false);

        optionpagenum = 1;
        ui->label_option_curIndex->setText("1");

        //chushihua
        for(int j= 1;j<= (D_CAR_OPTION_NUM);j++)
        {
            QRCodeRuleStr[j-1] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/QRCodeRule").append(QString::number(j))).toString();
            ioBoxOption[j-1] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ioBoxOption").append(QString::number(j))).toInt();
            for(int i = 0;i<20;i++)
            {
                luoxuanlist[j-1][i]  = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/OPLuoSuanNum").append(QString::number(j)).append(QString::number(i+1))).toString();
                proxuanlist[j-1][i]  =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/OPProNum").append(QString::number(j)).append(QString::number(i+1))).toString();
                lsnumersxuanlist[j-1][i]=  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/OPLSNumber").append(QString::number(j)).append(QString::number(i+1))).toString();
                channelxuanlist[j-1][i]=  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/OPChannel").append(QString::number(j)).append(QString::number(i+1))).toString();
                taotongxuanlist[j-1][i]=  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/OPTaotong").append(QString::number(j)).append(QString::number(i+1))).toString();
                bxuannamelist[j-1][i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/bxuanname").append(QString::number(j)).append(QString::number(i+1))).toString();
                bxuancodelist[j-1][i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/bxuancode").append(QString::number(j)).append(QString::number(i+1))).toString();

                kxuannamelist[j-1][i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/kxuanname").append(QString::number(j)).append(QString::number(i+1))).toString();
                kxuancodelist[j-1][i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/kxuancode").append(QString::number(j)).append(QString::number(i+1))).toString();
                luoxuanlist2[j-1][i] = luoxuanlist[j-1][i];
                proxuanlist2[j-1][i]  = proxuanlist[j-1][i] ;
                lsnumersxuanlist2[j-1][i] = lsnumersxuanlist[j-1][i];
                channelxuanlist2[j-1][i] = channelxuanlist[j-1][i];
                taotongxuanlist2[j-1][i] = taotongxuanlist[j-1][i];
                bxuannamelist2[j-1][i] = bxuannamelist[j-1][i];
                bxuancodelist2[j-1][i] = bxuancodelist[j-1][i];
                kxuannamelist2[j-1][i] =  kxuannamelist[j-1][i];
                kxuancodelist2[j-1][i] = kxuancodelist[j-1][i];
            }
            for(int i= 0;i<4;i++)
            {
                PDMxuan_Name2[j-1][i]=  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong").append(QString::number(j)).append(QString::number(i+1))).toString();
                PDMxuan_Name[j-1][i] = PDMxuan_Name2[j-1][i];
            }
        }

        ReinitOptionState();

        for(int j= 1;j<= 5;j++)
        {
            editBox[j-1]->setText(QRCodeRuleStr[j-1]);
        }


        if(isJS)
        {
            ui->pushButton_12->hide();
            ui->stackedWidget_13->setCurrentIndex(1);
        }

    }else // 非选配  再判断是否有值
    {
        int i = 1;
        for(i = 1;i < 6;i++)
        {
            if(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i))).toInt())
            {
                //有车型  把编辑 使能

                if(isJS)
                    ui->pushButton_12->hide();
                else
                    ui->pushButton_12->show();
                //没选配
                //将5个螺栓编号 全部取出放到数组
                carStyle =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString();
                mIoBox = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ioBox")).toString();
                G9tmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/G9")).toString();
                Vintmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/VIN")).toString();
                strExecCarType = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/execCarType")).toString();
                QRtmp  = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/QR")).toString();
                for(int i=0;i<4;i++)
                {
                    Out[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/Out"+QString::number(i+1))).toInt();
                    PDM_Name[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong0").append(QString::number(i+1))).toString();
                    PDM_Name2[i] = PDM_Name[i];
                }
                for(int i =0;i<20;i++)
                {
                    luo[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LuoSuanNum").append(QString::number(i+1))).toString();
                    pro[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ProNum").append(QString::number(i+1))).toString();
                    lsnumers[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i+1))).toString();
                    taotong[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/Taotong").append(QString::number(i+1))).toString();
                    channel[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/Channel").append(QString::number(i+1))).toString();
                    luo2[i] = luo[i];
                    pro2[i] =  pro[i];
                    lsnumers2[i]  =  lsnumers[i] ;
                    //                    DTdebug() << "**************taotong***1111111***" << taotong[i];
                    bool condition=false;
                    if(Factory=="SVW3" ||Factory=="SVW2")
                    {
                        condition = (taotong[i].toInt() >4) || (taotong[i].toInt() < 1);
                    }
                    else    // if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
                    {
                        condition = (taotong[i].toInt() >8) || (taotong[i].toInt() < 1);
                    }
                    if(condition)
                    {
                        configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Taotong").append(QString::number(i+1)),"0");
                        taotong[i] = "0";
                        //                        DTdebug() << "****************3333333333333333******************************";
                    }
                    taotong2[i]  = taotong[i];
                    //                    DTdebug() << "**************taotong***222222****" << taotong[i];
                    condition=false;
                    condition = (channel[i].toInt() >4) || (channel[i].toInt() < 1);
                    if(condition)
                    {
                        configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Channel").append(QString::number(i+1)),"1");
                        channel[i] = "1";
                    }
                    channel2[i]  = channel[i];
                }

                for(int k = 0;k< ui->comboBox_2->count();k++)
                {
                    if(PDM_Name[channel[0].toInt()-1] == ui->comboBox_2->itemText(k))
                    {
                        ui->comboBox_2->setCurrentIndex(k);
                        break;
                    }
                }

                isedit = 0;
                isoption = false;
                ui->label_126->show();
                ui->comboBox_2->show();
                ui->pushButton_15->hide();
                ui->stackedWidget_5->setCurrentIndex(0);
                ui->label_isxuan->setText("no");
                ui->pushButton_14->hide();
                ui->pushButton_16->hide();

                ui->label_115->show();
                ui->label_116->show();
                ui->label_117->show();
                ui->label_119->show();
                ui->label_60->show();
                ui->pushButton_6->show();
                ui->pushButton_7->show();
                ui->lineEdit_Lsnumber->show();
                ui->lineEdit_pronum->show();
                ui->lineEdit_number->show();
                ui->pushButton_pronum_add->show();
                ui->pushButton_pronum_minus->show();
                ui->pushButton_number_add->show();
                ui->pushButton_number_minus->show();
                ui->pushButton_52->show();
                ui->pushButton_5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
                optionIscheck = false;
                ui->label_Lsnum->show();
                ui->label_pronum->show();
                ui->label_number->show();
                ui->label_108->show();
                ui->label_109->show();
                ui->label_110->show();
                ui->pushButton_10->show();
                ui->pushButton_11->show();
                ui->label_currentindex->show();
                //ui->label_52->hide();

                ui->pushButton_70->hide();
                ui->pushButton_71->hide();
                ui->pushButton_72->hide();
                ui->pushButton_73->hide();
                ui->pushButton_74->hide();
                ui->pushButton_75->hide();
                ui->pushButton_76->hide();
                ui->pushButton_77->hide();
                ui->pushButton_78->hide();
                ui->pushButton_79->hide();

                ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
                ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
                ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
                ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
                ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
                ui->label_carname->setText(carStyle);
                ui->label_iobox->setText(mIoBox);
                ui->label_G9->setText(G9tmp);
                ui->label_VIN->setText(Vintmp);
                ui->checkBox_output1->setChecked(Out[0]);
                ui->checkBox_output2->setChecked(Out[1]);
                ui->checkBox_output3->setChecked(Out[2]);
                ui->checkBox_output4->setChecked(Out[3]);
                ui->label_Lsnum->setText(luo[0]);
                ui->label_pronum->setText(pro[0]);
                ui->label_number->setText(lsnumers[0]);
                break;
            }
        }
        if(i == 6)
        {
            if(!isJS)
            {
                isedit = 0;
                isoption = false;
                ui->stackedWidget_3->setCurrentIndex(1);
                ui->pushButton_8->setEnabled(false);
                ui->pushButton_9->setEnabled(false);
                ui->pushButton_12->hide();
                ui->pushButton_16->show();
                ui->pushButton_15->show();
                ui->pushButton_14->show();
                ui->label_115->show();
                ui->label_116->show();
                ui->label_117->show();
                ui->label_119->show();
                ui->label_60->show();
                ui->pushButton_6->show();
                ui->pushButton_7->show();
                ui->lineEdit_Lsnumber->show();
                ui->lineEdit_pronum->show();
                ui->lineEdit_number->show();

                ui->pushButton_pronum_add->show();
                ui->pushButton_pronum_minus->show();
                ui->pushButton_number_add->show();
                ui->pushButton_number_minus->show();

                if(Factory != "Haima")
                {
                    ui->label_taotongg->show();
                    ui->pushButton_taotong_add->show();
                    ui->pushButton_taotong_minus->show();
                    ui->lineEdit_taotong->show();
                }
                ui->label_channel->show();
                ui->lineEdit_channel->show();
                ui->pushButton_channel_add->show();
                ui->pushButton_channel_minus->show();

                ui->pushButton_52->show();
                ui->label_126->show();
                ui->comboBox_2->show();
                ui->pushButton_5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
                ui->label_carname->clear();
                ui->label_iobox->clear();
                ui->label_G9->clear();
                ui->label_VIN->clear();
                ui->label_isxuan->clear();
                ui->label_Lsnum->clear();
                ui->label_pronum->clear();
                ui->label_number->clear();
                ui->lineEdit_taotong->setText("OFF");
                ui->lineEdit_channel->setText("1");
                ui->checkBox_output1->setChecked(false);
                ui->checkBox_output2->setChecked(false);
                ui->checkBox_output3->setChecked(false);
                ui->checkBox_output4->setChecked(false);
                optionIscheck = false;
                if(whichcar%5 == 1 && whichcar <101)
                {
                    ui->pushButton_butt2->setEnabled(false);
                    ui->pushButton_butt3->setEnabled(false);
                    ui->pushButton_butt4->setEnabled(false);
                    ui->pushButton_butt5->setEnabled(false);
                    ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                }else if(whichcar%5 == 2 && whichcar <101)
                {
                    ui->pushButton_butt1->setEnabled(false);
                    ui->pushButton_butt3->setEnabled(false);
                    ui->pushButton_butt4->setEnabled(false);
                    ui->pushButton_butt5->setEnabled(false);
                    ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                }
                else if(whichcar%5 == 3 && whichcar <101)
                {
                    ui->pushButton_butt1->setEnabled(false);
                    ui->pushButton_butt2->setEnabled(false);
                    ui->pushButton_butt4->setEnabled(false);
                    ui->pushButton_butt5->setEnabled(false);
                    ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                }else if(whichcar%5 == 4 && whichcar <101)
                {
                    ui->pushButton_butt1->setEnabled(false);
                    ui->pushButton_butt2->setEnabled(false);
                    ui->pushButton_butt3->setEnabled(false);
                    ui->pushButton_butt5->setEnabled(false);
                    ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                }else if(whichcar%5 == 0 && whichcar <101)
                {
                    ui->pushButton_butt1->setEnabled(false);
                    ui->pushButton_butt2->setEnabled(false);
                    ui->pushButton_butt3->setEnabled(false);
                    ui->pushButton_butt4->setEnabled(false);
                    ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                }
                ui->pushButton_5->setEnabled(true);  //选配按钮

                ui->stackedWidget_5->setCurrentIndex(1);
            }
            else
            {
                //技术操作员
                //                if(whichcar == 1 || whichcar == 6 || whichcar == 11 || whichcar == 16)
                //                {
                //                    ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                //                }else if(whichcar == 2 || whichcar == 7 || whichcar == 12 || whichcar == 17)
                //                {
                //                    ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                //                }
                //                else if(whichcar == 3 || whichcar == 8 || whichcar == 13 || whichcar == 18)
                //                {
                //                    ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");

                //                }else if(whichcar == 4 || whichcar == 9 || whichcar == 14 || whichcar == 19)
                //                {
                //                    ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");

                //                }else if(whichcar == 5 || whichcar == 10 || whichcar == 15 || whichcar == 20)
                //                {
                //                    ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                //                }
                //                ui->pushButton_12->hide();
            }

        }


    }
    previouswhichar = whichcar;
    delete configIniRead;
}
void Newconfiginfo::on_pushButton_butt1_clicked()
{
    //butt1 即第一个车型选中之后效果
    ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
    int toolpagenum = pagenum -1;
    for(int i = 0;i< (CAR_TYPE_AMOUNT/5);i++)
    {
        if(toolpagenum == i)
        {
            whichcar = toolpagenum*5 + 1;  //算出当前是哪辆车被选中
            break;
        }

    }
    buttclicked();


}
void Newconfiginfo::on_pushButton_butt2_clicked()
{
    //butt2 即第2个车型选中之后效果
    ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
    int toolpagenum = pagenum -1;
    for(int i = 0;i< (CAR_TYPE_AMOUNT/5);i++)
    {

        if(toolpagenum == i)
        {
            whichcar = toolpagenum*5 + 2;  //算出当前是哪辆车被选中
            break;
        }

    }
    buttclicked();
}

void Newconfiginfo::on_pushButton_butt3_clicked()
{
    //butt3 即第3个车型选中之后效果
    ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
    int toolpagenum = pagenum -1;
    for(int i = 0;i< (CAR_TYPE_AMOUNT/5);i++)
    {

        if(toolpagenum == i)
        {
            whichcar = toolpagenum*5 + 3;  //算出当前是哪辆车被选中
            break;
        }

    }
    buttclicked();
}

void Newconfiginfo::on_pushButton_butt4_clicked()
{
    //butt4 即第4个车型选中之后效果
    ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
    int toolpagenum = pagenum -1;
    for(int i = 0;i< (CAR_TYPE_AMOUNT/5);i++)
    {

        if(toolpagenum == i)
        {
            whichcar = toolpagenum*5 + 4;  //算出当前是哪辆车被选中
            break;
        }

    }
    buttclicked();

}

void Newconfiginfo::on_pushButton_butt5_clicked()
{
    //butt5 即第5个车型选中之后效果
    ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
    int toolpagenum = pagenum -1;
    for(int i = 0;i< (CAR_TYPE_AMOUNT/5);i++)
    {

        if(toolpagenum == i)
        {
            whichcar = toolpagenum*5 + 5;  //算出当前是哪辆车被选中
            break;
        }

    }
    buttclicked();
}
void Newconfiginfo::updownReadOperate(int judges)
{
    //up  currentindex值改变对应操作
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    //察看  up down
    if(!isedit)
    {
        if(judges)
        {
            currentpages = ui->label_currentindex->text().toInt() + 1;
            if(currentpages == (D_BOLTNUM+1))
            {
                currentpages = 1;
            }
        }
        else
        {
            currentpages = ui->label_currentindex->text().toInt() - 1;
            if(currentpages == 0)
            {
                currentpages = D_BOLTNUM;
            }
        }
        ui->label_currentindex->setText(QString::number(currentpages));
        ui->label_carname->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString());
        ui->label_iobox->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ioBox")).toString());
        ui->label_G9->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/G9")).toString());
        ui->label_VIN->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/VIN")).toString());

        ui->label_Lsnum->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LuoSuanNum").append(QString::number(currentpages))).toString());
        ui->label_pronum->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ProNum").append(QString::number(currentpages))).toString());
        ui->label_number->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(currentpages))).toString());


    }else
    {

        if(judges)
        {
            currentpages = ui->label_74->text().toInt() + 1;
            if(currentpages == (D_BOLTNUM+1))
            {
                currentpages = 1;
            }
        }
        else
        {
            currentpages = ui->label_74->text().toInt() - 1;
            if(currentpages == 0)
            {
                currentpages = D_BOLTNUM;
            }
        }
        ui->label_ioBoxOption->setText(QString::number(ioBoxOption[whichoption-1]));
        ui->label_74->setText(QString::number(currentpages));
        ui->label_75->setText(luoxuanlist2[whichoption-1][currentpages-1]);
        ui->label_76->setText(proxuanlist2[whichoption-1][currentpages-1]);
        ui->label_77->setText(lsnumersxuanlist2[whichoption-1][currentpages-1]);
        //        ui->lineEdit_channel_2->setText(channelxuanlist2[whichoption-1][currentpages-1]);
        //        ui->lineEdit_taotong_2->setText(taotongxuanlist2[whichoption-1][currentpages-1]);
        //        ui->label_78->setText(pdmxuanlist2[whichoption-1]);
        //QString pathpdm = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong").append(QString::number(whichoption)).append(QString::number(currentpages))).toString();
        // DTdebug() << "";
        //ui->label_78->setText(configIniRead->value(pathpdm.append("/pdmname")).toString());
    }


    delete   configIniRead;
}
void Newconfiginfo::on_pushButton_10_clicked()
{
    // read up
    updownReadOperate(1);

}

void Newconfiginfo::on_pushButton_11_clicked()
{
    //read down
    updownReadOperate(0);
}

void Newconfiginfo::on_pushButton_5_clicked()
{
    //选配按钮
    if(Factory == "SVW3"||Factory=="Ningbo"|| Factory=="SVW2" || Factory=="BYDSZ"  || Factory=="NOVAT" || Factory == "AQCHERY" || Factory == "KFCHERY")
    {
        if(optionIscheck)
        {
            isedit = 0;
            ui->label_115->show();
            ui->label_116->show();
            ui->label_117->show();
            ui->label_119->show();
            ui->label_60->show();
            ui->pushButton_6->show();
            ui->pushButton_7->show();
            ui->lineEdit_Lsnumber->show();
            ui->lineEdit_pronum->show();
            ui->lineEdit_number->show();
            if(Factory == "Haima")
            {
                ui->label_taotongg->show();
                ui->pushButton_taotong_add->show();
                ui->pushButton_taotong_minus->show();
                ui->lineEdit_taotong->show();
            }
            if(Factory == "SVW2")
            {
                ui->editQRCodeRule1->show();
                ui->editQRCodeRule2->show();
                ui->editQRCodeRule3->show();
                ui->editQRCodeRule4->show();
                ui->editQRCodeRule5->show();
            }
            ui->label_channel->show();
            ui->lineEdit_channel->show();
            ui->pushButton_channel_add->show();
            ui->pushButton_channel_minus->show();

            ui->pushButton_pronum_add->show();
            ui->pushButton_pronum_minus->show();
            ui->pushButton_number_add->show();
            ui->pushButton_number_minus->show();
            ui->pushButton_52->show();
            ui->label_126->show();
            ui->comboBox_2->show();

            ui->editQRCodeRule1->setEnabled(false);
            ui->editQRCodeRule2->setEnabled(false);
            ui->editQRCodeRule3->setEnabled(false);
            ui->editQRCodeRule4->setEnabled(false);
            ui->editQRCodeRule5->setEnabled(false);

            ui->pushButton_xuan1->setEnabled(false);
            ui->pushButton_xuan2->setEnabled(false);
            ui->pushButton_xuan3->setEnabled(false);
            ui->pushButton_xuan4->setEnabled(false);
            ui->pushButton_xuan5->setEnabled(false);
            ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
            ui->pushButton_70->hide();
            ui->pushButton_71->hide();
            ui->pushButton_72->hide();
            ui->pushButton_73->hide();
            ui->pushButton_74->hide();
            ui->pushButton_75->hide();
            ui->pushButton_76->hide();
            ui->pushButton_77->hide();
            ui->pushButton_78->hide();
            ui->pushButton_79->hide();

            optionIscheck = false;

        }
        else
        {
            isedit = 1;
            isxuanlook = 0;
            ui->label_115->hide();
            ui->label_116->hide();
            ui->label_117->hide();
            ui->label_119->hide();
            ui->label_60->hide();
            ui->pushButton_6->hide();
            ui->pushButton_7->hide();
            ui->lineEdit_Lsnumber->hide();
            ui->lineEdit_pronum->hide();
            ui->lineEdit_number->hide();
            ui->label_taotongg->hide();
            ui->pushButton_taotong_add->hide();
            ui->pushButton_taotong_minus->hide();
            ui->lineEdit_taotong->hide();
            ui->label_channel->hide();
            ui->lineEdit_channel->hide();
            ui->pushButton_channel_add->hide();
            ui->pushButton_channel_minus->hide();
            ui->pushButton_pronum_add->hide();
            ui->pushButton_pronum_minus->hide();
            ui->pushButton_number_add->hide();
            ui->pushButton_number_minus->hide();
            ui->pushButton_52->hide();
            ui->pushButton_xuan1->setEnabled(true);
            ui->pushButton_xuan2->setEnabled(true);
            ui->pushButton_xuan3->setEnabled(true);
            ui->pushButton_xuan4->setEnabled(true);
            ui->pushButton_xuan5->setEnabled(true);

            ui->editQRCodeRule1->setEnabled(true);
            ui->editQRCodeRule2->setEnabled(true);
            ui->editQRCodeRule3->setEnabled(true);
            ui->editQRCodeRule4->setEnabled(true);
            ui->editQRCodeRule5->setEnabled(true);

            ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            ui->pushButton_5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
            ui->pushButton_70->show();
            ui->pushButton_71->show();
            ui->pushButton_72->show();
            ui->pushButton_73->show();
            ui->pushButton_74->show();
            ui->pushButton_75->show();
            ui->pushButton_76->show();
            ui->pushButton_77->show();
            ui->pushButton_78->show();
            ui->pushButton_79->show();
            ui->label_126->hide();
            ui->comboBox_2->hide();
            QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);

            //D_CAR_OPTION_NUM
            for(int i = 0;i<5;i++)
            {

                if(lsnumersxuanlist[i][0].toInt())
                {
                    if(i == 0)
                    {
                        //DTdebug() << "fsd";
                        ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    }
                    else if(i == 1)
                        ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    else if(i == 2)
                        ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    else if(i == 3)
                        ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    else if(i == 4)
                        ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");

                }else
                {
                    if(i == 0)
                        ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else if(i == 1)
                        ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else if(i == 2)
                        ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else if(i == 3)
                        ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else if(i == 4)
                        ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");

                }
            }
            delete  configIniRead;
            optionIscheck = true;
        }
    }
}
void Newconfiginfo::updownWriteOperate(int judges)
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    //编辑  up   down
    if(isedit == 1)
    {
        int tmpPage = (CAR_TYPE_AMOUNT/5);
        if(judges)
        {
            currentpages = ui->label_51->text().toInt() + 1;
            if(currentpages == (tmpPage+1))
            {
                currentpages = 1;
            }
        }
        else
        {
            currentpages = ui->label_51->text().toInt() - 1;
            if(currentpages == 0)
            {
                currentpages = tmpPage;
            }
        }

        ui->label_51->setText(QString::number(currentpages));
        ui->lineEdit_ioBoxOption->setText(QString::number(ioBoxOption[whichoption-1]));
        ui->lineEdit_xuanLsnum->setText(luoxuanlist2[whichoption-1][currentpages-1]);
        if (proxuanlist2[whichoption-1][currentpages-1] == "")
            ui->lineEdit_xuanpronum->setText("0");
        else ui->lineEdit_xuanpronum->setText(proxuanlist2[whichoption-1][currentpages-1]);
        if (lsnumersxuanlist2[whichoption-1][currentpages-1] == "")
            ui->lineEdit_xuannumber->setText("0");
        else ui->lineEdit_xuannumber->setText(lsnumersxuanlist2[whichoption-1][currentpages-1]);
        if (channelxuanlist2[whichoption-1][currentpages-1] == "")
            ui->lineEdit_channel_2->setText("1");
        else ui->lineEdit_channel_2->setText(channelxuanlist2[whichoption-1][currentpages-1]);
        if (taotongxuanlist2[whichoption-1][currentpages-1] == ""||taotongxuanlist2[whichoption-1][currentpages-1] == "0")
            ui->lineEdit_taotong_2->setText("OFF");
        else ui->lineEdit_taotong_2->setText(taotongxuanlist2[whichoption-1][currentpages-1]);
        for(int k = 0;k< ui->comboBox_3->count();k++)
        {
            if(PDMxuan_Name2[whichoption-1][ui->lineEdit_channel_2->text().toInt()-1] == ui->comboBox_3->itemText(k))
            {
                ui->comboBox_3->setCurrentIndex(k);
                break;
            }
        }
    }
    else
    {
        if(judges)
        {
            currentpages = ui->label_119->text().toInt() + 1;
            if(currentpages == (D_BOLTNUM+1))
            {
                currentpages = 1;
            }
        }
        else
        {
            currentpages = ui->label_119->text().toInt() - 1;
            if(currentpages == 0)
            {
                currentpages = D_BOLTNUM;
            }
        }
        ui->label_119->setText(QString::number(currentpages));
        ui->lineEdit_carname->setText(carStyle);
        ui->lineEdit_iobox->setText(mIoBox);
        ui->lineEdit_G9->setText(G9tmp);
        ui->lineEdit_VIN->setText(Vintmp);
        ui->editExecCarType->setText(strExecCarType);
        ui->checkBox_output1->setChecked(Out[0]);
        ui->checkBox_output2->setChecked(Out[1]);
        ui->checkBox_output3->setChecked(Out[2]);
        ui->checkBox_output4->setChecked(Out[3]);

        ui->lineEdit_Lsnumber->setText(luo2[currentpages-1]);
        if (pro2[currentpages-1] == "")
            ui->lineEdit_pronum->setText("0");
        else
            ui->lineEdit_pronum->setText(pro2[currentpages-1]);
        if (lsnumers2[currentpages-1] == "")
            ui->lineEdit_number->setText("0");
        else
            ui->lineEdit_number->setText(lsnumers2[currentpages-1]);
        if(taotong2[currentpages-1] == "0" || taotong2[currentpages-1] == "")
            ui->lineEdit_taotong->setText("OFF");
        else
            ui->lineEdit_taotong->setText(taotong2[currentpages-1]);
        if (channel2[currentpages-1] == "")
            ui->lineEdit_channel->setText("1");
        else
            ui->lineEdit_channel->setText(channel2[currentpages-1]);

        for(int k = 0;k< ui->comboBox_2->count();k++)
        {
            if(PDM_Name2[channel2[currentpages-1].toInt()-1] == ui->comboBox_2->itemText(k))
            {
                ui->comboBox_2->setCurrentIndex(k);
                break;
            }
        }

    }
    delete configIniRead;
}
void Newconfiginfo::on_pushButton_6_clicked()
{
    // edit up
    // ui->label_119->text().toInt();
    //上下滑动 取值
    carStyle = ui->lineEdit_carname->text();
    mIoBox = ui->lineEdit_iobox->text();
    G9tmp = ui->lineEdit_G9->text();
    Vintmp = ui->lineEdit_VIN->text();
    strExecCarType = ui->editExecCarType->text();
    Out[0]= ui->checkBox_output1->isChecked();
    Out[1]= ui->checkBox_output2->isChecked();
    Out[2]= ui->checkBox_output3->isChecked();
    Out[3]= ui->checkBox_output4->isChecked();
    luo2[ui->label_119->text().toInt()-1] = ui->lineEdit_Lsnumber->text();
    pro2[ui->label_119->text().toInt()-1] = ui->lineEdit_pronum->text();
    lsnumers2[ui->label_119->text().toInt()-1] = ui->lineEdit_number->text();
    if(ui->lineEdit_taotong->text() != "OFF")
        taotong2[ui->label_119->text().toInt()-1] = ui->lineEdit_taotong->text();
    else
        taotong2[ui->label_119->text().toInt()-1] = "0";
    channel2[ui->label_119->text().toInt()-1] = ui->lineEdit_channel->text();
    PDM_Name2[channel2[ui->label_119->text().toInt()-1].toInt()-1]=ui->comboBox_2->currentText();
    updownWriteOperate(1);

}

void Newconfiginfo::on_pushButton_7_clicked()
{
    //edit down
    carStyle = ui->lineEdit_carname->text();
    mIoBox = ui->lineEdit_iobox->text();
    G9tmp = ui->lineEdit_G9->text();
    Vintmp = ui->lineEdit_VIN->text();
    strExecCarType = ui->editExecCarType->text() ;
    Out[0]= ui->checkBox_output1->isChecked();
    Out[1]= ui->checkBox_output2->isChecked();
    Out[2]= ui->checkBox_output3->isChecked();
    Out[3]= ui->checkBox_output4->isChecked();
    luo2[ui->label_119->text().toInt()-1] = ui->lineEdit_Lsnumber->text();
    pro2[ui->label_119->text().toInt()-1] = ui->lineEdit_pronum->text();
    lsnumers2[ui->label_119->text().toInt()-1] = ui->lineEdit_number->text();
    if(ui->lineEdit_taotong->text() != "OFF")
        taotong2[ui->label_119->text().toInt()-1] = ui->lineEdit_taotong->text();
    else
        taotong2[ui->label_119->text().toInt()-1] = "0";
    channel2[ui->label_119->text().toInt()-1] = ui->lineEdit_channel->text();
    updownWriteOperate(0);
}

void Newconfiginfo::on_pushButton_14_clicked()
{
    DTdebug() << SaveWhat << isedit ;
    //edit 保存
    //    ui->stackedWidget_5->setCurrentIndex(0);
    ui->stackedWidget_3->setCurrentIndex(3);

    //    ui->stackedWidget_3->setCurrentIndex(0);
    ui->pushButton_5->setEnabled(false);


    if(SaveWhat == "" || SaveWhat == "de" || SaveWhat == "deall")
    {

        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();

    }
    else if((SaveWhat == "desingle") || (SaveWhat == "saveself"))
    {
        if(isedit == 0)
        {
            //非选配信息  保存
            carStyle = ui->lineEdit_carname->text();
            mIoBox = ui->lineEdit_iobox->text();
            G9tmp = ui->lineEdit_G9->text();
            Vintmp = ui->lineEdit_VIN->text();
            strExecCarType = ui->editExecCarType->text();
            Out[0]= ui->checkBox_output1->isChecked();
            Out[1]= ui->checkBox_output2->isChecked();
            Out[2]= ui->checkBox_output3->isChecked();
            Out[3]= ui->checkBox_output4->isChecked();
            luo2[ui->label_119->text().toInt()-1] = ui->lineEdit_Lsnumber->text();
            pro2[ui->label_119->text().toInt()-1] = ui->lineEdit_pronum->text();
            lsnumers2[ui->label_119->text().toInt()-1] = ui->lineEdit_number->text();

            bool condition = false;
            if(Factory=="SVW3" ||Factory=="SVW2")
            {
                condition = (ui->lineEdit_taotong->text().toInt()>4) || (ui->lineEdit_taotong->text().toInt()<1);
            }
            else //if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
            {
                condition = (ui->lineEdit_taotong->text().toInt()>8) || (ui->lineEdit_taotong->text().toInt()<1);
            }
            if(condition)
                taotong2[ui->label_119->text().toInt()-1] = "0";
            else
                taotong2[ui->label_119->text().toInt()-1] = ui->lineEdit_taotong->text();


            condition = false;
            condition = (ui->lineEdit_channel->text().toInt()>4) || (ui->lineEdit_channel->text().toInt()<1);
            if(condition)
                channel2[ui->label_119->text().toInt()-1] = "1";
            else
                channel2[ui->label_119->text().toInt()-1] = ui->lineEdit_channel->text();

            PDM_Name2[ui->lineEdit_channel->text().toInt()-1]=ui->comboBox_2->currentText();
            //DTdebug() << ui->label_119->text().toInt()-1;
            QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/carcx"),ui->lineEdit_carname->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/ioBox"),ui->lineEdit_iobox->text());
//            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/QRCodeRule"),ui->editQRCodeRule1->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/G9"),ui->lineEdit_G9->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/VIN"),ui->lineEdit_VIN->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out1"),ui->checkBox_output1->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out2"),ui->checkBox_output2->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out3"),ui->checkBox_output3->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out4"),ui->checkBox_output4->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/execCarType"),ui->editExecCarType->text());

            for(int i = 0;i< 4; i++)
            {
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong0").append(QString::number(i+1)),PDM_Name2[i]);
                PDM_Name[i] = PDM_Name2[i];
            }
            for(int i = 0;i< 20;i++)
            {
                luo[i] = luo2[i];
                pro[i] =  pro2[i];
                lsnumers[i]  =  lsnumers2[i] ;
                taotong[i] = taotong2[i];
                channel[i] = channel2[i];
                bool condition = false;
                if(Factory=="SVW3" ||Factory=="SVW2")
                {
                    condition = (taotong[i].toInt() >4) || (taotong[i].toInt() < 1);
                }
                else   // if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
                {
                    condition = (taotong[i].toInt() >8) || (taotong[i].toInt() < 1);
                }
                if( condition )
                {
                    //configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Taotong").append(QString::number(i+1)),"0");
                    taotong[i] = "0";
                }

                condition = false;
                condition = (channel[i].toInt() >4) || (channel[i].toInt() < 1);
                if( condition )
                {
                    channel[i] = "1";
                }
            }
            int counts = 0;
            for(int i = 0;i<20;i++)
            {
                if(lsnumers2[i].toInt())
                {
                    counts++;
                }
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/LuoSuanNum").append(QString::number(i+1)),luo[i]);
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/ProNum").append(QString::number(i+1)),pro[i]);
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i+1)),lsnumers[i]);
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Taotong").append(QString::number(i+1)),taotong[i]);
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Channel").append(QString::number(i+1)),channel[i]);
            }

            for(int j = 1;j <= D_CAR_OPTION_NUM;j++)
            {
                for(int i = 0;i< 20;i++)
                {
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPLuoSuanNum").append(QString::number(j)).append(QString::number(i+1)),"0");
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPProNum").append(QString::number(j)).append(QString::number(i+1)),"0");
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPLSNumber").append(QString::number(j)).append(QString::number(i+1)),"0");

                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/bxuanname").append(QString::number(j)).append(QString::number(i+1)),"");
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/bxuancode").append(QString::number(j)).append(QString::number(i+1)),"");
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/kxuanname").append(QString::number(j)).append(QString::number(i+1)),"");
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/kxuancode").append(QString::number(j)).append(QString::number(i+1)),"");

                }
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong").append(QString::number(j)),"");


            }

            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/IsOptionalStation"),"0");

            //存储几条 有效配置信息
            if(!counts)
            {
                if(whichcar%5 == 1 && whichcar <(CAR_TYPE_AMOUNT+1))
                {
                    ui->pushButton_butt1->setText("");
                }else if(whichcar%5 == 2 && whichcar <(CAR_TYPE_AMOUNT+1))
                {
                    ui->pushButton_butt2->setText("");
                }
                else if(whichcar%5 == 3 && whichcar <(CAR_TYPE_AMOUNT+1))
                {
                    ui->pushButton_butt3->setText("");
                }else if(whichcar%5 == 4 && whichcar <(CAR_TYPE_AMOUNT+1))
                {
                    ui->pushButton_butt4->setText("");
                }else if(whichcar%5 == 0 && whichcar <(CAR_TYPE_AMOUNT+1))
                {
                    ui->pushButton_butt5->setText("");
                }
            }
            //            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/IsSelect"),QString::number(counts));

            // QMessageBox::warning(this,tr("结果"),tr("保存成功!"),QMessageBox::Yes);
            ui->stackedWidget_5->setCurrentIndex(0);
            ui->label_carname->setText(carStyle);
            ui->label_iobox->setText(mIoBox);
            ui->label_VIN->setText(Vintmp);
            ui->checkBox_output1->setChecked(Out[0]);
            ui->checkBox_output2->setChecked(Out[1]);
            ui->checkBox_output3->setChecked(Out[2]);
            ui->checkBox_output4->setChecked(Out[3]);
            ui->label_G9->setText(G9tmp);
            ui->label_Lsnum->setText(luo[0]);
            ui->label_pronum->setText(pro[0]);
            ui->label_number->setText(lsnumers[0]);
            ui->label_isxuan->setText("no");

            ui->label_Lsnum->show();
            ui->label_Lsnum->show();
            ui->label_pronum->show();
            ui->label_number->show();
            ui->label_108->show();
            ui->label_109->show();
            ui->label_110->show();
            ui->label_52->show();
            ui->pushButton_10->show();
            ui->pushButton_11->show();
            ui->label_currentindex->show();
            delete configIniRead;
            backShow();
            isoption = false;

        }
        else if(isedit == 1)
        {
            //  DTdebug() << "llllllllllllllllllsdf";
            QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/carcx"),ui->lineEdit_carname->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/ioBox"),ui->lineEdit_iobox->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/G9"),ui->lineEdit_G9->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/VIN"),ui->lineEdit_VIN->text());
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out1"),ui->checkBox_output1->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out2"),ui->checkBox_output2->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out3"),ui->checkBox_output3->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Out4"),ui->checkBox_output4->isChecked()?1:0);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/execCarType"),ui->editExecCarType->text());

            carStyle = ui->lineEdit_carname->text();
            mIoBox = ui->lineEdit_iobox->text();
            G9tmp = ui->lineEdit_G9->text();
            Vintmp = ui->lineEdit_VIN->text();
            strExecCarType = ui->editExecCarType->text() ;
            Out[0]= ui->checkBox_output1->isChecked();
            Out[1]= ui->checkBox_output2->isChecked();
            Out[2]= ui->checkBox_output3->isChecked();
            Out[3]= ui->checkBox_output4->isChecked();
            ui->label_carname->setText(carStyle);
            ui->label_iobox->setText(mIoBox);
            ui->label_VIN->setText(Vintmp);
            ui->label_G9->setText(G9tmp);
            int counts = 0;
            int countnum = 0;
            for(int j = 1;j <= D_CAR_OPTION_NUM;j++)
            {
                DTdebug() << j  << QRCodeRuleStr[j-1];
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/QRCodeRule").append(QString::number(j)),QRCodeRuleStr[j-1]);

                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/ioBoxOption").append(QString::number(j)), QString::number(ioBoxOption[j-1]));

                for(int i = 0;i< 20;i++)
                {
                    if(lsnumersxuanlist2[j-1][i].toInt())
                    {
                        counts++;
                    }

                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPLuoSuanNum").append(QString::number(j)).append(QString::number(i+1)),luoxuanlist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPProNum").append(QString::number(j)).append(QString::number(i+1)),proxuanlist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPLSNumber").append(QString::number(j)).append(QString::number(i+1)),lsnumersxuanlist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPChannel").append(QString::number(j)).append(QString::number(i+1)),channelxuanlist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OPTaotong").append(QString::number(j)).append(QString::number(i+1)),taotongxuanlist2[j-1][i]);

                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/bxuanname").append(QString::number(j)).append(QString::number(i+1)),bxuannamelist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/bxuancode").append(QString::number(j)).append(QString::number(i+1)),bxuancodelist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/kxuanname").append(QString::number(j)).append(QString::number(i+1)),kxuannamelist2[j-1][i]);
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/kxuancode").append(QString::number(j)).append(QString::number(i+1)),kxuancodelist2[j-1][i]);

                }
                for(int i= 0;i<4;i++)
                {
                    configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong").append(QString::number(j).append(QString::number(i+1))),PDMxuan_Name2[j-1][i]);
                }
                //                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong").append(QString::number(j)),pdmxuanlist[j-1]);
                //                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/OptionInfoNum").append(QString::number(j)),QString::number(counts));
                if(!counts)
                    countnum++;
            }
            if(countnum == 20)
            {
                // int whichbuttons = 0;

                // if()
                buttonbox[whichcar -((pagenum-1)*5+1)]->setText("");
                //                if(pagenum == 1)
                //                {
                //                    buttonbox[whichcar-1]->setText("");
                //                }
                //                else if(pagenum == 2)
                //                {
                //                    buttonbox[whichcar-6]->setText("");
                //                }
                //                else if(pagenum == 3)
                //                {
                //                    buttonbox[whichcar-11]->setText("");
                //                }
                //                else if(pagenum == 4)
                //                {
                //                    buttonbox[whichcar-16]->setText("");
                //                }
                ui->label_carname->clear();
                ui->label_iobox->clear();
                ui->label_G9->clear();
                ui->label_VIN->clear();
                ui->label_isxuan->clear();

                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/IsOptionalStation"),"0");
            }else
            {
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/IsOptionalStation"),"1");
            }


            //            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/IsSelect"),"0");



            for(int i = 0;i<20;i++)
            {

                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/LuoSuanNum").append(QString::number(i+1)),"0");
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/ProNum").append(QString::number(i+1)),"0");
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i+1)),"0");
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Taotong").append(QString::number(i+1)),"0");
                configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/Channel").append(QString::number(i+1)),"1");
            }
            //            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/pdmyinyong"),"");
            SaveWhat = "";
            delete configIniRead;
            ui->pushButton_xuan1->setEnabled(false);
            ui->pushButton_xuan2->setEnabled(false);
            ui->pushButton_xuan3->setEnabled(false);
            ui->pushButton_xuan4->setEnabled(false);
            ui->pushButton_xuan5->setEnabled(false);

            ui->editQRCodeRule1->setEnabled(false);
            ui->editQRCodeRule2->setEnabled(false);
            ui->editQRCodeRule3->setEnabled(false);
            ui->editQRCodeRule4->setEnabled(false);
            ui->editQRCodeRule5->setEnabled(false);

            backShow();
            isoptionsaved = true;
            ui->label_Lsnum->hide();
            ui->label_Lsnum->hide();
            ui->label_pronum->hide();
            ui->label_number->hide();
            ui->label_isxuan->setText("yes");
            ui->label_108->hide();
            ui->label_109->hide();
            ui->label_110->hide();
            ui->label_52->hide();
            ui->pushButton_10->hide();
            ui->pushButton_11->hide();
            ui->label_currentindex->hide();
            isoption = true; //选配与否

        }

        SaveWhat = "";

    }



}

void Newconfiginfo::receiveDesignle(bool tmp)
{
    DTdebug() << SaveWhat << tmp ;
    if(SaveWhat == "de")
    {
        if(tmp)
        {
            SaveWhat = "desingle";
            on_pushButton_14_clicked();
        }
        else
        {
            SaveWhat = "";
            QSettings *configIniRead = new QSettings("/config.ini",QSettings::IniFormat);
            carStyle =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString();
            mIoBox = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ioBox")).toString();
            G9tmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/G9")).toString();
            Vintmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/VIN")).toString();
            strExecCarType = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/execCarType")).toString();
            for(int i=0;i<4;i++)
                Out[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/Out"+QString::number(i+1))).toInt();
            for(int i =0;i<20;i++)
            {
                luo[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LuoSuanNum").append(QString::number(i+1))).toString();
                pro[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ProNum").append(QString::number(i+1))).toString();
                lsnumers[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i+1))).toString();
            }
            delete configIniRead;
            // on_pushButton_15_clicked();
        }

    }
    else if(SaveWhat == "")
    {

        if(tmp)
        {
            SaveWhat = "saveself";
            on_pushButton_14_clicked();

        }
        else
        {
            SaveWhat = "";
            if(!isedit)
            {
                QSettings *configIniRead = new QSettings("/config.ini",QSettings::IniFormat);
                carStyle =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString();
                mIoBox = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ioBox")).toString();
                G9tmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/G9")).toString();
                Vintmp = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/VIN")).toString();
                strExecCarType = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/execCarType")).toString();

                for(int i=0;i<4;i++)
                    Out[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/Out"+QString::number(i+1))).toInt();
                for(int i =0;i<20;i++)
                {
                    luo[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LuoSuanNum").append(QString::number(i+1))).toString();
                    pro[i] =  configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/ProNum").append(QString::number(i+1))).toString();
                    lsnumers[i] = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i+1))).toString();
                }
                delete configIniRead;
                backShow();
            }else
            {
                if(whichoption >=1 && whichoption < 6)
                {
                    for(int j = 0;j< 20;j++)
                    {
                        //luoxuanlist2[whichoption-1][j] = luoxuanlist[whichoption-1][j];
                        //proxuanlist2[whichoption-1][j] = proxuanlist[whichoption-1][j];

                        lsnumersxuanlist2[whichoption-1][j] = lsnumersxuanlist[whichoption-1][j];
                        channelxuanlist2[whichoption-1][j] = channelxuanlist[whichoption-1][j];
                        taotongxuanlist2[whichoption-1][j] = taotongxuanlist[whichoption-1][j];
                        bxuannamelist2[whichoption-1][j] = bxuannamelist[whichoption-1][j];
                        bxuancodelist2[whichoption-1][j] = bxuancodelist[whichoption-1][j];
                        kxuannamelist2[whichoption-1][j] = kxuannamelist[whichoption-1][j];
                        kxuancodelist2[whichoption-1][j] = kxuancodelist[whichoption-1][j];

                    }
                    for(int j =0;j<4;j++)
                    {
                        PDMxuan_Name2[whichoption-1][j] = PDMxuan_Name[whichoption-1][j];
                    }
                    backShow();
                    //                    pdmxuanlist2[whichoption-1]=  pdmxuanlist[whichoption-1];
                    ui->pushButton_xuan1->setEnabled(false);
                    ui->pushButton_xuan2->setEnabled(false);
                    ui->pushButton_xuan3->setEnabled(false);
                    ui->pushButton_xuan4->setEnabled(false);
                    ui->pushButton_xuan5->setEnabled(false);

                    ui->editQRCodeRule1->setEnabled(false);
                    ui->editQRCodeRule2->setEnabled(false);
                    ui->editQRCodeRule3->setEnabled(false);
                    ui->editQRCodeRule4->setEnabled(false);
                    ui->editQRCodeRule5->setEnabled(false);

                    isoptionsaved = true;

                }
            }

        }
        ui->stackedWidget_5->setCurrentIndex(0);
        SaveWhat ="";
        if(buttonbox[whichcar-(pagenum-1)*5-1]->text() == "")
        {
            ui->pushButton_12->hide();
        }
    }
    else if(SaveWhat == "save2")
    {
        if(tmp)
        {
            SaveWhat = "saveself";
            on_pushButton_25_clicked();
        }
        else
        {
            SaveWhat = "";

        }
        SaveWhat ="";
    }
    else if(SaveWhat == "deall")
    {
        if(tmp)
        {
            ui->lineEdit_carname->clear();
            ui->lineEdit_iobox->clear();
            ui->lineEdit_G9->clear();
            ui->lineEdit_VIN->clear();
            ui->editExecCarType->clear();
            ui->lineEdit_Lsnumber->setText("");
            ui->lineEdit_pronum->setText("0");
            ui->lineEdit_number->setText("0");
            ui->lineEdit_taotong->setText("OFF");
            ui->lineEdit_channel->setText("1");
            ui->label_carname->clear();
            ui->label_iobox->clear();
            ui->label_G9->clear();
            ui->label_VIN->clear();
            ui->label_Lsnum->clear();
            ui->label_number->clear();
            ui->label_isxuan->clear();

            ui->comboBox_2->setCurrentIndex(0);
            SaveWhat = "saveself";
            carStyle="";
            mIoBox = "";
            G9tmp="";
            Vintmp="";
            strExecCarType = "" ;
            for(int i=0;i<4;i++)
                Out[i]= false;

            for(int k = 1;k < 6;k++) //5个程序号 删除
            {
                luo2[k-1]="0";
                pro2[k-1]="0";
                lsnumers2[k-1]="0";
                taotong2[k-1] = "0";
                channel2[k-1] = "1";
            }
            on_pushButton_14_clicked();

        }
        SaveWhat ="";
        if(buttonbox[whichcar-(pagenum-1)*5-1]->text() == "")
        {
            ui->pushButton_12->hide();
        }

    }
    else if(SaveWhat == "OPCANCEL")
    {
        if(tmp)
        {
            SaveWhat = "saveself";
            on_pushButton_25_clicked();

        }else
        {
            for(int j = 0;j< 20;j++)
            {

                luoxuanlist2[whichoption-1][j] = luoxuanlist[whichoption-1][j];
                proxuanlist2[whichoption-1][j] = proxuanlist[whichoption-1][j];
                lsnumersxuanlist2[whichoption-1][j] = lsnumersxuanlist[whichoption-1][j];
                channelxuanlist2[whichoption-1][j] = channelxuanlist[whichoption-1][j];
                taotongxuanlist2[whichoption-1][j] = taotongxuanlist[whichoption-1][j];
                bxuannamelist2[whichoption-1][j] = bxuannamelist[whichoption-1][j];
                bxuancodelist2[whichoption-1][j] = bxuancodelist[whichoption-1][j];
                kxuannamelist2[whichoption-1][j] = kxuannamelist[whichoption-1][j];
                kxuancodelist2[whichoption-1][j] = kxuancodelist[whichoption-1][j];

            }
            for(int j=0;j<4;j++)
            {
                PDMxuan_Name2[whichoption-1][j] = PDMxuan_Name[whichoption-1][j];
            }
            //            pdmxuanlist2[whichoption-1]=  pdmxuanlist[whichoption-1];
            // ui->pushButton_12->hide();
            ui->stackedWidget->setCurrentIndex(1);
            ui->stackedWidget_2->setCurrentIndex(4);
            //ui->pushButton_15->show();
            SaveWhat = "";
        }
        SaveWhat = "";
    }
    else if(SaveWhat == "CANCEL")
    {
        //        ui->label_isxuan->setText("no");
        //        ui->label_carname->setText(ui->lineEdit_carname->text());
        //        ui->label_G9->setText(ui->lineEdit_G9->text());
        //        ui->label_VIN->setText(ui->lineEdit_VIN->text());
        if(tmp)
        {
            for(int i = 0;i< 20;i++)
            {
                luo[i] = luo2[i];
                pro[i] =  pro2[i];
                lsnumers[i]  =  lsnumers2[i] ;
            }
            SaveWhat = "saveself";
            ui->label_Lsnum->setText(luo2[0]);
            ui->label_pronum->setText(pro2[0]);
            ui->label_number->setText(lsnumers2[0]);
            on_pushButton_14_clicked();

        }else
        {
            for(int i = 0;i< 20;i++)
            {
                luo2[i] = luo[i];
                pro2[i] =  pro[i];
                lsnumers2[i]  =  lsnumers[i] ;
            }
            ui->stackedWidget_5->setCurrentIndex(0);

            ui->label_Lsnum->setText(luo[0]);
            ui->label_pronum->setText(pro[0]);
            ui->label_number->setText(lsnumers[0]);

            backShow();
            SaveWhat = "";
        }
    }
    else if(SaveWhat == "OPDELONE")
    {
        if(tmp)
        {// 删除一个选配
            isoptionsaved = false;
            ioBoxOption[whichoption-1] = 0;
            for(int i = 0;i<20;i++)
            {

                luoxuanlist2[whichoption-1][i]  = "0";
                proxuanlist2[whichoption-1][i]  =  "0";
                lsnumersxuanlist2[whichoption-1][i]=  "0";
                channelxuanlist2[whichoption-1][i] = "1";
                taotongxuanlist2[whichoption-1][i] = "0";
                bxuannamelist2[whichoption-1][i] = "";
                bxuancodelist2[whichoption-1][i] = "";

                kxuannamelist2[whichoption-1][i] = "";
                kxuancodelist2[whichoption-1][i] = "";

            }
            for(int i=0;i<4;i++)
            {
                PDMxuan_Name2[whichoption-1][i] = "";
            }
            //            pdmxuanlist2[whichoption-1]=  "";
            ui->stackedWidget->setCurrentIndex(1);

            int tmpIndex = (whichoption-1)%5+1;
            if(tmpIndex == 1)
            {
                ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                ui->pushButton_70->setText("");
                ui->pushButton_71->setText("");
            }else if(tmpIndex == 2)
            {
                ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                ui->pushButton_72->setText("");
                ui->pushButton_73->setText("");
            }else if(tmpIndex == 3)
            {
                ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                ui->pushButton_74->setText("");
                ui->pushButton_75->setText("");
            }else if(tmpIndex == 4)
            {
                ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                ui->pushButton_76->setText("");
                ui->pushButton_77->setText("");
            }else if(tmpIndex == 5)
            {
                ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                ui->pushButton_78->setText("");
                ui->pushButton_79->setText("");
            }

        }
        SaveWhat ="";
    }
    else if(SaveWhat == "OPDELALL")
    {
        if(tmp)
        {
            SaveWhat = "saveself";
            ui->lineEdit_carname->clear();
            ui->lineEdit_iobox->clear();
            ui->lineEdit_G9->clear();
            ui->lineEdit_VIN->clear();
            ui->editExecCarType->clear();
            ui->label_carname->clear();
            ui->label_iobox->clear();
            ui->label_G9->clear();
            ui->label_VIN->clear();
            ui->label_isxuan->clear();

            for(int j= 1;j< 6;j++)
            {
                ioBoxOption[j-1] = 0;
                for(int i = 0;i<20;i++)
                {

                    luoxuanlist[j-1][i]  = "0";
                    proxuanlist[j-1][i]  =  "0";
                    lsnumersxuanlist[j-1][i]=  "0";
                    channelxuanlist[j-1][i] = "1";
                    taotongxuanlist[j-1][i] = "0";

                    bxuannamelist[j-1][i] = "";
                    bxuancodelist[j-1][i] = "";

                    kxuannamelist[j-1][i] = "";
                    kxuancodelist[j-1][i] = "";
                    luoxuanlist2[j-1][i]  = "0";
                    proxuanlist2[j-1][i]  =  "0";
                    lsnumersxuanlist2[j-1][i]=  "0";
                    channelxuanlist2[j-1][i] = "1";
                    taotongxuanlist2[j-1][i] = "0";

                    bxuannamelist2[j-1][i] = "";
                    bxuancodelist2[j-1][i] = "";

                    kxuannamelist2[j-1][i] = "";
                    kxuancodelist2[j-1][i] = "";

                }
                for(int i = 0;i<4; i++)
                {
                    PDMxuan_Name[j-1][i]="";
                    PDMxuan_Name2[j-1][i]="";
                }
                //                pdmxuanlist[j-1]=  "";
                //                pdmxuanlist2[j-1]=  "";
            }
            on_pushButton_14_clicked();
            QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
            configIniRead->setValue(QString("carinfo").append(QString::number(whichcar)).append("/IsOptionalStation"),"0");
            delete configIniRead;
            ui->stackedWidget_5->setCurrentIndex(0);
            if(whichcar%5 == 1 && whichcar <(CAR_TYPE_AMOUNT+1))
            {
                ui->pushButton_butt1->setText("");
                ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            }
            else if(whichcar%5 == 2 && whichcar <(CAR_TYPE_AMOUNT+1))
            {
                ui->pushButton_butt2->setText("");
                ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            }

            else if(whichcar%5 == 3 && whichcar <(CAR_TYPE_AMOUNT+1))
            {
                ui->pushButton_butt3->setText("");
                ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            }
            else if(whichcar%5 == 4 && whichcar <(CAR_TYPE_AMOUNT+1))
            {
                ui->pushButton_butt4->setText("");
                ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            }
            else if(whichcar%5 == 0 && whichcar <(CAR_TYPE_AMOUNT+1))
            {
                ui->pushButton_butt5->setText("");
                ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            }

            ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
            ui->pushButton_70->setText("");
            ui->pushButton_71->setText("");
            ui->pushButton_72->setText("");
            ui->pushButton_73->setText("");
            ui->pushButton_74->setText("");
            ui->pushButton_75->setText("");
            ui->pushButton_76->setText("");
            ui->pushButton_77->setText("");
            ui->pushButton_78->setText("");
            ui->pushButton_79->setText("");

            ui->editQRCodeRule1->setText("");
            ui->editQRCodeRule2->setText("");
            ui->editQRCodeRule3->setText("");
            ui->editQRCodeRule4->setText("");
            ui->editQRCodeRule5->setText("");
        }
        SaveWhat ="";
    }
    else if(SaveWhat == "wifiunconnect")
    {


    }
    else if(SaveWhat == "pdmwaring")
    {
        SaveWhat = "pdm";
    }
    else if(SaveWhat == "pdmout")
    {
        if(tmp)
        {
            savePDM();
        }
        on_pushButton_22_clicked();
        //        this->close();
        //        SaveWhat = "";
    }
    else if(SaveWhat == "pdmoutmain")
    {
        if(tmp)
        {
            savePDM();
        }
        //SaveWhat ="";
        //on_pushButton_clicked();

    }
    else if(SaveWhat == "pdmoutabout")
    {
        if(tmp)
        {
            savePDM();
        }
        SaveWhat ="";
        on_pushButton_13_clicked();
    }
    else if(SaveWhat == "pdmdel")
    {

        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        int numpdmnow = 0;
        if(tmp)
        {
            for(int i = 0;i < 1000;i++ )
            {
                QString name = configIniRead->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString();

                if(name == ui->comboBox->itemText(nowcomboxnum))
                {
                    nowpdmnum = i + 1;
                    numpdmnow = nowpdmnum;
                    //DTdebug() << "fdddddddddddddddddddddddd" << nowpdmnum;
                    break;
                }
            }
            configIniRead->remove(QString("pdminfo").append(QString::number(nowpdmnum)));

            ui->comboBox->removeItem(nowcomboxnum);
            int pdmnumber = configIniRead->value("baseinfo/pdmnumber").toInt();
            int pdmmax = configIniRead->value("baseinfo/pdmmax").toInt();
            configIniRead->setValue(QString("baseinfo/pdmnumber"),QString::number(pdmnumber-1));
            //DTdebug() << "fdfffffffffffff" << pdmmax << pdmnumber;
            if(0 == (pdmnumber-1))
            {
                pdmmax = 0;
            }
            else
            {
                if(numpdmnow == pdmmax)
                {

                    for(int j= numpdmnow-1;j>0;j--)
                    {
                        if(configIniRead->value(QString("pdminfo").append(QString::number(j)).append("/pdmname")).toString() != "")
                        {
                            pdmmax = j;
                            //DTdebug() << "fdfdf" << pdmmax << j;
                            break;
                        }

                    }
                }
            }
            configIniRead->setValue(QString("baseinfo/pdmmax"),QString::number(pdmmax));
        }
        else
        {

        }
        SaveWhat = "";
        delete configIniRead;

    }
    else if(SaveWhat == "pdm")
    {
        if(tmp)
        {
            savePDM();
        }
        else
        {
            //             int tempdata = numpdm;
            //             for(int i = 0;i<tempdata;i++)
            //             {
            //                 delete butt1[i];
            //                 numpdm--;
            //             }
        }

        SaveWhat ="";

    }
    ui->label_100->hide();
    delete e3;
    delete save;
    DTdebug()<<"whywhy  SaveWhat"<<SaveWhat;
    if(SaveWhat == "pdmoutmain")
    {
        on_pushButton_clicked();
    }
}

void Newconfiginfo::savePDM()
{
    QSettings *configIniRead = new QSettings("/config.ini",QSettings::IniFormat);
    if(isSavedpdm)
    {
        // 代表  添加新的pdm图
        int pdmmax = configIniRead->value("baseinfo/pdmmax").toInt();
        configIniRead->setValue(QString("pdminfo").append(QString::number(pdmmax+1)).append(QString("/pdmname")),ui->lineEdit_pdmname->text());
        configIniRead->setValue(QString("pdminfo").append(QString::number(pdmmax+1)).append(QString("/pdmpath")),pathpdm);
        configIniRead->setValue(QString("pdminfo").append(QString::number(pdmmax+1)).append(QString("/num")),QString::number(numpdm));  //螺栓个数
        for(int i = 0;i < numpdm;i++)
        {
            //DTdebug() << "The coordinate of the butt[temp]"<< butt1[i]->x()<<","<<butt1[i]->y()<<")";
            double tempx = (((butt1[i]->x() - (50 + 200))* 1000) )/1051;
            double tempy = (((butt1[i]->y() - (20 + 120)) * 1000) )/531;

            configIniRead->setValue(QString("pdminfo").append(QString::number((pdmmax+1))).append(QString("/tempx").append(QString::number((i+1)))),QString::number(tempx));
            configIniRead->setValue(QString("pdminfo").append(QString::number((pdmmax+1))).append(QString("/tempy").append(QString::number((i+1)))),QString::number(tempy));
        }
        int pdmnumber = configIniRead->value("baseinfo/pdmnumber").toInt();
        configIniRead->setValue("baseinfo/pdmnumber",QString::number((pdmnumber+1)));
        configIniRead->setValue("baseinfo/pdmmax",QString::number((pdmmax+1)));
        ui->comboBox->addItem(ui->lineEdit_pdmname->text());

        //DTdebug() << "huahua";
        ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
        ui->label_69->show();
        ui->comboBox->show();

    }else  //修改的pdm保存
    {
        //DTdebug() << "没进来kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk";
        configIniRead->setValue(QString("pdminfo").append(QString::number(nowpdmnum)).append(QString("/pdmname")),ui->lineEdit_pdmname->text());
        configIniRead->setValue(QString("pdminfo").append(QString::number(nowpdmnum)).append(QString("/pdmpath")),pathpdm);
        configIniRead->setValue(QString("pdminfo").append(QString::number(nowpdmnum)).append(QString("/num")),QString::number(numpdm));
        for(int i = 0;i < numpdm;i++)
        {
            //DTdebug() << "The coordinate of the butt[temp]"<< butt1[i]->x()<<","<<butt1[i]->y()<<")";
            double tempx = (((butt1[i]->x() - (50 + 200))* 1000) )/1051;
            double tempy = (((butt1[i]->y() - (20 + 120)) * 1000) )/531;

            configIniRead->setValue(QString("pdminfo").append(QString::number((nowpdmnum))).append(QString("/tempx").append(QString::number((i+1)))),QString::number(tempx));
            configIniRead->setValue(QString("pdminfo").append(QString::number((nowpdmnum))).append(QString("/tempy").append(QString::number((i+1)))),QString::number(tempy));
        }
        ui->comboBox->setItemText(ui->comboBox->currentIndex(),ui->lineEdit_pdmname->text());
    }
    delete configIniRead;
}


void Newconfiginfo::backShow()
{
    QSettings *configIniRead = new QSettings("/config.ini",QSettings::IniFormat);
    ui->pushButton_14->hide();
    ui->pushButton_15->hide();
    ui->pushButton_16->hide();
    ui->pushButton_12->show();
    ui->pushButton_butt1->setEnabled(true);
    ui->pushButton_butt2->setEnabled(true);
    ui->pushButton_butt3->setEnabled(true);
    ui->pushButton_butt4->setEnabled(true);
    ui->pushButton_butt5->setEnabled(true);

    ui->pushButton_8->setEnabled(true);
    ui->pushButton_9->setEnabled(true);

    int buttnumber = 0;
    buttnumber = (pagenum -1)*5+1;
    int t = 0;
    for(int j = buttnumber;j < (buttnumber+5);j++)
    {

        if(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/IsOptionalStation")).toInt()) //有选配
        {
            buttonbox[t]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
            buttonbox[t]->setText(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/carcx")).toString());
//            editBox[t]  ->setText(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/QRCodeRule")).toString());
        }
        else //非选配
        {
            int i = 1;
            for(i = 1;i < 21;i++)
            {
                if(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/LSNumber").append(QString::number(i))).toInt())
                {
                    buttonbox[t]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font:14pt;color: rgb(248, 248, 255);");
                    buttonbox[t]->setText(configIniRead->value(QString("carinfo").append(QString::number(j)).append("/carcx")).toString());
                    break;
                }
            }
            if(i == 21)
            {
                buttonbox[t]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                buttonbox[t]->setText("");
            }
        }
        t++;
    }



    if(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/IsOptionalStation")).toInt()) //有选配
    {
        buttonbox[whichcar-buttnumber]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
        buttonbox[whichcar-buttnumber]->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString());
//        editBox[whichcar-buttnumber]  ->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/QRCodeRule")).toString());
    }
    else //非选配
    {
        int i = 1;
        for(i = 1;i < 21;i++)
        {
            int ttt = configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/LSNumber").append(QString::number(i))).toInt();
            if(ttt)
            {
                buttonbox[whichcar-buttnumber]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
                buttonbox[whichcar-buttnumber]->setText(configIniRead->value(QString("carinfo").append(QString::number(whichcar)).append("/carcx")).toString());
                break;
            }
        }
        if(i == 21)
        {
            buttonbox[whichcar-buttnumber]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
            buttonbox[whichcar-buttnumber]->setText("");
        }
    }
    if(isedit == 1)
    {
        ReinitOptionState();
    }

    delete configIniRead;

}


void Newconfiginfo::on_pushButton_15_clicked()
{
    //edit  取消
    ui->label_Lsnum->show();
    ui->label_pronum->show();
    ui->label_number->show();
    ui->label_108->show();
    ui->label_109->show();
    ui->label_110->show();

    if(isedit == isoption)
    {
    }
    else
    {
        if(!isoption)
        {
            ui->editQRCodeRule1->setEnabled(false);
            ui->editQRCodeRule2->setEnabled(false);
            ui->editQRCodeRule3->setEnabled(false);
            ui->editQRCodeRule4->setEnabled(false);
            ui->editQRCodeRule5->setEnabled(false);

            ui->pushButton_xuan1->setEnabled(false);
            ui->pushButton_xuan2->setEnabled(false);
            ui->pushButton_xuan3->setEnabled(false);
            ui->pushButton_xuan4->setEnabled(false);
            ui->pushButton_xuan5->setEnabled(false);
            ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
            ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
            ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
            ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
            ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/21.bmp);");
        }
        isedit = isoption;
    }
    ui->stackedWidget_3->setCurrentIndex(3);
    if(isedit == 1)
    {
        ui->label_carname->setText(ui->lineEdit_carname->text());
        ui->label_iobox->setText(ui->lineEdit_iobox->text());
        ui->label_VIN->setText(ui->lineEdit_VIN->text());
        ui->label_G9 ->setText(ui->lineEdit_G9->text());
        ui->label_isxuan->setText("yes");
        ui->label_126->hide();
        ui->comboBox_2->hide();

        if(!isoptionsaved)
        {
            on_pushButton_14_clicked();
        }else
        {
            backShow();
            ui->pushButton_xuan1->setEnabled(false);
            ui->pushButton_xuan2->setEnabled(false);
            ui->pushButton_xuan3->setEnabled(false);
            ui->pushButton_xuan4->setEnabled(false);
            ui->pushButton_xuan5->setEnabled(false);

            ui->editQRCodeRule1->setEnabled(false);
            ui->editQRCodeRule2->setEnabled(false);
            ui->editQRCodeRule3->setEnabled(false);
            ui->editQRCodeRule4->setEnabled(false);
            ui->editQRCodeRule5->setEnabled(false);

            ReinitOptionState();
        }
        ui->label_Lsnum->hide();
        ui->label_Lsnum->hide();
        ui->label_pronum->hide();
        ui->label_number->hide();
        ui->label_isxuan->setText("yes");
        ui->label_108->hide();
        ui->label_109->hide();
        ui->label_110->hide();
        ui->label_52->hide();
        ui->pushButton_10->hide();
        ui->pushButton_11->hide();
        ui->label_currentindex->hide();
        ui->stackedWidget_5->setCurrentIndex(0);
    }
    else if(!isedit)
    {
        luo2[ui->label_119->text().toInt()-1] = ui->lineEdit_Lsnumber->text();
        pro2[ui->label_119->text().toInt()-1] = ui->lineEdit_pronum->text();
        lsnumers2[ui->label_119->text().toInt()-1] = ui->lineEdit_number->text();
        //DTdebug() << pro2[ui->label_119->text().toInt()-1] << ui->label_119->text();
        int i = 0 ;
        for(i = 0;i<20;i++)
        {
            //DTdebug() << carStyle << ui->lineEdit_carname->text() << pro[i] << pro2[i] << i;
            if(carStyle!=ui->lineEdit_carname->text() || mIoBox!=ui->lineEdit_iobox->text() || Vintmp!=ui->lineEdit_VIN->text() ||
                    G9tmp!=ui->lineEdit_G9->text() || luo[i]!=luo2[i] ||
                    pro[i]!=pro2[i] || lsnumers2[i]!=lsnumers[i] ||Out[0]!=ui->checkBox_output1->isChecked() ||
                    Out[1]!=ui->checkBox_output2->isChecked()||Out[2]!=ui->checkBox_output2->isChecked() ||
                    Out[3]!=ui->checkBox_output4->isChecked())
            {
                SaveWhat = "CANCEL";
                e3 = new QGraphicsOpacityEffect(this);
                e3->setOpacity(0.5);
                ui->label_100->setGraphicsEffect(e3);
                ui->label_100->show();
                ui->label_100->setGeometry(0,0,1366,768);

                save = new Save(this);
                connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
                save->show();
                break;
            }
        }
        if(i == 20)
        {
            ui->stackedWidget_5->setCurrentIndex(0);
            backShow();
        }
        if(buttonbox[whichcar-(pagenum-1)*5-1]->text() == "")
        {
            ui->pushButton_12->hide();
        }
    }
}

void Newconfiginfo::on_pushButton_16_clicked()
{
    //删除全部
    if(!isedit)
    { //
        SaveWhat = "deall";

        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);

        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();

    }
    else
    { // 选配删除
        SaveWhat = "OPDELALL";
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);

        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();

    }


}

void Newconfiginfo::on_pushButton_12_clicked()
{
    //编辑 按钮
    isxuanlook = false;
    ui->stackedWidget_3->setCurrentIndex(1);
    ui->stackedWidget_5->setCurrentIndex(1);
    ui->label_119->setText("1");
    ui->pushButton_12->hide();
    ui->pushButton_16->show();
    ui->pushButton_15->show();
    ui->pushButton_14->show();
    currentpages = ui->label_119->text().toInt();
    ui->pushButton_8->setEnabled(false);
    ui->pushButton_5->setEnabled(true);
    ui->pushButton_9->setEnabled(false);

    ui->editQRCodeRule1->setEnabled(true);
    ui->editQRCodeRule2->setEnabled(true);
    ui->editQRCodeRule3->setEnabled(true);
    ui->editQRCodeRule4->setEnabled(true);
    ui->editQRCodeRule5->setEnabled(true);



    if(whichcar%5 == 1 && whichcar <(CAR_TYPE_AMOUNT+1))
    {
        //        ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
        ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt2->setEnabled(false);
        ui->pushButton_butt3->setEnabled(false);
        ui->pushButton_butt4->setEnabled(false);
        ui->pushButton_butt5->setEnabled(false);
        //        ui->pushButton_butt1->setEnabled(false);

    }
    else if(whichcar%5 == 2 && whichcar <(CAR_TYPE_AMOUNT+1))
    {
        ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        //        ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
        ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt1->setEnabled(false);
        //        ui->pushButton_butt2->setEnabled(false);
        ui->pushButton_butt4->setEnabled(false);
        ui->pushButton_butt3->setEnabled(false);
        ui->pushButton_butt5->setEnabled(false);

    }else if(whichcar%5 == 3 && whichcar <(CAR_TYPE_AMOUNT+1))
    {

        ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        //        ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
        ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt1->setEnabled(false);
        ui->pushButton_butt2->setEnabled(false);
        //        ui->pushButton_butt3->setEnabled(false);
        ui->pushButton_butt4->setEnabled(false);
        ui->pushButton_butt5->setEnabled(false);
    }else if(whichcar%5 == 4 && whichcar <(CAR_TYPE_AMOUNT+1))
    {
        ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        //        ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
        ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt1->setEnabled(false);
        ui->pushButton_butt2->setEnabled(false);
        //        ui->pushButton_butt4->setEnabled(false);
        ui->pushButton_butt3->setEnabled(false);
        ui->pushButton_butt5->setEnabled(false);
    }else if(whichcar%5 == 0 && whichcar <(CAR_TYPE_AMOUNT+1))
    {
        ui->pushButton_butt1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        ui->pushButton_butt4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
        //        ui->pushButton_butt5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font:14pt;color: rgb(248, 248, 255);");
        ui->pushButton_butt1->setEnabled(false);
        ui->pushButton_butt2->setEnabled(false);
        ui->pushButton_butt3->setEnabled(false);
        ui->pushButton_butt4->setEnabled(false);
        //        ui->pushButton_butt5->setEnabled(false);
    }

    ui->lineEdit_carname->setText(carStyle);
    ui->lineEdit_iobox->setText(mIoBox);
    ui->lineEdit_G9->setText(G9tmp);
    ui->lineEdit_VIN->setText(Vintmp);
    ui->editExecCarType->setText(strExecCarType);
    ui->checkBox_output1->setChecked(Out[0]);
    ui->checkBox_output2->setChecked(Out[1]);
    ui->checkBox_output3->setChecked(Out[2]);
    ui->checkBox_output4->setChecked(Out[3]);

    if(!isedit)
    {
        ui->lineEdit_Lsnumber->setText(luo[0]);
        ui->lineEdit_pronum->setText(pro[0]);
        ui->lineEdit_number->setText(lsnumers[0]);
        if(Factory != "Haima")
        {
            ui->label_taotongg->show();
            ui->pushButton_taotong_add->show();
            ui->pushButton_taotong_minus->show();
            ui->lineEdit_taotong->show();
        }
        if(taotong[0] == "" || taotong[0] == "0")
            ui->lineEdit_taotong->setText("OFF");
        else
            ui->lineEdit_taotong->setText(taotong[0]);
        ui->label_channel->show();
        ui->pushButton_channel_add->show();
        ui->pushButton_channel_minus->show();
        ui->lineEdit_channel->show();
        if(channel[0] == "" || channel[0] == "0")
            ui->lineEdit_channel->setText("1");
        else
            ui->lineEdit_channel->setText(channel[0]);
        for(int k = 0;k< ui->comboBox_2->count();k++)
        {
            if(PDM_Name[channel[0].toInt()-1] == ui->comboBox_2->itemText(k))
            {
                ui->comboBox_2->setCurrentIndex(k);
                break;
            }
        }
        ui->pushButton_5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
    }
    else
    {
        //选配 编辑
        ui->pushButton_6->hide();
        ui->pushButton_7->hide();
        ui->lineEdit_Lsnumber->hide();
        ui->lineEdit_pronum->hide();
        ui->lineEdit_number->hide();
        ui->pushButton_pronum_add->hide();
        ui->pushButton_pronum_minus->hide();
        ui->pushButton_number_add->hide();
        ui->pushButton_number_minus->hide();
        ui->pushButton_12->hide();
        ui->pushButton_52->hide();
        ui->label_60->hide();
        ui->label_115->hide();
        ui->label_116->hide();
        ui->label_117->hide();
        ui->label_119->hide();
        ui->label_taotongg->hide();
        ui->pushButton_taotong_add->hide();
        ui->pushButton_taotong_minus->hide();
        ui->lineEdit_taotong->hide();
        ui->label_channel->hide();
        ui->pushButton_channel_add->hide();
        ui->pushButton_channel_minus->hide();
        ui->lineEdit_channel->hide();
        ui->pushButton_xuan1->setEnabled(true);
        ui->pushButton_xuan2->setEnabled(true);
        ui->pushButton_xuan3->setEnabled(true);
        ui->pushButton_xuan4->setEnabled(true);
        ui->pushButton_xuan5->setEnabled(true);

        ui->editQRCodeRule1->setEnabled(true);
        ui->editQRCodeRule2->setEnabled(true);
        ui->editQRCodeRule3->setEnabled(true);
        ui->editQRCodeRule4->setEnabled(true);
        ui->editQRCodeRule5->setEnabled(true);

        ui->pushButton_5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
        optionIscheck = true;

        ReinitOptionState();
    }
}

//鼠标过滤事件  拖动
bool Newconfiginfo::eventFilter(QObject *target, QEvent *evt)
{
    static QPoint lastPnt;
    static bool isHover = false;
    //DTdebug()  << "isis";
    for(int i = 0;i < numpdm;i++)
    {
        if(target == butt1[i])
        {
            if(evt->type() == QEvent::MouseButtonPress)
            {
                QMouseEvent* e = static_cast<QMouseEvent*>(evt);
                //QPushButton *child = static_cast<QPushButton *>(childAt(e->pos()));


                // for( i = 0;i < numpdm; i++)
                //{
                if(butt1[i]->rect().contains(e->pos()) && //is the mouse is clicking the key
                        (e->button() == Qt::LeftButton)) //if the mouse click the left key
                {
                    lastPnt = e->pos();
                    //DTdebug() << e->pos().x() << e->pos().y();
                    isHover = true;
                }
                // }
            }
            else if(evt->type() == QEvent::MouseMove && isHover)
            {
                QMouseEvent* e = static_cast<QMouseEvent*>(evt);
                int dx = e->pos().x() - lastPnt.x();
                int dy = e->pos().y() - lastPnt.y();
                int x1,y1;
                if (butt1[i]->x()+dx<259)
                    x1=259;
                else if (butt1[i]->x()+dx>1265)
                    x1=1265;
                else
                    x1=butt1[i]->x()+dx;
                if (butt1[i]->y()+dy<130)
                    y1=130;
                else if (butt1[i]->y()+dy>616)
                    y1=616;
                else
                    y1=butt1[i]->y()+dy;
                butt1[i]->move(x1,y1);
            }else if(evt->type() == QEvent::MouseButtonRelease && isHover)
            {
                //DTdebug() << "The coordinate of the butt[temp]"<< butt1[i]->x()<<","<<butt1[i]->y()<<")";
                isHover = false;
                SaveWhat = "pdm";
            }
        }
    }

    return false;
}

void Newconfiginfo::xuanchoux(int tmp)
{
    ui->label_bx1name->clear();
    ui->label_bx1code->clear();
    ui->label_bx2name->clear();
    ui->label_bx2code->clear();
    ui->label_bx3name->clear();
    ui->label_bx3code->clear();
    ui->label_bx4name->clear();
    ui->label_bx4code->clear();
    ui->label_bx5name->clear();
    ui->label_bx5code->clear();
    ui->label_kx1name->clear();
    ui->label_kx1code->clear();
    ui->label_kx2name->clear();
    ui->label_kx2code->clear();
    ui->label_kx3name->clear();
    ui->label_kx3code->clear();
    ui->label_kx4name->clear();
    ui->label_kx4code->clear();
    ui->label_kx5name->clear();
    ui->label_kx5code->clear();
    ui->pushButton_bxuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_bxuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_bxuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_bxuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_bxuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");

    ui->pushButton_kxuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_kxuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_kxuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_kxuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->pushButton_kxuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
    ui->comboBox_3->setCurrentIndex(0);
    ui->label_51->setText("1");
    whichoption = tmp;

    ui->stackedWidget->setCurrentIndex(2);
    ui->stackedWidget_9->setCurrentIndex(0);

    for(int k = 0;k< ui->comboBox_3->count();k++)
    {
        if(PDMxuan_Name2[whichoption-1][0] == ui->comboBox_3->itemText(k))
        {
            ui->comboBox_3->setCurrentIndex(k);
            break;
        }
    }
    if(isJS)
    {
        ui->stackedWidget_13->setCurrentIndex(1);
        ui->label_ioBoxOption->setText(QString::number(ioBoxOption[whichoption-1]));
        ui->label_75->setText(luoxuanlist2[whichoption-1][0]);
        ui->label_76->setText(proxuanlist2[whichoption-1][0]);
        ui->label_77->setText(lsnumersxuanlist2[whichoption-1][0]);
        //        ui->label_78->setText(pdmxuanlist2[whichoption-1]);
        ui->label_75->setText(luoxuanlist2[whichoption-1][currentpages-1]);
        ui->label_76->setText(proxuanlist2[whichoption-1][currentpages-1]);
        ui->label_77->setText(lsnumersxuanlist2[whichoption-1][currentpages-1]);
        ui->pushButton_14->hide();
        ui->pushButton_15->hide();
        ui->pushButton_16->hide();
        ui->pushButton_23->hide();
        ui->pushButton_24->hide();
        ui->pushButton_25->hide();
        //        ui->label_78->setText(pdmxuanlist2[whichoption-1]);
        ui->pushButton_bxuan1->setEnabled(false);
        ui->pushButton_bxuan2->setEnabled(false);
        ui->pushButton_bxuan3->setEnabled(false);
        ui->pushButton_bxuan4->setEnabled(false);
        ui->pushButton_bxuan5->setEnabled(false);

        ui->pushButton_kxuan1->setEnabled(false);
        ui->pushButton_kxuan2->setEnabled(false);
        ui->pushButton_kxuan3->setEnabled(false);
        ui->pushButton_kxuan4->setEnabled(false);
        ui->pushButton_kxuan5->setEnabled(false);

    }
    else
    {
        ui->stackedWidget_13->setCurrentIndex(0);
    }

    ui->lineEdit_ioBoxOption->setText(QString::number(ioBoxOption[whichoption-1]));
    ui->lineEdit_xuanLsnum->setText(luoxuanlist2[whichoption-1][0]);
    if (proxuanlist2[whichoption-1][0]=="")
        ui->lineEdit_xuanpronum->setText("0");
    else ui->lineEdit_xuanpronum->setText(proxuanlist2[whichoption-1][0]);
    if (lsnumersxuanlist2[whichoption-1][0]=="")
        ui->lineEdit_xuannumber->setText("0");
    else ui->lineEdit_xuannumber->setText(lsnumersxuanlist2[whichoption-1][0]);
    if (channelxuanlist2[whichoption-1][0]=="")
        ui->lineEdit_channel_2->setText("1");
    else ui->lineEdit_channel_2->setText(channelxuanlist2[whichoption-1][0]);
    if (taotongxuanlist2[whichoption-1][0]==""||taotongxuanlist2[whichoption-1][0]=="0")
        ui->lineEdit_taotong_2->setText("OFF");
    else ui->lineEdit_taotong_2->setText(taotongxuanlist2[whichoption-1][0]);
    for(int k = 0;k< ui->comboBox_3->count();k++)
    {
        if(PDMxuan_Name[whichoption-1][ui->lineEdit_channel_2->text().toInt()-1] == ui->comboBox_3->itemText(k))
        {
            ui->comboBox_3->setCurrentIndex(k);
            break;
        }
    }

    if(bxuannamelist2[whichoption-1][0]!="" && bxuancodelist2[whichoption-1][0] !="")
    {
        ui->pushButton_bxuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
        ui->label_bx1name->setText(bxuannamelist2[whichoption-1][0]);
        ui->label_bx1code->setText(bxuancodelist2[whichoption-1][0]);
        //DTdebug() << "nbnb1" << bxuannamelist2[whichoption-1][0] << bxuancodelist[whichoption-1][0];

    }
    if(bxuannamelist2[whichoption-1][1]!="" && bxuancodelist2[whichoption-1][1] !="")
    {
        ui->pushButton_bxuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_bx2name->setText(bxuannamelist2[whichoption-1][1]);
        ui->label_bx2code->setText(bxuancodelist2[whichoption-1][1]);
        //DTdebug() << "nbnb1" << bxuannamelist2[whichoption-1][1] << bxuancodelist2[whichoption-1][1];

    }
    if(bxuannamelist2[whichoption-1][2]!="" && bxuancodelist2[whichoption-1][2] !="")
    {
        ui->pushButton_bxuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_bx3name->setText(bxuannamelist2[whichoption-1][2]);
        ui->label_bx3code->setText(bxuancodelist2[whichoption-1][2]);
        //DTdebug() << "nbnb1" << bxuannamelist2[whichoption-1][2] << bxuancodelist[whichoption-1][2];

    }
    if(bxuannamelist2[whichoption-1][3]!="" && bxuancodelist2[whichoption-1][3] !="")
    {
        ui->pushButton_bxuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_bx4name->setText(bxuannamelist2[whichoption-1][3]);
        ui->label_bx4code->setText(bxuancodelist2[whichoption-1][3]);
        //DTdebug() << "nbnb1" << bxuannamelist2[whichoption-1][3] << bxuancodelist[whichoption-1][3];

    }
    if(bxuannamelist2[whichoption-1][4]!="" && bxuancodelist2[whichoption-1][4] !="")
    {
        ui->pushButton_bxuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_bx5name->setText(bxuannamelist2[whichoption-1][4]);
        ui->label_bx5code->setText(bxuancodelist2[whichoption-1][4]);
        //DTdebug() << "nbnb1" << bxuannamelist[whichoption-1][4] << bxuancodelist[whichoption-1][4];

    }

    if(kxuannamelist2[whichoption-1][0]!="" && kxuancodelist2[whichoption-1][0] !="")
    {
        ui->pushButton_kxuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_kx1name->setText(kxuannamelist2[whichoption-1][0]);
        ui->label_kx1code->setText(kxuancodelist2[whichoption-1][0]);

    }
    if(kxuannamelist2[whichoption-1][1]!="" && kxuancodelist2[whichoption-1][1] !="")
    {
        ui->pushButton_kxuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_kx2name->setText(kxuannamelist2[whichoption-1][1]);
        ui->label_kx2code->setText(kxuancodelist2[whichoption-1][1]);

    }
    if(kxuannamelist2[whichoption-1][2]!="" && kxuancodelist2[whichoption-1][2] !="")
    {
        ui->pushButton_kxuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_kx3name->setText(kxuannamelist2[whichoption-1][2]);
        ui->label_kx3code->setText(kxuancodelist2[whichoption-1][2]);

    }
    if(kxuannamelist2[whichoption-1][3]!="" && kxuancodelist2[whichoption-1][3] !="")
    {
        ui->pushButton_kxuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_kx4name->setText(kxuannamelist2[whichoption-1][3]);
        ui->label_kx4code->setText(kxuancodelist2[whichoption-1][3]);

    }
    if(kxuannamelist2[whichoption-1][4]!="" && kxuancodelist2[whichoption-1][4] !="")
    {
        ui->pushButton_kxuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");

        ui->label_kx5name->setText(kxuannamelist2[whichoption-1][4]);
        ui->label_kx5code->setText(kxuancodelist2[whichoption-1][4]);

    }

}
//delete config;

void Newconfiginfo::on_pushButton_xuan1_clicked()
{
    //选配按钮  1

    xuanchoux(1+(optionpagenum-1)*5);


}

void Newconfiginfo::on_pushButton_xuan2_clicked()
{
    //选配按钮  2
    xuanchoux(2+(optionpagenum-1)*5);
}

void Newconfiginfo::on_pushButton_xuan3_clicked()
{
    //选配按钮  3
    xuanchoux(3+(optionpagenum-1)*5);
}

void Newconfiginfo::on_pushButton_xuan4_clicked()
{
    //选配按钮  4
    xuanchoux(4+(optionpagenum-1)*5);
}

void Newconfiginfo::on_pushButton_xuan5_clicked()
{
    //选配按钮  5
    xuanchoux(5+(optionpagenum-1)*5);
}






void Newconfiginfo::on_pushButton_20_clicked()
{
    //选配 关闭
    if(!isJS)
        on_pushButton_24_clicked();
    ui->stackedWidget->setCurrentIndex(1);
    ui->stackedWidget_2->setCurrentIndex(4);
}

void Newconfiginfo::on_pushButton_18_clicked()
{
    // 选up
    carStyle = ui->lineEdit_carname->text();
    mIoBox = ui->lineEdit_iobox->text();
    G9tmp = ui->lineEdit_G9->text();
    Vintmp = ui->lineEdit_VIN->text();
    strExecCarType = ui->editExecCarType->text() ;
    //    Out[0]= ui->checkBox_output1->isChecked();
    //    Out[1]= ui->checkBox_output2->isChecked();
    //    Out[2]= ui->checkBox_output3->isChecked();
    //    Out[3]= ui->checkBox_output4->isChecked();
    ioBoxOption[whichoption-1] = ui->lineEdit_ioBoxOption->text().toInt();
    luoxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanLsnum->text();
    proxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanpronum->text();
    lsnumersxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuannumber->text();
    channelxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_channel_2->text();
    if(ui->lineEdit_taotong_2->text()!= "OFF")
        taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_taotong_2->text();
    else
        taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = "0";
    PDMxuan_Name2[whichoption-1][ui->lineEdit_channel_2->text().toInt()-1] = ui->comboBox_3->currentText();
    updownWriteOperate(1);

}

void Newconfiginfo::on_pushButton_19_clicked()
{
    //   选down
    carStyle = ui->lineEdit_carname->text();
    mIoBox = ui->lineEdit_iobox->text();
    G9tmp = ui->lineEdit_G9->text();
    Vintmp = ui->lineEdit_VIN->text();
    strExecCarType = ui->editExecCarType->text() ;
    //    Out[0]= ui->checkBox_output1->isChecked();
    //    Out[1]= ui->checkBox_output2->isChecked();
    //    Out[2]= ui->checkBox_output3->isChecked();
    //    Out[3]= ui->checkBox_output4->isChecked();
    ioBoxOption[whichoption-1] = ui->lineEdit_ioBoxOption->text().toInt();
    luoxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanLsnum->text();
    proxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanpronum->text();
    lsnumersxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuannumber->text();
    channelxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_channel_2->text();
    if(ui->lineEdit_taotong_2->text()!= "OFF")
        taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_taotong_2->text();
    else
        taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = "0";
    PDMxuan_Name2[whichoption-1][ui->lineEdit_channel_2->text().toInt()-1] = ui->comboBox_3->currentText();
    updownWriteOperate(0);
}


void Newconfiginfo::on_pushButton_28_clicked()
{
    if(ui->stackedWidget_2->currentIndex()!=5)
        //pdm编辑
    {
        ispdminit = 0;
        // DTdebug()<<"????????????????????????????????????????";
        ui->groupBox_13->setTitle(tr("图片"));
        ui->groupBox_13->setStyleSheet("QGroupBox {font: 14pt;margin-top: 1ex;border-width:0.5px;border-style:solid;border-color: rgb(51, 153, 255);} QGroupBox::title {subcontrol-origin: margin;subcontrol-position: top left;left:15px;margin-left: 5px;margin-right: 5px;padding:1px;}");
        ui->listWidget->setCurrentRow(1);
        historyclear();
        if(isJS)
        {
            //////////////////////////////////////////////////////////////////////////////////////////未写
        }
        else
        {

            whichButtonClick = "PDMEdit";
            ui->comboBox->show();
            ui->label_69->show();
            ui->lineEdit_pdmname->hide();
            ui->pushButton_29->hide();
            ui->pushButton_30->hide();
            ui->pushButton_31->hide();

            if(!isbaseinfochange)
                baseInfoIsChange();
            else
            {
                ui->label_83->hide();
                ui->label_84->hide();
                ui->label_85->hide();
                ui->label_86->show();
                //            ui->pushButton_13->hide();
                ui->stackedWidget_2->setCurrentIndex(5);
                ui->stackedWidget_6->setCurrentIndex(0);
                //ui->stackedWidget_3->setCurrentIndex(4);
                //ui->pushButton_96->setText(tr("PDM编辑"));
                ui->comboBox->clear();
                QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
                ui->comboBox->addItem("");
                //int pdmnumber = config->value("baseinfo/pdmnumber").toInt();

                for(int i= 0;i < 1000 ;i++)
                {

                    if(config->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString()!= "")
                    {
                        ui->comboBox->addItem(config->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString());
                    }
                }
                SaveWhat = "";
                ispdminit = 1;
                delete config;
                //pdminit();
            }
        }
    }
}

void Newconfiginfo::pdmSelect(QListWidgetItem* item)
{
    if(numpdm > 0)
    {
        for(int i = 0;i< numpdm;i++)
            butt1[i]->show();
    }
    pathpdm =  item->text();
    //DTdebug() <<tr( ui->listWidget_2->currentItem()->text().toAscii().data());
    ui->stackedWidget_6->setCurrentIndex(0);
    //DTdebug() << tr(pathpdm.toAscii().data());
    ui->groupBox_13->setTitle("");
    ui->groupBox_13->setStyleSheet(QString("border-image: url(/PDM/").append(pathpdm.toAscii()).append(")"));

}


void Newconfiginfo::pdminit()
{//  pdm图 listwidget初始化
    ui->listWidget_2->setViewMode(QListView::IconMode);    //设置QListWidget的显示模式
    ui->listWidget_2->setResizeMode(QListView::Adjust);
    ui->listWidget_2->setMovement(QListView::Static);      //设置QListWidget中的单元项不可被拖动
    ui->listWidget_2->setIconSize(QSize(241, 179));        //设置QListWidget中的单元项的图片大小
    ui->listWidget_2->setTextElideMode(Qt::ElideRight);
    //ui->listWidget_2->setSpacing(10);                       //设置QListWidget中的单元项的间距
    ui->listWidget_2->setStyleSheet("QListWidget{border-width:0.5px;border-style:solid;border-color:rgb(51, 153, 255);} QListWidget::item{border-right-width:0.5px;border-bottom-width:0.5px;border-style:solid;border-color:rgb(51, 153, 255);}QScrollBar:vertical{width:36px;}");
    // ui->listWidget_2->verticalScrollBar()->setStyleSheet("QScrollBar{width:36px;}");

    QDir *dir = new QDir("/PDM");
    ui->listWidget_2->clear();
    QStringList list = dir->entryList();
    QString string;
    for (int index = 0; index < list.size(); index++)
    {
        string = list.at(index);
        if(string == "." || string == "..")
            continue;
        //QListWidgetItem *item = new QListWidgetItem(string);
        //ui->listWidget->addItem(item);
        QListWidgetItem *configButton = new QListWidgetItem(ui->listWidget_2);
        // DTdebug() << string;
        QImage image;


        image.load(QString("/PDM/").append(list.at(index)));
        QPixmap objPixmap = QPixmap::fromImage(image.scaled(QSize(241,179)));
        configButton->setIcon(QIcon(objPixmap));
        // configButton->setIcon(QIcon(QString("/PDM/").append(list.at(index))));
        configButton->setText(tr(string.toLatin1().data()));
        configButton->setSizeHint(QSize(253,210));

        configButton->setTextAlignment(Qt::AlignHCenter);
        configButton->setFlags(configButton->flags() & ~Qt::ItemIsSelectable);
    }
    connect(ui->listWidget_2,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(pdmSelect(QListWidgetItem*)));
    ui->listWidget_2->setFlow(QListView::LeftToRight);
    ui->listWidget_2->update();
    delete dir;
}
void Newconfiginfo::on_pushButton_29_clicked()
{

    //图片浏览
    SaveWhat = "pdm";
    if(numpdm > 0)
    {
        for(int i = 0;i< numpdm;i++)
            butt1[i]->hide();
    }
    ui->stackedWidget_6->setCurrentIndex(1);

}

//pdm 图片选择取消
void Newconfiginfo::receivecancelpdm()
{
    delete e3;
    ui->label_100->hide();
}
void Newconfiginfo::on_pushButton_30_clicked()
{
    //添加螺栓
    SaveWhat = "pdm";
    numpdm++;
    //       qsrand(time(NULL));

    //       int n = qrand() % 1000;
    //       qsrand(time(NULL));
    //       qsrand(time(NULL));
    //      int n1 = qrand() % 400;
    temppdm = numpdm - 1;
    //DTdebug() << "temp is" << temppdm;

    butt1[temppdm] = new QPushButton(this);
    butt1[temppdm]->setFocusPolicy(Qt::NoFocus);
    butt1[temppdm]->setFlat(true);

    //    butt1[temppdm]->setStyleSheet("font-size: 35px");
    butt1[temppdm]->setText(QString::number(numpdm));
    butt1[temppdm]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 14pt \"microsoft yahei\";color: rgb(248, 248, 255);");
    butt1[temppdm]->show();

    butt1[temppdm]->setGeometry(400,300,45,45);

    butt1[temppdm]->installEventFilter(this);
}

void Newconfiginfo::on_pushButton_31_clicked()
{
    //删除螺栓
    SaveWhat = "pdm";
    if((temppdm == -1) || (!numpdm))
    {
        temppdm++;
        return;
    }
    //DTdebug() << "temp is" << temppdm;
    delete butt1[temppdm];
    temppdm--;
    numpdm--;
}


void Newconfiginfo::on_pushButton_21_clicked()
{
    //pdm取消
    ui->groupBox_13->setTitle(tr("图片"));
    ui->groupBox_13->setStyleSheet("QGroupBox {font: 14pt;margin-top: 1ex;border-width:0.5px;border-style:solid;border-color: rgb(51, 153, 255);} QGroupBox::title {subcontrol-origin: margin;subcontrol-position: top left;left:15px;margin-left: 5px;margin-right: 5px;padding:1px;}");
    ui->label_69->show();
    ui->comboBox->show();
    ui->comboBox->setCurrentIndex(0);
    ui->lineEdit_pdmname->hide();
    ui->pushButton_29->hide();
    ui->pushButton_30->hide();
    ui->pushButton_31->hide();
    int tempdata = numpdm;
    for(int i = 0;i<tempdata;i++)
    {
        delete butt1[i];
        numpdm--;
    }
    ui->pushButton_33->show();
    SaveWhat = "";
}

void Newconfiginfo::on_pushButton_32_clicked()
{
    //pdm编辑保存按钮
    ui->pushButton_33->show();
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    SaveWhat = "pdm";
    int pdmmax = configIniRead->value("baseinfo/pdmmax").toInt();
    int k = 0;
    for( k = 0;k<pdmmax;k++)
    {
        if(!isSavedpdm)
        {
            if(ui->comboBox->currentText() == ui->lineEdit_pdmname->text())
                continue;
        }
        if(configIniRead->value(QString("pdminfo").append(QString::number(k+1)).append(QString("/pdmname"))).toString() == ui->lineEdit_pdmname->text())  // 名字重复报错
        {
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_100->setGraphicsEffect(e3);
            ui->label_100->show();
            ui->label_100->setGeometry(0,0,1366,768);
            SaveWhat = "pdmwaring";
            save = new Save(this);
            connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
            save->show();
            break;
        }
    }
    if(k == pdmmax)
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);

        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();
    }
    delete configIniRead;
}



void Newconfiginfo::on_comboBox_currentIndexChanged(int index)
{
    //combox 选中
    if (!index)
    {
        ui->groupBox_13->setTitle(tr("图片"));
        ui->groupBox_13->setStyleSheet("QGroupBox {font: 14pt;margin-top: 1ex;border-width:0.5px;border-style:solid;border-color: rgb(51, 153, 255);} QGroupBox::title {subcontrol-origin: margin;subcontrol-position: top left;left:15px;margin-left: 5px;margin-right: 5px;padding:1px;}");
        ui->lineEdit_pdmname->hide();
        ui->pushButton_29->hide();
        ui->pushButton_30->hide();
        ui->pushButton_31->hide();
        int tempdata = numpdm;
        for(int i = 0;i<tempdata;i++)
        {
            delete butt1[i];
            numpdm--;
        }
    }
    else
    {
        QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
        if((ispdminit))
        {
            isSavedpdm = false;
            ui->lineEdit_pdmname->show();
            ui->pushButton_29->show();
            ui->pushButton_30->show();
            ui->pushButton_31->show();
            ui->lineEdit_pdmname->setText(ui->comboBox->currentText());
            int tempdata = numpdm;
            for(int i = 0;i<tempdata;i++)
            {
                delete butt1[i];
                numpdm--;
            }
            nowcomboxnum = index ;
            for(int i = 0;i < 1000;i++ )
            {
                QString name = config->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString();
                if(name == ui->comboBox->itemText(index))
                {
                    nowpdmnum = i + 1;
                    break;
                }
            }
            pathpdm = config->value(QString("pdminfo").append(QString::number((nowpdmnum))).append("/pdmpath")).toString();
            ui->groupBox_13->setTitle("");
            ui->groupBox_13->setStyleSheet(QString("border-image: url(/PDM/").append(pathpdm).append(")"));

            int num = config->value(QString("pdminfo").append(QString::number((nowpdmnum))).append("/num")).toInt();
            for(int i = 0;i < num;i++)
            {
                butt1[i] = new QPushButton(this);
                butt1[i]->setFocusPolicy(Qt::NoFocus);
                numpdm++;
                temppdm = numpdm -1;
                butt1[i]->setFlat(true);
                butt1[i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/g01.png);font: 14pt \"microsoft yahei\";color: rgb(248, 248, 255);");
                butt1[i]->setText(QString::number((i+1)));

                int tempx = config->value(QString("pdminfo").append(QString::number((nowpdmnum))).append("/tempx").append(QString::number((i+1)))).toInt();
                int tempy =  config->value(QString("pdminfo").append(QString::number((nowpdmnum))).append("/tempy").append(QString::number((i+1)))).toInt();
                double x = (double)tempx/1000 * 1051 +  250;
                double y = (double)tempy/1000 * 531+ 140;
                butt1[i]->setGeometry(x,y,45,45);
                butt1[i]->show();
                butt1[i]->installEventFilter(this);
            }
        }
        delete config;
    }
    //DTdebug()<<"SaveWhat@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<SaveWhat;
}

void Newconfiginfo::on_pushButton_33_clicked()
{
    //删除
    if(ui->comboBox->currentText() != "")
    {
        if(!isSavedpdm)
        {
            SaveWhat = "pdmdel";
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_100->setGraphicsEffect(e3);
            ui->label_100->show();
            ui->label_100->setGeometry(0,0,1366,768);
            save = new Save(this);
            connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
            save->show();
        }
    }

}


void Newconfiginfo::on_comboBox_2_currentIndexChanged(const QString &arg1)
{
    // 选择pdm图

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    for(int i = 0;i < 1000;i++ )
    {
        QString name = configIniRead->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString();
        if(name == arg1)
        {
            whichpdmyinyong = QString("pdminfo").append(QString::number(i+1));
            break;
        }
    }


    delete configIniRead;

}

void Newconfiginfo::on_pushButton_24_clicked()
{
    //选配取消
    isxuanlook = false;
    int i,m = 0;
    ioBoxOption[whichoption-1] = ui->lineEdit_ioBoxOption->text().toInt();
    luoxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanLsnum->text();
    proxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanpronum->text();
    lsnumersxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuannumber->text();
    channelxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_channel_2->text();
    if(ui->lineEdit_taotong_2->text()!= "OFF")
        taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_taotong_2->text();
    else
        taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = "0";
    PDMxuan_Name2[whichoption-1][ui->lineEdit_channel_2->text().toInt()-1] = ui->comboBox_3->currentText();
    //    pdmxuanlist2[whichoption-1] = ui->comboBox_3->currentText();
    for(i = 0;i < 20;i++)
    {
        if(luoxuanlist2[whichoption-1][i]!=luoxuanlist[whichoption-1][i] || proxuanlist2[whichoption-1][i]!=proxuanlist[whichoption-1][i] ||
                lsnumersxuanlist2[whichoption-1][i]!=lsnumersxuanlist[whichoption-1][i] || bxuannamelist2[whichoption-1][i]!=bxuannamelist[whichoption-1][i] ||
                bxuancodelist2[whichoption-1][i]!=bxuancodelist[whichoption-1][i] || kxuannamelist2[whichoption-1][i]!=kxuannamelist[whichoption-1][i] ||
                kxuancodelist2[whichoption-1][i]!=kxuancodelist[whichoption-1][i] ||
                channelxuanlist2[whichoption-1][i]!=channelxuanlist[whichoption-1][i] || taotongxuanlist2[whichoption-1][i]!= taotongxuanlist[whichoption-1][i])
        {
            SaveWhat = "OPCANCEL";
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_100->setGraphicsEffect(e3);
            ui->label_100->show();
            ui->label_100->setGeometry(0,0,1366,768);
            save = new Save(this);
            connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
            save->show();
            break;
        }
    }
    if(i == 20)
    {
        for(m=0; m<4; m++)
        {
            if(PDMxuan_Name2[whichoption-1][m]!= PDMxuan_Name[whichoption-1][m])
            {
                SaveWhat = "OPCANCEL";
                e3 = new QGraphicsOpacityEffect(this);
                e3->setOpacity(0.5);
                ui->label_100->setGraphicsEffect(e3);
                ui->label_100->show();
                ui->label_100->setGeometry(0,0,1366,768);
                save = new Save(this);
                connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
                save->show();
                break;
            }
        }
    }
    if(i == 20 && m == 4)
    {
        for(int j = 0;j< 20;j++)
        {
            luoxuanlist2[whichoption-1][j] = luoxuanlist[whichoption-1][j];
            proxuanlist2[whichoption-1][j] = proxuanlist[whichoption-1][j];
            lsnumersxuanlist2[whichoption-1][j] = lsnumersxuanlist[whichoption-1][j];
            channelxuanlist2[whichoption-1][j] = channelxuanlist[whichoption-1][j];
            taotongxuanlist2[whichoption-1][j] = taotongxuanlist[whichoption-1][j];
            bxuannamelist2[whichoption-1][j] = bxuannamelist[whichoption-1][j];
            bxuancodelist2[whichoption-1][j] = bxuancodelist[whichoption-1][j];
            kxuannamelist2[whichoption-1][j] = kxuannamelist[whichoption-1][j];
            kxuancodelist2[whichoption-1][j] = kxuancodelist[whichoption-1][j];

        }
        for(int j=0;j<4;j++)
        {
            PDMxuan_Name2[whichoption-1][j] = PDMxuan_Name[whichoption-1][j];
        }
        //        pdmxuanlist2[whichoption-1]=  pdmxuanlist[whichoption-1];
        ui->pushButton_12->hide();
        ui->stackedWidget->setCurrentIndex(1);
        ui->stackedWidget_2->setCurrentIndex(4);
        ui->pushButton_15->show();
    }
}

void Newconfiginfo::on_pushButton_23_clicked()
{
    //选配删除
    SaveWhat = "OPDELONE";
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);

    save = new Save(this);
    connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
    save->show();
    // saved = false;

}

void Newconfiginfo::on_pushButton_25_clicked()
{
    //选配保存
    if(SaveWhat == "save2" || SaveWhat == "")
    {
        SaveWhat = "save2";
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();
    }
    else if(SaveWhat == "saveself")
    {
        //DTdebug() << "jiushiainininini";
        isoptionsaved = false;
        carStyle = ui->lineEdit_carname->text();
        mIoBox = ui->lineEdit_iobox->text();
        G9tmp = ui->lineEdit_G9->text();
        Vintmp = ui->lineEdit_VIN->text();
        strExecCarType = ui->editExecCarType->text() ;
        Out[0]= ui->checkBox_output1->isChecked();
        Out[1]= ui->checkBox_output2->isChecked();
        Out[2]= ui->checkBox_output3->isChecked();
        Out[3]= ui->checkBox_output4->isChecked();
        ioBoxOption[whichoption-1] = ui->lineEdit_ioBoxOption->text().toInt();
        luoxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanLsnum->text();
        proxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuanpronum->text();
        lsnumersxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_xuannumber->text();
        channelxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_channel_2->text();
        if(ui->lineEdit_taotong_2->text() != "OFF")
            taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = ui->lineEdit_taotong_2->text();
        else
            taotongxuanlist2[whichoption-1][ui->label_51->text().toInt()-1] = "0";
        PDMxuan_Name2[whichoption-1][ui->lineEdit_channel_2->text().toInt()-1] = ui->comboBox_3->currentText();
        //        pdmxuanlist2[whichoption-1] = ui->comboBox_3->currentText();
        for(int j = 0;j< 20;j++)
        {
            luoxuanlist[whichoption-1][j] = luoxuanlist2[whichoption-1][j];
            proxuanlist[whichoption-1][j] = proxuanlist2[whichoption-1][j];
            lsnumersxuanlist[whichoption-1][j] = lsnumersxuanlist2[whichoption-1][j];
            channelxuanlist[whichoption-1][j] = channelxuanlist2[whichoption-1][j];
            taotongxuanlist[whichoption-1][j] = taotongxuanlist2[whichoption-1][j];
            bxuannamelist[whichoption-1][j] = bxuannamelist2[whichoption-1][j];
            bxuancodelist[whichoption-1][j] = bxuancodelist2[whichoption-1][j];
            kxuannamelist[whichoption-1][j] = kxuannamelist2[whichoption-1][j];
            kxuancodelist[whichoption-1][j] = kxuancodelist2[whichoption-1][j];

        }
        for(int j =0;j<4;j++)
        {
            PDMxuan_Name[whichoption-1][j]= PDMxuan_Name2[whichoption-1][j];
            DTdebug()<<PDMxuan_Name[whichoption-1][j];
        }

        //        pdmxuanlist[whichoption-1] = pdmxuanlist2[whichoption-1];
        //DTdebug() << luoxuanlist[whichoption-1][ui->label_51->text().toInt()-1];
        //DTdebug() <<  proxuanlist[whichoption-1][ui->label_51->text().toInt()-1];
        //DTdebug() <<  lsnumersxuanlist[whichoption-1][ui->label_51->text().toInt()-1];
        //DTdebug() << whichoption << ui->label_51->text().toInt();
        ui->stackedWidget->setCurrentIndex(1);
        ReinitOptionState();
        ui->pushButton_14->show();
        ui->pushButton_15->show();
        ui->pushButton_16->show();
        SaveWhat = "";
    }

}

void Newconfiginfo::ReinitOptionState()
{
    int tmpStart = (optionpagenum-1)*5+1;
    for(int i =tmpStart;i<tmpStart+5;i++)
    {
        for(int k = 1;k < 21;k++)
        {
            //判断有几个选配
            if(lsnumersxuanlist[i-1][k-1].toInt() != 0)
            {
                int tmpIndex = (i-1)%5+1;
                if(tmpIndex == 1)
                {
                    ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    ui->pushButton_70->show();
                    ui->pushButton_71->show();
                    ui->pushButton_70->setText(proxuanlist[i-1][0]);
                    ui->pushButton_71->setText(lsnumersxuanlist[i-1][0]);
                    //ui->pushButton_xuan1->setEnabled(isJS);
                    break;
                }

                else if(tmpIndex == 2)
                {
                    ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    //DTdebug() << proxuanlist[i-1][0] << lsnumersxuanlist[i-1][0] << "DFFD";
                    ui->pushButton_72->show();
                    ui->pushButton_73->show();
                    ui->pushButton_72->setText(proxuanlist[i-1][0]);
                    ui->pushButton_73->setText(lsnumersxuanlist[i-1][0]);
                    //ui->pushButton_xuan2->setEnabled(isJS);
                    break;
                }
                else if(tmpIndex == 3)
                {
                    ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    ui->pushButton_74->show();
                    ui->pushButton_75->show();
                    ui->pushButton_74->setText(proxuanlist[i-1][0]);
                    ui->pushButton_75->setText(lsnumersxuanlist[i-1][0]);
                    //ui->pushButton_xuan3->setEnabled(isJS);
                    break;
                } else if(tmpIndex == 4)
                {
                    ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    ui->pushButton_76->setText(proxuanlist[i-1][0]);
                    ui->pushButton_77->setText(lsnumersxuanlist[i-1][0]);
                    //ui->pushButton_xuan4->setEnabled(isJS);
                    break;
                }
                else if(tmpIndex == 5)
                {
                    ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuan.bmp);");
                    ui->pushButton_78->setText(proxuanlist[i-1][0]);
                    ui->pushButton_79->setText(lsnumersxuanlist[i-1][0]);
                    //ui->pushButton_xuan5->setEnabled(isJS);
                    break;
                }
            }
            else
            {
                int tmpIndex = (i-1)%5+1;
                if(tmpIndex == 1)
                {
                    // ui->pushButton_xuan1->setEnabled(false);
                    if(ui->pushButton_12->isHidden())
                        ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else
                        ui->pushButton_xuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_70->setText("");
                    ui->pushButton_71->setText("");

                }
                else if(tmpIndex == 2)
                {
                    //ui->pushButton_xuan2->setEnabled(false);
                if(ui->pushButton_12->isHidden())
                    ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                else
                    ui->pushButton_xuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_72->setText("");
                    ui->pushButton_73->setText("");

                }
                else if(tmpIndex == 3)
                {
                    // ui->pushButton_xuan3->setEnabled(false);
                    if(ui->pushButton_12->isHidden())
                        ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else
                        ui->pushButton_xuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_74->setText("");
                    ui->pushButton_75->setText("");

                }
                else if(tmpIndex == 4)
                {
                    // ui->pushButton_xuan4->setEnabled(false);
                    if(ui->pushButton_12->isHidden())
                        ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else
                        ui->pushButton_xuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_76->setText("");
                    ui->pushButton_77->setText("");

                }
                else if(tmpIndex == 5)
                {
                    // ui->pushButton_xuan5->setEnabled(false);
                    if(ui->pushButton_12->isHidden())
                        ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/16.bmp);");
                    else
                        ui->pushButton_xuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/15.bmp);");
                    ui->pushButton_78->setText("");
                    ui->pushButton_79->setText("");

                }

            }
        }
    }
}


void Newconfiginfo::on_pushButton_52_clicked()
{
    //删除单个
    SaveWhat = "de";
    ui->lineEdit_Lsnumber->setText("");
    ui->lineEdit_pronum->setText("0");
    ui->lineEdit_number->setText("0");
    // ui->comboBox_2->setCurrentIndex(0);
    currentpages = ui->label_119->text().toInt();
    luo[currentpages-1] = "0";
    pro[currentpages-1] = "0";
    lsnumers[currentpages-1] = "0";
    for(int k = 0;k< ui->comboBox_2->count();k++)
    {
        if(PDM_Name[channel[0].toInt()-1] == ui->comboBox_2->itemText(k))
        {
            ui->comboBox_2->setCurrentIndex(k);
            break;
        }
    }
    ui->lineEdit_channel->setText("1");
    ui->lineEdit_taotong->setText("OFF");
    // pdmxuan = "";

}

void Newconfiginfo::on_pushButton_17_clicked()
{
    //删除 单个选配
    ui->lineEdit_xuanLsnum->clear();
    ui->lineEdit_xuanpronum->setText("0");
    ui->lineEdit_xuannumber->setText("0");
    ui->lineEdit_channel_2->setText("1");
    ui->lineEdit_taotong_2->setText("OFF");

    ioBoxOption[whichoption-1] = 0;

    int indextmp = ui->label_51->text().toInt() -1;
    luoxuanlist2[whichoption-1][indextmp] = "0";
    proxuanlist2[whichoption-1][indextmp]= "0";
    lsnumersxuanlist2[whichoption-1][indextmp] = "";
    channelxuanlist2[whichoption-1][indextmp] = "1";
    taotongxuanlist2[whichoption-1][indextmp] = "0";
    for(int k = 0;k< ui->comboBox_3->count();k++)
    {
        if(PDMxuan_Name[whichoption-1][0] == ui->comboBox_3->itemText(k))
        {
            ui->comboBox_3->setCurrentIndex(k);
            break;
        }
    }
    // ui->comboBox_3->setCurrentIndex(0);

}




void Newconfiginfo::on_pushButton_62_clicked()
{
    //高级选项
    whichButtonClick = "advanced";
    if(!isbaseinfochange)
        baseInfoIsChange();
    else
    {
        ui->stackedWidget_2->setCurrentIndex(6);
        ui->stackedWidget_3->setCurrentIndex(2);
        ui->label_87->show();
        ui->label_88->hide();
        ui->label_101->hide();
        ui->label_98->hide();
        ui->label_99->hide();
        ui->label_162->hide();
        ui->label_wirelessLocation->hide();
        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        ui->lineEdit_netmask->setText(configIniRead->value("baseinfo/netmask").toString());
        ui->lineEdit_gateway->setText(configIniRead->value("baseinfo/gateway").toString());
        ui->lineEdit_ssid->setText(configIniRead->value("baseinfo/SSID").toString());
        ui->lineEdit_psk->setText(configIniRead->value("baseinfo/psk").toString());
        ui->lineEdit_APssid->setText(configIniRead->value("baseinfo/APSSID").toString());
        ui->lineEdit_APpsk->setText(configIniRead->value("baseinfo/APpsk").toString());
        ui->lineEdit_tacktime->setText(configIniRead->value("baseinfo/TackTime").toString());   //节拍时间
        ui->lineEdit_offset->setText(configIniRead->value("baseinfo/OffSet").toString());   //offset
        QString strUpInversionCon = configIniRead->value("baseinfo/upInversion").toString();

        int offVOLTAGE_index = ui->comboBox_VOLTAGE->findText(configIniRead->value("baseinfo/offVOLTAGE").toString());
        if(offVOLTAGE_index >= 0){
            ui->comboBox_VOLTAGE->setCurrentIndex(offVOLTAGE_index);
        }
        int offTime = configIniRead->value("baseinfo/offTime").toInt();
        if(offTime > 0){
            ui->lineEdit_offTime->setText(QString::number(offTime));
        }


        if(strUpInversionCon == "true")
        {
            ui->upInversioncheck->setChecked(true);
            strUpInversion = "true";
            upInversion = true;
        }
        else if(strUpInversionCon == "false")
        {
            ui->upInversioncheck->setChecked(false);
            strUpInversion = "false";
            upInversion = false;
        }

        if(configIniRead->value("baseinfo/connectNet").toInt()==0)
            ui->radioButton_wireless->setChecked(true);
        else if(configIniRead->value("baseinfo/connectNet").toInt()==1)
            ui->radioButton_wire1->setChecked(true);
        else if(configIniRead->value("baseinfo/connectNet").toInt()==2)
            ui->radioButton_wire2->setChecked(true);

        if(configIniRead->value("baseinfo/restartTime").toString()!="")
        {
            ui->restart_hour->setText(configIniRead->value("baseinfo/restartTime").toString().mid(0,2));
            ui->restart_minute->setText(configIniRead->value("baseinfo/restartTime").toString().mid(2,2));
            ui->restart_second->setText(configIniRead->value("baseinfo/restartTime").toString().mid(4,2));
        }
        if(restart_enabled)
        {
            ui->pushButton_restart->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
            restartShow(true);
        }
        else
        {
            ui->pushButton_restart->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
            restartShow(false);
        }
        restart_enabled_temp = restart_enabled;

        if(configIniRead->value("baseinfo/Master_slave").toString()=="master")
        {
            ui->radioButton_1->setChecked(true);
            ui->lineEdit_slave_1->setText(configIniRead->value("baseinfo/slave1").toString());
            if(!ui->lineEdit_slave_1->text().isEmpty())
                ui->checkBox_1->setChecked(true);
            else
            {
                ui->lineEdit_slave_1->setText("");
                ui->checkBox_1->setChecked(false);
            }
            ui->lineEdit_slave_2->setText(configIniRead->value("baseinfo/slave2").toString());
            if(!ui->lineEdit_slave_2->text().isEmpty())
                ui->checkBox_2->setChecked(true);
            else
            {
                ui->lineEdit_slave_2->setText("");
                ui->checkBox_2->setChecked(false);
            }
            ui->lineEdit_slave_3->setText(configIniRead->value("baseinfo/slave3").toString());
            if(!ui->lineEdit_slave_3->text().isEmpty())
                ui->checkBox_3->setChecked(true);
            else
            {
                ui->lineEdit_slave_3->setText("");
                ui->checkBox_3->setChecked(false);
            }
        }
        else if(configIniRead->value("baseinfo/Master_slave").toString()=="slave")
        {
            ui->radioButton_2->setChecked(true);
            ui->checkBox_1->hide();
            ui->checkBox_2->hide();
            ui->checkBox_3->hide();
            ui->lineEdit_slave_1->hide();
            ui->lineEdit_slave_2->hide();
            ui->lineEdit_slave_3->hide();
        }
        else
            ui->radioButton_1->setChecked(true);

        if(isBarCode)
        {
            ui->pushButton_26->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/52.bmp);");
        }
        else
        {
            ui->pushButton_26->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/barcode.bmp);");
        }
        if(isRFID)
        {
            ui->pushButton_27->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/53.bmp);");
        }
        else
        {
            ui->pushButton_27->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/RFID_small.bmp);");
        }
        Queuelock.lockForRead();
        if(isQueue)
        {
            if(!BAICQueue && Factory == "BAIC")
            {
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font: 12pt \"黑体\";color: rgb(248, 248, 255);");
                ui->pushButton_queue->setText(tr("自动条码枪"));
            }
            else
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/queue1.bmp);");
        }
        else
        {
            if(!BAICQueue && Factory == "BAIC")
            {
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font: 12pt \"黑体\";color: rgb(248, 248, 255);");
                ui->pushButton_queue->setText(tr("自动条码枪"));
            }
            else
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/queue.bmp);");
        }
        Queuelock.unlock();

        if (Line_ID == 0)
        {
            ui->Line_radioButton_0->setChecked(true);
            ui->Line_radioButton_1->setChecked(false);
            ui->Line_radioButton_2->setChecked(false);
        }
        else if (Line_ID == 1)
        {
            ui->Line_radioButton_0->setChecked(false);
            ui->Line_radioButton_1->setChecked(true);
            ui->Line_radioButton_2->setChecked(false);
        }
        else if (Line_ID == 2)
        {
            ui->Line_radioButton_0->setChecked(false);
            ui->Line_radioButton_1->setChecked(false);
            ui->Line_radioButton_2->setChecked(true);
        }

        if(partY1V == 1)
        {
           ui->targetNumRadio->setChecked(true);
           ui->maxNumRadio   ->setChecked(false);
        }
        else if(partY1V == 2)
        {
            ui->targetNumRadio->setChecked(false);
            ui->maxNumRadio   ->setChecked(true);
        }
        else {
            ui->targetNumRadio->setChecked(true);
        }
        delete configIniRead;
        ShowTime();
    }
}

void Newconfiginfo::on_pushButton_26_clicked()
{
    //条码枪
    if(isbarcode)
    {
        ui->pushButton_26->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/barcode.bmp);");
    }
    else
    {
        ui->pushButton_26->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/52.bmp);");
    }
    isbarcode = !isbarcode;
}

void Newconfiginfo::on_pushButton_27_clicked()
{
    //RFID
    if(isrfid)
    {
        ui->pushButton_27->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/RFID_small.bmp);");
    }
    else
    {
        ui->pushButton_27->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/53.bmp);");
    }
    isrfid = !isrfid;
}

void Newconfiginfo::on_pushButton_queue_clicked()
{
    //队列
    if(isqueue)
    {
        if(!BAICQueue && Factory == "BAIC")
        {
            ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font: 12pt \"黑体\";color: rgb(248, 248, 255);");
            ui->pushButton_queue->setText(tr("自动条码枪"));
        }
        else
            ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/queue.bmp);");
    }
    else
    {
        if(!BAICQueue && Factory == "BAIC")
        {
            ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font: 12pt \"黑体\";color: rgb(248, 248, 255);");
            ui->pushButton_queue->setText(tr("自动条码枪"));
        }
        else
            ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/queue1.bmp);");
    }
    isqueue = !isqueue;
}

void Newconfiginfo::receiveSaveState(bool statetmp)
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    if(statetmp)
    {
        ui->label_100->hide();
        delete e3;
        delete save;

        configIniRead->setValue("baseinfo/offVOLTAGE", ui->comboBox_VOLTAGE->currentText().toInt());//关机的电量等级
        int offTime = ui->lineEdit_offTime->text().toInt();
        if(offTime > 0){
            configIniRead->setValue("baseinfo/offTime", offTime);//断电状态关机的时间
        }

        configIniRead->setValue("baseinfo/IoBoxSerial", ui->comboBox_IoBoxSerial->currentText());

        configIniRead->setValue("baseinfo/restartTime",ui->restart_hour->text()+ui->restart_minute->text()+ui->restart_second->text());
        restart_enabled = restart_enabled_temp;
        configIniRead->setValue("baseinfo/restart_enabled",restart_enabled?"1":"0");

        configIniRead->setValue("baseinfo/isBarCode",QString::number(isbarcode));
        isBarCode = isbarcode;    //need to restart
        configIniRead->setValue("baseinfo/isRFID",QString::number(isrfid));
        isRFID = isrfid;
        configIniRead->setValue("baseinfo/isQueue",QString::number(isqueue));
        Queuelock.lockForWrite();
        isQueue = isqueue;
        Queuelock.unlock();

        if(ui->Line_radioButton_0->isChecked())
        {
            configIniRead->setValue("baseinfo/Line_ID","0");
            Line_ID = 0;
            line_ID = 0;
        }
        else if(ui->Line_radioButton_1->isChecked())
        {
            configIniRead->setValue("baseinfo/Line_ID","1");
            Line_ID = 1;
            line_ID = 1;
        }
        else if(ui->Line_radioButton_2->isChecked())
        {
            configIniRead->setValue("baseinfo/Line_ID","2");
            Line_ID = 2;
            line_ID = 2;
        }

         if(ui->targetNumRadio->isChecked()) //目标值
         {
             configIniRead->setValue("baseinfo/Y1V","1");
             partY1V = 1 ;
             Y1V     = 1 ;
         }
         else if(ui->maxNumRadio->isChecked()) //最大值
         {
             configIniRead->setValue("baseinfo/Y1V","2");
             partY1V = 2 ;
             Y1V     = 2 ;
         }
        configIniRead->setValue("baseinfo/TackTime",ui->lineEdit_tacktime->text());    //节拍时间
        configIniRead->setValue("baseinfo/OffSet",ui->lineEdit_offset->text());

        if(ui->radioButton_wireless->isChecked())
            configIniRead->setValue("baseinfo/connectNet","0");
        else if(ui->radioButton_wire1->isChecked())
            configIniRead->setValue("baseinfo/connectNet","1");
        else if(ui->radioButton_wire2->isChecked())
            configIniRead->setValue("baseinfo/connectNet","2");


        configIniRead->setValue("baseinfo/upInversion",ui->upInversioncheck->isChecked());
        DTdebug() << "strUpVersion" << ui->upInversioncheck->isChecked();

        if (ui->year->text().toInt() != year||
                ui->month->text().toInt() != month||
                ui->date->text().toInt() != date||
                ui->hour->text().toInt() != hour||
                ui->minute->text().toInt() != minute||
                ui->second->text().toInt() != second)
        {
            QString str = "date -s \"" +ui->year->text()+"-"+ui->month->text() + "-" + ui->date->text()+ " "+ui->hour->text() + ":" + ui->minute->text() + ":" + ui->second->text()+"\" &";

            //DTdebug()<<"str"<<str;
            system(str.toLatin1().data());

            //将系统时间写入RTC
            system("hwclock -w &");
            year=ui->year->text().toInt();
            month=ui->month->text().toInt();
            date=ui->date->text().toInt();
            hour=ui->hour->text().toInt();
            minute=ui->minute->text().toInt();
            second=ui->second->text().toInt();

        }

        if (ui->lineEdit_netmask->text() != configIniRead->value("baseinfo/netmask"))
        {
            configIniRead->setValue("baseinfo/netmask",ui->lineEdit_netmask->text());
#if 0
                //20190725
            if(ui->radioButton_wire1->isChecked())
                system(QString("ifconfig eth0 netmask "+ui->lineEdit_netmask->text()+"  &").toLatin1().data());
            else if(ui->radioButton_wire2->isChecked())
                system(QString("ifconfig eth1 netmask "+ui->lineEdit_netmask->text()+"  &").toLatin1().data());
            else if(ui->radioButton_wireless->isChecked())
                system(QString("ifconfig wlan0 netmask "+ui->lineEdit_netmask->text()+" &").toLatin1().data());
            //                system((QString("ifconfig wlan0 netmask ")+ui->lineEdit_netmask->text()+" &").toLatin1().data());
            //            QString fileName = "/etc/profile";
            //            QFile file(fileName);
            //            if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
            //                DTdebug()   << "Cannot open profile file for Reading";
            //            }
            //            QString str (file.readAll());
            //            if(str.contains("netmask", Qt::CaseInsensitive)){
            //                str.replace(QRegExp("netmask \\S*"),QString("netmask ")+ui->lineEdit_netmask->text());
            //            }
            //            file.close();
            //            if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            //                DTdebug()   << "Cannot open profile file for Writing";
            //            }
            //            file.write(str.toUtf8());
            //            file.close();
#endif
        }


        if (ui->lineEdit_ssid->text() != configIniRead->value("baseinfo/SSID")||
                ui->lineEdit_psk->text() != configIniRead->value("baseinfo/psk"))
        {
            configIniRead->setValue("baseinfo/SSID",ui->lineEdit_ssid->text());
            configIniRead->setValue("baseinfo/psk",ui->lineEdit_psk->text());
        }

        configIniRead->setValue("baseinfo/gateway",ui->lineEdit_gateway->text());
        if(ui->radioButton_wire1->isChecked())
        {
            system("route del default eth0");
            system((QString("route add default gw ")+configIniRead->value("baseinfo/gateway").toString()+" eth0 &").toLatin1().data());
        }
        else if(ui->radioButton_wire2->isChecked())
        {
            system("route del default eth1");
            system((QString("route add default gw ")+configIniRead->value("baseinfo/gateway").toString()+" eth1 &").toLatin1().data());
        }
        else if(ui->radioButton_wireless->isChecked())
        {
            system("route del default wlan0");
            system((QString("route add default gw ")+configIniRead->value("baseinfo/gateway").toString()+" wlan0 &").toLatin1().data());
        }

        if (ui->lineEdit_APssid->text() != configIniRead->value("baseinfo/APSSID")||
                ui->lineEdit_APpsk->text() != configIniRead->value("baseinfo/APpsk")||
                ui->pushButton_restart_2->isChecked()!= configIniRead->value("baseinfo/WIFIHotSpot").toBool())
        {
            configIniRead->setValue("baseinfo/APSSID",ui->lineEdit_APssid->text());
            configIniRead->setValue("baseinfo/APpsk",ui->lineEdit_APpsk->text());

            {

                ModifySSID(ui->lineEdit_APssid->text(), ui->lineEdit_APpsk->text());

                if((ui->pushButton_restart_2->isChecked() == configIniRead->value("baseinfo/WIFIHotSpot").toBool()) && ui->pushButton_restart_2->isChecked())
                {
                    configIniRead->setValue("baseinfo/WIFIHotSpot","1");
                    ReStartWifi();
                }

                else if(ui->pushButton_restart_2->isChecked())
                {
                    configIniRead->setValue("baseinfo/WIFIHotSpot","1");
                    StartWifi();
                }
                else
                {
                    configIniRead->setValue("baseinfo/WIFIHotSpot","0");
                    StopWifi();
                }
            }

            QString fileName;
            fileName = "/etc/Wireless/RT2870AP/RT2870AP.dat";
            QFile file(fileName);
            if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
                DTdebug()   << "Cannot open AP file for Reading";
            }
            QString str (file.readAll());
            if(str.contains("\nSSID=", Qt::CaseInsensitive)&&str.contains("WPAPSK=", Qt::CaseInsensitive)){
#if 0
                //20190725
                str.replace(QRegExp("\nSSID=\\w*"),QString("\nSSID=")+ui->lineEdit_APssid->text());
                str.replace(QRegExp("WPAPSK=\\w*"),QString("WPAPSK=")+ui->lineEdit_APpsk->text());
#endif
                DTdebug()<<"AP changed";
            }
            file.close();
            QFile::remove("/etc/Wireless/RT2870AP/RT2870AP.dat");
            if(!file.open(QIODevice::WriteOnly| QIODevice::Text)){
                DTdebug() << "Cannot open AP file for Writing";
            }
            file.write(str.toUtf8());
            file.close();
            system("ifconfig ra0 down &");
            system("ifconfig ra0 up &");

        }

        delete configIniRead;
    }else
    {
        int offVOLTAGE_index = ui->comboBox_VOLTAGE->findText(configIniRead->value("baseinfo/offVOLTAGE").toString());
        if(offVOLTAGE_index >= 0){
            ui->comboBox_VOLTAGE->setCurrentIndex(offVOLTAGE_index);
        }
        int offTime = configIniRead->value("baseinfo/offTime").toInt();
        if(offTime > 0){
            ui->lineEdit_offTime->setText(QString::number(offTime));
        }


        int IoBoxSerial_index = ui->comboBox_IoBoxSerial->findText(configIniRead->value("baseinfo/IoBoxSerial").toString());
        if(IoBoxSerial_index >= 0){
            ui->comboBox_IoBoxSerial->setCurrentIndex(IoBoxSerial_index);
        }

        ui->lineEdit_netmask->setText(configIniRead->value("baseinfo/netmask").toString());
        ui->lineEdit_gateway->setText(configIniRead->value("baseinfo/gateway").toString());
        ui->lineEdit_ssid->setText(configIniRead->value("baseinfo/SSID").toString());
        ui->lineEdit_psk->setText(configIniRead->value("baseinfo/psk").toString());
        ui->lineEdit_APssid->setText(configIniRead->value("baseinfo/APSSID").toString());
        ui->lineEdit_APpsk->setText(configIniRead->value("baseinfo/APpsk").toString());
        ui->lineEdit_tacktime->setText(configIniRead->value("baseinfo/TackTime").toString());   //节拍时间
        ui->lineEdit_offset->setText(configIniRead->value("baseinfo/OffSet").toString());
        ui->restart_hour->setText(configIniRead->value("baseinfo/restartTime").toString().mid(0,2));
        ui->restart_minute->setText(configIniRead->value("baseinfo/restartTime").toString().mid(2,2));
        ui->restart_second->setText(configIniRead->value("baseinfo/restartTime").toString().mid(4,2));


        if(configIniRead->value("baseinfo/connectNet").toInt()==0)
            ui->radioButton_wireless->setChecked(true);
        else if(configIniRead->value("baseinfo/connectNet").toInt()==1)
            ui->radioButton_wire1->setChecked(true);
        else if(configIniRead->value("baseinfo/connectNet").toInt()==2)
            ui->radioButton_wire2->setChecked(true);

        delete configIniRead;

        restart_enabled_temp = restart_enabled;
        if(restart_enabled_temp)
        {
            ui->pushButton_restart->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
            restartShow(true);
        }
        else
        {
            ui->pushButton_restart->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
            restartShow(false);
        }

        isbarcode = isBarCode;
        isrfid = isRFID;
        Queuelock.lockForRead();
        isqueue = isQueue;
        Queuelock.unlock();
        if(Line_ID == 1)
        {
            line_ID = 1;
            ui->Line_radioButton_1->setChecked(true);
            ui->Line_radioButton_2->setChecked(false);
        }
        else if(Line_ID == 2)
        {
            line_ID = 2;
            ui->Line_radioButton_1->setChecked(false);
            ui->Line_radioButton_2->setChecked(true);
        }

        if(Y1V == 1)
        {
            partY1V = 1;
            ui->targetNumRadio->setChecked(true);
            ui->maxNumRadio   ->setChecked(false);
        }
        else if(Y1V == 2)
        {
            partY1V = 2;
            ui->targetNumRadio->setChecked(false);
            ui->maxNumRadio   ->setChecked(true);
        }

        if(isbarcode)
        {
            ui->pushButton_26->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/52.bmp);");
        }
        else
        {
            ui->pushButton_26->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/barcode.bmp);");
        }
        if(isrfid)
        {
            ui->pushButton_27->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/53.bmp);");
        }
        else
        {
            ui->pushButton_27->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/RFID_small.bmp);");
        }
        //        Queuelock.lockFo rRead();
        if(isqueue)
        {
            if(Factory == "BAIC")
            {
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/9.bmp);font: 12pt \"黑体\";color: rgb(248, 248, 255);");
                ui->pushButton_queue->setText(tr("自动条码枪"));
            }
            else
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/queue1.bmp);");
        }
        else
        {
            if(Factory == "BAIC")
            {
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/67.bmp);font: 12pt \"黑体\";color: rgb(248, 248, 255);");
                ui->pushButton_queue->setText(tr("自动条码枪"));
            }
            else
                ui->pushButton_queue->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/queue.bmp);");
        }
        //        Queuelock.unlock();
        ui->label_100->hide();
        delete e3;
        delete save;
    }
    if(whichButtonClick == "advancedback")
    {
        on_pushButton_3_clicked();
        isadvancedchange = false;
    }
    else if(whichButtonClick == "passwordchange")
    {
        on_pushButton_100_clicked();
        isadvancedchange = false;
    }
    else if(whichButtonClick == "testinterface")
    {
        on_pushButton_66_clicked();
        isadvancedchange = false;
    }
    else if(whichButtonClick == "fisupdatecolumn")
    {
        on_pushButton_67_clicked();
        isadvancedchange = false;
    }
    else if(whichButtonClick == "master_slave")
    {
        on_pushButton_86_clicked();
        isadvancedchange = false;
    }
    else if(whichButtonClick == "bound_config")
    {
        on_pushButton_55_clicked();
        isadvancedchange = false;
    }
    else if (whichButtonClick == "wirelessLocation")
    {
        on_pushButton_wirelessLocation_clicked();
        isadvancedchange = false;
    }
    else if(whichButtonClick == "saveadvanced")
    {
        isadvancedchange = false;
        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        QString SingleOK = GetLightState(ui->comboBox_SingleOK_red, ui->comboBox_SingleOK_green, ui->comboBox_SingleOK_yellow, ui->comboBox_SingleOK_white, ui->comboBox_SingleOK_lockled);
        configIniRead->setValue("baseinfo/LightLogic_SingleOK",SingleOK);

        QString SingleFail = GetLightState(ui->comboBox_SingleFail_red, ui->comboBox_SingleFail_green, ui->comboBox_SingleFail_yellow, ui->comboBox_SingleFail_white, ui->comboBox_SingleFail_lockled);
        configIniRead->setValue("baseinfo/LightLogic_SingleFail",SingleFail);


        QString GroupOK = GetLightState(ui->comboBox_GroupOK_red, ui->comboBox_GroupOK_green, ui->comboBox_GroupOK_yellow, ui->comboBox_GroupOK_white, ui->comboBox_GroupOK_lockled);
        configIniRead->setValue("baseinfo/LightLogic_GroupOK",GroupOK);

        QString GroupFail = GetLightState(ui->comboBox_GroupFail_red, ui->comboBox_GroupFail_green, ui->comboBox_GroupFail_yellow, ui->comboBox_GroupFail_white, ui->comboBox_GroupFail_lockled);
        configIniRead->setValue("baseinfo/LightLogic_GroupFail",GroupFail);

        QString ING = GetLightState(ui->comboBox_ING_red, ui->comboBox_ING_green, ui->comboBox_ING_yellow, ui->comboBox_ING_white, ui->comboBox_ING_lockled);
        configIniRead->setValue("baseinfo/LightLogic_ING",ING);

        QString NotING = GetLightState(ui->comboBox_NotING_red, ui->comboBox_NotING_green, ui->comboBox_NotING_yellow, ui->comboBox_NotING_white, ui->comboBox_NotING_lockled);
        configIniRead->setValue("baseinfo/LightLogic_NotING",NotING);

        QString Enable = GetLightState(ui->comboBox_Enable_red, ui->comboBox_Enable_green, ui->comboBox_Enable_yellow, ui->comboBox_Enable_white, ui->comboBox_Enable_lockled);
        configIniRead->setValue("baseinfo/LightLogic_Enable",Enable);

        QString Disable = GetLightState(ui->comboBox_Disable_red, ui->comboBox_Disable_green, ui->comboBox_Disable_yellow, ui->comboBox_Disable_white, ui->comboBox_Disable_lockled);
        configIniRead->setValue("baseinfo/LightLogic_Disable",Disable);

        QString WaitForScan = GetLightState(ui->comboBox_WaitForScan_red, ui->comboBox_WaitForScan_green, ui->comboBox_WaitForScan_yellow, ui->comboBox_WaitForScan_white, ui->comboBox_WaitForScan_lockled);
        configIniRead->setValue("baseinfo/LightLogic_WaitForScan",WaitForScan);

        QString ScanFinishThenWait = GetLightState(ui->comboBox_ScanFinishThenWait_red, ui->comboBox_ScanFinishThenWait_green, ui->comboBox_ScanFinishThenWait_yellow, ui->comboBox_ScanFinishThenWait_white, ui->comboBox_ScanFinishThenWait_lockled);
        configIniRead->setValue("baseinfo/LightLogic_ScanFinishThenWait",ScanFinishThenWait);

        QString TightenConnect = GetLightState(ui->comboBox_TightenConnect_red, ui->comboBox_TightenConnect_green, ui->comboBox_TightenConnect_yellow, ui->comboBox_TightenConnect_white, ui->comboBox_TightenConnect_lockled);
        configIniRead->setValue("baseinfo/LightLogic_TightenConnect",TightenConnect);

        QString TightenDisConnect = GetLightState(ui->comboBox_TightenDisConnect_red, ui->comboBox_TightenDisConnect_green, ui->comboBox_TightenDisConnect_yellow, ui->comboBox_TightenDisConnect_white, ui->comboBox_TightenDisConnect_lockled);
        configIniRead->setValue("baseinfo/LightLogic_TightenDisConnect",TightenDisConnect);

        QString NetWorkConnect = GetLightState(ui->comboBox_NetWorkConnect_red, ui->comboBox_NetWorkConnect_green, ui->comboBox_NetWorkConnect_yellow, ui->comboBox_NetWorkConnect_white, ui->comboBox_NetWorkConnect_lockled);
        configIniRead->setValue("baseinfo/LightLogic_NetWorkConnect",NetWorkConnect);

        QString NetWorkDisConnect = GetLightState(ui->comboBox_NetWorkDisConnect_red, ui->comboBox_NetWorkDisConnect_green, ui->comboBox_NetWorkDisConnect_yellow, ui->comboBox_NetWorkDisConnect_white, ui->comboBox_NetWorkDisConnect_lockled);
        configIniRead->setValue("baseinfo/LightLogic_NetWorkDisConnect",NetWorkDisConnect);

        QString SingleSkip = GetLightState(ui->comboBox_SingleSkip_red, ui->comboBox_SingleSkip_green, ui->comboBox_SingleSkip_yellow, ui->comboBox_SingleSkip_white, ui->comboBox_SingleSkip_lockled);
        configIniRead->setValue("baseinfo/LightLogic_SingleSkip",SingleSkip);

        configIniRead->deleteLater();
    }
}


void Newconfiginfo::on_pushButton_34_clicked()
{
    //保存
    whichButtonClick = "saveadvanced";
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);

    SaveWhat = "advanced";
    save = new Save(this);
    connect(save,SIGNAL(sendSaveAdvancedState(bool)),this,SLOT(receiveSaveState(bool)));
    save->show();
}


void Newconfiginfo::on_pushButton_13_clicked()
{
    if(Factory == "Benz")
        ui->stackedWidget_2->setCurrentIndex(13);
    else
        ui->stackedWidget_2->setCurrentIndex(8);
    ui->stackedWidget_3->setCurrentIndex(3);
    ui->pushButton_96->setText("关于");
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    ui->label_SN->setText(configIniRead->value("baseinfo/SN").toString());
    ui->label_SN_2->setText(configIniRead->value("baseinfo/SN").toString());
    delete configIniRead;
}

void Newconfiginfo::on_pushButton_2_clicked()
{
    //    if (whichButtonClick == "about")
    //    {
    //        this->close();
    //        if(isJS)
    //            ui->stackedWidget_2->setCurrentIndex(0);
    //        else
    //            ui->stackedWidget_2->setCurrentIndex(2);
    //        ui->listWidget->setCurrentRow(1);
    //        ui->stackedWidget_3->setCurrentIndex(0);
    //        ui->label_83->show();
    //        ui->label_84->hide();
    //        ui->label_85->hide();
    //        ui->label_86->hide();
    //        ui->label_157->show();
    //        ui->pushButton_54->show();
    //    }
    //    else
    //    {
    //        ui->stackedWidget_3->setCurrentIndex(0);
    ui->stackedWidget_5->setCurrentIndex(0);
    //        ui->stackedWidget_2->setCurrentIndex(4);
    ui->stackedWidget_3->setCurrentIndex(3);
    //        if(isJS)
    //        {
    //            ui->stackedWidget_2->setCurrentIndex(0);
    //            ui->label_83->show();
    //            ui->label_84->hide();
    //            ui->label_85->hide();
    //        }
    //        else
    //        {
    //ui->stackedWidget_2->setCurrentIndex(2);
    on_pushButton_14_clicked();
    //        }

    //    }
}

void Newconfiginfo::on_pushButton_3_clicked()
{
    whichButtonClick = "advancedback";
    if(!isadvancedchange)
        advancedIsChange();
    else if(!ismasterslavechange)
        masterslaveIsChange();
    else if(!isboundchange)
        boundIsChange();
    else if(!islocationchange)
        locationIsChange();
    else
    {
        ui->stackedWidget_2->setCurrentIndex(2);
        ui->stackedWidget_3->setCurrentIndex(3);
        ui->lineEdit_column->setText("");

        //        if(isRFID == 1)   //RFID 条码抢模式的 界面初始化
        //        {
        //            ui->stackedWidget_4->setCurrentIndex(0);
        //        }
        //        else
        //        {
        //            ui->stackedWidget_4->setCurrentIndex(1);
        //        }
    }
}



//! [时钟更改]

void Newconfiginfo::ShowTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QTime current_time = QTime::currentTime();
    year=dateTime.date().year();
    month=dateTime.date().month();
    date=dateTime.date().day();

    hour = current_time.hour();
    minute = current_time.minute();
    second = current_time.second();

    ui->year->setText(QString::number(year));
    ui->month->setText(QString::number(month));
    ui->date->setText(QString::number(date));
    ui->hour->setText(QString::number(hour));
    ui->minute->setText(QString::number(minute));
    ui->second->setText(QString::number(second));
}

void Newconfiginfo::on_pushButton_68_clicked()
{
    int new_year=(ui->year->text()).toInt()+1;
    ui->year->setText(QString::number(new_year));
}

void Newconfiginfo::on_pushButton_69_clicked()
{
    int new_year=(ui->year->text()).toInt()-1;
    ui->year->setText(QString::number(new_year));
}

void Newconfiginfo::on_pushButton_80_clicked()
{
    if(ui->month->text().toInt()<12)
    {
        int new_month=(ui->month->text()).toInt()+1;
        ui->month->setText(QString::number(new_month));
    }
    else
        ui->month->setText(QString::number(1));
}

void Newconfiginfo::on_pushButton_85_clicked()
{
    if(ui->month->text().toInt()>1)
    {
        int new_month=(ui->month->text()).toInt()-1;
        ui->month->setText(QString::number(new_month));
    }
    else
        ui->month->setText(QString::number(12));
}

void Newconfiginfo::on_pushButton_81_clicked()
{
    if(ui->date->text().toInt()<31)
    {
        int new_date=(ui->date->text()).toInt()+1;
        ui->date->setText(QString::number(new_date));
    }
    else
        ui->date->setText(QString::number(1));
}

void Newconfiginfo::on_pushButton_92_clicked()
{
    if(ui->date->text().toInt()>1)
    {
        int new_date=(ui->date->text()).toInt()-1;
        ui->date->setText(QString::number(new_date));
    }
    else
        ui->date->setText(QString::number(31));
}

void Newconfiginfo::on_pushButton_82_clicked()
{
    if(ui->hour->text().toInt()<23)
    {
        int new_hour=(ui->hour->text()).toInt()+1;
        ui->hour->setText(QString::number(new_hour));
    }
    else
        ui->hour->setText(QString::number(0));
}

void Newconfiginfo::on_pushButton_93_clicked()
{
    if(ui->hour->text().toInt()>0)
    {
        int new_hour=(ui->hour->text()).toInt()-1;
        ui->hour->setText(QString::number(new_hour));
    }
    else
        ui->hour->setText(QString::number(23));
}

void Newconfiginfo::on_pushButton_83_clicked()
{
    if(ui->minute->text().toInt()<59)
    {
        int new_minute=(ui->minute->text()).toInt()+1;
        ui->minute->setText(QString::number(new_minute));
    }
    else
        ui->minute->setText(QString::number(0));
}

void Newconfiginfo::on_pushButton_94_clicked()
{
    if(ui->minute->text().toInt()>0)
    {
        int new_minute=(ui->minute->text()).toInt()-1;
        ui->minute->setText(QString::number(new_minute));
    }
    else
        ui->minute->setText(QString::number(59));
}

void Newconfiginfo::on_pushButton_84_clicked()
{
    if(ui->second->text().toInt()<59)
    {
        int new_second=(ui->second->text()).toInt()+1;
        ui->second->setText(QString::number(new_second));
    }
    else
        ui->second->setText(QString::number(0));
}

void Newconfiginfo::on_pushButton_95_clicked()
{
    if(ui->second->text().toInt()>0)
    {
        int new_second=(ui->second->text()).toInt()-1;
        ui->second->setText(QString::number(new_second));
    }
    else
        ui->second->setText(QString::number(59));
}

//! [时钟更改]

void Newconfiginfo::on_pushButton_58_clicked()
{
    if(ui->stackedWidget_2->currentIndex()!=6)
    {
        whichButtonClick = "advanceset";
        if(!ismasterslavechange)
            masterslaveIsChange();
        else if(!isboundchange)
            boundIsChange();
        else if(!islocationchange)
            locationIsChange();
        else
        {
            ui->lineEdit_column->setText("");
            ShowTime();
            ui->stackedWidget_2->setCurrentIndex(6);
            ui->label_87->show();
            ui->label_88->hide();
            ui->label_101->hide();
            ui->label_98->hide();
            ui->label_99->hide();
            ui->label_162->hide();
            ui->label_wirelessLocation->hide();
            //            if( system("ping -c 3 10.0.0.1 -i 0.1")!= 0)
            //            {
            //                DTdebug()<<"网络连接失败";
            //                //ui->label_wifi->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/unconnect.PNG);");
            //            }
            //            else
            //            {
            //                DTdebug()<<"网络连接成功";
            //                //ui->label_wifi->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/connect.PNG);");
            //            }
        }
    }
}

void Newconfiginfo::on_pushButton_66_clicked()
{
    DebugMode = true;
    if(ui->stackedWidget_2->currentIndex() != 7)
    {
        whichButtonClick = "testinterface";
        if(!isadvancedchange)
            advancedIsChange();
        else if(!ismasterslavechange)
            masterslaveIsChange();
        else if(!isboundchange)
            boundIsChange();
        else if(!islocationchange)
            locationIsChange();
        else
        {
            ui->lineEdit_staname_5->clear();
            ui->lineEdit_column->setText("");
            ui->stackedWidget_2->setCurrentIndex(7);

            ui->label_87->hide();
            ui->label_101->hide();
            ui->label_88->show();
            ui->label_98->hide();
            ui->label_99->hide();
            ui->label_162->hide();
            ui->label_wirelessLocation->hide();
            //        ui->stackedWidget_3->setCurrentIndex(3);
            //        ui->pushButton_96->setText(tr("测试界面"));
        }
    }
}

//! [密码更改]

void Newconfiginfo::on_pushButton_100_clicked()
{
    //    if(ui->stackedWidget_2->currentIndex() != 9)
    //    {
    //        whichButtonClick = "passwordchange";
    //        if(!isadvancedchange)
    //            advancedIsChange();
    //        else if(!ismasterslavechange)
    //            masterslaveIsChange();
    //        else if(!isboundchange)
    //            boundIsChange();
    //        else
    //        {
    //            ui->lineEdit_column->setText("");

    ui->stackedWidget_3->setCurrentIndex(3);
    ui->pushButton_96->setText("更改密码");
    ui->stackedWidget_2->setCurrentIndex(9);
    ui->label_87->hide();
    ui->label_101->show();
    ui->label_88->hide();
    ui->label_98->hide();
    ui->label_99->hide();
    ui->label_162->hide();
    ui->label_wirelessLocation->hide();
    //        ui->stackedWidget_3->setCurrentIndex(3);
    //        ui->pushButton_96->setText(tr("更改密码"));
    isFull = 0;
    person = 0;
    change = 0;
    temp = "";
    newpassword = "";
    ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
    ui->pushButton_error->setStyleSheet("border-image : url(:)");
    ui->label_hint->setStyleSheet("border-image : url(:/re/111.bmp)");
    //    ui->pushButton_98->setStyleSheet("border-image : url(:/re/93.bmp)");
    //    ui->pushButton_97->setStyleSheet("border-image : url(:/re/100.png)");
    //        }
    //    }
}


void Newconfiginfo::on_pushButton_num1_clicked()
{
    temp.append("1");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num2_clicked()
{
    temp.append("2");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num3_clicked()
{
    temp.append("3");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num4_clicked()
{
    temp.append("4");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num5_clicked()
{
    temp.append("5");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num6_clicked()
{
    temp.append("6");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num7_clicked()
{
    temp.append("7");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num8_clicked()
{
    temp.append("8");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num9_clicked()
{
    temp.append("9");
    isFull++;
    judge();
}

void Newconfiginfo::on_pushButton_num0_clicked()
{
    temp.append("0");
    isFull++;
    judge();
}


void Newconfiginfo::on_pushButton_delete_clicked()
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

void Newconfiginfo::judge()
{
    if(isFull == 1)
    {
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/95.bmp)");
        ui->pushButton_error->setStyleSheet("border-image: url(:)");
    }
    else if(isFull == 2)
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/96.bmp)");
    else if(isFull == 3)
        ui->pushButton_password->setStyleSheet("border-image : url(:/re/97.bmp)");
    else if(isFull == 4)
    {
        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        //        if (person ==0)
        //        {
        if (change == 0)
        {
            if( (temp == configIniRead->value(QString("baseinfo/GCpassword")).toString())
                    ||(temp == "5027") )
            {
                ui->label_hint->setStyleSheet("border-image : url(:/re/112.bmp)");
                ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
                ui->pushButton_error->setStyleSheet("border-image : url(:)");
                change = 1;
                isFull = 0;
                temp = "";
            }
            else
            {
                ui->pushButton_error->setStyleSheet("border-image : url(:/re/124.bmp)");
                ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
                isFull = 0;
                temp = "";
            }
        }
        else if (change == 1)
        {
            newpassword=temp;
            isFull = 0;
            temp = "";
            ui->label_hint->setStyleSheet("border-image : url(:/re/113.bmp)");
            ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
            ui->pushButton_error->setStyleSheet("border-image : url(:)");
            change = 2;
        }
        else if (change == 2)
        {
            if(temp == newpassword)
            {
                ui->pushButton_password->setStyleSheet("border-image : url(:/re/98.bmp)");
                configIniRead->setValue("baseinfo/GCpassword",newpassword);
                //                    ui->stackedWidget_2->setCurrentIndex(6);
                ui->label_87->show();
                ui->label_101->hide();
                this->close();
                //                    ui->stackedWidget_3->setCurrentIndex(2);
            }
            else
            {
                ui->label_hint->setStyleSheet("border-image : url(:/re/113.bmp)");
                ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
                ui->pushButton_error->setStyleSheet("border-image : url(:/re/125.bmp)");
                isFull = 0;
                temp = "";
            }
        }
        //        }
        //        else if (person ==1)
        //        {
        //            if (change == 0)
        //            {
        //                if(temp == configIniRead->value(QString("baseinfo/JSpassword")).toString())
        //                {
        //                    ui->label_hint->setStyleSheet("border-image : url(:/re/112.bmp)");
        //                    ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        //                    ui->pushButton_error->setStyleSheet("border-image : url(:)");
        //                    change = 1;
        //                    isFull = 0;
        //                    temp = "";
        //                }
        //                else
        //                {
        //                    ui->pushButton_error->setStyleSheet("border-image : url(:/re/124.bmp)");
        //                    ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        //                    isFull = 0;
        //                    temp = "";
        //                }
        //            }
        //            else if (change == 1)
        //            {
        //                newpassword=temp;
        //                isFull = 0;
        //                temp = "";
        //                ui->label_hint->setStyleSheet("border-image : url(:/re/113.bmp)");
        //                ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        //                ui->pushButton_error->setStyleSheet("border-image : url(:)");
        //                change = 2;
        //            }
        //            else if (change == 2)
        //            {
        //                if(temp == newpassword)
        //                {
        //                    ui->pushButton_password->setStyleSheet("border-image : url(:/re/98.bmp)");
        //                    configIniRead->setValue("baseinfo/JSpassword",newpassword);
        //                    ui->stackedWidget_2->setCurrentIndex(6);
        //                    ui->label_87->show();
        //                    ui->label_101->hide();
        //                    //                    ui->stackedWidget_3->setCurrentIndex(2);
        //                }
        //                else
        //                {
        //                    ui->label_hint->setStyleSheet("border-image : url(:/re/113.bmp)");
        //                    ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
        //                    ui->pushButton_error->setStyleSheet("border-image : url(:/re/125.bmp)");
        //                    isFull = 0;
        //                    temp = "";
        //                }
        //            }
        //        }
        delete configIniRead;
    }
}


void Newconfiginfo::on_pushButton_98_clicked()
{
    //    if(person != 0)
    //    {
    //        ui->pushButton_98->setStyleSheet("border-image : url(:/re/93.bmp)");
    //        ui->pushButton_97->setStyleSheet("border-image: url(:/re/100.png)");
    //        ui->label_hint->setStyleSheet("border-image : url(:/re/111.bmp)");
    //        ui->pushButton_error->setStyleSheet("border-image : url(:)");
    //        ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
    //        isFull = 0;
    //        temp = "";
    //        person = 0;
    //        change = 0;
    //        newpassword = "";
    //    }
}

void Newconfiginfo::on_pushButton_97_clicked()
{
    //    if(person != 1)
    //    {
    //        ui->pushButton_97->setStyleSheet("border-image : url(:/re/92.bmp)");
    //        ui->pushButton_98->setStyleSheet("border-image: url(:/re/99.png)");
    //        ui->label_hint->setStyleSheet("border-image : url(:/re/111.bmp)");
    //        ui->pushButton_error->setStyleSheet("border-image : url(:)");
    //        ui->pushButton_password->setStyleSheet("border-image : url(:/re/94.bmp)");
    //        isFull = 0;
    //        temp = "";
    //        person = 1;
    //        change = 0;
    //        newpassword = "";
    //    }
}
//! [密码更改]


void Newconfiginfo::receiveoptioninfo(QString a,QString b,bool c)
{
    if(c)
    {
        if(bxuanstate)
        {
            // 赋值
            bxuannamelist2[whichoption-1][bxuanwhich-1] = a;
            bxuancodelist2[whichoption-1][bxuanwhich-1] = b;
        }else
        {
            kxuannamelist2[whichoption-1][bxuanwhich-1] = a;
            kxuancodelist2[whichoption-1][bxuanwhich-1] = b;
        }

        if(bxuanwhich == 1)
        {
            if(bxuanstate)
            {
                ui->pushButton_bxuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_bx1name->setText(a);
                ui->label_bx1code->setText(b);
            }else
            {
                ui->pushButton_kxuan1->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_kx1name->setText(a);
                ui->label_kx1code->setText(b);
            }
        }else if(bxuanwhich == 2)
        {
            if(bxuanstate)
            {
                ui->pushButton_bxuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_bx2name->setText(a);
                ui->label_bx2code->setText(b);
            }
            else
            {
                ui->pushButton_kxuan2->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_kx2name->setText(a);
                ui->label_kx2code->setText(b);
            }
        }else if(bxuanwhich == 3)
        {
            if(bxuanstate)
            {
                ui->pushButton_bxuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_bx3name->setText(a);
                ui->label_bx3code->setText(b);
            }
            else
            {
                ui->pushButton_kxuan3->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_kx3name->setText(a);
                ui->label_kx3code->setText(b);
            }
        }
        else if(bxuanwhich == 4)
        {
            if(bxuanstate)
            {
                ui->pushButton_bxuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_bx4name->setText(a);
                ui->label_bx4code->setText(b);
            }
            else
            {
                ui->pushButton_kxuan4->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_kx4name->setText(a);
                ui->label_kx4code->setText(b);
            }
        }
        else if(bxuanwhich == 5)
        {
            if(bxuanstate)
            {
                ui->pushButton_bxuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_bx5name->setText(a);
                ui->label_bx5code->setText(b);
            }else
            {
                ui->pushButton_kxuan5->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/xuanpei.bmp);");
                ui->label_kx5name->setText(a);
                ui->label_kx5code->setText(b);
            }
        }
    }
    delete od;
    ui->label_100->hide();
    delete e3;



}
void Newconfiginfo::on_pushButton_bxuan1_clicked()
{
    //选配代码 弹框
    bxuanwhich = 1;
    bxuanstate = true;
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);
    od = new OptionDialog(this);


    QObject::connect(od,SIGNAL(sendoptioninfo(QString,QString,bool)),this,SLOT(receiveoptioninfo(QString,QString,bool)),Qt::AutoConnection);
    //this->setEnabled(false);
    od->show();
    od->setOptext(ui->label_bx1name->text(),ui->label_bx1code->text());

}

void Newconfiginfo::on_pushButton_bxuan2_clicked()
{

    on_pushButton_bxuan1_clicked();
    bxuanwhich = 2;
    bxuanstate = true;
    od->setOptext(ui->label_bx2name->text(),ui->label_bx2code->text());
}

void Newconfiginfo::on_pushButton_bxuan3_clicked()
{
    on_pushButton_bxuan1_clicked();
    bxuanwhich = 3;
    bxuanstate = true;
    od->setOptext(ui->label_bx3name->text(),ui->label_bx3code->text());
}

void Newconfiginfo::on_pushButton_bxuan4_clicked()
{

    on_pushButton_bxuan1_clicked();
    bxuanwhich = 4;
    bxuanstate = true;
    od->setOptext(ui->label_bx4name->text(),ui->label_bx4code->text());
}

void Newconfiginfo::on_pushButton_bxuan5_clicked()
{

    on_pushButton_bxuan1_clicked();
    bxuanwhich = 5;
    bxuanstate = true;
    od->setOptext(ui->label_bx5name->text(),ui->label_bx5code->text());
}

void Newconfiginfo::on_pushButton_kxuan1_clicked()
{
    on_pushButton_bxuan1_clicked();
    bxuanwhich =1 ;
    bxuanstate = false;
    od->setOptext(ui->label_kx1name->text(),ui->label_kx1code->text());
}

void Newconfiginfo::on_pushButton_kxuan2_clicked()
{
    on_pushButton_bxuan1_clicked();
    bxuanwhich =2 ;
    bxuanstate = false;
    od->setOptext(ui->label_kx2name->text(),ui->label_kx2code->text());
}

void Newconfiginfo::on_pushButton_kxuan3_clicked()
{
    on_pushButton_bxuan1_clicked();
    bxuanwhich =3 ;
    bxuanstate = false;
    od->setOptext(ui->label_kx3name->text(),ui->label_kx3code->text());
}

void Newconfiginfo::on_pushButton_kxuan4_clicked()
{
    on_pushButton_bxuan1_clicked();
    bxuanwhich =4 ;
    bxuanstate = false;
    od->setOptext(ui->label_kx4name->text(),ui->label_kx4code->text());
}

void Newconfiginfo::on_pushButton_kxuan5_clicked()
{
    on_pushButton_bxuan1_clicked();
    bxuanwhich =5;
    bxuanstate = false;
    od->setOptext(ui->label_kx5name->text(),ui->label_kx5code->text());
}

void Newconfiginfo::on_pushButton_89_clicked()
{
    //xuan look up
    updownReadOperate(1);

}

void Newconfiginfo::on_pushButton_90_clicked()
{
    //xuan look down
    updownReadOperate(0);

}


void Newconfiginfo::advancedIsChange()
{
    // 监听
    int connectNet = 0;
    if(ui->radioButton_wireless->isChecked())
        connectNet = 0;
    else if(ui->radioButton_wire1->isChecked())
        connectNet = 1;
    else if(ui->radioButton_wire2->isChecked())
        connectNet = 2;
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    Queuelock.lockForRead();
    if (   ui->lineEdit_netmask->text() != configIniRead->value("baseinfo/netmask")||
           ui->lineEdit_gateway->text() != configIniRead->value("baseinfo/gateway")||
           ui->lineEdit_ssid->text() != configIniRead->value("baseinfo/SSID")||
           ui->lineEdit_psk->text() != configIniRead->value("baseinfo/psk")||
           ui->lineEdit_APssid->text() != configIniRead->value("baseinfo/APSSID")||
           ui->lineEdit_APpsk->text() != configIniRead->value("baseinfo/APpsk")||
           ui->lineEdit_tacktime->text() != configIniRead->value("baseinfo/TackTime")||    //节拍时间
           ui->lineEdit_offset->text() != configIniRead->value("baseinfo/OffSet")||
           ui->comboBox_VOLTAGE->currentText() != configIniRead->value("baseinfo/offVOLTAGE")||
           ui->lineEdit_offTime->text() != configIniRead->value("baseinfo/offTime")||
           isBarCode != isbarcode||isRFID != isrfid||isQueue != isqueue||
           ui->year->text().toInt() != year||
           ui->month->text().toInt() != month||
           ui->date->text().toInt() != date||
           ui->hour->text().toInt() != hour||
           ui->minute->text().toInt() != minute||
           ui->second->text().toInt() != second||
           line_ID != Line_ID || partY1V != Y1V ||
           connectNet != configIniRead->value("baseinfo/connectNet")||
           restart_enabled_temp != restart_enabled||
           ui->restart_hour->text()+ui->restart_minute->text()+ui->restart_second->text() != configIniRead->value("baseinfo/restartTime") )

    {
        //        DTdebug()<<"??????????????????????????????????????????????????????";
        Queuelock.unlock();
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        delete configIniRead;
        SaveWhat = "advanced";
        save = new Save(this);
        connect(save,SIGNAL(sendSaveAdvancedState(bool)),this,SLOT(receiveSaveState(bool)));
        save->show();
        isadvancedchange = true;
    }
    else
    {
        Queuelock.unlock();
        delete configIniRead;
        isadvancedchange = true;
        if (whichButtonClick == "advancedback")
        {
            on_pushButton_3_clicked();
            isadvancedchange = false;
        }
        else if (whichButtonClick == "passwordchange")
        {
            on_pushButton_100_clicked();
            isadvancedchange = false;
        }
        else if (whichButtonClick == "testinterface")
        {
            on_pushButton_66_clicked();
            isadvancedchange = false;
        }
        else if (whichButtonClick == "fisupdatecolumn")
        {
            on_pushButton_67_clicked();
            isadvancedchange = false;
        }
        else if (whichButtonClick == "master_slave")
        {
            on_pushButton_86_clicked();
            isadvancedchange = false;
        }
        else if (whichButtonClick == "bound_config")
        {
            on_pushButton_55_clicked();
            isadvancedchange = false;
        }
        else if (whichButtonClick == "wirelessLocation")
        {
            on_pushButton_wirelessLocation_clicked();
            isadvancedchange = false;
        }
    }

}


void Newconfiginfo::wifi_connect()
{
    //    system("wpa_supplicant -Dwext -wlan0 -c/etc/wpa_supplicant.conf -dd&");
    //    system("udhcpc -i wlan0");
    //    system("ifconfig ra0 192.168.1.252");

    QString wifiname=ui->lineEdit_ssid->text();
    QString password=ui->lineEdit_psk->text();

    QString fileName = "/etc/wpa_supplicant.conf";
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly| QIODevice::Text)){
        DTdebug()   << "Cannot open wifi file for Reading";
    }
    QString str (file.readAll());
    if(str.contains("ssid=", Qt::CaseInsensitive)&&str.contains("psk=", Qt::CaseInsensitive)){
        //DTdebug()<<"write success!";
        str.replace(QRegExp("\nssid=\\S*"),QString("\nssid=\"")+wifiname+QString("\""));
        str.replace(QRegExp("psk=\\S*"),QString("psk=\"")+password+QString("\""));
    }
    file.close();
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
        DTdebug()   << "Cannot open wifi file for Writing";
    }
    file.write(str.toUtf8());
    file.close();

    system((QString("wpa_cli set_network 0 ssid '\"")+wifiname+QString("\"' &")).toLatin1().data());
    system((QString("wpa_cli set_network 0 psk  '\"")+password+QString("\"' &")).toLatin1().data());
    system("wpa_cli disable_network 0 &");
    system("wpa_cli enable_network 0 &");
    //    system("udhcpc -i wlan0");
    //    if( system("ping -c 3 10.0.0.1")!= 0)
    //    {
    //        DTdebug()<<"网络连接失败";
    //       // ui->label_wifi->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/unconnect.PNG);");

    //        e3 = new QGraphicsOpacityEffect(this);
    //        e3->setOpacity(0.5);
    //        ui->label_100->setGraphicsEffect(e3);
    //        ui->label_100->show();
    //        ui->label_100->setGeometry(0,0,1366,768);
    //        SaveWhat = "wifiunconnect";
    //        save = new Save(this);
    //        connect(save,SIGNAL(sendSaveBaseinfo(bool)),this,SLOT(receiveBaseinfoSave(bool)));
    //        save->show();
    //    }
    //    else
    //    {
    //        DTdebug()<<"网络连接成功";
    //       // ui->label_wifi->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/connect.PNG);");
    //    }
}


//! [历史查询翻页]
void Newconfiginfo::pagechange()
{
    thepages =QString::number((thepage-1)*10);
    aff = "select IDCode,ScrewID,Program,Torque,Angle,DATE_FORMAT(TighteningTime,'%Y-%m-%d %H:%i:%s'),TighteningStatus,UploadMark,Cycle from "+Localtable+condition+" order by RecordID DESC limit "+thepages +", 10";
    //    aff = "select top(10) IDCode,ScrewID,Torque,Angle,DATE_FORMAT(TighteningTime,'%Y-%m-%d %H:%i:%s'),TighteningStatus,UploadMark,Cycle from TighteningDatas WHERE "+condition+" and RecordID not in (select top(("+ thepages +"-1)*10) RecordID from TighteningDatas WHERE "+condition+")";
    DTdebug()<< "aff="<< aff;
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
    }
    else
    {
        model->setQuery(aff, mDataBase);
        DTdebug()<< mDataBase.lastError().text();
    }
    int i;
    for(i=0;i<10;i++)
    {
        ui->tableView->setRowHeight(i,43);
    }
}

#include <QStandardItem>
void Newconfiginfo::on_pushButton_search_clicked()
{

    //             Vin | Screwid | Nok_Reset | Nok_status
    // Selectvalue:  3 |    2    |   1       |   0         (bit)
    if(ui->tabWidget->currentIndex()==0)
    {
        int Selectvalue=0;
        VIN = ui->lineEdit_VIN_2->text();
        ScrewID = ui->lineEdit_ScrewID->text();
        TighteningStatusNOK = "NOK";
        TighteningStatusManual = "-1";
        Programno = "99";
        bool NOK_strStatus = ui->checkBox_NOK->isChecked();//TighteningStatusNOK
        bool Nok_Reset = ui->checkBox_Reset->isChecked();//TighteningStatusManual

        if(VIN != "")       Selectvalue |= 0x08;    else Selectvalue&= ~(0x08);
        if(ScrewID != "")   Selectvalue |= 0x04;    else Selectvalue&= ~(0x04);
        if(NOK_strStatus)   Selectvalue |= 0x02;    else Selectvalue &= ~(0x02);//TighteningStatusNOK = "NOK"
        if(Nok_Reset)       Selectvalue |= 0x01;    else Selectvalue &= ~(0x01);//TighteningStatusManual = "-1"

        //二次查询百分比 添加功能  查询条件
        switch (Selectvalue)
        {
        case 0x0f:
            Selectsecond = true;
            condition = " WHERE Program!='99' and IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\' and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE Program!='99'  and IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x0e:
            Selectsecond = true;
            condition = " WHERE Program!='99' and  IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\' and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\'";
            condition1 = " WHERE Program!='99' and IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x0d:
            Selectsecond = true;
            condition = " WHERE Program!='99' and IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE Program!='99' and IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x0c:
            Selectsecond = false;
            condition = " WHERE IDCode LIKE \'%"+VIN+"%\' and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x0b:
            Selectsecond = true;
            condition = " WHERE Program!='99'and IDCode LIKE \'%"+VIN+"%\' and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE Program!='99'and IDCode LIKE \'%"+VIN+"%\'";
            break;
        case 0x0a:
            Selectsecond = true;
            condition = " WHERE Program!='99'and IDCode LIKE \'%"+VIN+"%\' and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\'";
           condition1 = " WHERE Program!='99'and IDCode LIKE \'%"+VIN+"%\'";
            break;
        case 0x09:
            Selectsecond = true;
            condition = " WHERE Program!='99'  and IDCode LIKE \'%"+VIN+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE Program!='99'  and IDCode LIKE \'%"+VIN+"%\'";
            break;
        case 0x08:
            Selectsecond = false;
            condition = " WHERE IDCode LIKE \'%"+VIN+"%\'";
            break;
        case 0x07:
            Selectsecond = true;
            condition = " WHERE Program!='99'  and ScrewID LIKE \'%"+ScrewID+"%\' and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE Program!='99'  and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x06:
            Selectsecond = true;
            condition = " WHERE Program!='99'  and ScrewID LIKE \'%"+ScrewID+"%\' and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\'";
            condition1 = " WHERE Program!='99'  and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x05:
            Selectsecond = true;
            condition = " WHERE Program!='99'  and ScrewID LIKE \'%"+ScrewID+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
           condition1 = " WHERE Program!='99'  and ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x04:
            Selectsecond = false;
            condition = " WHERE ScrewID LIKE \'%"+ScrewID+"%\'";
            break;
        case 0x03:
            Selectsecond = true;
            condition = " WHERE Program!='99'  and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\' and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE  Program!='99'";
            break;
        case 0x02:
            Selectsecond = true;
            condition = " WHERE Program!='99'  and TighteningStatus LIKE \'%"+TighteningStatusNOK+"%\'";
            condition1 = " WHERE  Program!='99'";
            break;
        case 0x01:
            Selectsecond = true;
            condition = " WHERE  Program!='99'  and Cycle LIKE \'%"+TighteningStatusManual+"%\'";
            condition1 = " WHERE  Program!='99'";
            break;
        case 0x00:
            Selectsecond = false;
            condition = "";
            break;
        default:
            Selectsecond = false;
            condition = "";
            break;
        }

        if(ui->stackedWidget_dateSelect->currentIndex() == 0)//固定日期查询
        {
            QString date = "";
            if(ui->pushButton_threeDays->isChecked())
                date = "DATE_SUB(CURDATE(),INTERVAL 3 DAY) <= TighteningTime";
            else if(ui->pushButton_oneWeek->isChecked())
                date = "DATE_SUB(CURDATE(),INTERVAL 7 DAY) <= TighteningTime";
            else if(ui->pushButton_oneMonth->isChecked())
                date = "DATE_SUB(CURDATE(),INTERVAL 30 DAY) <= TighteningTime";

            if(condition != "" && date != "")
            {
                condition = condition + " and " + date;
                if(Selectsecond)
                     condition1 = condition1 + " and " + date;
            }
            else if(condition == "" && date != "")
            {
                condition = " WHERE "+date;
                if(Selectsecond)
                      condition1 = condition1 + " and " + date;
            }

        }
        else if(ui->stackedWidget_dateSelect->currentIndex() == 1)//选定日期查询
        {
            if(ui->dateEdit_max->text()<ui->dateEdit_min->text())
            {
                ui->label_dateWrong->show();
                ui->dateEdit_max->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(250, 0, 0); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                ui->dateEdit_min->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(250, 0, 0); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                return;
            }
            else
            {
                ui->dateEdit_max->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                ui->dateEdit_min->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                ui->label_dateWrong->hide();
                QString date = "TighteningTime>='"+ui->dateEdit_min->text()+"' and TighteningTime<= DATE_ADD('"+ui->dateEdit_max->text()+"',INTERVAL 1 DAY)";
                if(condition != "")
                {
                    condition = condition + " and " + date;
                    if(Selectsecond)
                       condition1 = condition1 + " and " + date;
                }
                else
                {
                    condition = " WHERE " + date;
                    if(Selectsecond)
                        condition1 = condition1 + " and " + date;
                }
            }
        }

        if(Selectsecond)//  查询全部数据    //查询指定条件的全部数据
        {
            affall = "SELECT count(*) FROM "+Localtable+condition1;
            DTdebug()<< "affall="<< affall;
            if(mDataBase.isOpen())
            {
                if(!query->exec(affall))
                {
                    DTdebug()<<query->lastError();
                }
                else
                {
                    query->next();
                    Totalnum = query->value(0).toInt(); //numRows
                    DTdebug() << "All number: " << Totalnum;//numRows;
                }
            }
            else
            {
                DTdebug()<<"mDataBase is not open";
            }
        }

        //如果需要百分比则查询全部数据
        affall = "SELECT count(*) FROM "+Localtable+condition;
        DTdebug()<< "affall="<< affall;
        if(!mDataBase.isOpen())
        {
            DTdebug()<<"mDataBase is not open";
        }
        else if(!query->exec(affall))
        {
            DTdebug()<<query->lastError();
        }
        else
        {
            query->next();
            numRows = query->value(0).toInt();
            DTdebug() << "select total row number: " << numRows;//总条数
            if(Selectsecond)
            {
                Selectsecond = false;
                if(Totalnum!=0)
                    Nok_percent = (float)numRows/Totalnum*100;
                else
                    Nok_percent = 0;
                DTdebug()<< "Nok_percent:"<< Nok_percent;
                Nok_percent=( (float)( (int)( (Nok_percent+0.005)*100 ) ) )/100;//保留小数点后两位数字
                ui->label_Totalnum->setText(QString::number(Totalnum));
                ui->label_percent->setText(QString::number(Nok_percent));
            }
            else
            {
                ui->label_Totalnum->setText(QString::number(0));
                ui->label_percent->setText(QString::number(0));
            }
            ui->label_num->setText(QString::number(numRows));

            if (numRows%10 != 0)
            {
                pages = numRows/10+1;
            }
            else
            {
                pages = numRows/10;
            }
            DTdebug() << "pages: " << pages;
            ui->label_M->setText(QString::number(pages));
            //thepage = 1;
            if (numRows==0)
            {
                ui->label_N->setText("0");
                thepage = 0;
            }
            else
            {
                thepage = 1;
                ui->label_N->setText(QString::number(thepage));
            }
            //thepages =QString::number((thepage-1)*10);
            thepages = "0";
            aff = "select IDCode,ScrewID,Program,Torque,Angle,DATE_FORMAT(TighteningTime,'%Y-%m-%d %H:%i:%s'),TighteningStatus,UploadMark,Cycle from "+Localtable+condition+" order by RecordID DESC limit "+thepages +", 10";
            //    aff = "select top(10) IDCode,ScrewID,Torque,Angle,TighteningTime,TighteningStatus,UploadMark,Cycle from TighteningDatas WHERE "+condition+" and RecordID not in (select top(("+ thepages +"-1)*10) RecordID from TighteningDatas WHERE "+condition+")";
            DTdebug()<< "aff="<< aff;

            if(!mDataBase.isOpen())
            {
                DTdebug()<<"mDataBase is not open";
            }
            else
            {
                model->setQuery(aff, mDataBase);
                DTdebug()<< mDataBase.lastError().text();

                if(factory == "AQCHERY")
                {
                    ui->tableView->scrollToTop();
                    ui->tableView->update();
                }
            }

            int i;
            for(i=0;i<10;i++)
            {
                ui->tableView->setRowHeight(i,43);
            }
        }
    }
    else
    {
        condition = "";
        if(ui->stackedWidget_dateSelect->currentIndex()==1)
        {
            if(ui->dateEdit_max->text()<ui->dateEdit_min->text())
            {
                ui->label_dateWrong->show();
                ui->dateEdit_max->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(250, 0, 0); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                ui->dateEdit_min->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(250, 0, 0); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                return;
            }
            else
            {
                ui->dateEdit_max->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                ui->dateEdit_min->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
                ui->label_dateWrong->hide();
                condition = " WHERE TighteningTime>='"+ui->dateEdit_min->text()+"' and TighteningTime<= DATE_ADD('"+ui->dateEdit_max->text()+"',INTERVAL 1 DAY) and Torque !=-1 and Program = "+ui->lineEdit_pronum_history->text();
            }
        }
        else
        {
            if(ui->pushButton_threeDays->isChecked())
                condition = " WHERE DATE_SUB(CURDATE(),INTERVAL 3 DAY) <= TighteningTime and Torque !=-1 and Program = "+ui->lineEdit_pronum_history->text();
            else if(ui->pushButton_oneWeek->isChecked())
                condition = " WHERE DATE_SUB(CURDATE(),INTERVAL 7 DAY) <= TighteningTime and Torque !=-1 and Program = "+ui->lineEdit_pronum_history->text();
            else if(ui->pushButton_oneMonth->isChecked())
                condition = " WHERE DATE_SUB(CURDATE(),INTERVAL 30 DAY) <= TighteningTime and Torque !=-1 and Program = "+ui->lineEdit_pronum_history->text();
            else
                condition = " WHERE Torque !=-1 and Program = "+ui->lineEdit_pronum_history->text();
        }
        queryResult(condition);
    }
}

void Newconfiginfo::on_pushButton_first_clicked()
{
    if(thepage == 0)
        thepage = 0;
    else
        thepage = 1;
    ui->label_N->setText(QString::number(thepage));
    pagechange();
}

void Newconfiginfo::on_pushButton_last_clicked()
{
    thepage = pages;
    ui->label_N->setText(QString::number(thepage));
    pagechange();
}

void Newconfiginfo::on_pushButton_previous_clicked()
{
    //left
    if(thepage == 1)
        thepage = 2;
    if(thepage == 0)
        thepage =1;
    thepage--;
    ui->label_N->setText(QString::number(thepage));
    pagechange();
}

void Newconfiginfo::on_pushButton_next_clicked()
{
    //right
    if(thepage == pages)
        thepage = pages-1;
    thepage++;
    ui->label_N->setText(QString::number(thepage));
    pagechange();
}

void Newconfiginfo::ResetModel(void)
{
    if(Factory == "Benz")
    {
        model->setHeaderData(0, Qt::Horizontal, tr("PN码"));
        ui->label_150->setText("VIN码：");
    }
    else
    {
        model->setHeaderData(0, Qt::Horizontal, tr("条码"));
        ui->label_150->setText("条码：");
    }
    model->setHeaderData(1, Qt::Horizontal, tr("螺栓编号"));
    model->setHeaderData(2, Qt::Horizontal, tr("程序号"));
    model->setHeaderData(3, Qt::Horizontal, tr("扭矩"));
    model->setHeaderData(4, Qt::Horizontal, tr("角度"));
    model->setHeaderData(5, Qt::Horizontal, tr("拧紧时间"));
    model->setHeaderData(6, Qt::Horizontal, tr("拧紧状态"));
    model->setHeaderData(7, Qt::Horizontal, tr("上传标志"));
    model->setHeaderData(8, Qt::Horizontal, tr("循环号"));

    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑

    ui->tableView->setShowGrid(false);//显示表格线

    ui->tableView->setColumnWidth(0,190);//第一列宽度
    ui->tableView->setColumnWidth(1,110);
    ui->tableView->setColumnWidth(2,70);
    ui->tableView->setColumnWidth(3,90);
    ui->tableView->setColumnWidth(4,100);
    ui->tableView->setColumnWidth(5,220);
    ui->tableView->setColumnWidth(6,101);
    ui->tableView->setColumnWidth(7,100);
    ui->tableView->setColumnWidth(8,80);


    int i;
    for(i=0;i<10;i++)
    {
        ui->tableView->setRowHeight(i,43);//行宽
    }
    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(51, 153, 255);"
                                                     "color: rgb(248, 248, 255);border: 0px; font:14pt}");
    ui->tableView->horizontalHeader()->setFixedHeight(51);

    ui->tableView->verticalHeader()->setVisible(false);   //隐藏列表头
    ui->tableView->horizontalHeader()->setVisible(true); //隐藏行表头
}

void Newconfiginfo::on_pushButton_pronum_add_clicked()
{
    new_pronum = ui->lineEdit_pronum->text().toInt()+1;
    ui->lineEdit_pronum->setText(QString::number(new_pronum));
}

void Newconfiginfo::on_pushButton_pronum_minus_clicked()
{
    if (ui->lineEdit_pronum->text().toInt()>0)
    {
        new_pronum=ui->lineEdit_pronum->text().toInt()-1;
        ui->lineEdit_pronum->setText(QString::number(new_pronum));
    }
}

void Newconfiginfo::on_pushButton_number_add_clicked()
{
    new_number=ui->lineEdit_number->text().toInt()+1;
    ui->lineEdit_number->setText(QString::number(new_number));
}

void Newconfiginfo::on_pushButton_number_minus_clicked()
{
    if (ui->lineEdit_number->text().toInt()>0)
    {
        new_number=(ui->lineEdit_number->text()).toInt()-1;
        ui->lineEdit_number->setText(QString::number(new_number));
    }
}

void Newconfiginfo::on_pushButton_ioBoxOption_add_clicked()
{
    int tmpIobox = ui->lineEdit_ioBoxOption->text().toInt()+1;
    ui->lineEdit_ioBoxOption->setText(QString::number(tmpIobox));
}

void Newconfiginfo::on_pushButton_ioBoxOption_minus_clicked()
{
    if (ui->lineEdit_ioBoxOption->text().toInt()>0)
    {
        int tmpIobox=(ui->lineEdit_ioBoxOption->text()).toInt()-1;
        ui->lineEdit_ioBoxOption->setText(QString::number(tmpIobox));
    }
}

void Newconfiginfo::on_pushButton_iobox_add_clicked()
{
    int tmpIobox = ui->lineEdit_iobox->text().toInt()+1;
    ui->lineEdit_iobox->setText(QString::number(tmpIobox));
}

void Newconfiginfo::on_pushButton_iobox_minus_clicked()
{
    if (ui->lineEdit_iobox->text().toInt()>0)
    {
        int tmpIobox=(ui->lineEdit_iobox->text()).toInt()-1;
        ui->lineEdit_iobox->setText(QString::number(tmpIobox));
    }
}

void Newconfiginfo::on_pushButton_xuanpronum_add_clicked()
{
    new_pronum = ui->lineEdit_xuanpronum->text().toInt()+1;
    ui->lineEdit_xuanpronum->setText(QString::number(new_pronum));
}

void Newconfiginfo::on_pushButton_xuanpronum_minus_clicked()
{
    if (ui->lineEdit_xuanpronum->text().toInt()>0)
    {
        new_pronum=ui->lineEdit_xuanpronum->text().toInt()-1;
        ui->lineEdit_xuanpronum->setText(QString::number(new_pronum));
    }
}

void Newconfiginfo::on_pushButton_xuannumber_add_clicked()
{
    new_number=ui->lineEdit_xuannumber->text().toInt()+1;
    ui->lineEdit_xuannumber->setText(QString::number(new_number));
}

void Newconfiginfo::on_pushButton_xuannumber_minus_clicked()
{
    if (ui->lineEdit_xuannumber->text().toInt()>0)
    {
        new_number=(ui->lineEdit_xuannumber->text()).toInt()-1;
        ui->lineEdit_xuannumber->setText(QString::number(new_number));
    }
}


void Newconfiginfo::historyclear()
{
    //    if(ui->tabWidget->currentIndex() == 0)
    //    {
    pages = 0;
    thepage = 0;
    ui->label_M->setText("0");
    ui->label_N->setText("0");
    ui->label_num->setText("________");
    ui->label_percent->setText("________");
    ui->label_Totalnum->setText("________");
    ui->lineEdit_VIN_2->setText("");

    ui->checkBox_NOK->setChecked(FALSE);
    ui->checkBox_Reset->setChecked(FALSE);

    ui->lineEdit_ScrewID->setText("");

    aff = "select IDCode,ScrewID,Program,Torque,Angle,TighteningTime,TighteningStatus,UploadMark,Cycle from "+Localtable+" WHERE RecordID <0";
    DTdebug()<< "aff="<< aff;
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
    }
    else
    {
        model->setQuery(aff, mDataBase);
        DTdebug()<< mDataBase.lastError().text();
    }

    //    }
    //    else if(ui->tabWidget->currentIndex() == 1)
    //    {
    ui->label_dateWrong->hide();
    ui->dateEdit_max->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
    ui->dateEdit_min->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
    QString conditon = " WHERE Program = -2";
    queryResult(conditon);
    ui->label_num_2->setText("________");
    ui->lineEdit_pronum_history->setText("0");
    QDate date = QDate::currentDate();//获取系统现在的时间
    ui->dateEdit_min->setDate(date);
    ui->dateEdit_max->setDate(date);
    //    }
}
//Fis更新列
void Newconfiginfo::on_pushButton_67_clicked()
{
    //    DTdebug()<<"isboundchang"<<isboundchange;
    if(ui->stackedWidget_2-> currentIndex() != 10)
    {
        whichButtonClick = "fisupdatecolumn";
        if(!isadvancedchange)
            advancedIsChange();
        else if(!ismasterslavechange)
            masterslaveIsChange();
        else if(!isboundchange)
            boundIsChange();
        else if(!islocationchange)
            locationIsChange();
        else
        {
            ui->stackedWidget_2->setCurrentIndex(10);
            ui->label_87->hide();
            ui->label_101->hide();
            ui->label_88->hide();
            ui->label_98->show();
            ui->label_99->hide();
            ui->label_162->hide();
            ui->label_wirelessLocation->hide();

            if(!mDataBase.isOpen())
            {
                DTdebug()<<"mDataBase is not open";
            }
            else
            {
                query->exec("select column_name from information_schema.columns where table_name='"+tablePreview+"'");
                for(int i=0;i<6;i++)
                    query->next();
                QString columns = "";
                if(query->next())
                    columns = query->value(0).toString();
                while(query->next())
                {
                    columns = columns + ", "+query->value(0).toString();
                    //DTdebug()<<query->value(0).toString();
                }
                DTdebug()<< "columns" << columns;
                ui->textBrowser->setText(columns);
                ui->lineEdit_column->clear();
                ui->lineEdit_column_2->clear();
                //ui->label_39->setText(columns);
            }
        }
    }
}

//添加列
void Newconfiginfo::on_pushButton_4_clicked()
{
    bool isColumnName=false;
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
        return;
    }
    query->exec("select column_name from information_schema.columns where table_name='"+tablePreview+"'");
    while(query->next())
    {
        if(ui->lineEdit_column->text()==query->value(0).toString())
            isColumnName=true;
    }
    if(!isColumnName)
    {
        if(!ui->lineEdit_column->text().isEmpty())
        {
            QString column =ui->lineEdit_column->text();
            query->exec("alter table "+tablePreview+" add " + column + " char(3) null");
            DTdebug()<<"tablePreview add column "<<column;
            query->exec("select column_name from information_schema.columns where table_name='"+tablePreview+"'");
            for(int i=0;i<6;i++)
                query->next();
            QString columns = "";
            if(query->next())
                columns = query->value(0).toString();
            while(query->next())
            {
                columns = columns + ", "+query->value(0).toString();
                //DTdebug()<<query->value(0).toString();
            }
            DTdebug()<< "columns" << columns;
            //ui->label_39->setText(columns);
            ui->textBrowser->setText(columns);
            emit column_update(column);
            ui->lineEdit_column->clear();
        }
    }
}

//删除列
void Newconfiginfo::on_pushButton_51_clicked()
{
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
        return;
    }
    query1->exec("select column_name from information_schema.columns where table_name='"+tablePreview+"'");
    for(int i=0;i<6;i++)
        query1->next();
    while(query1->next())
    {
        if(ui->lineEdit_column_2->text() == query1->value(0).toString())
        {
            query->exec("alter table "+tablePreview+" drop " + ui->lineEdit_column_2->text());
            DTdebug()<<"tablePreview drop column "<<ui->lineEdit_column_2->text();
            query->exec("select column_name from information_schema.columns where table_name='"+tablePreview+"'");
            for(int i=0;i<6;i++)
                query->next();
            QString columns = "";
            if(query->next())
                columns = query->value(0).toString();
            while(query->next())
            {
                columns = columns + ", "+query->value(0).toString();
                //DTdebug()<<query->value(0).toString();
            }
            //DTdebug()<< "columns" << columns;
            //ui->label_39->setText(columns);
            ui->textBrowser->setText(columns);
            ui->lineEdit_column_2->clear();
        }
        //DTdebug()<<query->value(0).toString();
    }
}

void Newconfiginfo::on_pushButton_36_clicked()
{
    //添加PDM
    ui->lineEdit_pdmname->show();
    ui->pushButton_29->show();
    ui->pushButton_30->show();
    ui->pushButton_31->show();
    ui->label_69->hide();
    ui->comboBox->hide();
    pathpdm = "";
    ui->lineEdit_pdmname->clear();
    SaveWhat = "pdm";
    isSavedpdm = true;
    int tempdata = numpdm;
    ui->lineEdit_pdmname->setFocus();
    for(int i = 0;i<tempdata;i++)
    {
        delete butt1[i];
        numpdm--;
    }
    ui->groupBox_13->setTitle(tr("图片"));
    ui->groupBox_13->setStyleSheet("QGroupBox {font: 14pt;margin-top: 1ex;border-width:0.5px;border-style:solid;border-color: rgb(51, 153, 255);} QGroupBox::title {subcontrol-origin: margin;subcontrol-position: top left;left:15px;margin-left: 5px;margin-right: 5px;padding:1px;}");
    ui->pushButton_33->hide();
}


void Newconfiginfo::on_pushButton_86_clicked()
{
    if(ui->stackedWidget_2-> currentIndex() != 11)
    {
        whichButtonClick = "master_slave";
        if(!isadvancedchange)
            advancedIsChange();
        else if(!isboundchange)
            boundIsChange();
        else if(!islocationchange)
            locationIsChange();
        else
        {
            ismasterslavechange = false;
            ui->stackedWidget_2->setCurrentIndex(11);
            ui->label_87->hide();
            ui->label_101->hide();
            ui->label_88->hide();
            ui->label_98->hide();
            ui->label_99->show();
            ui->label_162->hide();
            ui->label_wirelessLocation->hide();
        }
    }
}

void Newconfiginfo::on_radioButton_2_clicked()
{
    ui->checkBox_1->hide();
    ui->checkBox_2->hide();
    ui->checkBox_3->hide();
    ui->lineEdit_slave_1->hide();
    ui->lineEdit_slave_2->hide();
    ui->lineEdit_slave_3->hide();

}

void Newconfiginfo::on_radioButton_1_clicked()
{
    ui->checkBox_1->show();
    ui->checkBox_2->show();
    ui->checkBox_3->show();
    ui->lineEdit_slave_1->show();
    ui->lineEdit_slave_2->show();
    ui->lineEdit_slave_3->show();
}

void Newconfiginfo::on_pushButton_37_clicked()
{
    whichButtonClick = "savemasterslave";
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);

    SaveWhat = "Master_slave";
    save = new Save(this);
    connect(save,SIGNAL(sendSaveMasterSlaveState(bool)),this,SLOT(receiveMasterSlaveState(bool)));
    save->show();
}

void Newconfiginfo::receiveMasterSlaveState(bool statetmp)
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    if(statetmp)
    {
        QButtonGroup getModeSelect;
        getModeSelect.addButton(ui->radioButton_1,1);//将radioButton加进组
        getModeSelect.addButton(ui->radioButton_2,2);

        if(getModeSelect.checkedId()==1)
        {
            //DTdebug()<<"master";
            configIniRead->setValue("baseinfo/Master_slave","master");
            if(ui->checkBox_1->isChecked())
            {
                if(!ui->lineEdit_slave_1->text().isEmpty())
                    configIniRead->setValue("baseinfo/slave1",ui->lineEdit_slave_1->text());
                else
                    ui->checkBox_1->setChecked(false);
            }
            else
            {
                ui->lineEdit_slave_1->clear();
                configIniRead->setValue("baseinfo/slave1","");
            }
            if(ui->checkBox_2->isChecked())
            {
                if(!ui->lineEdit_slave_2->text().isEmpty())
                    configIniRead->setValue("baseinfo/slave2",ui->lineEdit_slave_2->text());
                else
                    ui->checkBox_2->setChecked(false);
            }
            else
            {
                ui->lineEdit_slave_2->clear();
                configIniRead->setValue("baseinfo/slave2","");
            }
            if(ui->checkBox_3->isChecked())
            {
                if(!ui->lineEdit_slave_3->text().isEmpty())
                    configIniRead->setValue("baseinfo/slave3",ui->lineEdit_slave_3->text());
                else
                    ui->checkBox_3->setChecked(false);
            }
            else
            {
                ui->lineEdit_slave_3->clear();
                configIniRead->setValue("baseinfo/slave3","");
            }
            if(Factory != "BAIC")
            {
                system("killall -9 client &");
                system("echo 0 > /sys/class/graphics/fb2/blank &");
                system("/etc/data2Child/client -qws -display \"LinuxFb:/dev/fb2:mmWidth500:mmHeight300:0\" &");
            }
        }
        else if(getModeSelect.checkedId()==2)
        {
            //DTdebug()<<"slave";
            configIniRead->setValue("baseinfo/Master_slave","slave");
            configIniRead->setValue("baseinfo/slave1","");
            configIniRead->setValue("baseinfo/slave2","");
            configIniRead->setValue("baseinfo/slave3","");
            ui->lineEdit_slave_1->setText("");
            ui->lineEdit_slave_2->setText("");
            ui->lineEdit_slave_3->setText("");
            ui->checkBox_1->setChecked(false);
            ui->checkBox_2->setChecked(false);
            ui->checkBox_3->setChecked(false);
        }
    }
    else
    {
        if(configIniRead->value("baseinfo/Master_slave").toString()=="master")
        {
            ui->radioButton_1->setChecked(true);
            ui->lineEdit_slave_1->setText(configIniRead->value("baseinfo/slave1").toString());
            if(!ui->lineEdit_slave_1->text().isEmpty())
                ui->checkBox_1->setChecked(true);
            else
            {
                ui->lineEdit_slave_1->setText("");
                ui->checkBox_1->setChecked(false);
            }
            ui->lineEdit_slave_2->setText(configIniRead->value("baseinfo/slave2").toString());
            if(!ui->lineEdit_slave_2->text().isEmpty())
                ui->checkBox_2->setChecked(true);
            else
            {
                ui->lineEdit_slave_2->setText("");
                ui->checkBox_2->setChecked(false);
            }
            ui->lineEdit_slave_3->setText(configIniRead->value("baseinfo/slave3").toString());
            if(!ui->lineEdit_slave_3->text().isEmpty())
                ui->checkBox_3->setChecked(true);
            else
            {
                ui->lineEdit_slave_3->setText("");
                ui->checkBox_3->setChecked(false);
            }
        }
        else if(configIniRead->value("baseinfo/Master_slave").toString()=="slave")
        {
            ui->radioButton_2->setChecked(true);
            ui->checkBox_1->hide();
            ui->checkBox_2->hide();
            ui->checkBox_3->hide();
            ui->lineEdit_slave_1->hide();
            ui->lineEdit_slave_2->hide();
            ui->lineEdit_slave_3->hide();
        }
        else
            ui->radioButton_1->setChecked(true);
    }
    delete configIniRead;
    ui->label_100->hide();
    delete e3;
    delete save;

    if(whichButtonClick == "advancedback")
    {
        on_pushButton_3_clicked();
        //ismasterslavechange = false;
    }
    else if(whichButtonClick == "passwordchange")
    {
        on_pushButton_100_clicked();
        //ismasterslavechange = false;
    }
    else if(whichButtonClick == "testinterface")
    {
        on_pushButton_66_clicked();
        //ismasterslavechange = false;
    }
    else if(whichButtonClick == "fisupdatecolumn")
    {
        on_pushButton_67_clicked();
        //ismasterslavechange = false;
    }
    else if(whichButtonClick == "advanceset")
    {
        on_pushButton_58_clicked();
        //ismasterslavechange = false;
    }
    else if(whichButtonClick == "savemasterslave")
    {
        //        ismasterslavechange = false;
    }
    else if(whichButtonClick == "bound_config")
    {
        on_pushButton_55_clicked();
        //ismasterslavechange = false;
    }
    else if (whichButtonClick == "wirelessLocation")
    {
        on_pushButton_wirelessLocation_clicked();
    }
}

void Newconfiginfo::receiveDebug(QString vinbuf)
{
    ui->lineEdit_staname_5->setText(vinbuf);
    ui->label_repairVin->setText(vinbuf);
    repairSearch();
}


void Newconfiginfo::masterslaveIsChange()
{
    // Master_slave监听
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString temp;
    bool temp1,temp2,temp3;
    QButtonGroup getModeSelect;
    getModeSelect.addButton(ui->radioButton_1,1);//将radioButton加进组
    getModeSelect.addButton(ui->radioButton_2,2);

    if(getModeSelect.checkedId()==1)
    {
        temp="master";
    }
    else if(getModeSelect.checkedId()==2)
    {
        temp="slave";
    }

    if(configIniRead->value("baseinfo/slave1").toString().isEmpty())
        temp1=false;
    else
        temp1=true;
    if(configIniRead->value("baseinfo/slave2").toString().isEmpty())
        temp2=false;
    else
        temp2=true;
    if(configIniRead->value("baseinfo/slave3").toString().isEmpty())
        temp3=false;
    else
        temp3=true;
    if (   temp != configIniRead->value("baseinfo/Master_slave")||
           configIniRead->value("baseinfo/slave1").toString()!=ui->lineEdit_slave_1->text()||
           configIniRead->value("baseinfo/slave2").toString()!=ui->lineEdit_slave_2->text()||
           configIniRead->value("baseinfo/slave3").toString()!=ui->lineEdit_slave_3->text()||
           ui->checkBox_1->isChecked()!=temp1||
           ui->checkBox_2->isChecked()!=temp2||
           ui->checkBox_3->isChecked()!=temp3   )
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);

        SaveWhat = "Master_slave";
        save = new Save(this);
        connect(save,SIGNAL(sendSaveMasterSlaveState(bool)),this,SLOT(receiveMasterSlaveState(bool)));
        save->show();
        ismasterslavechange = true;
        delete configIniRead;
    }
    else
    {
        delete configIniRead;
        ismasterslavechange = true;
        if (whichButtonClick == "advancedback")
        {
            on_pushButton_3_clicked();
            //ismasterslavechange = false;
        }
        else if (whichButtonClick == "passwordchange")
        {
            on_pushButton_100_clicked();
            //ismasterslavechange = false;
        }
        else if (whichButtonClick == "testinterface")
        {
            on_pushButton_66_clicked();
            //ismasterslavechange = false;
        }
        else if (whichButtonClick == "fisupdatecolumn")
        {
            on_pushButton_67_clicked();
            //ismasterslavechange = false;
        }
        else if (whichButtonClick == "advanceset")
        {
            on_pushButton_58_clicked();
            //ismasterslavechange = false;
        }
        else if (whichButtonClick == "bound_config")
        {
            on_pushButton_55_clicked();
            //ismasterslavechange = false;
        }
        else if (whichButtonClick == "wirelessLocation")
        {
            on_pushButton_wirelessLocation_clicked();
        }
    }
}

void Newconfiginfo::receivetime(QString  datetime)
{
    //DTdebug()<<"datetime"<<datetime;
    ui->year->setText(datetime.mid(0,4));
    ui->month->setText(datetime.mid(5,2));
    ui->date->setText(datetime.mid(8,2));
    ui->hour->setText(datetime.mid(11,2));
    ui->minute->setText(datetime.mid(14,2));
    ui->second->setText(datetime.mid(17,2));
}
void Newconfiginfo::on_pushButton_38_clicked()
{
    emit sendGetTime();
}

void Newconfiginfo::on_pushButton_49_clicked()
{
    if(numpdm > 0)
    {
        for(int i = 0;i< numpdm;i++)
            butt1[i]->show();
    }
    ui->stackedWidget_6->setCurrentIndex(0);
}

//***********************套筒按钮加减**********************************
void Newconfiginfo::on_pushButton_taotong_add_clicked()
{
    bool condition=false;
    if(Factory=="SVW3" ||Factory=="SVW2")
    {
        condition = (ui->lineEdit_taotong->text().toInt()>8) || (ui->lineEdit_taotong->text().toInt()<1);  //>4 ---->8
    }
    else // if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
    {
        condition = (ui->lineEdit_taotong->text().toInt()>8) || (ui->lineEdit_taotong->text().toInt()<1);
    }
    if( condition )
        ui->lineEdit_taotong->setText("0");

    if((Factory == "SVW3"||Factory=="SVW2")&&(ui->lineEdit_taotong->text().toInt()+1) == 9)  //5----->9
        ui->lineEdit_taotong->setText("4");
    else if((ui->lineEdit_taotong->text().toInt()+1) == 9)
        ui->lineEdit_taotong->setText("8");
    else
        ui->lineEdit_taotong->setText(QString::number(ui->lineEdit_taotong->text().toInt()+1));

}

void Newconfiginfo::on_pushButton_taotong_minus_clicked()
{
    bool condition=false;
    if(Factory=="SVW3"||Factory=="SVW2")
    {
        condition = (ui->lineEdit_taotong->text().toInt()>4) || (ui->lineEdit_taotong->text().toInt()<1) ;
    }
    else // if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
    {
        condition = (ui->lineEdit_taotong->text().toInt()>8) || (ui->lineEdit_taotong->text().toInt()<1) ;
    }
    if(condition)
        ui->lineEdit_taotong->setText("1");

    if((ui->lineEdit_taotong->text().toInt()-1) <= 0)
        ui->lineEdit_taotong->setText("OFF");
    else
    {
        ui->lineEdit_taotong->setText(QString::number(ui->lineEdit_taotong->text().toInt()-1));
    }

}

void Newconfiginfo::on_pushButton_50_clicked()
{
    system("rm /etc/pointercal &");
    system("/usr/local/tslib-instal/bin/ts_calibrate &");
}

void Newconfiginfo::on_Line_radioButton_0_clicked()
{
    ui->Line_radioButton_1->setChecked(false);
    ui->Line_radioButton_2->setChecked(false);
    line_ID = 0;
}

void Newconfiginfo::on_Line_radioButton_1_clicked()
{
    ui->Line_radioButton_0->setChecked(false);
    ui->Line_radioButton_2->setChecked(false);
    line_ID = 1;
}

void Newconfiginfo::on_Line_radioButton_2_clicked()
{
    ui->Line_radioButton_1->setChecked(false);
    ui->Line_radioButton_2->setChecked(false);
    line_ID = 2;
}

void Newconfiginfo::on_pushButton_104_clicked()
{
    ui->lineEdit_staname_5->clear();
}

void Newconfiginfo::on_pushButton_55_clicked()
{
    if(ui->stackedWidget_2->currentIndex() != 12)
    {
        whichButtonClick = "bound_config";
        if(!isadvancedchange)
            advancedIsChange();
        else if(!ismasterslavechange)
            masterslaveIsChange();
        else if(!islocationchange)
            locationIsChange();
        else
        {
            ui->stackedWidget_2->setCurrentIndex(12);
            ui->label_87->hide();
            ui->label_101->hide();
            ui->label_88->hide();
            ui->label_98->hide();
            ui->label_99->hide();
            ui->label_162->show();
            ui->label_wirelessLocation->hide();

            isboundchange = false;
            on_pushButton_88_clicked();
        }
    }
}

void Newconfiginfo::bound_show()
{
    ui->lineEdit_ProNumber->setText(QString::number(PronumNow));
    ui->lineEdit_torque_max->setText(bound[PronumNow][0]);
    ui->lineEdit_torque_min->setText(bound[PronumNow][1]);
    ui->lineEdit_angle_max->setText(bound[PronumNow][2]);
    ui->lineEdit_angle_min->setText(bound[PronumNow][3]);
}

void Newconfiginfo::bound_save()
{
    bound[PronumNow][0]=ui->lineEdit_torque_max->text();
    bound[PronumNow][1]=ui->lineEdit_torque_min->text();
    bound[PronumNow][2]=ui->lineEdit_angle_max->text();
    bound[PronumNow][3]=ui->lineEdit_angle_min->text();
}

void Newconfiginfo::on_pushButton_tens_add_clicked()
{
    bool wrong=false;
    if(ui->lineEdit_torque_max->text()!=""&& ui->lineEdit_torque_min->text()!=""&&
            ui->lineEdit_torque_max->text().toDouble()<ui->lineEdit_torque_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(ui->lineEdit_angle_max->text()!=""&& ui->lineEdit_angle_min->text()!=""&&
            ui->lineEdit_angle_max->text().toDouble()<ui->lineEdit_angle_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(!wrong)
    {
        bound_save();
        if(PronumNow+10<100)
        {
            PronumNow=PronumNow+10;
            bound_show();
        }
    }
}

void Newconfiginfo::on_pushButton_tens_minus_clicked()
{
    bool wrong=false;
    if(ui->lineEdit_torque_max->text()!=""&& ui->lineEdit_torque_min->text()!=""&&
            ui->lineEdit_torque_max->text().toDouble()<ui->lineEdit_torque_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(ui->lineEdit_angle_max->text()!=""&& ui->lineEdit_angle_min->text()!=""&&
            ui->lineEdit_angle_max->text().toDouble() < ui->lineEdit_angle_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(!wrong)
    {
        bound_save();
        if(PronumNow-10>=0)
        {
            PronumNow=PronumNow-10;
            bound_show();
        }
    }
}

void Newconfiginfo::on_pushButton_ones_add_clicked()
{
    bool wrong=false;
    if(ui->lineEdit_torque_max->text()!=""&& ui->lineEdit_torque_min->text()!=""&&
            ui->lineEdit_torque_max->text().toDouble()<ui->lineEdit_torque_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(ui->lineEdit_angle_max->text()!=""&& ui->lineEdit_angle_min->text()!=""&&
            ui->lineEdit_angle_max->text().toDouble()<ui->lineEdit_angle_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(!wrong)
    {
        bound_save();
        if(PronumNow+1<100)
        {
            PronumNow=PronumNow+1;
            bound_show();
        }
    }
}

void Newconfiginfo::on_pushButton_ones_minus_clicked()
{
    bool wrong=false;
    if(ui->lineEdit_torque_max->text()!=""&& ui->lineEdit_torque_min->text()!=""&&
            ui->lineEdit_torque_max->text().toDouble()<ui->lineEdit_torque_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(ui->lineEdit_angle_max->text()!=""&& ui->lineEdit_angle_min->text()!=""&&
            ui->lineEdit_angle_max->text().toDouble()<ui->lineEdit_angle_min->text().toDouble())
    {
        wrong = true;
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
    }
    else
    {
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    if(!wrong)
    {
        bound_save();
        if(PronumNow-1>=0)
        {
            PronumNow=PronumNow-1;
            bound_show();
        }
    }
}

void Newconfiginfo::on_pushButton_88_clicked()
{
    QSettings *config_bound = new QSettings("/config_bound.ini", QSettings::IniFormat);
    for(int i=0;i<100;i++)
    {
        bound[i][0]=config_bound->value("ProNumber"+QString::number(i)+"/Torque_max").toString();
        bound[i][1]=config_bound->value("ProNumber"+QString::number(i)+"/Torque_min").toString();
        bound[i][2]=config_bound->value("ProNumber"+QString::number(i)+"/Angle_max").toString();
        bound[i][3]=config_bound->value("ProNumber"+QString::number(i)+"/Angle_min").toString();
        for(int j=0;j<4;j++)
            bound_temp[i][j]=bound[i][j];
    }
    delete config_bound;
    bound_enabled_temp=bound_enabled;
    if(bound_enabled)
    {
        ui->pushButton_bound->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
    }
    else
    {
        ui->pushButton_bound->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
    }
    PronumNow = 0;
    bound_show();
    bound_update();
    ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
}

void Newconfiginfo::on_pushButton_87_clicked()
{
    bound_save();
    whichButtonClick = "bound_save";
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);

    SaveWhat = "config_bound";
    save = new Save(this);
    connect(save,SIGNAL(sendSaveBound(bool)),this,SLOT(receiveBound(bool)));
    save->show();
}

void Newconfiginfo::receiveBound(bool isSave)
{
    bool wrong=false;
    if(isSave)
    {
        if(ui->lineEdit_torque_max->text()!=""&& ui->lineEdit_torque_min->text()!=""&&
                ui->lineEdit_torque_max->text().toDouble()<ui->lineEdit_torque_min->text().toDouble())
        {
            wrong = true;
            ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
            ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        }
        else
        {
            ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
            ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        }
        if(ui->lineEdit_angle_max->text()!=""&& ui->lineEdit_angle_min->text()!=""&&
                ui->lineEdit_angle_max->text().toDouble()<ui->lineEdit_angle_min->text().toDouble())
        {
            wrong = true;
            ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
            ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(255, 0, 0);");
        }
        else
        {
            ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
            ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        }
        if(!wrong)
        {
            QSettings *config_bound = new QSettings("/config_bound.ini", QSettings::IniFormat);
            for(int i=0;i<100;i++)
            {
                config_bound->setValue("ProNumber"+QString::number(i)+"/Torque_max",bound[i][0]);
                config_bound->setValue("ProNumber"+QString::number(i)+"/Torque_min",bound[i][1]);
                config_bound->setValue("ProNumber"+QString::number(i)+"/Angle_max",bound[i][2]);
                config_bound->setValue("ProNumber"+QString::number(i)+"/Angle_min",bound[i][3]);
                for(int j=0;j<4;j++)
                    bound_temp[i][j]=bound[i][j];
            }
            bound_enabled = bound_enabled_temp;
            QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
            //        DTdebug()<<"bound_enabled"<<bound_enabled;
            config->setValue("baseinfo/bound",bound_enabled?"1":"0");
            delete config;
            bound_update();
            delete config_bound;
            isboundchange = true;
        }
    }
    else
    {
        ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
        ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    }
    ui->label_100->hide();
    delete e3;
    delete save;
    if(!wrong)
    {
        if(whichButtonClick == "advancedback")
        {
            isboundchange = true;
            on_pushButton_3_clicked();
        }
        else if (whichButtonClick == "advanceset")
        {
            isboundchange = true;
            on_pushButton_58_clicked();
        }
        else if(whichButtonClick == "passwordchange")
        {
            isboundchange = true;
            on_pushButton_100_clicked();
        }
        else if(whichButtonClick == "testinterface")
        {
            isboundchange = true;
            on_pushButton_66_clicked();
        }
        else if(whichButtonClick == "fisupdatecolumn")
        {
            isboundchange = true;
            on_pushButton_67_clicked();
        }
        else if(whichButtonClick == "master_slave")
        {
            isboundchange = true;
            on_pushButton_86_clicked();
        }
        else if (whichButtonClick == "wirelessLocation")
        {
            isboundchange = false;
            on_pushButton_wirelessLocation_clicked();
        }
        else if(whichButtonClick == "bound_save")
        {
            isboundchange = false;
        }
    }
}

void Newconfiginfo::boundIsChange()
{
    bound_save();
    QSettings *config_bound = new QSettings("/config_bound.ini", QSettings::IniFormat);
    bool change = false;
    for(int i=0;i<100;i++)
    {
        if(bound[i][0]!=config_bound->value("ProNumber"+QString::number(i)+"/Torque_max").toString()||
                bound[i][1]!=config_bound->value("ProNumber"+QString::number(i)+"/Torque_min").toString()||
                bound[i][2]!=config_bound->value("ProNumber"+QString::number(i)+"/Angle_max").toString()||
                bound[i][3]!=config_bound->value("ProNumber"+QString::number(i)+"/Angle_min").toString())
            change = true;
    }
    delete config_bound;
    if(bound_enabled_temp != bound_enabled)
        change = true;
    if (change)
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);

        SaveWhat = "config_bound";
        save = new Save(this);
        connect(save,SIGNAL(sendSaveBound(bool)),this,SLOT(receiveBound(bool)));
        save->show();
        //        isboundchange = true;
    }
    else
    {
        isboundchange = true;
        if (whichButtonClick == "advancedback")
        {
            on_pushButton_3_clicked();
        }
        else if (whichButtonClick == "advanceset")
        {
            on_pushButton_58_clicked();
        }
        else if (whichButtonClick == "passwordchange")
        {
            on_pushButton_100_clicked();
        }
        else if (whichButtonClick == "testinterface")
        {
            on_pushButton_66_clicked();
        }
        else if (whichButtonClick == "fisupdatecolumn")
        {
            on_pushButton_67_clicked();
        }
        else if (whichButtonClick == "advanceset")
        {
            on_pushButton_58_clicked();
        }
        else if (whichButtonClick == "master_slave")
        {
            on_pushButton_86_clicked();
        }
        else if (whichButtonClick == "wirelessLocation")
        {
            on_pushButton_wirelessLocation_clicked();
        }
    }
}

void Newconfiginfo::on_pushButton_bound_prev_clicked()
{
    if(bound_pages!=0)
    {
        if(--bound_current_page != 0)
            show_bound();
        else
            bound_current_page=1;
    }
}

void Newconfiginfo::on_pushButton_bound_next_clicked()
{
    if(++bound_current_page != bound_pages+1)
    {
        show_bound();
    }
    else
        bound_current_page = bound_pages;
}

void Newconfiginfo::show_bound()
{
    for(int i=0;i<10;i++)
    {
        for(int j=0;j<5;j++)
        {
            tableWidgetItem[i][j]->setText("");
        }
        //        header_vertical[i]="";
    }

    if(bound_pages != 0)
    {
        current =0;
        for(iter=list.begin()+10*(bound_current_page-1); iter!=list.end() && iter!=list.begin()+10*(bound_current_page); iter++)
        {
            tableWidgetItem[current][0]->setText("P"+QString::number(*iter));
            //            header_vertical[current]="P"+QString::number(*iter);
            for(int j=1;j<5;j++)
            {
                tableWidgetItem[current][j]->setText(bound_temp[*iter][j-1]);
            }
            current++;
        }
    }
    //    ui->tableWidget->setVerticalHeaderLabels(header_vertical);
}

void Newconfiginfo::bound_init()
{
    isboundchange = true;
    bound_enabled_temp=bound_enabled;
    bound_pages=0;
    bound_current_page=0;
    current =0;
    iter=0;
    ui->tableWidget->setRowCount(10);
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    ui->tableWidget->setShowGrid(true);//显示表格线

    ui->tableWidget->setColumnWidth(0,69);
    for(int i=1;i<5;i++)
        ui->tableWidget->setColumnWidth(i,125);
    for(int i=0;i<10;i++)
        ui->tableWidget->setRowHeight(i,52);

    ui->tableWidget->verticalHeader()->setVisible(false);

    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(51, 153, 255);"
                                                       "color: rgb(248, 248, 255);border: 0px; font:14pt}");
    //    ui->tableWidget->verticalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(248, 248, 255);"
    //                                                     "color:black; border: 0px; font:14pt}");
    ui->tableWidget->horizontalHeader()->setFixedHeight(51);
    //    ui->tableWidget->verticalHeader()->setFixedWidth(49);

    //    ui->tableWidget->verticalHeader()->setMovable(false);
    ui->tableWidget->horizontalHeader()->setMovable(false);

    //    ui->tableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//去掉水平滚动条
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//去掉水平滚动条
    ui->tableWidget->setAutoScroll(false);//去掉自动滚动

    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    //    ui->tableWidget->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    //    table->setStyleSheet("QTableCornerButton::section{background-color:red;}");

    //    tableWidget->setTextAlignment.
    // 创建表格项目，并插入到指定单元
    //    QTableWidgetItem *tableWidgetItem = new QTableWidgetItem("qt");
    //    tableWidget->setItem(1, 1, tableWidgetItem);
    // 创建表格项目，并将它们作为标头
    //    QTableWidgetItem *headerV = new QTableWidgetItem("P1");
    //    tableWidget->setVerticalHeaderItem(0,headerV);
    //    QTableWidgetItem *headerH0 = new QTableWidgetItem(tr("扭矩上限"));
    //    QTableWidgetItem *headerH1 = new QTableWidgetItem(tr("扭矩下限"));
    //    QTableWidgetItem *headerH2 = new QTableWidgetItem(tr("角度上限"));
    //    QTableWidgetItem *headerH3 = new QTableWidgetItem(tr("角度上限"));
    //    tableWidget->setHorizontalHeaderItem(0,headerH0);
    //    tableWidget->setHorizontalHeaderItem(1,headerH1);
    //    tableWidget->setHorizontalHeaderItem(2,headerH2);
    //    tableWidget->setHorizontalHeaderItem(3,headerH3);

    for(int i=0;i<10;++i)
        for(int j=0;j<5;++j)
        {
            //            ui->tableWidget->item(i,j)->setText("");
            //            ui->tableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
            tableWidgetItem[i][j] = new QTableWidgetItem("");
            ui->tableWidget->setItem(i,j,tableWidgetItem[i][j]);
            ui->tableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
        }

    //列名
    QStringList header;
    header<<"P"<<tr("扭矩上限(Nm)")<<tr("扭矩下限(Nm)")<<tr("角度上限(Deg)")<<tr("角度下限(Deg)");
    ui->tableWidget->setHorizontalHeaderLabels(header);

    //    header_vertical<<""<<""<<""<<""<<""<<""<<""<<""<<""<<"";
    //    ui->tableWidget->setVerticalHeaderLabels(header_vertical);
}

void Newconfiginfo::bound_update()
{
    list.clear();
    for(int i=0;i<100;i++)
    {
        if(bound_temp[i][0]!=""||bound_temp[i][1]!=""||bound_temp[i][2]!=""||bound_temp[i][3]!="")
        {
            list<<i;
        }
    }

    bound_pages = 0;
    bound_current_page = 0;

    if(list.size()%10==0 && list.size()!=0)
    {
        bound_pages=list.size()/10;
        bound_current_page = 1;
    }
    else if(list.size()%10!=0 && list.size() != 0)
    {
        bound_pages=list.size()/10+1;
        bound_current_page = 1;
    }
    ui->label_bound_current_page->setText(QString::number(bound_current_page));
    ui->label_bound_pages->setText(QString::number(bound_pages));
    //    DTdebug()<<bound_pages<<bound_current_page;
    show_bound();
}

void Newconfiginfo::history()
{
    //    ui->tabWidget->setCurrentIndex(1);
    //    historyclear();
    ui->label_history->show();
    ui->label_repair->hide();
    ui->tabWidget->setCurrentIndex(0);
    //    historyclear();
    on_tabWidget_currentChanged(0);
    ui->stackedWidget_dateSelect->setCurrentIndex(0);
    ui->stackedWidget_2->setCurrentIndex(3);
    ui->stackedWidget_3->setCurrentIndex(4);
    ui->pushButton_96->setText("历史结果");
}

void Newconfiginfo::PDMEdit()
{
    ui->stackedWidget_2->setCurrentIndex(5);
    ui->stackedWidget_3->setCurrentIndex(3);
    ui->pushButton_96->setText("作业指导");


    ispdminit = 0;
    ui->groupBox_13->setTitle(tr("图片"));
    ui->groupBox_13->setStyleSheet("QGroupBox {font: 14pt;margin-top: 1ex;border-width:0.5px;border-style:solid;border-color: rgb(51, 153, 255);} QGroupBox::title {subcontrol-origin: margin;subcontrol-position: top left;left:15px;margin-left: 5px;margin-right: 5px;padding:1px;}");
    //    ui->listWidget->setCurrentRow(1);
    //    historyclear();


    whichButtonClick = "PDMEdit";
    ui->comboBox->show();
    ui->label_69->show();
    ui->lineEdit_pdmname->hide();
    ui->pushButton_29->hide();
    ui->pushButton_30->hide();
    ui->pushButton_31->hide();


    ui->comboBox->clear();
    QSettings *config = new QSettings("/config.ini", QSettings::IniFormat);
    ui->comboBox->addItem("");

    for(int i= 0;i < 1000 ;i++)
    {
        if(config->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString()!= "")
        {
            ui->comboBox->addItem(config->value(QString("pdminfo").append(QString::number(i+1)).append("/pdmname")).toString());
        }
    }
    SaveWhat = "";
    ispdminit = 1;
    delete config;
}

void Newconfiginfo::configList()
{
    ui->stackedWidget_2->setCurrentIndex(4);
    ui->stackedWidget_3->setCurrentIndex(3);
    ui->stackedWidget_5->setCurrentIndex(0);
    ui->pushButton_96->setText("配置列表");
}

void Newconfiginfo::systemConfigure()
{
    ui->stackedWidget_2->setCurrentIndex(2);
    ui->stackedWidget_3->setCurrentIndex(3);
    ui->pushButton_96->setText("基本信息");
}

void Newconfiginfo::on_pushButton_bound_clicked()
{
    //    DTdebug()<<"bound_enabled_temp:"<<bound_enabled_temp;
    if(bound_enabled_temp)
    {
        bound_enabled_temp=false;
        ui->pushButton_bound->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
    }
    else
    {
        bound_enabled_temp=true;
        ui->pushButton_bound->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
    }
}

void Newconfiginfo::on_pushButton_56_clicked()
{
    ui->lineEdit_torque_max->setText("");
    ui->lineEdit_torque_min->setText("");
    ui->lineEdit_angle_max->setText("");
    ui->lineEdit_angle_min->setText("");
    ui->lineEdit_torque_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    ui->lineEdit_torque_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    ui->lineEdit_angle_max->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
    ui->lineEdit_angle_min->setStyleSheet("font: 14pt \"黑体\";border-width:1px; border-style:solid;border-color:rgb(51, 153, 255);");
}


//test red led
void Newconfiginfo::on_Led_red_clicked()
{
    //    int k1=1,k2=1;
    int k1=1,k2=1,k3=1,k4=1,k5=1,k6=1,k7=1,k8=1,k9=1,k10=1;

    system("echo 1 > /root/gpio/OUT4");
    system("echo 1 > /sys/class/leds/OUTC4/brightness");       //EN
    system("echo 1 > /sys/class/leds/OUTC0/brightness");        //rack6

    /*
     * 添加时间：2018年8月1 日
    */
    system("echo 1 > /root/gpio/OUT0");
    system("echo 1 > /sys/class/leds/OUTC1/brightness");
    system("echo 1 > /sys/class/leds/OUTC2/brightness");
    system("echo 1 > /sys/class/leds/OUTC3/brightness");
    system("echo 1 > /sys/class/leds/OUTC6/brightness");
    system("echo 1 > /sys/class/leds/OUTD7/brightness");
    system("echo 1 > /sys/class/leds/OUTD6/brightness");
    system("echo 1 > /sys/class/leds/OUTD5/brightness");

    usleep(30000) ;

    QFile input1("/sys/class/gpio/gpio162/value");
    QFile input2("/sys/class/gpio/gpio160/value");
    QFile input3("/sys/class/gpio/gpio165/value");
    QFile input4("/sys/class/gpio/gpio164/value");
    QFile input5("/sys/class/gpio/gpio159/value");
    QFile input6("/sys/class/gpio/gpio158/value");
    QFile input7("/sys/class/gpio/gpio149/value");
    QFile input8("/sys/class/gpio/gpio146/value");
    QFile input9("/sys/class/gpio/gpio147/value");
    QFile input10("/sys/class/gpio/gpio176/value");
    input1.open(QIODevice::ReadOnly);
    input2.open(QIODevice::ReadOnly);
    input3.open(QIODevice::ReadOnly);
    input4.open(QIODevice::ReadOnly);
    input5.open(QIODevice::ReadOnly);
    input6.open(QIODevice::ReadOnly);
    input7.open(QIODevice::ReadOnly);
    input8.open(QIODevice::ReadOnly);
    input9.open(QIODevice::ReadOnly);
    input10.open(QIODevice::ReadOnly);
    QTextStream in1(&input1);
    QTextStream in2(&input2);
    QTextStream in3(&input3);
    QTextStream in4(&input4);
    QTextStream in5(&input5);
    QTextStream in6(&input6);
    QTextStream in7(&input7);
    QTextStream in8(&input8);
    QTextStream in9(&input9);
    QTextStream in10(&input10);

    in1 >> k1;
    in2 >> k2;
    in3 >> k3;
    in4 >> k4;
    in5 >> k5;
    in6 >> k6;
    in7 >> k7;
    in8 >> k8;
    in9 >> k9;
    in10 >> k10;
    input1.close();
    input2.close();
    input3.close();
    input4.close();
    input5.close();
    input6.close();
    input7.close();
    input8.close();
    input9.close();
    input10.close();

    if(k1 == 0)
        ui->INA0->setStyleSheet(INAStyle0);
    if(k2 == 0)
        ui->INA1->setStyleSheet(INAStyle0);
    if(k3 == 0)
        ui->INA2->setStyleSheet(INAStyle0);
    if(k4 == 0)
        ui->INA3->setStyleSheet(INAStyle0);
    if(k5 == 0)
        ui->INA4->setStyleSheet(INAStyle0);
    if(k6 == 0)
        ui->INA5->setStyleSheet(INAStyle0);
    if(k7 == 0)
        ui->INA6->setStyleSheet(INAStyle0);
    if(k8 == 0)
        ui->INA7->setStyleSheet(INAStyle0);
    if(k9 == 0)
        ui->INB0->setStyleSheet(INAStyle0);
    if(k10 == 0)
        ui->INB1->setStyleSheet(INAStyle0);

}
//test green led
void Newconfiginfo::on_Led_green_clicked()
{
    system("echo 1 > /root/gpio/OUT3");
    system("echo 1 > /sys/class/leds/OUTC3/brightness");        //rack2
    system("echo 1 > /sys/class/leds/OUTD7/brightness");        //rack6
}
//test yellow led
void Newconfiginfo::on_Led_yellow_clicked()
{
    //    int k1=1;
    system("echo 1 > /root/gpio/OUT2");
    system("echo 1 > /sys/class/leds/OUTC2/brightness");       //rack3
    system("echo 1 > /sys/class/leds/OUTD6/brightness");        //rack7
}
//test white led
void Newconfiginfo::on_Led_white_clicked()
{
    //    int k1=1;
    system("echo 1 > /root/gpio/OUT1");
    system("echo 1 > /sys/class/leds/OUTC1/brightness");       //rack4
    system("echo 1 > /sys/class/leds/OUTD5/brightness");        //rack8
}
//test nok led
void Newconfiginfo::on_Led_nok_clicked()
{
    system("echo 1 > /root/gpio/OUT0");
}
//reset all led
void Newconfiginfo::on_IO_Reset_clicked()
{
    int k1=1,k2=1,k3=1,k4=1,k5=1,k6=1,k7=1,k8=1,k9=1,k10=1;
    system("echo 0 > /root/gpio/OUT4");
    system("echo 0 > /root/gpio/OUT3");
    system("echo 0 > /root/gpio/OUT2");
    system("echo 0 > /root/gpio/OUT1");
    system("echo 0 > /root/gpio/OUT0");
    system("echo 0 > /root/gpio/OUT5");
    system("echo 0 > /root/gpio/OUT6");
    system("echo 0 > /root/gpio/OUT7");
    system("echo 0 > /root/gpio/COM0");
    system("echo 0 > /root/gpio/COM1");


#if 0
    system("echo 0 > /sys/class/leds/OUTD5/brightness");       //rack8
    system("echo 0 > /sys/class/leds/OUTD6/brightness");       //rack7
    system("echo 0 > /sys/class/leds/OUTD7/brightness");       //rack6
    system("echo 0 > /sys/class/leds/OUTC0/brightness");       //rack5
    system("echo 0 > /sys/class/leds/OUTC1/brightness");       //rack4
    system("echo 0 > /sys/class/leds/OUTC2/brightness");       //rack3
    system("echo 0 > /sys/class/leds/OUTC3/brightness");       //rack2
    system("echo 0 > /sys/class/leds/OUTC4/brightness");       //rack1
    system("echo 0 > /sys/class/leds/OUTC6/brightness");       //
#else
    system("echo 0 > /root/gpio/OUT5");
    system("echo 0 > /root/gpio/OUT6");
    system("echo 0 > /root/gpio/OUT7");
#endif
    usleep(30000);                                              //about 30ms
#if 0
    QFile input1("/sys/class/gpio/gpio162/value");
    QFile input2("/sys/class/gpio/gpio160/value");
    QFile input3("/sys/class/gpio/gpio165/value");
    QFile input4("/sys/class/gpio/gpio164/value");
    QFile input5("/sys/class/gpio/gpio159/value");
    QFile input6("/sys/class/gpio/gpio158/value");
    QFile input7("/sys/class/gpio/gpio149/value");
    QFile input8("/sys/class/gpio/gpio146/value");
    QFile input9("/sys/class/gpio/gpio147/value");
    QFile input10("/sys/class/gpio/gpio176/value");
    input1.open(QIODevice::ReadOnly);
    input2.open(QIODevice::ReadOnly);
    input3.open(QIODevice::ReadOnly);
    input4.open(QIODevice::ReadOnly);
    input5.open(QIODevice::ReadOnly);
    input6.open(QIODevice::ReadOnly);
    input7.open(QIODevice::ReadOnly);
    input8.open(QIODevice::ReadOnly);
    input9.open(QIODevice::ReadOnly);
    input10.open(QIODevice::ReadOnly);
    QTextStream in1(&input1);
    QTextStream in2(&input2);
    QTextStream in3(&input3);
    QTextStream in4(&input4);
    QTextStream in5(&input5);
    QTextStream in6(&input6);
    QTextStream in7(&input7);
    QTextStream in8(&input8);
    QTextStream in9(&input9);
    QTextStream in10(&input10);
    in1 >> k1;
    in2 >> k2;
    in3 >> k3;
    in4 >> k4;
    in5 >> k5;
    in6 >> k6;
    in7 >> k7;
    in8 >> k8;
    in9 >> k9;
    in10 >> k10;
    input1.close();
    input2.close();
    input3.close();
    input4.close();
    input5.close();
    input6.close();
    input7.close();
    input8.close();
    input9.close();
    input10.close();
#else
    QFile input1("/root/gpio/IN0/value");
    QFile input2("/root/gpio/IN1/value");
    QFile input3("/root/gpio/IN2/value");
    QFile input4("/root/gpio/IN3/value");
    QFile input5("/root/gpio/IN4/value");
    input1.open(QIODevice::ReadOnly);
    input2.open(QIODevice::ReadOnly);
    input3.open(QIODevice::ReadOnly);
    input4.open(QIODevice::ReadOnly);
    input5.open(QIODevice::ReadOnly);
    QTextStream in1(&input1);
    QTextStream in2(&input2);
    QTextStream in3(&input3);
    QTextStream in4(&input4);
    QTextStream in5(&input5);
    in1 >> k1;
    in2 >> k2;
    in3 >> k3;
    in4 >> k4;
    in5 >> k5;
    input1.close();
    input2.close();
    input3.close();
    input4.close();
    input5.close();
#endif


    if(k1 == 1)
    {
        ui->INA0->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k2 == 1)
    {
        ui->INA1->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k3 == 1)
    {
        ui->INA2->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k4 == 1)
    {
        ui->INA3->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k5 == 1)
    {
        ui->INA4->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
#if 0
    if(k6 == 1)
    {
        ui->INA5->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k7 == 1)
    {
        ui->INA6->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k8 == 1)
    {
        ui->INA7->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k9 == 1)
    {
        ui->INB0->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
    if(k10 == 1)
    {
        ui->INB1->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    }
#endif
    //    ui->INA1->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INA2->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INA3->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INA4->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INA5->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INA6->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INA7->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INB0->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INB1->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
    //    ui->INB2->setStyleSheet("background-color: rgb(100, 100, 100);color: rgb(248, 248, 248);font: 14pt \"黑体\";");
}

void Newconfiginfo::on_pushButton_197_clicked()
{
    if(ui->restart_hour->text().toInt()<23)
    {
        int new_hour=(ui->restart_hour->text()).toInt()+1;
        if(new_hour<10)
            ui->restart_hour->setText("0"+QString::number(new_hour));
        else
            ui->restart_hour->setText(QString::number(new_hour));
    }
    else
        ui->restart_hour->setText("00");
}

void Newconfiginfo::on_pushButton_198_clicked()
{
    if(ui->restart_hour->text().toInt()>0)
    {
        int new_hour=(ui->restart_hour->text()).toInt()-1;
        if(new_hour<10)
            ui->restart_hour->setText("0"+QString::number(new_hour));
        else
            ui->restart_hour->setText(QString::number(new_hour));
    }
    else
        ui->restart_hour->setText(QString::number(23));
}

void Newconfiginfo::on_pushButton_199_clicked()
{
    if(ui->restart_minute->text().toInt()<59)
    {
        int new_minute=(ui->restart_minute->text()).toInt()+1;
        if(new_minute<10)
            ui->restart_minute->setText("0"+QString::number(new_minute));
        else
            ui->restart_minute->setText(QString::number(new_minute));
    }
    else
        ui->restart_minute->setText("00");
}

void Newconfiginfo::on_pushButton_196_clicked()
{
    if(ui->restart_minute->text().toInt()>0)
    {
        int new_minute=(ui->restart_minute->text()).toInt()-1;
        if(new_minute<10)
            ui->restart_minute->setText("0"+QString::number(new_minute));
        else
            ui->restart_minute->setText(QString::number(new_minute));
    }
    else
        ui->restart_minute->setText(QString::number(59));
}

void Newconfiginfo::on_pushButton_201_clicked()
{
    if(ui->restart_second->text().toInt()<59)
    {
        int new_second=(ui->restart_second->text()).toInt()+1;
        if(new_second<10)
            ui->restart_second->setText("0"+QString::number(new_second));
        else
            ui->restart_second->setText(QString::number(new_second));
    }
    else
        ui->restart_second->setText("00");
}

void Newconfiginfo::on_pushButton_200_clicked()
{
    if(ui->restart_second->text().toInt()>0)
    {
        int new_second=(ui->restart_second->text()).toInt()-1;
        if(new_second<10)
            ui->restart_second->setText("0"+QString::number(new_second));
        else
            ui->restart_second->setText(QString::number(new_second));
    }
    else
        ui->restart_second->setText(QString::number(59));
}

void Newconfiginfo::on_pushButton_restart_clicked()
{
    restart_enabled_temp=!restart_enabled_temp;
    if(restart_enabled_temp)
    {
        ui->pushButton_restart->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/13.bmp);");
        restartShow(true);
    }
    else
    {
        ui->pushButton_restart->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/14.bmp);");
        restartShow(false);
    }
}

void Newconfiginfo::restartShow(bool isShow)
{
    if(isShow)
    {
        ui->label_359->show();
        ui->pushButton_196->show();
        ui->pushButton_197->show();
        ui->pushButton_198->show();
        ui->pushButton_199->show();
        ui->pushButton_200->show();
        ui->pushButton_201->show();
        ui->restart_hour->show();
        ui->restart_minute->show();
        ui->restart_second->show();
        ui->label_361->show();
        ui->label_362->show();
    }
    else
    {
        ui->label_359->hide();
        ui->pushButton_196->hide();
        ui->pushButton_197->hide();
        ui->pushButton_198->hide();
        ui->pushButton_199->hide();
        ui->pushButton_200->hide();
        ui->pushButton_201->hide();
        ui->restart_hour->hide();
        ui->restart_minute->hide();
        ui->restart_second->hide();
        ui->label_361->hide();
        ui->label_362->hide();
    }
}

void Newconfiginfo::on_pushButton_channel_add_clicked()
{
    bool condition=false;
    condition = (ui->lineEdit_channel->text().toInt()>4) || (ui->lineEdit_channel->text().toInt()<1);
    if( condition )
        ui->lineEdit_channel->setText("1");

    if((ui->lineEdit_channel->text().toInt()+1) == 5)
        ui->lineEdit_channel->setText("4");
    else
        ui->lineEdit_channel->setText(QString::number(ui->lineEdit_channel->text().toInt()+1));
}

void Newconfiginfo::on_pushButton_channel_minus_clicked()
{
    bool condition=false;
    condition = (ui->lineEdit_channel->text().toInt()>4) || (ui->lineEdit_channel->text().toInt()<1) ;
    if(condition)
        ui->lineEdit_channel->setText("1");

    if((ui->lineEdit_channel->text().toInt()-1) <= 0)
        ui->lineEdit_channel->setText(tr("1"));
    else
        ui->lineEdit_channel->setText(QString::number(ui->lineEdit_channel->text().toInt()-1));
}

void Newconfiginfo::on_pushButton_22_clicked()
{
    if(ui->stackedWidget_2->currentIndex()==2)
    {
        whichButtonClick = "baseback";
        if(!isbaseinfochange)
        {
            baseInfoIsChange();
        }
        else
        {
            DebugMode = false;
            system("cp /config.ini /config1.ini &");
            this->close();
            if(ControlType_1 !="SB356")
                emit closeconfig();
        }
    }
    else if(SaveWhat == "pdm")
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_100->setGraphicsEffect(e3);
        ui->label_100->show();
        ui->label_100->setGeometry(0,0,1366,768);
        SaveWhat = "pdmout";
        save = new Save(this);
        connect(save,SIGNAL(sendDeSingle(bool)),this,SLOT(receiveDesignle(bool)));
        save->show();
    }
    else if(ui->stackedWidget_2->currentIndex()==5|| SaveWhat == "pdmout")
    {
        int tempdata = numpdm;
        for(int i = 0;i<tempdata;i++)
        {
            delete butt1[i];
            numpdm--;
        }
        ui->stackedWidget_6->setCurrentIndex(0);
        system("cp /config.ini /config1.ini &");
        this->close();
        SaveWhat = "";
    }
    else
    {
        ui->label_166->show();
        system("cp /config.ini /config1.ini &");
        this->close();
        SaveWhat = "";
    }
}

void Newconfiginfo::on_pushButton_channel_add_2_clicked()
{
    bool condition=false;
    condition = (ui->lineEdit_channel_2->text().toInt()>4) || (ui->lineEdit_channel_2->text().toInt()<1);
    if( condition )
        ui->lineEdit_channel_2->setText("1");

    if((ui->lineEdit_channel_2->text().toInt()+1) == 5)
        ui->lineEdit_channel_2->setText("4");
    else
        ui->lineEdit_channel_2->setText(QString::number(ui->lineEdit_channel_2->text().toInt()+1));
}

void Newconfiginfo::on_pushButton_channel_minus_2_clicked()
{
    bool condition=false;
    condition = (ui->lineEdit_channel_2->text().toInt()>4) || (ui->lineEdit_channel_2->text().toInt()<1) ;
    if(condition)
        ui->lineEdit_channel_2->setText("1");

    if((ui->lineEdit_channel_2->text().toInt()-1) <= 0)
        ui->lineEdit_channel_2->setText(tr("1"));
    else
        ui->lineEdit_channel_2->setText(QString::number(ui->lineEdit_channel_2->text().toInt()-1));
}

void Newconfiginfo::on_pushButton_taotong_add_2_clicked()
{
    bool condition=false;
    if(Factory=="SVW3"||Factory=="SVW2")
    {
        condition = (ui->lineEdit_taotong_2->text().toInt()>4) || (ui->lineEdit_taotong_2->text().toInt()<1);
    }
    else //if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
    {
        condition = (ui->lineEdit_taotong_2->text().toInt()>8) || (ui->lineEdit_taotong_2->text().toInt()<1);
    }
    if( condition )
        ui->lineEdit_taotong_2->setText("OFF");


    if((Factory == "SVW3"||Factory=="SVW2")&&(ui->lineEdit_taotong_2->text().toInt()+1) == 5)
        ui->lineEdit_taotong_2->setText("4");
    else if((ui->lineEdit_taotong_2->text().toInt()+1) == 9)
        ui->lineEdit_taotong_2->setText("8");
    else
        ui->lineEdit_taotong_2->setText(QString::number(ui->lineEdit_taotong_2->text().toInt()+1));
}

void Newconfiginfo::on_pushButton_taotong_minus_2_clicked()
{
    bool condition=false;
    if(Factory=="SVW3"||Factory=="SVW2")
    {
        condition = (ui->lineEdit_taotong_2->text().toInt()>4) || (ui->lineEdit_taotong_2->text().toInt()<1) ;
    }
    else //if (Factory =="BYDSZ" ||Factory =="BYDXA" || Factory == "Ningbo")
    {
        condition = (ui->lineEdit_taotong_2->text().toInt()>8) || (ui->lineEdit_taotong_2->text().toInt()<1) ;
    }
    if(condition)
        ui->lineEdit_taotong_2->setText("OFF");

    if((ui->lineEdit_taotong_2->text().toInt()-1) <= 0)
        ui->lineEdit_taotong_2->setText("OFF");
    else
    {
        ui->lineEdit_taotong_2->setText(QString::number(ui->lineEdit_taotong_2->text().toInt()-1));
    }
}

void Newconfiginfo::on_pushButton_pronum_add_history_clicked()
{
    new_pronum = ui->lineEdit_pronum_history->text().toInt()+1;
    ui->lineEdit_pronum_history->setText(QString::number(new_pronum));
}

void Newconfiginfo::on_pushButton_pronum_minus_history_clicked()
{
    if (ui->lineEdit_pronum_history->text().toInt()>0)
    {
        new_pronum=ui->lineEdit_pronum_history->text().toInt()-1;
        ui->lineEdit_pronum_history->setText(QString::number(new_pronum));
    }
}

void Newconfiginfo::queryResult(QString condition)
{
    if(!mDataBase.isOpen())
    {
        DTdebug()<<"mDataBase is not open";
    }
    else
    {
        if(!query_number.exec("SELECT count(Torque) FROM "+Localtable+condition))
            DTdebug()<< "search three days fail "<<query_number.lastError();
        else
        {
            query_number.next();
            int n = query_number.value(0).toInt();//n为数据个数
            ui->label_num_2->setText(QString::number(n));
            DTdebug()<<"SELECT Torque FROM " + Localtable + condition;
            query_datas.exec("SELECT Torque FROM " + Localtable + condition);

            //            DTdebug()<<"SELECT `MaxValue`, MinValue From " + Localtable + condition+" ORDER BY RecordID DESC LIMIT 1";
            if(!query_bound.exec("SELECT `MaxValue`, MinValue From " + Localtable + condition+" ORDER BY RecordID DESC LIMIT 1"))
                DTdebug()<<"query_bound.lastError() "<<query_bound.lastError();
            area->setQuery(query_number,query_datas,query_bound);
        }
    }
}

void Newconfiginfo::on_pushButton_down_clicked()
{
    ui->stackedWidget_dateSelect->setCurrentIndex(1);
    ui->pushButton_threeDays->setChecked(false);
    ui->pushButton_oneWeek->setChecked(false);
    ui->pushButton_oneMonth->setChecked(false);
}

void Newconfiginfo::on_pushButton_up_clicked()
{
    ui->stackedWidget_dateSelect->setCurrentIndex(0);
    ui->label_dateWrong->hide();
    ui->dateEdit_max->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
    ui->dateEdit_min->setStyleSheet("QDateEdit {font: 14pt \"Arial\";border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::drop-down { width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QDateEdit::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}");
    QDate date = QDate::currentDate();//获取系统现在的时间
    ui->dateEdit_min->setDate(date);
    ui->dateEdit_max->setDate(date);
}

void Newconfiginfo::on_pushButton_threeDays_clicked()
{
    if(ui->pushButton_threeDays->isChecked())
    {
        ui->pushButton_oneWeek->setChecked(false);
        ui->pushButton_oneMonth->setChecked(false);
    }
}

void Newconfiginfo::on_pushButton_oneWeek_clicked()
{
    if(ui->pushButton_oneWeek->isChecked())
    {
        ui->pushButton_threeDays->setChecked(false);
        ui->pushButton_oneMonth->setChecked(false);
    }
}

void Newconfiginfo::on_pushButton_oneMonth_clicked()
{
    if(ui->pushButton_oneMonth->isChecked())
    {
        ui->pushButton_oneWeek->setChecked(false);
        ui->pushButton_threeDays->setChecked(false);
    }
}

void Newconfiginfo::on_tabWidget_currentChanged(int currentTab)
{
    historyclear();
    if(currentTab == 0)
    {
        ui->stackedWidget_condition->setCurrentIndex(0);
        ui->checkBox_NOK->show();
        ui->checkBox_Reset->show();
    }
    else if(currentTab ==1)
    {
        ui->stackedWidget_condition->setCurrentIndex(1);
        ui->checkBox_NOK->hide();
        ui->checkBox_Reset->hide();
    }
}

void Newconfiginfo::on_pushButton_repair_clicked()
{
    DebugMode = true;
    ui->label_repair->show();
    ui->label_history->hide();
    on_pushButton_repairClear_clicked();
    ui->stackedWidget_2->setCurrentIndex(1);
}

void Newconfiginfo::on_pushButton_history_clicked()
{
    DebugMode = false;
    ui->label_repair->hide();
    ui->label_history->show();
    ui->stackedWidget_2->setCurrentIndex(3);
}

void Newconfiginfo::on_pushButton_historyReturn_clicked()
{
    DebugMode = false;
    system("cp /config.ini /config1.ini &");
    this->close();
    SaveWhat = "";
}

void Newconfiginfo::repairSearch()
{
    if(!ui->label_repairVin->text().isEmpty())
    {
        emit sendRepairVIN(ui->label_repairVin->text());
    }
}

void Newconfiginfo::receiveRepairReply(int errorCode, QVariantMap replyMap)
{
    if(errorCode==0)
    {
        bool Result = replyMap["Result"].toBool() ;
        if(Result)
        {
            ui->label_repairWrong->setText(tr(""));
            repairDataList = replyMap["Data"].toList();
            setRepairData();
        }
        else
        {
            setColor("red");
            ui->label_repairWrong->setText(tr("回复操作失败！"));
            DTdebug()<< "Get repair fail errcode:"<<replyMap["ErrorCode"].toString();
        }
    }
    else if(errorCode == 1)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("未连网，查询失败！"));
    }
    else if(errorCode == 2)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("序列化失败，查询失败！"));
    }
    else if(errorCode == 3)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("HTTP请求失败，查询失败！"));
    }
    else if(errorCode == 4)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("解析失败，查询失败！"));
    }
    else if(errorCode == 6)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("回复超时，查询失败！"));
    }
}

void Newconfiginfo::setRepairData()
{
    ui->tableWidget_repair->setRowCount(0);
    ui->tableWidget_repair->setRowCount(repairDataList.size());//总行数增加1

    setColor("black");
    ui->label_repairWrong->setText(tr("结果: "+QByteArray::number(repairDataList.size())+" 条"));

    for(int i=0;i<repairDataList.size();i++)
    {
        ui->tableWidget_repair->setRowHeight(i,43);//行
        QVariantMap repairMap = repairDataList[i].toMap();
        QComboBox *comBox = new QComboBox();
        comBox->setStyleSheet("QComboBox {font: 14pt \"Arial\";border-width:0px; border-style:solid; margin-top: 0ex;}QComboBox::drop-down {width:30px; border-width:1px; border-style:solid; border-color:rgb(51, 153, 255); }QComboBox::down-arrow {image: url(:/LCD_CS351/LCD_CS351/logo/arrow.png);}QComboBox QAbstractItemView{height:50px;border: 1px solid rgb(51, 153, 255); selection-background-color: rgb(51, 153, 255); }QScrollBar:vertical{width:30px; border: 2px solid grey;}QComboBox QAbstractItemView::item{height:20px}");
        comBox->addItem("NOK");
        comBox->addItem("OK");

        ////        QGroupBox *radioGroup = new QGroupBox("Options OK", this);
        ////        radioGroup->setGeometry(0,0,160,43);

        //        QRadioButton *radio1 = new QRadioButton("NOK", this);
        //        radio1->move(0,0);
        //        QRadioButton *radio2 = new QRadioButton("OK", this);
        //        radio2->move(80,0);
        //        QButtonGroup getModeSelect;
        ////        getModeSelect.setGeometry(0,0,160,43);
        //        getModeSelect.addButton(radio1,1);//将radioButton加进组
        //        getModeSelect.addButton(radio2,2);
        QRegExp rx("[0-9.]{7,15}");
        QValidator *validator = new QRegExpValidator(rx, this );

        QLineEdit *lineEdit1 = new QLineEdit();
        lineEdit1->setValidator(validator);
        lineEdit1->setStyleSheet("border-width:0px; border-style:solid; font: 14pt \"黑体\";");
        QLineEdit *lineEdit2 = new QLineEdit();
        lineEdit2->setValidator(validator);
        lineEdit2->setStyleSheet("border-width:0px; border-style:solid; font: 14pt \"黑体\";");

        ui->tableWidget_repair->setItem(i,0,new QTableWidgetItem(repairMap["Station"].toString()));
        ui->tableWidget_repair->setItem(i,1,new QTableWidgetItem(repairMap["ScrewID"].toString()));
        ui->tableWidget_repair->setCellWidget(i,2,lineEdit1);
        ui->tableWidget_repair->setCellWidget(i,3,lineEdit2);
        //        ui->tableWidget_repair->setItem(i,2,new QTableWidgetItem(""));
        //        ui->tableWidget_repair->setItem(i,3,new QTableWidgetItem(""));
        ui->tableWidget_repair->setCellWidget(i,4,comBox);
        ui->tableWidget_repair->setItem(i,5,new QTableWidgetItem(repairMap["AssemblyContent"].toString()));

        //        QTableWidgetItem *item = ui->tableWidget_repair->item(i, 2);
        //        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        //        QTableWidgetItem *item1 = ui->tableWidget_repair->item(i, 3);
        //        item1->setFlags(item1->flags() & (~Qt::ItemIsEditable));
        //        QTableWidgetItem *item2 = ui->tableWidget_repair->item(i, 4);
        //        item2->setFlags(item2->flags() & (~Qt::ItemIsEditable));
        QTableWidgetItem *item0 = ui->tableWidget_repair->item(i, 0);
        if(item0)
        {
            item0->setFlags(item0->flags() & (~Qt::ItemIsEditable));
        }
        else
        {
            item0 = new QTableWidgetItem;
            item0->setFlags(item0->flags() & (~Qt::ItemIsEditable));
            ui->tableWidget_repair->setItem(i,0,item0);
        }
        QTableWidgetItem *item1 = ui->tableWidget_repair->item(i, 1);
        if(item1)
        {
            item1->setFlags(item1->flags() & (~Qt::ItemIsEditable));
        }
        else
        {
            item1 = new QTableWidgetItem;
            item1->setFlags(item1->flags() & (~Qt::ItemIsEditable));
            ui->tableWidget_repair->setItem(i,1,item1);
        }
        QTableWidgetItem *item4 = ui->tableWidget_repair->item(i, 4);
        if(item4)
        {
            item4->setFlags(item4->flags() & (~Qt::ItemIsEditable));
        }
        else
        {
            item4 = new QTableWidgetItem;
            item4->setFlags(item4->flags() & (~Qt::ItemIsEditable));
            ui->tableWidget_repair->setItem(i,4,item4);
        }
        QTableWidgetItem *item5 = ui->tableWidget_repair->item(i, 5);
        if(item5)
        {
            item5->setFlags(item5->flags() & (~Qt::ItemIsEditable));
        }
        else
        {
            item5 = new QTableWidgetItem;
            item5->setFlags(item5->flags() & (~Qt::ItemIsEditable));
            ui->tableWidget_repair->setItem(i,5,item5);
        }
    }
}

void Newconfiginfo::initRepairTable()
{
    ui->tableWidget_repair->setColumnCount(6);
    ui->tableWidget_repair->setRowCount(0);

    //列名
    QStringList header;
    header<<tr("工位")<<tr("螺栓编号")<<tr("扭矩")<<tr("角度")<<tr("拧紧状态")<<tr("说明");
    ui->tableWidget_repair->setHorizontalHeaderLabels(header);

    ui->tableWidget_repair->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget_repair->setColumnWidth(0,160);//第1列宽度
    ui->tableWidget_repair->setColumnWidth(1,110);//第2列宽度
    ui->tableWidget_repair->setColumnWidth(2,160);//第3列宽度
    ui->tableWidget_repair->setColumnWidth(3,160);//第4列宽度
    ui->tableWidget_repair->setColumnWidth(4,110);//第5列宽度
    ui->tableWidget_repair->setColumnWidth(5,310);//第6列宽度
    ui->tableWidget_repair->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(51, 153, 255);"
                                                              "color: rgb(248, 248, 255);border: 0px; font:14pt}");
    ui->tableWidget_repair->verticalHeader()->setStyleSheet("QHeaderView::section{border-bottom:1px groove #bdac96;border-top:0px;border-left:0px;border-right:1px groove #e9dbc4;background-color: rgb(250, 250, 250);font: 12pt}");
    ui->tableWidget_repair->horizontalHeader()->setFixedHeight(43);
    ui->tableWidget_repair->verticalHeader()->setFixedWidth(51);
    //    ui->tableWidget->horizontalHeader()->setFixedWidth(369);
    ui->tableWidget_repair->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidget_repair->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    //    ui->tableWidget_repair->setSelectionBehavior(QAbstractItemView::SelectRows);
    //    for(int i = 0; i<ui->tableWidget_repair->rowCount();i++)
    //    {
    //        //        item(i, 0)->setFlags(Qt::NoItemFlags);
    //        //        item(i, 1)->setFlags(Qt::NoItemFlags);
    //        //        item(i, 2)->setFlags(Qt::NoItemFlags);

    //        QTableWidgetItem *item = ui->tableWidget_repair->item(i, 2);
    //        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    //        QTableWidgetItem *item1 = ui->tableWidget_repair->item(i, 3);
    //        item1->setFlags(item1->flags() & (~Qt::ItemIsEditable));
    //        QTableWidgetItem *item2 = ui->tableWidget_repair->item(i, 4);
    //        item2->setFlags(item2->flags() & (~Qt::ItemIsEditable));
    //        //        if(item) {
    //        //            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    //        //        } else {
    //        //            item = new QTableWidgetItem;
    //        //            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    //        //            table->setItem(i, col, item);
    //        //        }
    //    }

    //    insertRow = -1;
}

void Newconfiginfo::on_checkBox_isRepair_clicked(bool checked)
{
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    configIniRead->setValue("baseinfo/isRepair",checked?"1":"0");
    delete configIniRead;
}

void Newconfiginfo::setRepair(bool checked)
{
    ui->checkBox_isRepair->setChecked(checked);
}

void Newconfiginfo::on_pushButton_repairClear_clicked()
{
    ui->label_repairVin->clear();
    ui->label_repairWrong->clear();
    ui->tableWidget_repair->setRowCount(0);
}

void Newconfiginfo::on_pushButton_repairUpload_clicked()
{
    if(ui->label_repairVin->text().isEmpty()||ui->tableWidget_repair->rowCount()==0)
        return;
    bool isUpload = true;
    DTdebug()<<"repairDataList"<<repairDataList<<repairDataList.size();
    for(int i=0;i<repairDataList.size();i++)
    {
        QWidget * widget=ui->tableWidget_repair->cellWidget(i,4);//获得widget
        QComboBox *combox=(QComboBox*)widget;//强制转化为QComboBox
        QString isOK=combox->currentText();
        if(isOK=="NOK")
        {
            isUpload = false;
            break;
        }
    }
    bool isNull = false;
    for(int i=0;i<repairDataList.size();i++)
    {
        QWidget * widget1=ui->tableWidget_repair->cellWidget(i,2);//获得widget
        QLineEdit *lineEdit1=(QLineEdit*)widget1;//强制转化为QComboBox
        QWidget * widget2=ui->tableWidget_repair->cellWidget(i,3);//获得widget
        QLineEdit *lineEdit2=(QLineEdit*)widget2;//强制转化为QComboBox
        if(lineEdit1->text().isEmpty()||lineEdit2->text().isEmpty())
            isNull = true;
    }
    if(isUpload && !isNull)
    {
        for(int i=0;i<repairDataList.size();i++)
        {
            QVariantMap repairMap = repairDataList[i].toMap();

            QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间

            DATA_STRUCT demo;
            demo.data_model[0] = time.addSecs(i).toString("yyyy-MM-dd");;
            demo.data_model[1] = time.addSecs(i).toString("hh:mm:ss");
            demo.data_model[2] = "OK";

            QWidget * widget1=ui->tableWidget_repair->cellWidget(i,2);//获得widget
            QLineEdit *lineEdit1=(QLineEdit*)widget1;//强制转化为QComboBox
            QWidget * widget2=ui->tableWidget_repair->cellWidget(i,3);//获得widget
            QLineEdit *lineEdit2=(QLineEdit*)widget2;//强制转化为QComboBox

            demo.data_model[3] = lineEdit1->text();
            demo.data_model[4] = lineEdit2->text();
            demo.data_model[5] = repairMap["ScrewID"].toString();
            demo.data_model[6] = repairMap["IDCode"].toString();
            demo.data_model[7] = repairMap["Cycle"].toString();
            demo.data_model[8] = "Curve is null";
            demo.data_model[9] = repairMap["Program"].toString();
            demo.data_model[10] = repairMap["JobStatus"].toString();
            demo.data_model[11] = "";
            demo.data_model[12] = repairMap["IIO"].toString();
            demo.data_model[13] = "0.0";
            demo.data_model[14] = "0.0";
            demo.data_model[15] = repairMap["Channel"].toString();
            demo.data_model[16] = repairMap["LineID"].toString();
            demo.data_model[17] = repairMap["Station"].toString();
            demo.data_model[18] = repairMap["IPAddress"].toString();

            QVariant DataVar;
            DataVar.setValue(demo);
            emit sendfromworkthread(DataVar);
        }
        on_pushButton_repairClear_clicked();
        setColor("black");
        ui->label_repairWrong->setText(tr("上传成功"));
    }
    else if(!isUpload)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("有拧紧状态为NOK"));
    }
    else if(isNull)
    {
        setColor("red");
        ui->label_repairWrong->setText(tr("有扭矩或角度为空"));
    }
}

void Newconfiginfo::setColor(QString color)
{
    if(color == "red")
        ui->label_repairWrong->setStyleSheet("font: 14pt \"黑体\";color: rgb(250, 0, 0);");
    else if(color == "black")
        ui->label_repairWrong->setStyleSheet("font: 14pt \"黑体\";color: rgb(0, 0, 0);");
}

void Newconfiginfo::on_pushButton_wirelessLocation_clicked()
{
    if(ui->stackedWidget_2-> currentIndex() != 14)
    {
        whichButtonClick = "wirelessLocation";
        if(!isadvancedchange)
            advancedIsChange();
        else if(!ismasterslavechange)
            masterslaveIsChange();
        else if(!isboundchange)
            boundIsChange();
        else
        {
            islocationchange = false;
            ui->label_locationWrong->hide();
            ui->stackedWidget_2->setCurrentIndex(14);
            ui->label_87->hide();
            ui->label_101->hide();
            ui->label_88->hide();
            ui->label_98->hide();
            ui->label_99->hide();
            ui->label_162->hide();
            ui->label_wirelessLocation->show();

            //            initAnchorTable();
            //            initDataConfigTable();
            //            LocationParseJson * locationParseJson = new LocationParseJson;
            //            currentLocation = locationParseJson->parseJson();
            //            delete locationParseJson;
            //            if(currentLocation.value("errorCode")!=1)
            //                readLocation(currentLocation);
        }
    }
}

void Newconfiginfo::initAnchorTable()
{
    ui->tableWidget_anchor->setColumnCount(4);
    ui->tableWidget_anchor->setRowCount(0);

    // 创建表格项目，并将它们作为标头
    QTableWidgetItem *headerH = new QTableWidgetItem(tr("基站"));
    ui->tableWidget_anchor->setHorizontalHeaderItem(0,headerH);
    QTableWidgetItem *headerH1 = new QTableWidgetItem(tr("基站编号"));
    ui->tableWidget_anchor->setHorizontalHeaderItem(1,headerH1);
    QTableWidgetItem *headerH2 = new QTableWidgetItem(tr("基站坐标"));
    ui->tableWidget_anchor->setHorizontalHeaderItem(2,headerH2);
    QTableWidgetItem *headerH3 = new QTableWidgetItem(tr("标定基站"));
    ui->tableWidget_anchor->setHorizontalHeaderItem(3,headerH3);

    ui->tableWidget_anchor->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget_anchor->setColumnWidth(0,100);//第1列宽度
    ui->tableWidget_anchor->setColumnWidth(1,100);//第2列宽度
    ui->tableWidget_anchor->setColumnWidth(2,200);//第3列宽度
    ui->tableWidget_anchor->setColumnWidth(3,100);//第3列宽度
    ui->tableWidget_anchor->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(51, 153, 255);"
                                                              "color: rgb(248, 248, 255);border: 0px; font:14pt}");
    //    ui->tableWidget_anchor->verticalHeader()->setStyleSheet("QHeaderView::section{border-bottom:1px groove #bdac96;border-top:0px;border-left:0px;border-right:1px groove #e9dbc4;background-color: rgb(250, 250, 250);font: 12pt}");
    ui->tableWidget_anchor->horizontalHeader()->setFixedHeight(43);
    ui->tableWidget_anchor->horizontalHeader()->setVisible(true);
    ui->tableWidget_anchor->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidget_anchor->setSelectionBehavior(QAbstractItemView::SelectRows);

    anchorInsertRow = -1;
}

void Newconfiginfo::initDataConfigTable()
{
    ui->tableWidget_dataConfig->setColumnCount(4);
    ui->tableWidget_dataConfig->setRowCount(0);

    // 创建表格项目，并将它们作为标头
    QTableWidgetItem *headerH = new QTableWidgetItem(tr("Data"));
    ui->tableWidget_dataConfig->setHorizontalHeaderItem(0,headerH);
    QTableWidgetItem *headerH1 = new QTableWidgetItem(tr("IP地址"));
    ui->tableWidget_dataConfig->setHorizontalHeaderItem(1,headerH1);
//    QTableWidgetItem *headerH2 = new QTableWidgetItem(tr("定位区域"));
//    ui->tableWidget_dataConfig->setHorizontalHeaderItem(2,headerH2);
    QTableWidgetItem *headerH2 = new QTableWidgetItem(tr("标签"));
    ui->tableWidget_dataConfig->setHorizontalHeaderItem(2,headerH2);

    ui->tableWidget_dataConfig->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget_dataConfig->setColumnWidth(0,60);//第1列宽度
    ui->tableWidget_dataConfig->setColumnWidth(1,150);//第2列宽度
//    ui->tableWidget_dataConfig->setColumnWidth(2,200);//第3列宽度
    ui->tableWidget_dataConfig->setColumnWidth(2,290);//第3列宽度
    ui->tableWidget_dataConfig->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(51, 153, 255);"
                                                                  "color: rgb(248, 248, 255);border: 0px; font:14pt}");
    //    ui->tableWidget_anchor->verticalHeader()->setStyleSheet("QHeaderView::section{border-bottom:1px groove #bdac96;border-top:0px;border-left:0px;border-right:1px groove #e9dbc4;background-color: rgb(250, 250, 250);font: 12pt}");
    ui->tableWidget_dataConfig->horizontalHeader()->setFixedHeight(43);
    ui->tableWidget_dataConfig->horizontalHeader()->setVisible(true);
    ui->tableWidget_dataConfig->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidget_dataConfig->setSelectionBehavior(QAbstractItemView::SelectRows);

    dataConfigInsertRow = -1;
}

void Newconfiginfo::readLocation(QVariantMap currentLocation)    //读基站配置信息
{
    anchorList.clear();
    dataList.clear();
    offsetList.clear();
    ui->tableWidget_anchor->setRowCount(0);
    ui->tableWidget_dataConfig->setRowCount(0);
    offsetList = currentLocation["offsetList"].toList();
    int i = 0;
	int archornum = 0;
    foreach (QVariant anchor, currentLocation["anchors"].toList()) {
        QVariantMap anchorMap = anchor.toMap();
        anchorList.append(anchorMap);
        int row = ui->tableWidget_anchor->rowCount();
        ui->tableWidget_anchor->setRowCount(row + 1);//总行数增加1
        ui->tableWidget_anchor->setRowHeight(row,43);//行
        QTableWidgetItem *tableWidgetItem0;
        if(i==0)
            tableWidgetItem0 = new QTableWidgetItem(tr("同步器"));
        else
            tableWidgetItem0= new QTableWidgetItem("基站"+QString::number(i));
        QTableWidgetItem *tableWidgetItem1 = new QTableWidgetItem(anchorMap["anchor_no"].toString());
        QString x = anchorMap["x"].toString();
        QString y = anchorMap["y"].toString();
        QString z = anchorMap["z"].toString();
        QString coords = "("+x+","+y+","+z+")";
        QTableWidgetItem *tableWidgetItem2 = new QTableWidgetItem(coords);
        QTableWidgetItem *tableWidgetItem3 = new QTableWidgetItem(anchorMap["checked"].toString());
        ui->tableWidget_anchor->setItem(row, 0, tableWidgetItem0);
        ui->tableWidget_anchor->setItem(row, 1, tableWidgetItem1);
        ui->tableWidget_anchor->setItem(row, 2, tableWidgetItem2);
        ui->tableWidget_anchor->setItem(row, 3, tableWidgetItem3);
        i++;
    }
    int j=0;
    foreach (QVariant data, currentLocation["datas"].toList()) {
        QVariantMap dataMap = data.toMap();
        dataList.append(dataMap);
        int row = ui->tableWidget_dataConfig->rowCount();
        ui->tableWidget_dataConfig->setRowCount(row + 1);//总行数增加1
        ui->tableWidget_dataConfig->setRowHeight(row,43);//行
        QTableWidgetItem *tableWidgetItem0;
        if(j==0)
            tableWidgetItem0 = new QTableWidgetItem(tr("主机"));
        else
            tableWidgetItem0= new QTableWidgetItem("从机"+QString::number(j));
        QTableWidgetItem *tableWidgetItem1 = new QTableWidgetItem(dataMap["IP"].toString());
//        QTableWidgetItem *tableWidgetItem2 = new QTableWidgetItem("");
        QList<QVariant> mac = dataMap["tag_mac"].toList();
        QString macs="";
        for(int i=0;i<mac.size();i++)
        {
            if(i==0)
                macs = mac[i].toString();
            else
                macs = macs+","+mac[i].toString();
        }
        QTableWidgetItem *tableWidgetItem2 = new QTableWidgetItem(macs);
        ui->tableWidget_dataConfig->setItem(row, 0, tableWidgetItem0);
        ui->tableWidget_dataConfig->setItem(row, 1, tableWidgetItem1);
        ui->tableWidget_dataConfig->setItem(row, 2, tableWidgetItem2);
//        ui->tableWidget_dataConfig->setItem(row, 3, tableWidgetItem3);
        j++;
    }
    int locationStatus = currentLocation["locationStatus"].toInt();
    if(locationStatus == 2)
    {
        ui->radioButton_locationMaster->setChecked(true);
        on_radioButton_locationMaster_clicked();
    }
    else if(locationStatus == 1)
    {
        ui->radioButton_locationSlave->setChecked(true);
        on_radioButton_locationSlave_clicked();
    }
    else if(locationStatus == 0)
    {
        ui->radioButton_locationOff->setChecked(true);
        on_radioButton_locationOff_clicked();
    }
    ui->lineEdit_leastNum->setText(currentLocation["leastNum"].toString());
	ui->lineEdit_tag_timeout->setText(currentLocation["tag_timeout"].toString());
    archornum  = currentLocation["leastNum"].toInt();
    if(offsetList.size()==archornum)
    {
        locationstation = 1;//已标定
    }else
    {
        offsetList.clear();
        locationstation = 0;
    }
    Cell_Calibration_status(locationstation);
}

void Newconfiginfo::receiveAnchor(QVariantMap anchorMap)
{
    //    QTableWidgetItem *tableWidgetItem0;
    //    if(anchorInsertRow==0)
    //        tableWidgetItem0 = new QTableWidgetItem(tr("同步器"));
    //    else
    //        tableWidgetItem0= new QTableWidgetItem("基站"+QString::number(anchorInsertRow));
    QTableWidgetItem *tableWidgetItem1 = new QTableWidgetItem(anchorMap["anchor_no"].toString());
    QString x = anchorMap["x"].toString();
    QString y = anchorMap["y"].toString();
    QString z = anchorMap["z"].toString();
    QString coords = "("+x+","+y+","+z+")";
    QTableWidgetItem *tableWidgetItem2 = new QTableWidgetItem(coords);
    QTableWidgetItem *tableWidgetItem3 = new QTableWidgetItem(anchorMap["checked"].toString());

    //    QTableWidgetItem *tableWidgetItem = new QTableWidgetItem(singleMap["singleStep"].toString());
    //    QTableWidgetItem *tableWidgetItem1 = new QTableWidgetItem(singleMap["parameter"].toString());
    if(currentAnchorOperate == anchorAdd)
    {
        ui->tableWidget_anchor->setRowCount(anchorInsertRow + 1);//总行数增加1
        ui->tableWidget_anchor->setRowHeight(anchorInsertRow,43);//行
        anchorList.append(anchorMap);
    }
    else if(currentAnchorOperate == anchorInsert)
    {
        ui->tableWidget_anchor->insertRow(anchorInsertRow);
        ui->tableWidget_anchor->setRowHeight(anchorInsertRow,43);//行
        anchorList.insert(anchorInsertRow,anchorMap);
    }
    else if(currentAnchorOperate == anchorUpdate)
    {
        anchorList.replace(anchorInsertRow,anchorMap);
    }
    //    ui->tableWidget_anchor->setItem(anchorInsertRow, 0, tableWidgetItem0);
    ui->tableWidget_anchor->setItem(anchorInsertRow, 1, tableWidgetItem1);
    ui->tableWidget_anchor->setItem(anchorInsertRow, 2, tableWidgetItem2);
    ui->tableWidget_anchor->setItem(anchorInsertRow, 3, tableWidgetItem3);
    updateAnchorItem0();
    closeBaseStation();
}

void Newconfiginfo::receiveDataConfig(QVariantMap dataConfigMap)
{
    QTableWidgetItem *tableWidgetItem1 = new QTableWidgetItem(dataConfigMap["IP"].toString());
//    QString x = anchorMap["x"].toString();
//    QString y = anchorMap["y"].toString();
//    QString z = anchorMap["z"].toString();
//    QString coords = "("+x+","+y+","+z+")";
//    QTableWidgetItem *tableWidgetItem2 = new QTableWidgetItem("");
    QList<QVariant> mac = dataConfigMap["tag_mac"].toList();
    QString macs="";
    for(int i=0;i<mac.size();i++)
    {
        if(i==0)
            macs = mac[i].toString();
        else
            macs = macs+","+mac[i].toString();
    }
    QTableWidgetItem *tableWidgetItem2 = new QTableWidgetItem(macs);

    //    QTableWidgetItem *tableWidgetItem = new QTableWidgetItem(singleMap["singleStep"].toString());
    //    QTableWidgetItem *tableWidgetItem1 = new QTableWidgetItem(singleMap["parameter"].toString());
    if(currentDataConfigOperate == dataConfigAdd)
    {
        ui->tableWidget_dataConfig->setRowCount(dataConfigInsertRow + 1);//总行数增加1
        ui->tableWidget_dataConfig->setRowHeight(dataConfigInsertRow,43);//行
        dataList.append(dataConfigMap);
    }
    else if(currentDataConfigOperate == dataConfigInsert)
    {
        ui->tableWidget_dataConfig->insertRow(dataConfigInsertRow);
        ui->tableWidget_dataConfig->setRowHeight(dataConfigInsertRow,43);//行
        dataList.insert(dataConfigInsertRow,dataConfigMap);
    }
    else if(currentDataConfigOperate == dataConfigUpdate)
    {
        dataList.replace(dataConfigInsertRow,dataConfigMap);
    }
    //    ui->tableWidget_anchor->setItem(anchorInsertRow, 0, tableWidgetItem0);
    ui->tableWidget_dataConfig->setItem(dataConfigInsertRow, 1, tableWidgetItem1);
    ui->tableWidget_dataConfig->setItem(dataConfigInsertRow, 2, tableWidgetItem2);
//    ui->tableWidget_dataConfig->setItem(dataConfigInsertRow, 3, tableWidgetItem3);
    updateDataConfigItem0();
    closeLocationConfig();
}

void Newconfiginfo::updateAnchorItem0()
{
    for(int i=0;i<anchorList.length();i++)
    {
        QTableWidgetItem *tableWidgetItem0;
        if(i==0)
            tableWidgetItem0 = new QTableWidgetItem(tr("同步器"));
        else
            tableWidgetItem0= new QTableWidgetItem("基站"+QString::number(i));
        ui->tableWidget_anchor->setItem(i, 0, tableWidgetItem0);
    }
}

void Newconfiginfo::updateDataConfigItem0()
{
    for(int i=0;i<dataList.length();i++)
    {
        QTableWidgetItem *tableWidgetItem0;
        if(i==0)
            tableWidgetItem0 = new QTableWidgetItem(tr("主机"));
        else
            tableWidgetItem0= new QTableWidgetItem("从机"+QString::number(i));
        ui->tableWidget_dataConfig->setItem(i, 0, tableWidgetItem0);
    }
}

void Newconfiginfo::on_pushButton_Anchor_append_clicked()
{
    newBaseStation();
    anchorInsertRow = ui->tableWidget_anchor->rowCount();
    currentAnchorOperate = anchorAdd;
}

void Newconfiginfo::on_pushButton_Anchor_insert_clicked()
{
    if(ui->tableWidget_anchor->currentRow()!= -1)
    {
        newBaseStation();
        anchorInsertRow = ui->tableWidget_anchor->currentRow();
        currentAnchorOperate = anchorInsert;
    }
}

void Newconfiginfo::on_pushButton_Anchor_update_clicked()
{
    if(ui->tableWidget_anchor->currentRow()!= -1)
    {
        newBaseStation();
        anchorInsertRow = ui->tableWidget_anchor->currentRow();
        baseStation->initUI(anchorList[anchorInsertRow].toMap());
        currentAnchorOperate = anchorUpdate;
    }
}

void Newconfiginfo::on_pushButton_Anchor_remove_clicked()
{
    int rowIndex = ui->tableWidget_anchor->currentRow();
    if (rowIndex != -1 && rowIndex != 0)
    {
        ui->tableWidget_anchor->removeRow(rowIndex);
        anchorList.removeAt(rowIndex);
    }
    updateAnchorItem0();
}

void Newconfiginfo::newBaseStation()
{
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);
    baseStation = new BaseStation(this);
    connect(baseStation,SIGNAL(sendParameter(QVariantMap)),this,SLOT(receiveAnchor(QVariantMap)));
    connect(baseStation,SIGNAL(closeDiglag()),this,SLOT(closeBaseStation()));
    baseStation->show();
}

void Newconfiginfo::newLocationConfig()
{
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);
    locationConfig = new LocationConfig(this);
    connect(locationConfig,SIGNAL(sendParameter(QVariantMap)),this,SLOT(receiveDataConfig(QVariantMap)));
    connect(locationConfig,SIGNAL(closeDiglag()),this,SLOT(closeLocationConfig()));
    locationConfig->show();
}

void Newconfiginfo::closeBaseStation()
{
    delete e3;
    ui->label_100->hide();
    delete baseStation;
}

void Newconfiginfo::closeLocationConfig()
{
    delete e3;
    ui->label_100->hide();
    delete locationConfig;
}

void Newconfiginfo::on_pushButton_saveLocation_clicked()
{
    whichButtonClick = "LocationSave";
    newSaveLocation();
}

void Newconfiginfo::newSaveLocation()
{
    SaveWhat = "Save";
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_100->setGraphicsEffect(e3);
    ui->label_100->show();
    ui->label_100->setGeometry(0,0,1366,768);

    save = new Save(this);
    connect(save,SIGNAL(sendSave(bool)),this,SLOT(saveLocation(bool)));
    save->show();
}

void Newconfiginfo::saveLocation(bool isSave)
{
    if(isSave)
    {
        if(ui->radioButton_locationMaster->isChecked() && ui->lineEdit_leastNum->text().isEmpty())
        {
            delete e3;
            ui->label_100->hide();
            delete save;
            ui->label_locationWrong->show();
            islocationchange = false;
            return;
        }
        QVariantMap LocationMap;
        //        Step.insert("isManual",ui->radioButton_manual->isChecked());
        //        if(ui->radioButton_auto->isChecked())
        //        {
        //            if(ui->lineEdit_plcDB->text().isEmpty() || ui->lineEdit_plcIP->text().isEmpty())
        //            {
        //                ui->label_warning->show();
        //                delete e3;
        //                ui->label_black->hide();
        //                delete save;
        //                return;
        //            }
        //            Step.insert("plcDB",ui->lineEdit_plcDB->text());
        //            Step.insert("plcIP",ui->lineEdit_plcIP->text());
        //        }

        int locationStatus = 0;
        if(ui->radioButton_locationMaster->isChecked())
            locationStatus = 2;
        else if(ui->radioButton_locationSlave->isChecked())
            locationStatus = 1;
        else if(ui->radioButton_locationOff->isChecked())
            locationStatus = 0;
        LocationMap.insert("anchors",anchorList);
        LocationMap.insert("datas",dataList);
        LocationMap.insert("leastNum",ui->lineEdit_leastNum->text());
		LocationMap.insert("tag_timeout",ui->lineEdit_tag_timeout->text());//add lw 2017/9/19
        LocationMap.insert("locationStatus",locationStatus);
        LocationMap.insert("offsetList",offsetList);

        QJson::Serializer serializer;
        bool ok;
        QByteArray json = serializer.serialize(LocationMap, &ok);

        if (ok)
        {
            DTdebug() << json;
            QFile locationJson("/location.json");
            if(locationJson.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                QTextStream text_stream(&locationJson);
                text_stream << json << endl;
                locationJson.flush();
                locationJson.close();
                currentLocation = LocationMap;
            }
            else
                DTdebug()<<"locationJson.json write open fail";
        }
        else
            qCritical() << "Something went wrong:" << serializer.errorMessage();
    }
    else
    {
        if(whichButtonClick != "LocationSave")
        {
            readLocation(currentLocation);
            if(currentLocation["locationStatus"].toInt()==2 && currentLocation["leastNum"].toString().isEmpty())
            {
                delete e3;
                ui->label_100->hide();
                delete save;
                ui->label_locationWrong->show();
                islocationchange = false;
                return;
            }
        }
    }

    //    if(whichSave == "return")
    //    {
    //        ui->label_warning->hide();
    //        this->close();
    //    }
    delete e3;
    ui->label_100->hide();
    delete save;
    if (whichButtonClick == "advancedback")
    {
        on_pushButton_3_clicked();
    }
    else if (whichButtonClick == "passwordchange")
    {
        on_pushButton_100_clicked();
    }
    else if (whichButtonClick == "testinterface")
    {
        on_pushButton_66_clicked();
    }
    else if (whichButtonClick == "fisupdatecolumn")
    {
        on_pushButton_67_clicked();
    }
    else if (whichButtonClick == "advanceset")
    {
        on_pushButton_58_clicked();
    }
    else if (whichButtonClick == "master_slave")
    {
        on_pushButton_86_clicked();
    }
    else if (whichButtonClick == "bound_config")
    {
        on_pushButton_55_clicked();
    }
    islocationchange = false;
}

void Newconfiginfo::locationIsChange()
{
    QVariantMap LocationMap;
    int locationStatus = 0;
    if(ui->radioButton_locationMaster->isChecked())
        locationStatus = 2;
    else if(ui->radioButton_locationSlave->isChecked())
        locationStatus = 1;
    else if(ui->radioButton_locationOff->isChecked())
        locationStatus = 0;
    LocationMap.insert("anchors",anchorList);
    LocationMap.insert("datas",dataList);
    LocationMap.insert("leastNum",ui->lineEdit_leastNum->text());
	LocationMap.insert("tag_timeout",ui->lineEdit_tag_timeout->text());//add lw 2017/9/19
    LocationMap.insert("locationStatus",locationStatus);
    LocationMap.insert("offsetList",offsetList);
    if(currentLocation != LocationMap)
    {
        newSaveLocation();
        islocationchange = true;
    }
    else
    {
        //        ui->label_warning->hide();
        //        this->close();
        islocationchange = true;
        if (whichButtonClick == "advancedback")
        {
            on_pushButton_3_clicked();
        }
        else if (whichButtonClick == "passwordchange")
        {
            on_pushButton_100_clicked();
        }
        else if (whichButtonClick == "testinterface")
        {
            on_pushButton_66_clicked();
        }
        else if (whichButtonClick == "fisupdatecolumn")
        {
            on_pushButton_67_clicked();
        }
        else if (whichButtonClick == "advanceset")
        {
            on_pushButton_58_clicked();
        }
        else if (whichButtonClick == "master_slave")
        {
            on_pushButton_86_clicked();
        }
        else if (whichButtonClick == "bound_config")
        {
            on_pushButton_55_clicked();
        }
        //        else if (whichButtonClick == "wirelessLocation")
        //        {
        //            on_pushButton_wirelessLocation_clicked();
        //        }
    }
}

void Newconfiginfo::on_pushButton_dataConfig_append_clicked()
{
    newLocationConfig();
    dataConfigInsertRow = ui->tableWidget_dataConfig->rowCount();
    if(dataConfigInsertRow == -1)
        locationConfig->initIp();
    currentDataConfigOperate = dataConfigAdd;
}

void Newconfiginfo::on_pushButton_dataConfig_insert_clicked()
{
    if(ui->tableWidget_dataConfig->currentRow()!= -1 && ui->tableWidget_dataConfig->currentRow()!= 0)
    {
        newLocationConfig();
        dataConfigInsertRow = ui->tableWidget_dataConfig->currentRow();
        currentDataConfigOperate = dataConfigInsert;
    }
}

void Newconfiginfo::on_pushButton_dataConfig_update_clicked()
{
    if(ui->tableWidget_dataConfig->currentRow()!= -1)
    {
        newLocationConfig();
        dataConfigInsertRow = ui->tableWidget_dataConfig->currentRow();
        locationConfig->initUI(dataList[dataConfigInsertRow].toMap());
        if(dataConfigInsertRow == 0)
            locationConfig->initIp();
        currentDataConfigOperate = dataConfigUpdate;
    }
}

void Newconfiginfo::on_pushButton_dataConfig_remove_clicked()
{
    int rowIndex = ui->tableWidget_dataConfig->currentRow();
    if (rowIndex != -1 && rowIndex != 0)
    {
        ui->tableWidget_dataConfig->removeRow(rowIndex);
        dataList.removeAt(rowIndex);
    }
    updateDataConfigItem0();
}

void Newconfiginfo::on_radioButton_locationMaster_clicked()
{
    ui->stackedWidget_location->show();
    ui->label_leastNum->show();
    ui->lineEdit_leastNum->show();
 	ui->lineEdit_tag_timeout->show();
    ui->label_timeout->show();
    ui->label_timeouts->show();
	 ui->pushButton_Calibration->show();
    ui->label_Calibration->show();
}

void Newconfiginfo::on_radioButton_locationSlave_clicked()
{
    ui->stackedWidget_location->hide();
    ui->label_locationWrong->hide();
    ui->label_leastNum->hide();
    ui->lineEdit_leastNum->hide();
    ui->lineEdit_tag_timeout->hide();
    ui->label_timeout->hide();
    ui->label_timeouts->hide();
 	ui->pushButton_Calibration->hide();
    ui->label_Calibration->hide();
}

void Newconfiginfo::on_radioButton_locationOff_clicked()
{
    ui->stackedWidget_location->hide();
    ui->label_locationWrong->hide();
    ui->label_leastNum->hide();
    ui->lineEdit_leastNum->hide();
	ui->lineEdit_tag_timeout->hide();
    ui->label_timeout->hide();
    ui->label_timeouts->hide();
	ui->pushButton_Calibration->hide();
    ui->label_Calibration->hide();
}
void Newconfiginfo::on_pushButton_Calibration_clicked()
{
    //标定基站
    SendCalibration();
    ui->label_Calibration->setText("标定中");
}

void Newconfiginfo::Cell_Calibration_status(int calibrationstatus)
{
    int status = calibrationstatus;
    switch(status)
    {
    case 0:
        ui->label_Calibration->setText("未标定");
        break;
    case 1:
        ui->label_Calibration->setText("已标定");
        break;
    case 2:
        ui->label_Calibration->setText("标定成功");
        break;
    case 3:
        ui->label_Calibration->setText("标定失败");
        break;
    case 4:
        ui->label_Calibration->setText("基站未连接");
        break;
    default:
        break;
    }
}

void Newconfiginfo::Receiveoffsetlist(QVariantList Inoffsetlist)
{
    offsetList.clear();
    offsetList = Inoffsetlist;
}


void Newconfiginfo::on_targetNumRadio_clicked()
{
    partY1V = 1 ;
}

void Newconfiginfo::on_maxNumRadio_clicked()
{
    partY1V = 2 ;
}

void Newconfiginfo::on_editQRCodeRule1_textChanged(const QString &arg1)
{
    isedit = 1 ;
}


//to do

void Newconfiginfo::on_editQRCodeRule2_textChanged(const QString &arg1)
{
    isedit = 1 ;
}


void Newconfiginfo::on_editQRCodeRule3_textChanged(const QString &arg1)
{
    isedit = 1 ;
}

void Newconfiginfo::on_editQRCodeRule4_textChanged(const QString &arg1)
{

}

void Newconfiginfo::on_editQRCodeRule5_cursorPositionChanged(int arg1, int arg2)
{
    isedit = 1 ;
}

void Newconfiginfo::on_btnIOOut5_clicked()
{
    system("echo 1 > /root/gpio/OUT5");
}

void Newconfiginfo::on_btnIOOut6_clicked()
{
    system("echo 1 > /root/gpio/OUT6");
}

void Newconfiginfo::on_btnIOOut7_clicked()
{
    system("echo 1 > /root/gpio/OUT7");
}

void Newconfiginfo::on_btnSerial1_clicked()
{
    system("echo 1 > /root/gpio/COM0");
}

void Newconfiginfo::on_btnSerial2_clicked()
{
    system("echo 1 > /root/gpio/COM1");
}

void Newconfiginfo::on_btn_testLight_pressed()
{
    system("echo 1 > /root/gpio/OUT1");
    system("echo 1 > /root/gpio/OUT2");
    system("echo 1 > /root/gpio/OUT3");
    system("echo 1 > /root/gpio/OUT4");

}

void Newconfiginfo::on_btn_testLight_released()
{
    system("echo 0 > /root/gpio/OUT1");
    system("echo 0 > /root/gpio/OUT2");
    system("echo 0 > /root/gpio/OUT3");
    system("echo 0 > /root/gpio/OUT4");
}


void Newconfiginfo::on_pushButton_restart_2_clicked(bool checked)
{

}



void Newconfiginfo::on_pushButton_ledSetting_clicked()
{
    ui->stackedWidget_2->setCurrentWidget(ui->page_ledsetting);
}

void Newconfiginfo::on_pushButton_ledSetting_back_clicked()
{
    ui->stackedWidget_2->setCurrentWidget(ui->page_37);
}

void Newconfiginfo::on_btn_option_left_clicked()
{
    //left
    if(optionpagenum <= 1)
        optionpagenum = 2;
    optionpagenum--;
    ui->label_option_curIndex->setText(QString::number(optionpagenum));
    OptionMoveDo();
}

void Newconfiginfo::on_btn_option_right_clicked()
{
    //right
    if(optionpagenum >= 4)    //车型页数
        optionpagenum = 3;
    optionpagenum++;
    ui->label_option_curIndex->setText(QString::number(optionpagenum));
    OptionMoveDo();
}

void Newconfiginfo::OptionMoveDo()
{
     ReinitOptionState();
}

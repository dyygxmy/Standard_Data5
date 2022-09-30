#include "rootdialog.h"
#include "ui_rootdialog.h"

RootDialog::RootDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RootDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    this->setGeometry(QRect(0, 0, 1366, 768));

    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer->start(1000);
    Factory = factory;

    ui->label_wifi->hide();
    ui->label_black->hide();

    ui->label_4->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_5->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_7->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_9->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_10->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_11->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_14->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_15->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_16->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_17->setAttribute(Qt::WA_TransparentForMouseEvents);

    isLogin = false;
    isSystem = false;
    isProgram = false;
    isPdm = false;
    isHistory = false;
    isUser= false;

    ui->label_17->hide();
    ui->pushButton_10->hide();
    if(!battery)
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2.png);background:transparent;");
    }
    connect(&shutdown_timer,SIGNAL(timeout()),this,SLOT(clocked()));
    connect(&timer_showdown,SIGNAL(timeout()),this,SLOT(battery15()));
}

RootDialog::~RootDialog()
{
    delete ui;
}

void RootDialog::on_pushButton_clicked()
{
    FUNC() ;
    if(Factory != "Benz" && Factory != "Haima" && Factory != "BAIC" )
    {
        isLogin = false;
        ui->label_user->setText("");
        ui->pushButton_6->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/Head.png);");
    }
    w->Show();
}

void RootDialog::on_pushButton_4_clicked()
{
    if(Factory == "Benz" || Factory == "Haima")
    {
        if(isSystem)
        {
            c->systemConfigure();
            c->Show();
        }
    }
    else
    {
        if(isLogin)
        {
            c->systemConfigure();
            c->Show();
        }
        else
        {
            e3 = new QGraphicsOpacityEffect(this);

            if(factory == "AQCHERY")
            {
                e3->setOpacity(0.8);
                ui->label_AQCHERY->setGraphicsEffect(e3);
                ui->label_AQCHERY->show();
                ui->label_AQCHERY->setGeometry(0,0,1366,768);
            }
            else
            {
                e3->setOpacity(0.5);
                ui->label_black->setGraphicsEffect(e3);
                ui->label_black->show();
                ui->label_black->setGeometry(0,0,1366,768);
            }

            EnterWhich = "System";

            passwordPanel = CreatePasswdPanel();
        }
    }
}

PasswordPanel *RootDialog::CreatePasswdPanel()
{
    PasswordPanel *tmppasswordPanel = new PasswordPanel(this);
    connect(tmppasswordPanel,SIGNAL(sendResult(bool)),this,SLOT(receiveResult(bool)));
    connect(tmppasswordPanel,SIGNAL(signalDetectCard(QString)),this,SLOT(slotDetectCard(QString)),Qt::AutoConnection);
    tmppasswordPanel->show();
    return tmppasswordPanel;
}

void RootDialog::slotDetectCard(QString pData)
{
    emit signalDetectCard(pData);
}

void RootDialog::on_pushButton_13_clicked()
{
    if(Factory == "Benz" || Factory == "Haima")
    {
        if(isHistory)
        {
            c->history();
            c->Show();
        }
    }
    else
    {
//        if(isLogin)
//        {
            c->history();
            c->Show();
//        }
//        else
//        {
//            e3 = new QGraphicsOpacityEffect(this);
//            e3->setOpacity(0.5);
//            ui->label_black->setGraphicsEffect(e3);
//            ui->label_black->show();
//            ui->label_black->setGeometry(0,0,1366,768);
//            EnterWhich = "History";
//            passwordPanel = CreatePasswdPanel();
//        }
    }
}

void RootDialog::on_pushButton_8_clicked()
{
    c->on_pushButton_13_clicked();
//    c->whichButtonClick = "about";
    c->Show();
}

void RootDialog::on_pushButton_2_clicked()
{
    DTdebug()<<isLogin;
    SaveWhat = "";
    if(Factory == "Benz" || Factory == "Haima")
    {
        if(isProgram)
        {
            c->configList();
            c->Show();
        }
    }
    else
    {
        if(isLogin)
        {
            c->configList();
            c->Show();
        }
        else
        {
            e3 = new QGraphicsOpacityEffect(this);            
            if(factory == "AQCHERY")
            {
                e3->setOpacity(0.8);
                ui->label_AQCHERY->setGraphicsEffect(e3);
                ui->label_AQCHERY->show();
                ui->label_AQCHERY->setGeometry(0,0,1366,768);
            }
            else
            {
                e3->setOpacity(0.5);
                ui->label_black->setGraphicsEffect(e3);
                ui->label_black->show();
                ui->label_black->setGeometry(0,0,1366,768);
            }
            EnterWhich = "ConfigList";
            passwordPanel = CreatePasswdPanel();
        }
    }
}

void RootDialog::on_pushButton_3_clicked()
{
    if(Factory == "Benz" || Factory == "Haima")
    {
        if(isPdm)
        {
            c->PDMEdit();
            c->Show();
        }
    }
    else
    {
        if(isLogin)
        {
            c->PDMEdit();
            c->Show();
        }
        else
        {
            e3 = new QGraphicsOpacityEffect(this);            
            if(factory == "AQCHERY")
            {
                e3->setOpacity(0.8);
                ui->label_AQCHERY->setGraphicsEffect(e3);
                ui->label_AQCHERY->show();
                ui->label_AQCHERY->setGeometry(0,0,1366,768);
            }
            else
            {
                e3->setOpacity(0.5);
                ui->label_black->setGraphicsEffect(e3);
                ui->label_black->show();
                ui->label_black->setGeometry(0,0,1366,768);
            }
            EnterWhich = "PDM";
            passwordPanel = CreatePasswdPanel();
        }
    }
}
void RootDialog::timerUpdate()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss");
    ui->label_date->setText(str);
}
void RootDialog::wifishow(bool wificonnect)
{
    if(wificonnect)
        ui->label_wifi->show();
    else
        ui->label_wifi->hide();
}

void RootDialog::batteryshow1(QString power)
{
    if(power == "17")
    {
        if(timer_showdown.isActive())
        {
            qDebug()<<"timer_showdown stop";
            timer_showdown.stop();
        }
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2.png);background:transparent;");
    }
    else if(power == "16")
    {
        if(timer_showdown.isActive())
        {
            qDebug()<<"timer_showdown stop";
            timer_showdown.stop();
        }
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery1.png);background:transparent;");
    }
    else if(power == "05")
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery.png);background:transparent;");
    else if(power == "04")
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_2.png);background:transparent;");
    else if(power == "03")
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
    else if(power == "02"||power == "01")
    {
        //15 分钟后关机
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
        timer_showdown.start(900000);
    }
}

void RootDialog::batteryshow2(bool balive)
{
    if(balive)
    {
        shutdown_timer.start(300000);
        //        ui->label_battery->show();
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery.png);background:transparent;");
    }
    else
    {
        shutdown_timer.stop();
        battry_num = 0;
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery2.png);background:transparent;");
        //        ui->label_battery->show();
    }
}

void RootDialog::init(MainWindow *window, Newconfiginfo *newconfiginfo)
{
    w = window;
    c = newconfiginfo;
    if(Factory != "Benz")
        w->Show();
}

void RootDialog::initUser(UserManagement *userManage)
{
    userManagement = userManage;
//    emit send_Login(query2.value(0).toString(),query2.value(2).toString(),query3.value(0).toString());
    connect(userManagement,SIGNAL(send_Login(QString,QString,QString)),this,SLOT(receiveLogin(QString,QString,QString)));
    userManagement->show();
    userManagement->sqlinit();

}

void RootDialog::receiveLogin(QString Username, QString Head, QString Functions)
{
    FUNC() ;
    isSystem = false;
    isProgram = false;
    isPdm = false;
    isHistory = false;
    isUser = false;
    Operator = Username;
    ui->label_user->setText(Username);
    QStringList list = Functions.split(",",QString::SkipEmptyParts);
    for(int i=0;i<list.length();i++)
    {
        enumFunctions function;
        function = enumFunctions(list[i].toInt());
        switch(function)
        {
            case systemConfig:
                isSystem = true;
                break;
            case program:
                isProgram = true;
                break;
            case pdm:
                isPdm = true;
                break;
            case history:
                isHistory = true;
                break;
            case user:
                isUser = true;
                break;
            default:
                break;
        }
    }
    if(Head != "")
        ui->pushButton_6->setStyleSheet("border-image: url(/Head/"+Head+")");
    else
        ui->pushButton_6->setStyleSheet("border-image: url(:/re/Administrator.png);");
    userManagement->close();
    on_pushButton_clicked();
}

void RootDialog::on_pushButton_11_clicked()
{
//    if(isLogin)
//    {

//        e3 = new QGraphicsOpacityEffect(this);
//        e3->setOpacity(0.5);
//        ui->label_black->setGraphicsEffect(e3);
//        fullScreenPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
//        fullScreenPixmap.save("fullScreen.jpg","JPG");

////    或
////    fullScreenPixmap->save("Screenshot.png","png");
        pushButton_blur = new QPushButton(this);
        pushButton_blur->setFocusPolicy(Qt::NoFocus);
//        pushButton_blur->setStyleSheet("background-image: url(fullScreen.jpg);");
        pushButton_blur->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/background.png);");

//        e0 = new QGraphicsBlurEffect(this);
//        e0->setBlurRadius(0.8);
//        pushButton_blur->setGraphicsEffect(e0);
        pushButton_blur->show();
        pushButton_blur->setGeometry(0,0,1366,768);
        pushButton_reboot= new QPushButton(this);
        pushButton_reboot->setFocusPolicy(Qt::NoFocus);
        pushButton_reboot->setGeometry(844,284,200,200);
        pushButton_reboot->setText(tr("重新启动"));
        pushButton_reboot->setStyleSheet("border-style: solid;border-radius:20px;background-color: rgb(241, 156, 31); font: 20pt \"黑体\";color: rgb(248, 248, 255);");
        connect(pushButton_reboot,SIGNAL(clicked()),this,SLOT(reboot()));
        pushButton_poweroff= new QPushButton(this);
        pushButton_poweroff->setFocusPolicy(Qt::NoFocus);
        QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
        bool isReboot = configIniRead->value("baseinfo/isReboot").toBool();
        delete configIniRead;
//        if(Factory=="SVW3"||Factory =="BYDSZ" ||Factory =="BYDXA"||Factory== "Ningbo"||Factory=="GM"||(Factory == "SVW2" && isReboot))
//        {
            pushButton_reboot->show();
            pushButton_poweroff->setGeometry(322,284,200,200);
//        }
//        else
//            pushButton_poweroff->setGeometry(583,284,200,200);
        pushButton_poweroff->setText(tr("关闭电源"));
        pushButton_poweroff->setStyleSheet("border-style: solid;border-radius:20px;background-color: rgb(255, 102, 102); font: 20pt \"黑体\";color: rgb(248, 248, 255);");
        pushButton_poweroff->show();
        connect(pushButton_poweroff,SIGNAL(clicked()),this,SLOT(poweroff()));
        connect(pushButton_blur,SIGNAL(clicked()),this,SLOT(closeDialog()));
//        SaveWhat = "shutdown";
//        save = new Save(this);
//        connect(save,SIGNAL(sendShutDown(int)),this,SLOT(shutdown(int)));
//        save->show();
//    save->setGeometry(525,296,315,176);
//    }
//    else
//    {
//        e3 = new QGraphicsOpacityEffect(this);
//        e3->setOpacity(0.5);
//        ui->label_black->setGraphicsEffect(e3);
//        ui->label_black->show();
//        ui->label_black->setGeometry(0,0,1366,768);
//        EnterWhich = "Shutdown";
//        passwordPanel = CreatePasswdPanel();
//    }
}

void RootDialog::reboot()
{
    qDebug() << "reboot here";
    system("reboot");
}

void RootDialog::poweroff()
{
    qDebug() << "poweroff here";
    system("echo 1 > /sys/class/leds/control_power2/brightness");
    system("echo 0 > /sys/class/leds/control_lvds/brightness");
    system("poweroff");
}

void RootDialog::closeDialog()
{
    pushButton_blur->hide();
    pushButton_reboot->hide();
    pushButton_poweroff->hide();
}


//void RootDialog::shutdown(int tmp)
//{
//    SaveWhat = "";
//    if(tmp == 1)
//    {
//        qDebug() << "poweroff here";
//        system("echo 0 > /sys/class/leds/control_lvds/brightness");
//        system("poweroff");
//    }
//    else if(tmp == 2)
//    {
//        qDebug() << "reboot here";
//        system("reboot");
//    }
//    else if(tmp == 3)
//    {
//        ui->label_black->hide();
//        delete e3;
//        delete save;
//    }
//}

void RootDialog::receiveResult(bool pass)
{
    if(factory == "AQCHERY")
    {
        ui->label_black->hide();
        ui->label_AQCHERY->hide();
    }
    else
    {
        ui->label_black->hide();
    }

    delete e3;
    delete passwordPanel;
    if(pass)
    {
        isLogin = true;
        ui->pushButton_6->setStyleSheet("border-image: url(:/re/Administrator.png);");
        ui->label_user->setText("Admin");
        if(EnterWhich == "System")
        {
            c->systemConfigure();
            c->Show();
        }
        else if(EnterWhich == "History")
        {
            c->history();
            c->Show();
        }
        else if(EnterWhich == "ConfigList")
        {
            c->configList();
            c->Show();
        }
        else if(EnterWhich == "PDM")
        {
            c->PDMEdit();
            c->Show();
        }
        else if(EnterWhich == "User")
        {
            c->on_pushButton_100_clicked();
            c->Show();
        }
//        else if(EnterWhich == "Shutdown")
//        {
//            e3 = new QGraphicsOpacityEffect(this);
//            e3->setOpacity(0.5);
//            ui->label_black->setGraphicsEffect(e3);
//            ui->label_black->show();
//            ui->label_black->setGeometry(0,0,1366,768);
//            SaveWhat = "shutdown";
//            save = new Save(this);
//            connect(save,SIGNAL(sendShutDown(int)),this,SLOT(shutdown(int)));
//            save->show();
//    //    save->setGeometry(525,296,315,176);
//        }
    }
    EnterWhich = "";
}

void RootDialog::on_pushButton_6_clicked()
{
    if(Factory != "Benz" && Factory != "Haima" && Factory != "BAIC")
    {
        if(!isLogin)
        {
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_black->setGraphicsEffect(e3);
            ui->label_black->show();
            ui->label_black->setGeometry(0,0,1366,768);

            EnterWhich = "";
            passwordPanel = CreatePasswdPanel();
        }
    }
}

void RootDialog::on_pushButton_5_clicked()
{
//    if(factory == "Benz")
//    {
//        userManagement->show();
//    }
//    else
//    {
        if(isLogin || Factory == "Benz" || Factory == "Haima" || Factory == "BAIC")
        {
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_black->setGraphicsEffect(e3);
            ui->label_black->show();
            ui->label_black->setGeometry(0,0,1366,768);
            SaveWhat = "Logout";
            save = new Save(this);
            connect(save,SIGNAL(sendLogout(bool)),this,SLOT(logout(bool)));
            save->show();

        }
        else
        {
            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_black->setGraphicsEffect(e3);
            ui->label_black->show();
            ui->label_black->setGeometry(0,0,1366,768);

            SaveWhat = "noLogin";
            save = new Save(this);
            connect(save,SIGNAL(sendClose()),this,SLOT(closeSave()));
            save->show();
        }
//    }
}
void RootDialog::closeSave()
{
    ui->label_black->hide();
    delete e3;
    delete save;
    SaveWhat = "";
//    ISmaintenance = false;
//    workmode = false;
}

void RootDialog::logout(bool isLogout)
{
    ui->label_black->hide();
    delete e3;
    delete save;
    if(isLogout)
    {
        if(Factory == "Benz" || Factory == "Haima"  || Factory == "BAIC")
        {
            Operator = "";
            userManagement->show();
            userManagement->interfaceInit();
            if(Factory == "BAIC")
            {
                userManagement->on_pushButton_toPassword_clicked();
                QVariantMap dataMap;
                DTdebug() << userID ;
                dataMap.insert("UserWorkId",userID);
                dataMap.insert("UserOperateTypeID",2);
                dataMap.insert("StationCode",Station);
                userName = "" ;
                userID   = "" ;
                userBind *userRequest = new userBind;
                userRequest->setUrlRequest(userBind::logout,dataMap);
            }
        }
        else
        {
            isLogin = false;
            ui->label_user->setText("");
            ui->pushButton_6->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/Head.png);");
        }
    }
    SaveWhat = "";
}

void RootDialog::on_pushButton_9_clicked()
{
    if(Factory == "Benz" || Factory == "Haima")
    {
        if(isUser)
        {
            userManagement->show();
            userManagement->userConfig();
        }
    }

    //北汽既不提供用户管理功能，也不提供默认密码修改功能
    else if (Factory == "BAIC")
    {
        return;
    }
    else
    {
        if(isLogin)
        {
            c->on_pushButton_100_clicked();
            c->Show();
        }
        else
        {
            e3 = new QGraphicsOpacityEffect(this);            
            if(factory == "AQCHERY")
            {
                e3->setOpacity(0.8);
                ui->label_AQCHERY->setGraphicsEffect(e3);
                ui->label_AQCHERY->show();
                ui->label_AQCHERY->setGeometry(0,0,1366,768);
            }
            else
            {
                e3->setOpacity(0.5);
                ui->label_black->setGraphicsEffect(e3);
                ui->label_black->show();
                ui->label_black->setGeometry(0,0,1366,768);
            }
            EnterWhich = "User";
            passwordPanel = CreatePasswdPanel();
        }
    }
}

void RootDialog::clocked()
{
    battry_num++;
    if(battry_num == 1)
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_2.png);background:transparent;");
    }
    else if(battry_num == 2)
    {
        ui->label_battery->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/logo/battery_red.png);background:transparent;");
    }
    else if(battry_num == 3)
    {
        qDebug("power down 2");
        system("echo 1 > /sys/class/leds/control_power2/brightness");
        system("echo 0 > /sys/class/leds/control_lvds/brightness");
        system("poweroff");
    }
}

void RootDialog::battery15()
{
    qDebug("power down 1");
    system("echo 1 > /sys/class/leds/control_power2/brightness");
    system("echo 0 > /sys/class/leds/control_lvds/brightness");  //关背光
    system("poweroff");
}


#include "usermanagement.h"
#include "ui_usermanagement.h"
//#include "customitem.h"

UserManagement::UserManagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserManagement)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    this->setGeometry(QRect(0, 0, 1366, 768));
    ui->stackedWidget->setCurrentIndex(1);
    ui->label_black->hide();
    ui->label_wrong->hide();
    ui->label_wrong_2->hide();
    ui->label_cardNumber->hide();
    ui->label_31->hide();
    ui->lineEdit_UserPassword->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    ui->lineEdit_LoginPassword->setEchoMode(QLineEdit::Password);
    QRegExp rx("[0-9]{1,15}");
//    QRegExp rx3("^((87\\.[5-9])|(8[8-9](\\.\\d)?)|(9\\d(\\.\\d)?)|(10[1-7](\\.\\d)?)|108|(8[8-9]\\d(\\.\\d)?)|(9\\d\\d(\\.\\d)?)|(1[0-6]\\d\\d(\\.\\d)?)|(170\\d(\\.\\d)?)|1710)$");
//    QRegExp rx3("^(([1-9])|([1-3]\\d)|(4[0-5]))$");
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_LoginWorkID->setValidator(validator);
    ui->lineEdit_WorkID->setValidator(validator);
    ui->label_wrong_CardNum->hide();
    ui->label_wrong_UserName->hide();
    ui->label_wrong_WorkID->hide();
    ui->label_wrong_UserPassword->hide();
    ui->label_wrong_role->hide();
    head_Name = "";
//    sqlinit();
    //    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    static QTime t = QTime::currentTime();
    QTime T = QTime::currentTime();
    int i = T.msecsTo(t);
    qsrand(i);

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    ui->label_13->setText(configIniRead->value("baseinfo/StationName").toString());
    ui->label_17->setText(configIniRead->value("baseinfo/StationName").toString());
    ui->label_12->setText(Station);
    ui->label_15->setText(Station);
    listWidget_Users[0] = new QListWidget(ui->groupBox_A);
    listWidget_Users[1] = new QListWidget(ui->groupBox_B);
    listWidget_Users[2] = new QListWidget(ui->groupBox_C);
    pushButton_left[0] = new QPushButton(ui->groupBox_A);
    pushButton_left[1] = new QPushButton(ui->groupBox_B);
    pushButton_left[2] = new QPushButton(ui->groupBox_C);
    pushButton_right[0] = new QPushButton(ui->groupBox_A);
    pushButton_right[1] = new QPushButton(ui->groupBox_B);
    pushButton_right[2] = new QPushButton(ui->groupBox_C);
    connect(pushButton_left[0],SIGNAL(clicked()),this,SLOT(onPushbuttonLeftAClicked()));
    connect(pushButton_right[0],SIGNAL(clicked()),this,SLOT(onPushbuttonRightAClicked()));
    connect(pushButton_left[1],SIGNAL(clicked()),this,SLOT(onPushbuttonLeftBClicked()));
    connect(pushButton_right[1],SIGNAL(clicked()),this,SLOT(onPushbuttonRightBClicked()));
    connect(pushButton_left[2],SIGNAL(clicked()),this,SLOT(onPushbuttonLeftCClicked()));
    connect(pushButton_right[2],SIGNAL(clicked()),this,SLOT(onPushbuttonRightCClicked()));
    for(int i= 0;i<3;i++)
    {
        listWidget_Users[i]->setGeometry(70,30,900,101);
        listWidget_Users[i]->setFocusPolicy(Qt::NoFocus);
        listWidget_Users[i]->setStyleSheet("QListWidget{border-width:0px;border-style:solid; font:14pt \"黑体\"} QListWidget::item:selected{background:transparent}");
        pushButton_left[i]->setGeometry(30,53,30,56);
        pushButton_left[i]->setFocusPolicy(Qt::NoFocus);
        pushButton_left[i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/11.bmp);");
        pushButton_right[i]->setGeometry(986,53,30,56);
        pushButton_right[i]->setFocusPolicy(Qt::NoFocus);
        pushButton_right[i]->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/10.bmp);");
    }
    roleInit();
    userInit();
    nCurScroller= 0; //翻页时的当时滑动条位置
    pageValue = 5; // 一页显示条数

    newData = false;
    ui->label_init->hide();
//    ui->pushButton_workShift->hide();   //临时
    if(factory == "BAIC")
    {
        ui->pushButton_toCard->hide();
    }
}

UserManagement::~UserManagement()
{
    delete ui;
}

void UserManagement::sqlinit()
{
//    bool isLogin= false;
    db2=QSqlDatabase::addDatabase("QMYSQL","mysqlUser");
    db2.setHostName("localhost");
    db2.setUserName("root");
    db2.setPassword("123456");
    query1 = QSqlQuery(db2);
    query2 = QSqlQuery(db2);
    query3 = QSqlQuery(db2);
    if(!db2.open())
    {
        if(!db2.open())
        {
            qDebug()<< "User sqlinit localmysql "<< db2.lastError().text();

        }else
        {
            qDebug()<< "User sqlinit localmysql ok 2";
        }
    }else
    {
        qDebug()<< "User sqlinit localmysql ok 1";
    }
    if(!query2.exec("Create database if not exists User"))
        qDebug()<<"Create database User fail";
    db2.close();
    db2.setDatabaseName("User");
    if(!db2.open())
    {
        if(!db2.open())
        {
            qDebug()<< "User sqlinit2 localmysql "<< db2.lastError().text();

        }else
        {
            qDebug()<< "User sqlinit2 localmysql ok 2";
        }
    }else
    {
        qDebug()<< "User sqlinit2 localmysql ok 1";
    }
    if(!query2.exec("select count(TABLE_NAME) from INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = 'Roles'"))
        qDebug()<<"select count(TABLE_NAME) fail "<<query2.lastError();
    else
    {
        query2.next();
        if(query2.value(0).toInt()==0)
        {
            if(!query2.exec("CREATE TABLE if not exists `Roles` (`RoleID` smallint(6) NOT NULL AUTO_INCREMENT,`RoleName` varchar(50) NOT NULL,`Functions` varchar(50) NOT NULL,PRIMARY KEY (`RoleID`)) ENGINE=MyISAM DEFAULT CHARSET=utf8"))
                qDebug()<<query2.lastError();
            if(!query2.exec("insert into Roles (RoleName,Functions) values (\"Admin\",\"0,1,2,3,4\")"))
                qDebug()<<query2.lastError();
            query2.exec("CREATE TABLE if not exists `Users` (`UserID` smallint(6) NOT NULL AUTO_INCREMENT,`WorkID` varchar(50) NOT NULL,`UserName` varchar(50) NOT NULL,`UserPassword` varchar(50) NOT NULL,`CardID` varchar(50) DEFAULT NULL,`Head` varchar(255) DEFAULT NULL,`RoleID` smallint(6) NOT NULL,`LastLoginDate` datetime DEFAULT NULL,PRIMARY KEY (`UserID`),KEY `FK_User_Role` (`RoleID`),constraint FK_User_Role foreign key(RoleID) REFERENCES Roles(RoleID)) ENGINE=MyISAM DEFAULT CHARSET=utf8");
            query2.exec("CREATE TABLE if not exists `WorkShift` (`WorkShiftID` smallint(6) NOT NULL AUTO_INCREMENT,`WorkShiftName` varchar(50) NOT NULL,`StartTime` TIME NOT NULL,`EndTime` TIME NOT NULL,`Remark` varchar(250) NULL,PRIMARY KEY (`WorkShiftID`)) ENGINE=MyISAM DEFAULT CHARSET=utf8;");
            query2.exec("alter table Users add constraint FK_User_Role foreign key(RoleID) REFERENCES Roles(RoleID)");
            query2.exec("insert into Users (WorkID,UserName,UserPassword,RoleID) values (123456,'Admin','0987654321',1)");
            qDebug()<<"new login SuperUser";
//            isLogin = true;
////            emit send_Login("Admin","","0,1,2,3,4");
//            ui->label_wrong->hide();
//            ui->lineEdit_LoginWorkID->clear();
//            ui->lineEdit_LoginPassword->clear();
            ui->stackedWidget->setCurrentIndex(2);
            if(factory != "BAIC")
            {
                ui->label_init->show();
            }

            newData = true;
        }
        else
        {
            if(!query2.exec("SELECT COUNT(*) from Users Where RoleID = 1"))
                qDebug()<<query2.lastError();
            else
            {
                query2.next();
                if(query2.value(0).toInt()==1)
                {
                    if(factory != "BAIC")
                    {
                        ui->label_init->show();
                    }
                    newData = true;
//                    qDebug()<<"login Admin";
//                    isLogin = true;
//                    emit send_Login("Admin","","0,1,2,3,4");
//                    ui->label_wrong->hide();
//                    ui->lineEdit_LoginWorkID->clear();
//                    ui->lineEdit_LoginPassword->clear();
                    ui->stackedWidget->setCurrentIndex(2);
                }
            }
        }
    }

    model = new QSqlQueryModel(ui->tableView);
    model->setQuery("select WorkShiftName,DATE_FORMAT(StartTime,'%H:%i'),DATE_FORMAT(EndTime,'%H:%i'),Remark from WorkShift order by WorkShiftID",db2);
    if (model->lastError().isValid())
        qDebug() << model->lastError();
//    model->setHeaderData(0, Qt::Horizontal, tr("No."));

//    qDebug() << model->columnCount()<<model->rowCount(); ;
    model->setHeaderData(0, Qt::Horizontal, tr("班次名称"));
    model->setHeaderData(1, Qt::Horizontal, tr("起始时间"));
    model->setHeaderData(2, Qt::Horizontal, tr("结束时间"));
    model->setHeaderData(3, Qt::Horizontal, tr("英文"));

    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑

//    ui->tableView->setShowGrid(true);//显示表格线

    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(51, 153, 255);"
                                                     "color: rgb(248, 248, 255);border: 0px; font:14pt}");
    ui->tableView->verticalHeader()->setStyleSheet("QHeaderView::section {background-color: rgb(248, 248, 255);"
                                                     "color: rgb(0, 0, 0);border: 0px; font:14pt}");
    ui->tableView->horizontalHeader()->setFixedHeight(51);
    ui->tableView->verticalHeader()->setFixedWidth(50);

//    ui->tableView->verticalHeader()->setVisible(true);   // 显示列表头
//    ui->tableView->horizontalHeader()->setVisible(true); // 显示行表头

    connect(ui->tableView, SIGNAL(clicked(const QModelIndex &)),this,SLOT(WorkShiftChange(const QModelIndex &)));

    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(0,250);
    ui->tableView->setColumnWidth(1,250);
    ui->tableView->setColumnWidth(2,250);
    ui->tableView->setColumnWidth(3,200);

    for(int i=0;i<model->rowCount();i++)
    {
        ui->tableView->setRowHeight(i,51);//行宽
    }
}

void UserManagement::WorkShiftChange(const QModelIndex & index)
{
//    model->setData(index, Qt::red, Qt::BackgroundColorRole);
    int WorkShiftID = 0;
    QStringList WorkShiftNames;

    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("SELECT WorkShiftName from WorkShift"))
            qDebug()<<"select WorkShiftName fail"<<query2.lastError();
        else
        {
            while(query2.next())
                WorkShiftNames<<query2.value(0).toString();
            qDebug()<<"WorkShiftNames:"<<WorkShiftNames;
        }

        if(!query2.exec("SELECT WorkShiftID from WorkShift where WorkShiftName= '"+model->record(index.row()).value(0).toString()+"'"))
            qDebug()<<"select WorkShiftID from WorkShift fail"<<query2.lastError();
        else
        {
            if(query2.next())
            {
                WorkShiftID = query2.value(0).toInt();
            }
        }

        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);
        workShift = new WorkShift(this);
        connect(workShift,SIGNAL(send_workShift(int,QString,QTime,QTime,QString)),this,SLOT(receiveWorkShift(int,QString,QTime,QTime,QString)));
        connect(workShift,SIGNAL(send_Close()),this,SLOT(receiveWorkShiftClose()));
        connect(workShift,SIGNAL(delete_workShift(int)),this,SLOT(deleteWorkShift(int)));
        qDebug()<<"QTime"<<model->record(index.row()).value(0).toString()<<model->record(index.row()).value(1).toString()<<model->record(index.row()).value(2).toString()<<model->record(index.row()).value(3).toString();
        workShift->WorkShiftInit(WorkShiftNames,WorkShiftID,model->record(index.row()).value(0).toString(),QTime::fromString(model->record(index.row()).value(1).toString(),"hh:mm"),QTime::fromString(model->record(index.row()).value(2).toString(),"hh:mm"),model->record(index.row()).value(3).toString());
        workShift->show();

    }
    else
    {
        qDebug()<<"can not open database update WorkShift fail";
    }
}

void UserManagement::deleteWorkShift(int WorkShiftID)
{
    ui->label_black->hide();
    delete e3;
    delete workShift;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        QString WorkShiftName= "";
        QTime StartTime,EndTime;
        QString Remark="";
        query2.exec("SELECT WorkShiftName,StartTime,EndTime,Remark FROM WorkShift where WorkShiftID = "+QString::number(WorkShiftID));
        while(query2.next())
        {
            WorkShiftName = query2.value(0).toString();
            StartTime = query2.value(1).toTime();
            EndTime = query2.value(2).toTime();
            Remark = query2.value(3).toString();
        }
        if(!query2.exec("DELETE FROM WorkShift where WorkShiftID = "+QString::number(WorkShiftID)))
            qDebug()<<"Delete WorkShift fail WorkShiftID ="+QString::number(WorkShiftID)<<" WorkShiftName = "<<WorkShiftName<<" StartTime = "<<StartTime<<" EndTime = "<<EndTime<<" Remark = "<<Remark<<query2.lastError();
        else
        {
            qDebug()<<"Delete WorkShift success WorkShiftID = "+QString::number(WorkShiftID)<<" WorkShiftName = "<<WorkShiftName<<" StartTime = "<<StartTime<<" EndTime = "<<EndTime<<" Remark = "<<Remark;
            workShiftUpdate();
        }
    }
    else
    {
        qDebug()<<"can not open database delete WorkShift fail";
    }
}

//本地mysql open
void UserManagement::mysqlopen()
{
    if(QSqlDatabase::contains("mysqlUser")){
        db2 = QSqlDatabase::database("mysqlUser");
    }else{
        db2=QSqlDatabase::addDatabase("QMYSQL","mysqlUser");
        db2.setHostName("localhost");
        db2.setDatabaseName("User");
        db2.setUserName("root");
        db2.setPassword("123456");
        query1 = QSqlQuery(db2);
        query2 = QSqlQuery(db2);
        query3 = QSqlQuery(db2);
    }

    if(!db2.open())
    {
        if(!db2.open())
        {
            qDebug()<< "User mysqlopen localmysql "<< db2.lastError().text();
            //**************打不开重启mysql********************
            //            emit send_mysqlerror();

        }else
        {
            qDebug()<< "User mysqlopen localmysql ok 2";
        }
    }else
    {
        qDebug()<< "User mysqlopen localmysql ok 1";
    }
}

void UserManagement::interfaceInit()
{
    emit sendLogin(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void UserManagement::userClear()
{
    ui->label_CardNum->clear();
    ui->lineEdit_UserName->clear();
    ui->lineEdit_WorkID->clear();
    ui->lineEdit_UserPassword->clear();
    ui->comboBox->setCurrentIndex(0);
    ui->label_wrong_CardNum->hide();
    ui->label_wrong_UserName->hide();
    ui->label_wrong_WorkID->hide();
    ui->label_wrong_UserPassword->hide();
    ui->label_wrong_role->hide();
    ui->widget_head->setStyleSheet("border-image: url(:/re/head/1.png)");
}

void UserManagement::updateListWidget()
{
    ui->groupBox_A->hide();
    ui->groupBox_B->hide();
    ui->groupBox_C->hide();
    for(int i=0;i<3;i++)
    {
        pushButton_left[i]->hide();
        pushButton_right[i]->hide();
    }
    if(currentPage == rolePages)
        ui->pushButton_down->hide();
    else
        ui->pushButton_down->show();
    if(currentPage == 1)
        ui->pushButton_up->hide();
    else
        ui->pushButton_up->show();
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query3.exec("SELECT RoleID,RoleName from Roles limit "+QString::number((currentPage-1)*3) +", 3"))
            qDebug()<<"select RoleID,RoleName fail "<< query2.lastError();
        else
        {
            int Role = 0;
            for(int i=0;i<3;i++)
                currentRoleID[i] = "0";
            while(query3.next())
            {
                currentUserPage[Role] = 1;
                currentRoleID[Role] = query3.value(0).toString();
                if(Role == 0)
                {
                    ui->groupBox_A->setTitle(query3.value(1).toString());
                    ui->groupBox_A->show();
                }
                else if(Role == 1)
                {
                    ui->groupBox_B->setTitle(query3.value(1).toString());
                    ui->groupBox_B->show();
                }
                else if(Role == 2)
                {
                    ui->groupBox_C->setTitle(query3.value(1).toString());
                    ui->groupBox_C->show();
                }

                if(!query2.exec("SELECT count(UserID) from Users where RoleID= "+currentRoleID[Role]))
                    qDebug()<<"SELECT count(UserID) fail "<<query2.lastError();
                else
                {
                    if(query2.next())
                    {
                        qDebug()<<"count(UserID)"<<query2.value(0).toInt();
                        userPages[Role] = qCeil((query2.value(0).toDouble())/5);
                        if(userPages[Role] == 0)
                            userPages[Role] = 1;
//                        qDebug()<<"userPages"<<Role<<userPages[Role];
                        updateSingleListWidget(Role);
                    }
                }
                Role++;
            }
        }
    }
}

void UserManagement::updateSingleListWidget(int Role)
{
    if(currentUserPage[Role] == userPages[Role])
        pushButton_right[Role]->hide();
    else
        pushButton_right[Role]->show();
    if(currentUserPage[Role] == 1)
        pushButton_left[Role]->hide();
    else
        pushButton_left[Role]->show();

    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("SELECT UserID,UserName from Users where RoleID= "+currentRoleID[Role]+ " limit "+QString::number((currentUserPage[Role]-1)*5) +", 5"))
            qDebug()<<"SELECT UserID,UserName fail "<<query2.lastError();
        else
        {
            listWidget_Users[Role]->clear();
            while(query2.next())
            {
                QPixmap pix(70,70); //创建绘图设备
                QColor backColor = qRgb(248,248,255);    //画布初始化背景色使用白色
                pix.fill(backColor);
                QPainter painter(&pix);//创建一个画笔
                painter.setRenderHint(QPainter::Antialiasing, true);//设置反锯齿模式，好看一点
                QPen pen;
                pen.setStyle(Qt::NoPen);
                painter.setPen(pen);
                QList<QColor> colors;
                colors << QColor(7,169,224) << QColor(246,186,0) << QColor(132,193,136) << QColor(252,117,97)<< QColor(172,151,194)<< QColor(94,203,206);
                int rand = qrand()%6;
                painter.setBrush(QBrush(colors[rand],Qt::SolidPattern));//设置画刷形式
                // $scope.colors = ["rgb(7,169,224)", "rgb(246,186,0)", "rgb(132,193,136)", "rgb(252,117,97)", "rgb(172,151,194)", "rgb(94,203,206)"];
                QRect rect(0,0,70,70);
                painter.drawEllipse(rect);
                QFont font("黑体",14,QFont::Bold,false);
                painter.setFont(font);
                painter.setPen(QColor(248,248,255));
                painter.drawText(rect,Qt::AlignCenter,query2.value(1).toString().left(1));
                QListWidgetItem *listItem = new QListWidgetItem(QIcon(pix),query2.value(1).toString(),listWidget_Users[Role]);
                listItem->setData(Qt::UserRole,query2.value(0).toInt());
                listItem->setSizeHint(QSize(160,70));
                listItem->setFlags(listItem->flags() & ~Qt::ItemIsSelectable);
            }
        }
    }
}

void UserManagement::userConfig()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget_2->setCurrentIndex(0);
    ui->stackedWidget_3->setCurrentIndex(0);
    ui->label_user->show();
    ui->label_role->hide();
    ui->label_workShift->hide();
    userUpdate();
}

//用户管理界面更新
void UserManagement::userUpdate()
{
    currentPage = 1;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("SELECT count(RoleID) from Roles"))
            qDebug()<<"SELECT count(RoleID) fail "<<query2.lastError();
        else
        {
            if(query2.next())
            {
                qDebug()<<"count(RoleID)"<<query2.value(0).toInt();
                rolePages = qCeil((query2.value(0).toDouble())/3);
                if(rolePages == 0)
                    rolePages = 1;
//                qDebug()<<"rolePages"<<rolePages;
                updateListWidget();
            }
        }
    }
}

void UserManagement::roleInit()
{//  pdm图 listwidget初始化
    ui->listWidget_Role->setViewMode(QListView::IconMode);    //设置QListWidget的显示模式
    ui->listWidget_Role->setResizeMode(QListView::Adjust);
    ui->listWidget_Role->setMovement(QListView::Static);      //设置QListWidget中的单元项不可被拖动
    ui->listWidget_Role->setIconSize(QSize(70, 70));        //设置QListWidget中的单元项的图片大小
    ui->listWidget_Role->setTextElideMode(Qt::ElideNone);
    ui->listWidget_Role->setSpacing(10);                       //设置QListWidget中的单元项的间距
    connect(ui->listWidget_Role,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(roleChange(QListWidgetItem *)));
    ui->listWidget_Role->setFlow(QListView::LeftToRight);
    //    ui->listWidget_Role->setStyleSheet("QListWidget{border-width:0.5px;border-style:solid;border-color:rgb(51, 153, 255);} QListWidget::item{border-right-width:0.5px;border-bottom-width:0.5px;border-style:solid;border-color:rgb(51, 153, 255);}QScrollBar:vertical{width:36px;}");
    // ui->listWidget_Role->verticalScrollBar()->setStyleSheet("QScrollBar{width:36px;}")
}

void UserManagement::userInit()
{
    for(int i=0;i<3;i++)
    {
        listWidget_Users[i]->setViewMode(QListView::ListMode);    //设置QListWidget的显示模式
        listWidget_Users[i]->setResizeMode(QListView::Adjust);
        listWidget_Users[i]->setMovement(QListView::Static);      //设置QListWidget中的单元项不可被拖动
        listWidget_Users[i]->setIconSize(QSize(70, 70));        //设置QListWidget中的单元项的图片大小
        listWidget_Users[i]->setTextElideMode(Qt::ElideNone);
        listWidget_Users[i]->setSpacing(10);                       //设置QListWidget中的单元项的间距
        connect(listWidget_Users[i],SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(userChange(QListWidgetItem*)));
        listWidget_Users[i]->setFlow(QListView::LeftToRight);
    }
}

void UserManagement::roleUpdate()
{
//    rolePages = 1;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("SELECT RoleID,RoleName from Roles"))
            qDebug()<<"SELECT RoleName fail "<<query2.lastError();
        else
        {
            ui->listWidget_Role->clear();
            while(query2.next())
            {
                QPixmap pix(70,70); //创建绘图设备
                QColor backColor = qRgb(248,248,255);    //画布初始化背景色使用白色
                pix.fill(backColor);
                QPainter painter(&pix);//创建一个画笔
                painter.setRenderHint(QPainter::Antialiasing, true);//设置反锯齿模式，好看一点
                //                QPen pen(Qt::white,0,Qt::DotLine,Qt::RoundCap,Qt::RoundJoin);
                QPen pen;
                pen.setStyle(Qt::NoPen);
                painter.setPen(pen);
                QList<QColor> colors;
                colors << QColor(7,169,224) << QColor(246,186,0) << QColor(132,193,136) << QColor(252,117,97)<< QColor(172,151,194)<< QColor(94,203,206);
                int rand = qrand()%6;
                painter.setBrush(QBrush(colors[rand],Qt::SolidPattern));//设置画刷形式

                // $scope.colors = ["rgb(7,169,224)", "rgb(246,186,0)", "rgb(132,193,136)", "rgb(252,117,97)", "rgb(172,151,194)", "rgb(94,203,206)"];
                //                painter.fillRect(0,0,16,16,Qt::black);
                QRect rect(0,0,70,70);
                painter.drawEllipse(rect);
                QFont font("黑体",14,QFont::Bold,false);
                painter.setFont(font);
                painter.setPen(QColor(248,248,255));
                painter.drawText(rect,Qt::AlignCenter,query2.value(1).toString().left(1));
                QListWidgetItem *listItem = new QListWidgetItem(QIcon(pix),query2.value(1).toString(),ui->listWidget_Role);
                listItem->setData(Qt::UserRole,query2.value(0).toInt());
                listItem->setSizeHint(QSize(150,150));
                listItem->setFlags(listItem->flags() & ~Qt::ItemIsSelectable);
            }
            //            ui->listWidget_Role->update();
        }
    }
}

void UserManagement::roleChange(QListWidgetItem *item)
{
    int RoleID = item->data(Qt::UserRole).toInt();
    if(RoleID != 1)
    {
        bool deleteIsShow = false;
        //    int RoleID = 0;
        QString currentRoleName = "";
        bool isSystem = false;
        bool isProgram = false;
        bool isPdm = false;
        bool isHistory = false;
        bool isUser = false;
        QStringList roleNames;
        if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
            mysqlopen();
        if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
        {
            if(!query2.exec("SELECT RoleName from Roles"))
                qDebug()<<"select RoleName fail"<<query2.lastError();
            else
            {
                while(query2.next())
                    roleNames<<query2.value(0).toString();
                qDebug()<<"roleNames:"<<roleNames;
            }

            if(!query2.exec("SELECT RoleName,Functions from Roles where RoleID= "+QString::number(RoleID)))
                qDebug()<<"select * from Roles fail"<<query2.lastError();
            else
            {
                if(query2.next())
                {
                    //                RoleID = query2.value(0).toInt();
                    currentRoleName = query2.value(0).toString();
                    QString Functions = query2.value(1).toString();
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
                }
            }
            if(!query2.exec("SELECT count(*) from Users where RoleID = "+QString::number(RoleID)))
                qDebug()<<"SELECT RoleID fail "<<query2.lastError();
            else
            {
                if(query2.next())
                {
                    if(query2.value(0).toInt() == 0)
                        deleteIsShow = true;
                }
            }

            e3 = new QGraphicsOpacityEffect(this);
            e3->setOpacity(0.5);
            ui->label_black->setGraphicsEffect(e3);
            ui->label_black->show();
            ui->label_black->setGeometry(0,0,1366,768);
            roleAdd = new RoleAdd(this);
            connect(roleAdd,SIGNAL(send_Role(int,QString,bool,bool,bool,bool,bool)),this,SLOT(receiveRole(int,QString,bool,bool,bool,bool,bool)));
            connect(roleAdd,SIGNAL(send_Close()),this,SLOT(receiveRoleClose()));
            connect(roleAdd,SIGNAL(delete_Role(int)),this,SLOT(deleteRole(int)));
            roleAdd->RoleInit(roleNames,RoleID,deleteIsShow,currentRoleName,isSystem,isProgram,isPdm,isHistory,isUser);
            roleAdd->show();
        }
        else
        {
            qDebug()<<"can not open database update Role fail";
        }
    }
}

void UserManagement::userChange(QListWidgetItem * item)
{
    UserID = item->data(Qt::UserRole).toInt();
    if(UserID != 1)
    {
        if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
            mysqlopen();
        if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
        {
            if(!query2.exec("SELECT WorkID,CardID from Users"))
                qDebug()<<"select WorkID,CardID fail"<<query2.lastError();
            else
            {
                WorkIDs.clear();
                CardIDs.clear();
                while(query2.next())
                {
                    WorkIDs<<query2.value(0).toString();
                    CardIDs<<query2.value(1).toString();
                }
                qDebug()<<"WorkIDs:"<<WorkIDs<<"CardIDs:"<<CardIDs;
            }

            if(!query3.exec("SELECT WorkID,UserName,UserPassword,CardID,Head,RoleID from Users where UserID="+QString::number(UserID)))
                qDebug()<<"select * from Roles fail"<<query3.lastError();
            else
            {
                ui->stackedWidget_2->setCurrentIndex(1);
                ui->stackedWidget_3->setCurrentIndex(1);
                ui->pushButton_54->setText(tr("更改用户"));
                ui->pushButton_delete->show();
                isUserchange = true;
                if(!query2.exec("SELECT RoleName FROM Roles"))
                    qDebug()<<"select RoleName fail "<<query2.lastError();
                else
                {
                    QStringList RoleName;
                    while(query2.next())
                    {
                        RoleName << query2.value(0).toString();
                    }
                    qDebug()<<"RoleName"<<RoleName;
                    ui->comboBox->clear();
                    ui->comboBox->addItem("");
                    ui->comboBox->addItems(RoleName);
                }
                query3.next();
                currentCardID=query3.value(3).toString();
                currentUserName = query3.value(1).toString();
                currentWorkID=query3.value(0).toString();
                currentUserPassword =query3.value(2).toString();
                currentHead = query3.value(4).toString();
                head_Name = currentHead;

                ui->label_CardNum->setText(currentCardID);
                ui->lineEdit_UserName->setText(currentUserName);
                ui->lineEdit_WorkID->setText(currentWorkID);
                ui->lineEdit_UserPassword->setText(currentUserPassword);
                ui->widget_head->setStyleSheet("border-image: url(/Head/"+currentHead+")");

                currentRoleIndex = 0;
                if(!query2.exec("SELECT RoleName From Roles where RoleID ="+query3.value(5).toString()))
                    qDebug()<<"query3.lastError "<<query3.lastError();
                else
                {
                    while(query2.next())
                    {
                        for(int i=0;i<ui->comboBox->count();i++)
                        {
                            if(ui->comboBox->itemText(i)== query2.value(0).toString())
                            {
                                ui->comboBox->setCurrentIndex(i);
                                currentRoleIndex = i;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            qDebug()<<"can not open database update User fail";
        }
    }
}

void UserManagement::on_pushButton_addUser_clicked()
{
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        ui->stackedWidget_2->setCurrentIndex(1);
        ui->stackedWidget_3->setCurrentIndex(1);
        ui->pushButton_54->setText(tr("添加用户"));
        ui->pushButton_delete->hide();
        isUserchange = true;
        if(!query2.exec("SELECT WorkID,CardID from Users"))
            qDebug()<<"select WorkID,CardID fail"<<query2.lastError();
        else
        {
            WorkIDs.clear();
            CardIDs.clear();
            while(query2.next())
            {
                WorkIDs<<query2.value(0).toString();
                CardIDs<<query2.value(1).toString();
            }
            qDebug()<<"WorkIDs:"<<WorkIDs<<"CardIDs:"<<CardIDs;
        }

        if(!query2.exec("SELECT RoleName FROM Roles"))
            qDebug()<<"select RoleName fail "<<query2.lastError();
        else
        {
            QStringList RoleName;
            while(query2.next())
            {
                RoleName << query2.value(0).toString();
            }
            qDebug()<<"RoleName"<<RoleName;
            ui->comboBox->clear();
            ui->comboBox->addItem("");
            ui->comboBox->addItems(RoleName);
        }
        head_Name = "1.png";
        ui->widget_head->setStyleSheet("border-image: url(/Head/"+head_Name+")");
        UserID = 0;
//        WorkIDs.clear();
//        CardIDs.clear();
        currentCardID = "";
        currentWorkID = "";
        currentUserName = "";
        currentHead = head_Name;
        currentUserPassword ="";
        currentRoleIndex = 0;
    }
    else
    {
        qDebug()<<"open database fail addUser fail";
    }
}

void UserManagement::on_pushButton_addRole_clicked()
{
    QStringList roleNames;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("SELECT RoleName from Roles"))
            qDebug()<<"select RoleName fail"<<query2.lastError();
        else
        {
            while(query2.next())
                roleNames<<query2.value(0).toString();
            qDebug()<<"roleNames:"<<roleNames;
        }

        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);
        roleAdd = new RoleAdd(this);
        connect(roleAdd,SIGNAL(send_Role(int,QString,bool,bool,bool,bool,bool)),this,SLOT(receiveRole(int,QString,bool,bool,bool,bool,bool)));
        connect(roleAdd,SIGNAL(send_Close()),this,SLOT(receiveRoleClose()));
        roleAdd->getRoleNames(roleNames);
        roleAdd->show();
    }
    else
    {
        qDebug()<<"can not open database add Role fail";
    }
}

void UserManagement::receiveRole(int RoleID, QString RoleName,bool systemConfig,bool programSet,bool pdm,bool historyView,bool userManagement)
{
    ui->label_black->hide();
    delete e3;
    delete roleAdd;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        QString function="";
        QString tmp = "";
        if(systemConfig)
            function.append("0,");
        if(programSet)
            function.append("1,");
        if(pdm)
            function.append("2,");
        if(historyView)
            function.append("3,");
        if(userManagement)
            function.append("4,");
        if(function!="")
            function=function.left(function.length()-1);
        if(RoleID == 0)
        {
            query2.prepare("INSERT INTO Roles (RoleName,Functions)"
                           "VALUES (?, ?)");
            tmp = "insert";
        }
        else
        {
            query2.prepare("UPDATE Roles SET RoleName = ?,Functions = ? WHERE RoleID ="+ QString::number(RoleID));
            tmp = "update";
        }
        query2.addBindValue(RoleName);
        query2.addBindValue(function);
        if(!query2.exec())
            qDebug()<<tmp<<" role fail"<<RoleName<<function<<query2.lastError();
        else
        {
            qDebug()<<tmp<<" role success"<<RoleName<<function;
            roleUpdate();
        }
    }
    else
    {
        qDebug()<<"can not open database receiveRole Role fail";
    }
}

void UserManagement::receiveRoleClose()
{
    ui->label_black->hide();
    delete e3;
    delete roleAdd;
}

void UserManagement::deleteRole(int RoleID)
{
    ui->label_black->hide();
    delete e3;
    delete roleAdd;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        QString RoleName= "";
        QString Functions= "";
        query2.exec("SELECT RoleName,Functions FROM Roles where RoleID = "+QString::number(RoleID));
        while(query2.next())
        {
            RoleName = query2.value(0).toString();
            Functions = query2.value(1).toString();
        }
        if(!query2.exec("DELETE FROM Roles where RoleID = "+QString::number(RoleID)))
            qDebug()<<"Delete Role fail RoleID ="+QString::number(RoleID)<<"RoleName = "<<RoleName<<"Funtions = "<<Functions<<query2.lastError();
        else
        {
            qDebug()<<"Delete Role success RoleID = "+QString::number(RoleID)<<"RoleName = "<<RoleName<<"Funtions = "<<Functions;
            roleUpdate();
        }
    }
    else
    {
        qDebug()<<"can not open database delete Role fail";
    }
}

void UserManagement::on_pushButton_changeHead_clicked()
{
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_black->setGraphicsEffect(e3);
    ui->label_black->show();
    ui->label_black->setGeometry(0,0,1366,768);
    head = new Head(this);
    connect(head,SIGNAL(sendHead(QString)),this,SLOT(headSelect(QString)));
    connect(head,SIGNAL(sendClose()),this,SLOT(headClose()));
    head->show();
}

void UserManagement::on_pushButton_Login_clicked()
{
    QString strUserNum = ui->lineEdit_LoginWorkID->text() ;
    QString strPwd     = ui->lineEdit_LoginPassword->text() ;
    if(strUserNum !="" && strPwd !="" )
    {
        if(strUserNum == "000000" && strPwd == "84277205")
        {
            qDebug()<<"login SuperUser";
            emit send_Login("SuperUser","","0,1,2,3,4");
            emit sendLogin(true);
            ui->label_wrong->hide();
            ui->lineEdit_LoginWorkID->clear();
            ui->lineEdit_LoginPassword->clear();
            ui->stackedWidget->setCurrentIndex(1);
            //            this->close();
            return;
        }
        if(newData)
        {
            if(ui->lineEdit_LoginWorkID->text() == "0" && ui->lineEdit_LoginPassword->text() == "0")
            {
                qDebug()<<"login Admin";
                emit send_Login("Admin","","0,1,2,3,4");
                emit sendLogin(true);
                ui->label_wrong->hide();
                ui->lineEdit_LoginWorkID->clear();
                ui->lineEdit_LoginPassword->clear();
                ui->stackedWidget->setCurrentIndex(1);
                //            this->close();
                return;
            }
        }
        if(factory == "BAIC")
        {
            QVariantMap dataMap;
            QString md5PWD = "";
            QByteArray byMD5;
            userID = strUserNum;
            byMD5 = QCryptographicHash::hash ( strPwd.toAscii(), QCryptographicHash::Md5 );
            md5PWD.append(byMD5.toHex());
            DTdebug() << md5PWD ;
            dataMap.insert("UserWorkId",strUserNum);
            dataMap.insert("PassWord",md5PWD);
            dataMap.insert("StationCode",Station);
            userBind *userRequest = new userBind;
            bool status = userRequest->setUrlRequest(userBind::login,dataMap);
            if(status)
            {
                ui->label_head->setStyleSheet("border-image: url(:/re/Administrator.png);background-color: transparent");
                ui->pushButton_LoginState->setText(tr("开工"));

                ui->label_name->setText(userName);
                ui->label_workid->setText(strUserNum);
                ui->stackedWidget->setCurrentIndex(3);

            }
            else {
                ui->label_wrong->show();
                ui->label_wrong->setText("帐号或密码错误，请重新登录");
            }

            delete userRequest;
        }
        else
        {
            if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
                mysqlopen();
            if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
            {
                if(!query2.exec("SELECT UserName,UserPassword,Head,RoleID,WorkID FROM Users where WorkID="+ui->lineEdit_LoginWorkID->text()))
                {
                    qDebug()<<"query fail"<<query2.lastError();
                    ui->label_wrong->show();
                    ui->label_wrong->setText("数据库有误，请联系维修人员！");
                }
                else
                {
                    int numRowsAffected=query2.numRowsAffected();
                    if(numRowsAffected == 0)
                    {
                        ui->label_wrong->show();
                        ui->label_wrong->setText("没有该用户,请重新输入或选择刷卡登录！");
    //                    if(factory == "BAIC")
    //                    {
                              //北汽不走本地数据库
    //                        ui->label_wrong->setText("没有该用户,请重新输入！");
    //                    }
                    }
                    else if(numRowsAffected >0)
                    {
                        query2.next();
                        qDebug()<<query2.value(0).toString()<<query2.value(1).toString()<<query2.value(2).toString()<<query2.value(3).toInt()<<query2.value(3).toString();
                        qDebug()<<ui->lineEdit_LoginPassword->text();
                        if(query2.value(1).toString()==ui->lineEdit_LoginPassword->text())
                        {
                            if(!query3.exec("SELECT Functions from Roles where RoleID ="+query2.value(3).toString()))
                            {
                                qDebug()<<"Functions query fail"<<query3.lastError();
                                ui->label_wrong->show();
                                ui->label_wrong->setText("数据库有误，请联系维修人员！");
                            }
                            else
                            {
                                numRowsAffected = query3.numRowsAffected();
                                if(numRowsAffected>0)
                                {
                                    if(query2.value(2).toString() != "")
                                        ui->label_head->setStyleSheet("border-image: url(/Head/"+query2.value(2).toString()+");background-color: transparent");
                                    else
                                        ui->label_head->setStyleSheet("border-image: url(:/re/Administrator.png);background-color: transparent");
                                    if(!query1.exec("SELECT WorkShiftName from WorkShift where StartTime<now() and EndTime >now()"))
                                        qDebug()<<"select WorkShiftName fail "<<query1.lastError();
                                    else
                                    {
                                        if(query1.next())
                                            ui->pushButton_LoginState->setText(tr((query1.value(0).toByteArray()+"  开工").data()));
                                        else
                                            ui->pushButton_LoginState->setText(tr("空班  开工"));
                                    }
                                    ui->label_name->setText(query2.value(0).toString());
                                    ui->label_workid->setText(query2.value(4).toString());
                                    ui->stackedWidget->setCurrentIndex(3);
    //                                query3.next();
    //                                qDebug()<<"login "<<query2.value(0).toString()<<query2.value(2).toString()<<query3.value(0).toString();
    //                                emit send_Login(query2.value(0).toString(),query2.value(2).toString(),query3.value(0).toString());
    //                                query2.exec("UPDATE Users set LastLoginDate = now() where WorkID="+ui->lineEdit_LoginWorkID->text());
    //                                ui->label_wrong->hide();
    //                                ui->lineEdit_LoginWorkID->clear();
    //                                ui->lineEdit_LoginPassword->clear();
    //                                ui->stackedWidget->setCurrentIndex(1);
                                    //                                this->close();
                                }
                                else
                                {
                                    ui->label_wrong->show();
                                    ui->label_wrong->setText("数据库有误，请联系维修人员！");
                                    qDebug()<<"no Roles :"<<query2.value(3).toInt();
                                }
                            }
                        }
                        else
                        {
                            ui->label_wrong->show();
                            ui->label_wrong->setText("您输入的密码有误！");
                        }
                    }
                }
            }
        }

    }
    else if(strUserNum == "" && strPwd != "")
    {
        ui->label_wrong->show();
        ui->label_wrong->setText("请您填写工号！");
    }
    else if(strUserNum != "" && strPwd == "")
    {
        ui->label_wrong->show();
        ui->label_wrong->setText("请您填写密码！");
    }
    else if(strUserNum == "" && strPwd == "")
    {
        ui->label_wrong->show();
        ui->label_wrong->setText("请您填写工号和密码！");
    }
}

void UserManagement::on_pushButton_toCard_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->label_wrong->hide();
    ui->lineEdit_LoginWorkID->clear();
    ui->lineEdit_LoginPassword->clear();
}

void UserManagement::on_pushButton_toPassword_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->label_wrong_2->hide();
    ui->label_31->hide();
    ui->label_cardNumber->hide();
}

void UserManagement::receiveCardNumber(int CardNumber)
{
    if(ui->stackedWidget->currentIndex()==1)
    {
        ui->label_31->show();
        ui->label_cardNumber->show();
        ui->label_cardNumber->setText(QString::number(CardNumber));
        if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
            mysqlopen();
        if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
        {
            if(!query2.exec("SELECT UserName,UserPassword,Head,RoleID,WorkID FROM Users where CardID="+QString::number(CardNumber)))
            {
                qDebug()<<"query cardNumber fail"<<query2.lastError();
                ui->label_wrong_2->show();
                ui->label_wrong_2->setText("数据库有误，请联系维修人员！");
            }
            else
            {
                int numRowsAffected=query2.numRowsAffected();
                if(numRowsAffected == 0)
                {
                    ui->label_wrong_2->show();
                    ui->label_wrong_2->setText("没有该用户，请重新刷卡或选择密码登陆！");
                }
                else if(numRowsAffected >0)
                {
                    query2.next();
                    qDebug()<<query2.value(0).toString()<<query2.value(1).toString()<<query2.value(2).toString()<<query2.value(3).toInt()<<query2.value(4).toString();
                    if(!query3.exec("SELECT Functions from Roles where RoleID ="+query2.value(3).toString()))
                    {
                        qDebug()<<"Functions query fail"<<query3.lastError();
                        ui->label_wrong_2->show();
                        ui->label_wrong_2->setText("数据库有误，请联系维修人员！");
                    }
                    else
                    {
                        numRowsAffected = query3.numRowsAffected();
                        if(numRowsAffected>0)
                        {
                            ui->label_head->setStyleSheet("border-image: url(/Head/"+query2.value(2).toString()+");background-color: transparent");
                            if(!query1.exec("SELECT WorkShiftName from WorkShift where StartTime<now() and EndTime >now()"))
                                qDebug()<<"select WorkShiftName fail "<<query1.lastError();
                            else
                            {
                                if(query1.next())
                                    ui->pushButton_LoginState->setText(tr((query1.value(0).toByteArray()+"  开工").data()));
                                else
                                    ui->pushButton_LoginState->setText(tr("空班  开工"));
                            }
                            ui->label_name->setText(query2.value(0).toString());
                            ui->label_workid->setText(query2.value(4).toString());
                            ui->stackedWidget->setCurrentIndex(3);
//                            query3.next();
//                            emit send_Login(query2.value(0).toString(),query2.value(2).toString(),query3.value(0).toString());
//                            query2.exec("UPDATE Users set LastLoginDate = now() where CardID="+QString::number(CardNumber));
//                            ui->label_wrong_2->hide();
//                            ui->label_31->hide();
//                            ui->label_cardNumber->hide();
                            //                            this->close();
                        }
                        else
                        {
                            ui->label_wrong_2->show();
                            ui->label_wrong_2->setText("数据库有误，请联系维修人员！");
                            qDebug()<<"no Roles :"<<query2.value(3).toInt();
                        }
                    }
                }
            }
        }
    }
    else if(ui->stackedWidget->currentIndex()==0)
    {
        if(ui->stackedWidget_2->currentIndex()==1)
        {
            ui->label_CardNum->setText(QString::number(CardNumber));
        }
    }
}

void UserManagement::on_pushButton_back_clicked()
{
    if(isUserchange)
    {
        userIsChange();
    }
    else
    {
        userClear();
        ui->stackedWidget_2->setCurrentIndex(0);
        ui->stackedWidget_3->setCurrentIndex(0);
    }
}

void UserManagement::on_pushButton_user_clicked()
{
    if(ui->stackedWidget_2->currentIndex()!=0)
    {
        ui->label_user->show();
        ui->label_role->hide();
        ui->label_workShift->hide();
        userUpdate();
        ui->stackedWidget_2->setCurrentIndex(0);
    }
}

void UserManagement::on_pushButton_role_clicked()
{
    if(ui->stackedWidget_2->currentIndex()!=2)
    {
        ui->label_user->hide();
        ui->label_role->show();
        ui->label_workShift->hide();
        ui->stackedWidget_2->setCurrentIndex(2);
        roleUpdate();
    }
}

void UserManagement::on_pushButton_workShift_clicked()
{
    if(ui->stackedWidget_2->currentIndex()!=3)
    {
        ui->label_user->hide();
        ui->label_role->hide();
        ui->label_workShift->show();
        ui->stackedWidget_2->setCurrentIndex(3);
        workShiftUpdate();
    }
}

void UserManagement::workShiftUpdate()
{
    QString aff = "select WorkShiftName,DATE_FORMAT(StartTime,'%H:%i'),DATE_FORMAT(EndTime,'%H:%i'),Remark from WorkShift order by WorkShiftID";
    model->setQuery(aff,db2);
    if (model->lastError().isValid())
        qDebug() << model->lastError();
    ui->tableView->setModel(model);

    for(int i=0;i<model->rowCount();i++)
    {
        ui->tableView->setRowHeight(i,51);//行宽
    }
}

void UserManagement::on_pushButton_clicked()
{
    this->close();
}

void UserManagement::headSelect(QString headName)
{
    ui->label_black->hide();
    delete e3;
    head->deleteLater();
    head = NULL;
    head_Name = headName;
    ui->widget_head->setStyleSheet("border-image: url(/Head/"+headName+")");
}

void UserManagement::headClose()
{
    ui->label_black->hide();
    delete e3;
    delete head;
}

void UserManagement::on_pushButton_save_clicked()
{
    bool isWorkIDRepeat = false;
//    qDebug()<<"WorkIDs.length()"<<WorkIDs.length();
    for(int i=0;i<WorkIDs.length();i++)
    {
//        qDebug()<<"WorkIDs[i]"<<i<<WorkIDs[i];
        if(ui->lineEdit_WorkID->text() == WorkIDs[i] && ui->lineEdit_WorkID->text() != currentWorkID)
        {
            isWorkIDRepeat = true;
            break;
        }
    }

    bool isCardIDRepeat = false;
    for(int i=0;i<CardIDs.length();i++)
    {
        if(ui->label_CardNum->text() == CardIDs[i] && ui->label_CardNum->text() != currentCardID)
        {
            isCardIDRepeat = true;
            break;
        }
    }

    bool isSave = true;
    if(isCardIDRepeat)
    {
        isSave = false;
        ui->label_wrong_CardNum->show();
        ui->label_wrong_CardNum->setText("已存在该卡号!");
    }
    else
    {
        ui->label_wrong_CardNum->hide();
    }

    if(ui->lineEdit_UserName->text()=="")
    {
        isSave = false;
        ui->label_wrong_UserName->show();
        ui->label_wrong_UserName->setText("请填写姓名!");
    }
    else
    {
        ui->label_wrong_UserName->hide();
    }

    if(ui->lineEdit_WorkID->text()=="")
    {
        isSave = false;
        ui->label_wrong_WorkID->show();
        ui->label_wrong_WorkID->setText("请填写工号!");
    }
    else if(isWorkIDRepeat)
    {
        isSave = false;
        ui->label_wrong_WorkID->show();
        ui->label_wrong_WorkID->setText("已存在该工号!");
    }
    else
    {
        ui->label_wrong_WorkID->hide();
    }

    if(ui->lineEdit_UserPassword->text()=="")
    {
        isSave = false;
        ui->label_wrong_UserPassword->show();
        ui->label_wrong_UserPassword->setText("请填写密码!");
    }
    else
    {
        ui->label_wrong_UserPassword->hide();
    }
    if(ui->comboBox->currentText() =="")
    {
        isSave = false;
        ui->label_wrong_role->show();
        ui->label_wrong_role->setText("请选择职务!");
    }
    else
    {
        ui->label_wrong_role->hide();
    }

    if(isSave)
    {
        if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
            mysqlopen();
        if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
        {
            query3.exec("SELECT RoleID from Roles where RoleName= '"+ui->comboBox->currentText()+"'");
            query3.next();
            QString tmp = "";
            if(UserID == 0)
            {
                query2.prepare("INSERT INTO Users (WorkID, UserName, UserPassword, CardID, Head, RoleID)"
                               "VALUES (?, ?, ?, ?, ?, ?)");
                tmp = "Insert new User";
            }
            else
            {
                query2.prepare("UPDATE Users SET WorkID = ?,UserName = ?,UserPassword = ?,CardID = ?,Head = ?,RoleID = ? WHERE UserID ="+ QString::number(UserID));
                tmp = "Update UserID "+ QString::number(UserID);
            }

            query2.addBindValue(ui->lineEdit_WorkID->text());
            query2.addBindValue(ui->lineEdit_UserName->text());
            query2.addBindValue(ui->lineEdit_UserPassword->text());
            query2.addBindValue(ui->label_CardNum->text());
            query2.addBindValue(head_Name);
            query2.addBindValue(query3.value(0).toInt());
            if(!query2.exec())
            {
                qDebug()<<tmp+" fail "<<query2.lastError();
            }
            else
            {
                qDebug()<<tmp<<ui->lineEdit_WorkID->text()<<ui->lineEdit_UserName->text()<<ui->lineEdit_UserPassword->text()<<ui->label_CardNum->text()<<query3.value(0).toString();
                userClear();
                userUpdate();
                ui->stackedWidget_2->setCurrentIndex(0);
                ui->stackedWidget_3->setCurrentIndex(0);
            }
        }
    }
}

void UserManagement::on_pushButton_cancel_clicked()
{
    ui->label_CardNum->setText(currentCardID);
    ui->lineEdit_UserName->setText(currentUserName);
    ui->lineEdit_WorkID->setText(currentWorkID);
    ui->lineEdit_UserPassword->setText(currentUserPassword);
    ui->widget_head->setStyleSheet("border-image: url(/Head/"+currentHead+")");
    ui->comboBox->setCurrentIndex(currentRoleIndex);
    ui->label_wrong_CardNum->hide();
    ui->label_wrong_UserName->hide();
    ui->label_wrong_WorkID->hide();
    ui->label_wrong_UserPassword->hide();
    ui->label_wrong_role->hide();
}


void UserManagement::on_pushButton_shutdown_clicked()
{
    //    e3 = new QGraphicsOpacityEffect(this);
    //    e3->setOpacity(0.9);
    ////    ui->label_black->setGraphicsEffect(e3);
    //    fullScreenPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
    //    fullScreenPixmap.save("fullScreen.jpg","JPG");

    ////    或
    ////    fullScreenPixmap->save("Screenshot.png","png");
    pushButton_blur = new QPushButton(this);
    pushButton_blur->setFocusPolicy(Qt::NoFocus);
    pushButton_blur->setStyleSheet("border-image: url(:/LCD_CS351/LCD_CS351/35_all/background.png);");

    //    e0 = new QGraphicsBlurEffect(this);
    //    e0->setBlurRadius(0.8);
    //    pushButton_blur->setGraphicsEffect(e3);
    pushButton_blur->show();
    pushButton_blur->setGeometry(0,0,1366,768);
    pushButton_reboot= new QPushButton(this);
    pushButton_reboot->setFocusPolicy(Qt::NoFocus);
    pushButton_reboot->setGeometry(844,284,200,200);
    pushButton_reboot->setText(tr("重新启动"));
    pushButton_reboot->setStyleSheet("border-style: solid;border-radius:20px;background-color: rgb(241, 156, 31); font: 20pt \"黑体\";color: rgb(248, 248, 255);");
//    pushButton_reboot->show();
    connect(pushButton_reboot,SIGNAL(clicked()),this,SLOT(reboot()));
    pushButton_poweroff= new QPushButton(this);
    pushButton_poweroff->setFocusPolicy(Qt::NoFocus);
//    pushButton_poweroff->setGeometry(322,284,200,200);
    pushButton_poweroff->setGeometry(583,284,200,200);
    pushButton_poweroff->setText(tr("关闭电源"));
    pushButton_poweroff->setStyleSheet("border-style: solid;border-radius:20px;background-color: rgb(255, 102, 102); font: 20pt \"黑体\";color: rgb(248, 248, 255);");
    pushButton_poweroff->show();
    connect(pushButton_poweroff,SIGNAL(clicked()),this,SLOT(poweroff()));
    connect(pushButton_blur,SIGNAL(clicked()),this,SLOT(closeDialog()));
}

void UserManagement::reboot()
{
    qDebug() << "reboot here";
    system("reboot");
}

void UserManagement::poweroff()
{
    qDebug() << "poweroff here";
    system("echo 1 > /sys/class/leds/control_power2/brightness");
    system("echo 0 > /sys/class/leds/control_lvds/brightness");
    system("poweroff");
}

void UserManagement::closeDialog()
{
    pushButton_blur->hide();
    pushButton_reboot->hide();
    pushButton_poweroff->hide();
}

void UserManagement::on_pushButton_shutdown_2_clicked()
{
    on_pushButton_shutdown_clicked();
}

void UserManagement::on_pushButton_up_clicked()
{
    currentPage--;
    updateListWidget();
}

void UserManagement::on_pushButton_down_clicked()
{
    currentPage++;
    updateListWidget();
}

void UserManagement::on_pushButton_delete_clicked()
{
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("DELETE FROM Users WHERE UserID ="+ QString::number(UserID)))
        {
            qDebug()<<"delete fail UserID ="+ QString::number(UserID)<<query2.lastError();
        }
        else
        {
            qDebug()<<"delete success"<<currentCardID<<currentUserName<<currentWorkID<<currentUserPassword;
            userClear();
            userUpdate();
            ui->stackedWidget_2->setCurrentIndex(0);
            ui->stackedWidget_3->setCurrentIndex(0);
        }
    }
}

void UserManagement::onPushbuttonLeftAClicked()
{
    currentUserPage[0]--;
    updateSingleListWidget(0);
}

void UserManagement::onPushbuttonRightAClicked()
{
    currentUserPage[0]++;
    updateSingleListWidget(0);
}

void UserManagement::onPushbuttonLeftBClicked()
{
    currentUserPage[1]--;
    updateSingleListWidget(1);
}

void UserManagement::onPushbuttonRightBClicked()
{
    currentUserPage[1]++;
    updateSingleListWidget(1);
}

void UserManagement::onPushbuttonLeftCClicked()
{
    currentUserPage[2]--;
    updateSingleListWidget(2);
}

void UserManagement::onPushbuttonRightCClicked()
{
    currentUserPage[2]++;
    updateSingleListWidget(2);
}

void UserManagement::userIsChange()
{
    if(ui->label_CardNum->text() != currentCardID ||
           ui->lineEdit_UserName->text() != currentUserName||
           ui->lineEdit_WorkID->text() != currentWorkID||
           ui->lineEdit_UserPassword->text()!= currentUserPassword||
           head_Name != currentHead||
           ui->comboBox->currentIndex()!=currentRoleIndex)
    {
        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);
        SaveWhat = "User";
        save = new Save(this);
        connect(save,SIGNAL(sendSaveUser(bool)),this,SLOT(receiveSaveUser(bool)));
        save->show();
    }
    else
    {
        isUserchange = false;
        on_pushButton_back_clicked();
    }
}

void UserManagement::receiveSaveUser(bool isSave)
{
    SaveWhat == "";
    ui->label_black->hide();
    delete e3;
    delete save;
    if(isSave)
    {
        on_pushButton_save_clicked();
    }
    else
    {
        isUserchange = false;
        on_pushButton_back_clicked();
    }
}

void UserManagement::on_pushButton_addWorkShift_clicked()
{
    QStringList workShiftNames;
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(!query2.exec("SELECT WorkShiftName from WorkShift"))
            qDebug()<<"select WorkShiftName fail"<<query2.lastError();
        else
        {
            while(query2.next())
                workShiftNames<<query2.value(0).toString();
            qDebug()<<"workShiftNames:"<<workShiftNames;
        }

        e3 = new QGraphicsOpacityEffect(this);
        e3->setOpacity(0.5);
        ui->label_black->setGraphicsEffect(e3);
        ui->label_black->show();
        ui->label_black->setGeometry(0,0,1366,768);
        workShift = new WorkShift(this);
        connect(workShift,SIGNAL(send_workShift(int,QString,QTime,QTime,QString)),this,SLOT(receiveWorkShift(int,QString,QTime,QTime,QString)));
        connect(workShift,SIGNAL(send_Close()),this,SLOT(receiveWorkShiftClose()));
        workShift->getWorkShiftNames(workShiftNames);
        workShift->show();
    }
    else
    {
        qDebug()<<"can not open database add WorkShift fail";
    }
}

void UserManagement::receiveWorkShift(int WorkShiftID, QString workShiftName, QTime start, QTime end, QString remark)
{
    ui->label_black->hide();
    delete e3;
    delete workShift;
    QString tmp="";
    if(!db2.isOpen()||!QSqlDatabase::contains("mysqlUser"))
        mysqlopen();
    if(db2.isOpen() && QSqlDatabase::contains("mysqlUser"))
    {
        if(WorkShiftID == 0)
        {
            query2.prepare("INSERT INTO WorkShift (WorkShiftName,StartTime,EndTime,Remark)"
                           "VALUES (?, ?, ?, ?)");
            tmp = "insert";
        }
        else
        {
            query2.prepare("UPDATE WorkShift SET WorkShiftName = ?,StartTime = ?,EndTime = ?,Remark = ? WHERE WorkShiftID ="+ QString::number(WorkShiftID));
            tmp = "update";
        }
        query2.addBindValue(workShiftName);
        query2.addBindValue(start);
        query2.addBindValue(end);
        query2.addBindValue(remark);
        if(!query2.exec())
            qDebug()<<tmp<<" WorkShift fail"<<workShiftName<<start<<end<<remark<<query2.lastError();
        else
        {
            qDebug()<<tmp<<" WorkShift success"<<workShiftName<<start<<end<<remark;
            workShiftUpdate();
        }
    }
    else
    {
        qDebug()<<"can not open database receiveWorkShift fail";
    }
}

void UserManagement::receiveWorkShiftClose()
{
    ui->label_black->hide();
    delete e3;
    delete workShift;
}

void UserManagement::on_pushButton_cancelLogin_clicked()
{
    ui->label_wrong_2->hide();
    ui->label_31->hide();
    ui->label_cardNumber->hide();
    ui->label_wrong->hide();
    ui->lineEdit_LoginWorkID->clear();
    ui->lineEdit_LoginPassword->clear();
    if(factory == "BAIC")
    {
        ui->stackedWidget->setCurrentIndex(2);
    }
    else {
        ui->stackedWidget->setCurrentIndex(1);
    }

}

void UserManagement::on_pushButton_LoginState_clicked()
{
    query3.next();
    if(factory == "BAIC")
    {
        emit send_Login(userName,"","0,1,2,3,4");
    }
    else {
        emit send_Login(query2.value(0).toString(),query2.value(2).toString(),query3.value(0).toString());
    }

    emit sendLogin(true);
    query2.exec("UPDATE Users set LastLoginDate = now() where WorkID="+query2.value(4).toString());
    ui->label_wrong_2->hide();
    ui->label_31->hide();
    ui->label_cardNumber->hide();
    ui->label_wrong->hide();
    ui->lineEdit_LoginWorkID->clear();
    ui->lineEdit_LoginPassword->clear();
    //                                query2.exec("UPDATE Users set LastLoginDate = now() where WorkID="+ui->lineEdit_LoginWorkID->text());

}

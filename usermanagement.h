#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include "roleadd.h"
#include "head.h"
#include <QPainter>
#include <QTime>
#include <QDesktopWidget>
#include <QSettings>
#include <QtCore/qmath.h>
#include <QListWidget>
#include "GlobalVarible.h"
#include "save.h"
#include <QSqlQueryModel>
#include "workshift.h"
#include <QSqlRecord>
#include <QCryptographicHash>

#include "proto/userbind.h"

namespace Ui {
class UserManagement;
}

class UserManagement : public QDialog
{
    Q_OBJECT

public:
    explicit UserManagement(QWidget *parent = 0);
    ~UserManagement();
    void userConfig();
    void sqlinit();
    void mysqlopen();
    void interfaceInit();
    void userClear();
    void updateListWidget();
    void updateSingleListWidget(int);
    void userIsChange();

signals:
    void send_Login(QString,QString,QString);
    void sendLogin(bool);
    void signalSendUserData(QString,QString);

private slots:
    void on_pushButton_addUser_clicked();
    void on_pushButton_addRole_clicked();
    void receiveRole(int,QString,bool,bool,bool,bool,bool);
//    void updateRole(int,QString,bool,bool,bool,bool,bool);
    void receiveRoleClose();
    void deleteRole(int);

    void on_pushButton_changeHead_clicked();

    void on_pushButton_Login_clicked();

    void on_pushButton_toCard_clicked();


    void receiveCardNumber(int);

    void on_pushButton_back_clicked();

    void on_pushButton_user_clicked();

    void on_pushButton_role_clicked();

    void on_pushButton_clicked();
    void headSelect(QString);
    void headClose();

    void on_pushButton_save_clicked();

    void on_pushButton_cancel_clicked();
    void userUpdate();
    void roleInit();
    void userInit();
    void roleUpdate();
    void workShiftUpdate();
    void roleChange(QListWidgetItem*);

    void userChange(QListWidgetItem*);

    void on_pushButton_shutdown_clicked();
    void reboot();
    void poweroff();
    void closeDialog();

    void on_pushButton_shutdown_2_clicked();

    void on_pushButton_up_clicked();

    void on_pushButton_down_clicked();

    void on_pushButton_delete_clicked();

    void onPushbuttonLeftAClicked();
    void onPushbuttonRightAClicked();
    void onPushbuttonLeftBClicked();
    void onPushbuttonRightBClicked();
    void onPushbuttonLeftCClicked();
    void onPushbuttonRightCClicked();

    void receiveSaveUser(bool);

    void on_pushButton_workShift_clicked();

    void on_pushButton_addWorkShift_clicked();

    void receiveWorkShift(int,QString,QTime,QTime,QString);
    void receiveWorkShiftClose();

    void on_pushButton_cancelLogin_clicked();

    void on_pushButton_LoginState_clicked();

    void WorkShiftChange(const QModelIndex &);
    void deleteWorkShift(int);

public slots:
    void on_pushButton_toPassword_clicked();


private:
    Ui::UserManagement *ui;
    QGraphicsOpacityEffect *e3;
    QGraphicsBlurEffect *e0;
    RoleAdd * roleAdd;
    Head * head;
    QSqlDatabase db2;
    QSqlQuery query1;
    QSqlQuery query2;
    QSqlQuery query3;
    QString head_Name;
    enum enumFunctions{systemConfig,program,pdm,history,user};
    QPixmap fullScreenPixmap;
    QPushButton *pushButton_reboot;
    QPushButton *pushButton_poweroff;
    QPushButton *pushButton_blur;
    int UserID;
    QStringList WorkIDs;
    QStringList CardIDs;
    QString currentWorkID;
    QString currentCardID;
    QString currentUserName;
    QString currentHead;
    QString currentUserPassword;
    int currentRoleIndex;
    int currentPage;
    int rolePages;
    QListWidget *listWidget_Users[3];
    QPushButton *pushButton_left[3];
    QPushButton *pushButton_right[3];
    int userPages[3];
    int currentUserPage[3];
    int nCurScroller; //翻页时的当时滑动条位置
    int pageValue; // 一页显示条数
    QString currentRoleID[3];
    bool isUserchange;
    Save * save;
    QSqlQueryModel *model;
    WorkShift *workShift;
    bool newData;
};

#endif // USERMANAGEMENT_H

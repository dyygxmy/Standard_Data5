#ifndef ROOTDIALOG_H
#define ROOTDIALOG_H

#include <QDialog>
#include <QApplication>
#include <mainwindow.h>
#include <newconfiginfo.h>
#include <QDateTime>
#include "passwordpanel.h"
#include <usermanagement.h>
#include <QPixmap>
#include <QScreen>        // 用来截取全屏
#include <QDesktopWidget>
//#include <QGraphicsBlurEffect>

namespace Ui {
class RootDialog;
}

class RootDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RootDialog(QWidget *parent = 0);
    ~RootDialog();

    PasswordPanel *CreatePasswdPanel();

signals:
    void signalDetectCard(QString);

private slots:
    void slotDetectCard(QString);

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();
    void timerUpdate();
    void wifishow(bool);

    void on_pushButton_11_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_9_clicked();

public slots:
    void batteryshow1(QString);
    void batteryshow2(bool);
    void init(MainWindow *w, Newconfiginfo *newconfiginfo);
    void initUser(UserManagement * userManagement);
//    void shutdown(int);
    void receiveResult(bool);
    void logout(bool);
    void receiveLogin(QString,QString,QString);
    void reboot();
    void poweroff();
    void closeDialog();
    void closeSave();
    void clocked();
    void battery15();

private:
    Ui::RootDialog *ui;
    QTimer timer_showdown;
    QTimer shutdown_timer;
    int battry_num;
    QGraphicsOpacityEffect *e3;
    QGraphicsBlurEffect *e0;
    Save * save;
    bool isLogin;
    PasswordPanel * passwordPanel;
    bool isSystem;
    bool isProgram;
    bool isPdm;
    bool isHistory;
    bool isUser;
    enum enumFunctions{systemConfig,program,pdm,history,user};
    QPixmap fullScreenPixmap;
    QPushButton *pushButton_reboot;
    QPushButton *pushButton_poweroff;
    QPushButton *pushButton_blur;
    QString Factory;

public:
    MainWindow *w;
    Newconfiginfo *c;
    UserManagement *userManagement;
    QString EnterWhich;
};

#endif // ROOTDIALOG_H

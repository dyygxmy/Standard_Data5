#ifndef PASSWORDPANEL_H
#define PASSWORDPANEL_H

#include <QDialog>
//#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QDesktopWidget>
#include "qextserial/qextserialport.h"
#include "GlobalVarible.h"

namespace Ui {
class PasswordPanel;
}

class PasswordPanel : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordPanel(QWidget *parent = 0);
    ~PasswordPanel();
    void judge();

signals:
    void sendResult(bool);
    void signalDetectCard(QString);

private slots:
    void readCom();

    void on_pushButton_37_clicked();

    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_0_clicked();

    void on_pushButton_delete_clicked();

private:
    Ui::PasswordPanel *ui;
    int isFull;
    QString temp;
};


#endif // PASSWORDPANEL_H

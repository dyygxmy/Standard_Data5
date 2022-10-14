#ifndef ROLEADD_H
#define ROLEADD_H

#include <QDialog>

namespace Ui {
class RoleAdd;
}

class RoleAdd : public QDialog
{
    Q_OBJECT

public:
    explicit RoleAdd(QWidget *parent = 0);
    ~RoleAdd();
    void getRoleNames(QStringList);
    void RoleInit(QStringList,int,bool,QString,bool,bool,bool,bool,bool);

signals:
    void send_Role(int,QString,bool,bool,bool,bool,bool); // 原RoleID,当前角色名,systemConfig,program,pdm,history,user
//    void update_Role(int,QString,bool,bool,bool,bool,bool);
    void send_Close();
    void delete_Role(int);

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_cancel_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_delete_clicked();

private:
    Ui::RoleAdd *ui;
    QStringList roleNames;
    int RoleID;
    QString currentName;
};

#endif // ROLEADD_H

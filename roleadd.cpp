#include "roleadd.h"
#include "ui_roleadd.h"

RoleAdd::RoleAdd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoleAdd)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    ui->label_wrong->hide();
    ui->pushButton_delete->hide();
    RoleID = 0;
    currentName = "";
}

RoleAdd::~RoleAdd()
{
    delete ui;
}

void RoleAdd::getRoleNames(QStringList tmp)
{
    roleNames = tmp;
}

void RoleAdd::RoleInit(QStringList names, int ID, bool deleteIsShow, QString currentRoleName, bool isSystem, bool isProgram, bool isPdm, bool isHistory, bool isUser)
{
    ui->pushButton_cancel->hide();
    roleNames = names;
    RoleID = ID;
    if(deleteIsShow)
    {
        ui->pushButton_delete->show();
        ui->pushButton_delete->setGeometry(190,290,80,31);
    }
    ui->lineEdit_roleName->setText(currentRoleName);
    currentName = currentRoleName;
    ui->checkBox_1->setChecked(isSystem);
    ui->checkBox_2->setChecked(isProgram);
    ui->checkBox_3->setChecked(isPdm);
    ui->checkBox_4->setChecked(isHistory);
    ui->checkBox_5->setChecked(isUser);
}

void RoleAdd::on_pushButton_save_clicked()
{
    if(ui->lineEdit_roleName->text() != "")
    {
        bool isRepeat = false;
        for(int i=0;i<roleNames.length();i++)
        {
            if(ui->lineEdit_roleName->text() == roleNames[i] && ui->lineEdit_roleName->text() != currentName)
            {
                isRepeat = true;
                break;
            }
        }
        if(isRepeat)
        {
            ui->label_wrong->show();
            ui->label_wrong->setText("已存在该角色，请输入其他角色名！");
        }
        else
            emit send_Role(RoleID,ui->lineEdit_roleName->text(),ui->checkBox_1->isChecked(),ui->checkBox_2->isChecked(),ui->checkBox_3->isChecked(),ui->checkBox_4->isChecked(),ui->checkBox_5->isChecked());
    }
    else
    {
        ui->label_wrong->setText("请填写角色名！");
        ui->label_wrong->show();
    }
}

void RoleAdd::on_pushButton_cancel_clicked()
{
    ui->lineEdit_roleName->clear();
    ui->checkBox_1->setChecked(false);
    ui->checkBox_2->setChecked(false);
    ui->checkBox_3->setChecked(false);
    ui->checkBox_4->setChecked(false);
    ui->checkBox_5->setChecked(false);
}

void RoleAdd::on_pushButton_close_clicked()
{
    emit send_Close();
}

void RoleAdd::on_pushButton_delete_clicked()
{
    emit delete_Role(RoleID);
}

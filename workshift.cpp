#include "workshift.h"
#include "ui_workshift.h"

WorkShift::WorkShift(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WorkShift)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    ui->label_wrong->hide();
    ui->pushButton_delete->hide();
    WorkShiftID = 0;
    currentName = "";
}

WorkShift::~WorkShift()
{
    delete ui;
}

void WorkShift::getWorkShiftNames(QStringList tmp)
{
    workShiftNames = tmp;
}

void WorkShift::WorkShiftInit(QStringList names, int ID, QString currentWorkShiftName,QTime start, QTime end, QString remark)
{
    ui->label->setText(tr("更改班次"));
    ui->pushButton_cancel->hide();
    workShiftNames = names;
    WorkShiftID = ID;
    ui->pushButton_delete->show();
    ui->pushButton_delete->setGeometry(151,330,80,31);
    ui->lineEdit_workShiftName->setText(currentWorkShiftName);
    currentName = currentWorkShiftName;
    ui->timeEdit_StartHour->setTime(start);
    ui->timeEdit_StartMinute->setTime(start);
    ui->timeEdit_EndHour->setTime(end);
    ui->timeEdit_EndMinute->setTime(end);
    ui->lineEdit_remark->setText(remark);
}

void WorkShift::on_pushButton_save_clicked()
{
    if(ui->lineEdit_workShiftName->text() != "")
    {
        bool isRepeat = false;
        for(int i=0;i<workShiftNames.length();i++)
        {
            if(ui->lineEdit_workShiftName->text() == workShiftNames[i] && ui->lineEdit_workShiftName->text() != currentName)
            {
                isRepeat = true;
                break;
            }
        }
        if(isRepeat)
        {
            ui->label_wrong->show();
            ui->label_wrong->setText("已存在该班次，请输入其他班次名！");
        }
        else
            emit send_workShift(WorkShiftID,ui->lineEdit_workShiftName->text(),ui->timeEdit_StartHour->time().addSecs(ui->timeEdit_StartMinute->time().minute()*60), ui->timeEdit_EndHour->time().addSecs(ui->timeEdit_EndMinute->time().minute()*60),ui->lineEdit_remark->text());
    }
    else
    {
        ui->label_wrong->setText("请填写班次名！");
        ui->label_wrong->show();
    }
}

void WorkShift::on_pushButton_cancel_clicked()
{
    ui->lineEdit_workShiftName->clear();
    ui->timeEdit_StartHour->setTime(QTime::fromString("08","hh"));
    ui->timeEdit_StartMinute->setTime(QTime::fromString("00","mm"));
    ui->timeEdit_EndHour->setTime(QTime::fromString("17","hh"));
    ui->timeEdit_EndMinute->setTime(QTime::fromString("00","mm"));
    ui->lineEdit_remark->clear();
}

void WorkShift::on_pushButton_StartHourAdd_clicked()
{
    ui->timeEdit_StartHour->setTime(ui->timeEdit_StartHour->time().addSecs(3600));
}

void WorkShift::on_pushButton_StartHourSubstract_clicked()
{
    ui->timeEdit_StartHour->setTime(ui->timeEdit_StartHour->time().addSecs(-3600));
}

void WorkShift::on_pushButton_StartMinuteAdd_clicked()
{
    ui->timeEdit_StartMinute->setTime(ui->timeEdit_StartMinute->time().addSecs(60));
}

void WorkShift::on_pushButton_StartMinuteSubstract_clicked()
{
    ui->timeEdit_StartMinute->setTime(ui->timeEdit_StartMinute->time().addSecs(-60));
}

void WorkShift::on_pushButton_EndHourAdd_clicked()
{
    ui->timeEdit_EndHour->setTime(ui->timeEdit_EndHour->time().addSecs(3600));
}

void WorkShift::on_pushButton_EndHourSubstract_clicked()
{
    ui->timeEdit_EndHour->setTime(ui->timeEdit_EndHour->time().addSecs(-3600));
}

void WorkShift::on_pushButton_EndMinuteAdd_clicked()
{
    ui->timeEdit_EndMinute->setTime(ui->timeEdit_EndMinute->time().addSecs(60));
}

void WorkShift::on_pushButton_EndMinuteSubstract_clicked()
{
    ui->timeEdit_EndMinute->setTime(ui->timeEdit_EndMinute->time().addSecs(-60));
}

void WorkShift::on_pushButton_close_clicked()
{
    emit send_Close();
}

void WorkShift::on_pushButton_delete_clicked()
{
    emit delete_workShift(WorkShiftID);
}

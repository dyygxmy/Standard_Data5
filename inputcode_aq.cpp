#include "inputcode_aq.h"
#include "ui_inputcode_aq.h"
#include <QSettings>

Inputcode_AQ::Inputcode_AQ(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Inputcode_AQ)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
}

Inputcode_AQ::~Inputcode_AQ()
{
    delete ui;
}

void Inputcode_AQ::on_pushButton_cancel_clicked()
{
    emit sendClose();
}

void Inputcode_AQ::on_pushButton_clicked()
{
    QString tmpVin = ui->textEdit_vin->toPlainText();
    QString tmpMaterial = ui->textEdit_material->toPlainText();

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    int mPartCode = configIniRead->value("baseinfo/PartCode").toInt();
    configIniRead->deleteLater();
    if(
            ((tmpVin.length() == 17) && (tmpMaterial.length() == 15) && mPartCode == 0)||
            ((tmpVin.length() == 17) && (tmpMaterial.length() == 0) && mPartCode == 1)
            )
    {
        emit sendCode(tmpVin+tmpMaterial);
    }
}

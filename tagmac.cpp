#include "tagmac.h"
#include "ui_tagmac.h"

TagMac::TagMac(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TagMac)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    ui->label_warning->hide();
    QRegExp rx("[0-9,A-Z,a-z]{1,21}");
    QValidator *validator = new QRegExpValidator(rx, this );
    ui->lineEdit_tagMac->setValidator(validator);//add lw 207/9/19
}

TagMac::~TagMac()
{
    delete ui;
}

void TagMac::initUI(QString mac)
{
    ui->lineEdit_tagMac->setText(mac);
}

void TagMac::on_pushButton_save_clicked()
{
    if(!judgment())
    {
        ui->label_warning->show();
        return;
    }

    emit sendParameter(ui->lineEdit_tagMac->text());
}

void TagMac::on_pushButton_close_clicked()
{
    emit closeDiglag();
}

bool TagMac::judgment()
{
    if(ui->lineEdit_tagMac->text().isEmpty())
        return false;
    return true;
}

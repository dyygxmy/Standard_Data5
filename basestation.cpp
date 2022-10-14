#include "basestation.h"
#include "ui_basestation.h"

BaseStation::BaseStation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BaseStation)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    ui->label_warning->hide();

    QRegExp rx("[0-9.]{7,15}");
    QValidator *validator = new QRegExpValidator(rx, this );
    ui->lineEdit_x->setValidator(validator);
    ui->lineEdit_y->setValidator(validator);
    ui->lineEdit_z->setValidator(validator);
    QRegExp rx1("[0-9.:]{7,15}");
    QValidator *validator1 = new QRegExpValidator(rx1, this );
    ui->lineEdit_checked->setValidator(validator1);
    QRegExp rx2("[0-9A-Za-z]{7,15}");
    QValidator *validator2 = new QRegExpValidator(rx2, this );
    ui->lineEdit_anchor_no->setValidator(validator2);
}

BaseStation::~BaseStation()
{
    delete ui;
}

void BaseStation::initUI(QVariantMap anchorMap)
{
    ui->lineEdit_anchor_no->setText(anchorMap["anchor_no"].toString());
    ui->lineEdit_x->setText(anchorMap["x"].toString());
    ui->lineEdit_y->setText(anchorMap["y"].toString());
    ui->lineEdit_z->setText(anchorMap["z"].toString());
    ui->lineEdit_checked->setText(anchorMap["checked"].toString());
}

void BaseStation::on_pushButton_close_clicked()
{
    emit closeDiglag();
}

void BaseStation::on_pushButton_save_clicked()
{
    if(!judgment())
    {
        ui->label_warning->show();
        return;
    }

    QVariantMap singleAnchor;
    singleAnchor["anchor_no"]=ui->lineEdit_anchor_no->text();
    singleAnchor["x"]=ui->lineEdit_x->text();
    singleAnchor["y"]=ui->lineEdit_y->text();
    singleAnchor["z"]=ui->lineEdit_z->text();
    singleAnchor["checked"]=ui->lineEdit_checked->text();
    emit sendParameter(singleAnchor);
}

bool BaseStation::judgment()
{
    if(ui->lineEdit_anchor_no->text().isEmpty())
        return false;
    else if(ui->lineEdit_x->text().isEmpty())
        return false;
    else if(ui->lineEdit_y->text().isEmpty())
        return false;
    else if(ui->lineEdit_z->text().isEmpty())
        return false;
    else if(ui->lineEdit_checked->text().isEmpty())
        return false;
    else
        return true;
}

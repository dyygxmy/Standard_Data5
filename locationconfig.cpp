#include "locationconfig.h"
#include "ui_locationconfig.h"

LocationConfig::LocationConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocationConfig)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    ui->label_warning->hide();
    tagMacInsertRow = 0;
    ui->label_black->hide();
    QRegExp rx21("[0-9.]{7,15}");
    QValidator *validator21 = new QRegExpValidator(rx21, this );
    ui->lineEdit_IP->setValidator(validator21);
    ui->lineEdit_x0->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_y0->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_x1->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_y1->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_x2->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_y2->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_x3->setValidator(validator21);//add lw 207/9/19
    ui->lineEdit_y3->setValidator(validator21);//add lw 207/9/19

}

LocationConfig::~LocationConfig()
{
    delete ui;
}

void LocationConfig::initIp()
{
    ui->lineEdit_IP->setText("127.0.0.1");
    ui->lineEdit_IP->setEnabled(false);
}

void LocationConfig::initUI(QVariantMap dataConfigMap)
{
    ui->lineEdit_IP->setText(dataConfigMap["IP"].toString());
    ui->lineEdit_x0->setText(dataConfigMap["x0"].toString());
    ui->lineEdit_y0->setText(dataConfigMap["y0"].toString());
    ui->lineEdit_x1->setText(dataConfigMap["x1"].toString());
    ui->lineEdit_y1->setText(dataConfigMap["y1"].toString());
    ui->lineEdit_x2->setText(dataConfigMap["x2"].toString());
    ui->lineEdit_y2->setText(dataConfigMap["y2"].toString());
    ui->lineEdit_x3->setText(dataConfigMap["x3"].toString());
    ui->lineEdit_y3->setText(dataConfigMap["y3"].toString());
    tagMacList = dataConfigMap["tag_mac"].toList();
    for(int i=0;i<tagMacList.size();i++)
    {
        QListWidgetItem * pItem = new QListWidgetItem;
        pItem->setSizeHint(QSize(281, 43));  //每次改变Item的高度
        pItem->setText(tagMacList[i].toString());
        ui->listWidget_tag_mac->addItem(pItem);
    }
}

void LocationConfig::on_pushButton_close_clicked()
{
    emit closeDiglag();
}

void LocationConfig::on_pushButton_save_clicked()
{
    if(!judgment())
    {
        ui->label_warning->show();
        ui->label_warning->setText(tr("请填写完整!"));
        return;
    }

    QVariantMap dataMap;
    dataMap["IP"]=ui->lineEdit_IP->text();
    dataMap["x0"]=ui->lineEdit_x0->text();
    dataMap["y0"]=ui->lineEdit_y0->text();
    dataMap["x1"]=ui->lineEdit_x1->text();
    dataMap["y1"]=ui->lineEdit_y1->text();
    dataMap["x2"]=ui->lineEdit_x2->text();
    dataMap["y2"]=ui->lineEdit_y2->text();
    dataMap["x3"]=ui->lineEdit_x3->text();
    dataMap["y3"]=ui->lineEdit_y3->text();
    dataMap["tag_mac"]=tagMacList;
    emit sendParameter(dataMap);
}

bool LocationConfig::judgment()
{
    if(ui->lineEdit_IP->text().isEmpty())
        return false;
    else if(ui->lineEdit_x0->text().isEmpty())
        return false;
    else if(ui->lineEdit_y0->text().isEmpty())
        return false;
    else if(ui->lineEdit_x1->text().isEmpty())
        return false;
    else if(ui->lineEdit_y1->text().isEmpty())
        return false;
    else if(ui->lineEdit_x2->text().isEmpty())
        return false;
    else if(ui->lineEdit_y2->text().isEmpty())
        return false;
    else if(ui->lineEdit_x3->text().isEmpty())
        return false;
    else if(ui->lineEdit_y3->text().isEmpty())
        return false;

    else if(ui->listWidget_tag_mac->count()==0)
        return false;
    else
        return true;
}


void LocationConfig::on_pushButton_tagMac_append_clicked()
{
    if(tagMacList.size()<4)
    {
        newTagMac();
        tagMacInsertRow = ui->listWidget_tag_mac->count();
        currenttagMacOperate = tagMacAdd;
    }
    else
    {
        ui->label_warning->show();
        ui->label_warning->setText(tr("最多只可设置四个标签!"));
    }
}

void LocationConfig::on_pushButton_tagMac_update_clicked()
{
    if(ui->listWidget_tag_mac->currentRow()!= -1)
    {
        newTagMac();
        tagMacInsertRow = ui->listWidget_tag_mac->currentRow();
        tagMac->initUI(ui->listWidget_tag_mac->currentItem()->text());
        currenttagMacOperate = tagMacUpdate;
    }
}

void LocationConfig::on_pushButton_tagMac_remove_clicked()
{
    int rowIndex = ui->listWidget_tag_mac->currentRow();
    ui->listWidget_tag_mac->takeItem(rowIndex);
    tagMacList.removeAt(rowIndex);
}

void LocationConfig::newTagMac()
{
    e3 = new QGraphicsOpacityEffect(this);
    e3->setOpacity(0.5);
    ui->label_black->setGraphicsEffect(e3);
    ui->label_black->show();
    ui->label_black->setGeometry(0,0,539,678);
    tagMac = new TagMac(this);
    tagMac->move((this->width()-tagMac->width())/2,(this->height()-tagMac->height())/2);
    connect(tagMac,SIGNAL(sendParameter(QString)),this,SLOT(receiveTagMac(QString)));
    connect(tagMac,SIGNAL(closeDiglag()),this,SLOT(closeTagMac()));
    tagMac->show();
}

void LocationConfig::receiveTagMac(QString tagmac)
{
    if(currenttagMacOperate == tagMacAdd)
    {
        QListWidgetItem * pItem = new QListWidgetItem;
        pItem->setSizeHint(QSize(281, 43));  //每次改变Item的高度
        pItem->setText(tagmac);
        ui->listWidget_tag_mac->addItem(pItem);
        tagMacList.append(tagmac);
    }
    else if(currenttagMacOperate == tagMacUpdate)
    {
        ui->listWidget_tag_mac->currentItem()->setText(tagmac);
        tagMacList.replace(tagMacInsertRow,tagmac);
    }

    closeTagMac();
}

void LocationConfig::closeTagMac()
{
    delete e3;
    ui->label_black->hide();
    delete tagMac;
}

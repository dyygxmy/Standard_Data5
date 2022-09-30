#include "head.h"
#include "ui_head.h"

Head::Head(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Head)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    headinit();
}

Head::~Head()
{
    delete ui;
}

void Head::headinit()
{//  pdm图 listwidget初始化
    ui->listWidget->setViewMode(QListView::IconMode);    //设置QListWidget的显示模式
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setMovement(QListView::Static);      //设置QListWidget中的单元项不可被拖动
    ui->listWidget->setIconSize(QSize(110, 110));        //设置QListWidget中的单元项的图片大小
    ui->listWidget->setTextElideMode(Qt::ElideRight);
    //ui->listWidget_2->setSpacing(10);                       //设置QListWidget中的单元项的间距
    ui->listWidget->setStyleSheet("QListWidget{border-width:0.5px;border-style:solid;border-color:rgb(51, 153, 255);} QListWidget::item{border-right-width:0.5px;border-bottom-width:0.5px;border-style:solid;border-color:rgb(51, 153, 255);}QScrollBar:vertical{width:36px;}");
    // ui->listWidget_2->verticalScrollBar()->setStyleSheet("QScrollBar{width:36px;}");

    QDir *dir = new QDir("/Head");
    ui->listWidget->clear();
    QStringList list = dir->entryList();
//    qDebug()<<"88888888888888888888888888888888888"<<list;
    QString string;
    for (int index = 0; index < list.size(); index++)
    {
        string = list.at(index);
        if(string == "." || string == "..")
            continue;
        //QListWidgetItem *item = new QListWidgetItem(string);
        //ui->listWidget->addItem(item);
        QListWidgetItem *configButton = new QListWidgetItem(ui->listWidget);
        // qDebug() << string;
        QImage image;


        image.load(QString("/Head/").append(list.at(index)));
        QPixmap objPixmap = QPixmap::fromImage(image.scaled(QSize(110,110)));
        configButton->setIcon(QIcon(objPixmap));
        // configButton->setIcon(QIcon(QString("/PDM/").append(list.at(index))));
        configButton->setText(tr(string.toLatin1().data()));
        configButton->setSizeHint(QSize(110,110));

        configButton->setTextAlignment(Qt::AlignHCenter);
        configButton->setFlags(configButton->flags() & ~Qt::ItemIsSelectable);
    }
    connect(ui->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(headselect(QListWidgetItem*)));
    ui->listWidget->setFlow(QListView::LeftToRight);
    ui->listWidget->update();
    delete dir;
}
void Head::headselect(QListWidgetItem* item)
{
    qDebug()<<item->text();
    emit sendHead(item->text());
}

void Head::on_pushButton_cancel_clicked()
{
    emit sendClose();
}

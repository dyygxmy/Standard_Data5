#ifndef SERIALCOMS_H
#define SERIALCOMS_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <termios.h>
#include "GlobalVarible.h"
#include <QTimer>
#include "qextserial/qextserialport.h"
#include "mesuploadsvw2.h"


class SerialComs : public QObject
{
    Q_OBJECT
public:
    explicit SerialComs(QObject *parent = 0);

signals:
    void serialCom(QString,bool,QString);//发给UI线程
    void sendDebug(QString);
    void deleteCar(QString);
    void alignCar(QString);

    void signalSetIoBox(int pIndex,bool power);

public slots:
    void comInit();                      //初始化条码枪串口
    void readCom() ;
    void slot_ComTimeout();
    void slot_IoBoxInput(int pState);
    void slots_deleteCachedVIN(QByteArray);

public:
    QThread m_thread;

    QextSerialPort *myCom;
    QByteArray mBuffer;

    QList<QString> seriallist;//条码链表 存储10个条码以内不能重复
    int dev_fd;               //串口文件描述符
    bool isEqual;            //条码是否重复
    int whichindex;           //当前条码索引

    int vinLen;
    QByteArray vinHead;
    QString Factory;

    QTimer          mComTimer;

    QByteArray buff;//[20];

    int   mAQCHERYState;  //0:wait for vin, 1: vin has geted, wait for match
    QString mAQCHERYString;
    bool  mPartCode;

public:
    MESUploadSVW2       *mMESUploadSVW2;

    int                 mIoBoxIn[2];

};

#endif // SERIALCOMS_H

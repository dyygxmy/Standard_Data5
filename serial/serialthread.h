#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "qextserial/qextserialport.h"

#include "GlobalVarible.h"

class serialThread : public QObject
{
    Q_OBJECT
public:
    explicit serialThread(QObject *parent = 0);

private:
    QThread serial_thread;
    QString serialNums;
    QextSerialPort *myCom;

    QList<QString> seriallist;//条码链表 存储10个条码以内不能重复
    bool isEqual;            //条码是否重复
    int whichindex;           //当前条码索引

signals:
    void signalQRCodeFromSerial(QString QRCode) ;
    void signalSerialCom(QString,bool,QString) ;

public slots:
    void readMyCom();
    void initSerial() ;
    void slotTest();

};

#endif // SERIALTHREAD_H

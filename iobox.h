#ifndef IOBOX_H
#define IOBOX_H

#include <QObject>
#include "GlobalVarible.h"
#include "serialComs.h"
#include <QTimer>
#include <QDebug>
#include <QThread>

class QextSerialPort;

typedef struct{
    int deviceNum;
    int ctrlNum;
    int address_h;
    int address_l;
    int data_h;
    int data_l;
    int crc;
    int spare1;
    int spare2;
    int spare3;
}writeStruct;

class ioBox : public QObject
{
    Q_OBJECT
private:
    QByteArray sendBuf_temp;
    QByteArray recvBuf_temp;
public:
    explicit ioBox(QObject *parent = 0);

    int checkCRC(QByteArray buf);
    void sendFunc(int type,int ioNum,bool ONOFF);

signals:
    void signalIoBoxInput(int pState);

public slots:
    void slotAutoTest();
    void slotSetIoBox(int pIndex,bool power);
    void slotReadyRead();

public:
    QThread thread;
    QextSerialPort  *mSerialPort;
    int              mOldIoBoxInput;
};

#endif // IOBOX_H

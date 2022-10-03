#ifndef GWKQUANTECMCS_H
#define GWKQUANTECMCS_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <qextserial/qextserialport.h>
#include <QTimer>
class GwkQuanTecMCS : public QObject
{
    Q_OBJECT
public:
    explicit GwkQuanTecMCS(QObject *parent = 0);
    
private:
    QThread gwk_thread;
    QTimer *timerMs;
    QextSerialPort *myCom3;

    int startCount;
    int dataCount;
    int vinCount;

    int  curveTimerCount;
    bool curveTimerFlag;
    bool dataTimerFlag;
    int  dataNumber;
    int  dataLen;
    bool recFlag;

    int  programNumber;
    int  indexLen;
    int  readDataNO;
    QByteArray  programType;
    QByteArray  programName;
    QString  upLimitValue;
    QString  DownLimitValue;

    QByteArray boltNumberBuf;
    QByteArray gwkProNO;
    int programNO;
    int boltCount;
    int boltNokCount;

    int proNameLen;  //program len
    bool readProgramFlag;


public:
    QByteArray revRsDataBuf;
    QString strPeakAngle;
    QString strPeakTorque;
    QString strStopAngle;
    QString strStopTorque;
    QString strResult;
    QString strDateTime;
    QString strTmax;
    QString strTmin;
    unsigned char charTemp;
    int intTemp;
    float floatTemp;
    bool gwkLinkFlag;

    bool reSendFlag;
    int  reSendTimeCount;
    int  sendLen;
    QByteArray sendBuf;

    int testCount;

public slots:
    void timerFuncMs();
    void serialComInit();
    void slot_read_com();
    void sendReadOperate(bool,int);
    void gwkReadDataFunc();
    QString hexQBtyeArray_To_QString(QByteArray,int,int);

signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void IsTigntenReady(bool);
    void sendLimit(QString,QString,QString);
};

#endif // GWKQUANTECMCS_H

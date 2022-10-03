#ifndef PLUSCOMMUNICATION_H
#define PLUSCOMMUNICATION_H

#include <QObject>
#include <QThread>
#include <QtNetwork>

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSharedMemory>
#include <QBuffer>

class PlusCommunication : public QObject
{
    Q_OBJECT
public:
    explicit PlusCommunication(QObject *parent = 0);
    void mysqlOpen();
    void MysqlHandleFunc();
    void closePortTcpServer();


signals:
    void SendJOB(QString);
    void sendPlusFlag(bool);

public slots:    
    void PlusStart();
    void DataReceived();
    void PlusMessage();
    void TimerFunc();

    void IDcodeRequestJob(QString);
    void dataUpload();
//    void quickRequest();
    void receiveRequestJob(QString);

private:
    QTcpServer *tcpServer;
    QTcpSocket *Plus_pTcpSocket;
    QTimer *heartTimer;
    QThread plus_thread;

    QByteArray revBuf;
    QByteArray sendBuf;
    QByteArray requestJobBuf;
    QByteArray str_Cycle_Id;
    QByteArray str_Cycle_Id_Plus;
    QByteArray stationID;
    QByteArray JobInfor;
    QByteArray IDcodeInfor;
    QByteArray sendLen;
    QByteArray strBoltNum;
    QByteArray dataHeadBuf;
    QByteArray boltInforBuf;
    QByteArray torque_AngleBuf;
    QStringList RecodeIDList;
    int Tx_Len;
    int Cycle_Id;
    int timerCount1;
    int timerCount2;
    int timerValue;
    int aliveCount;
    bool plusSynFlag;
    bool requestJobFlag;
    bool JobInsertFlag;

    QSqlDatabase db2;
    QSqlQuery query2;

    QSharedMemory sharedMemory;

    QString Idcode_temp;
};

#endif // PLUSCOMMUNICATION_H

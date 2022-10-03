#ifndef TIGHTEN_OP_NET_CURVE_H
#define TIGHTEN_OP_NET_CURVE_H


#include <QObject>
#include <QThread>
#include "GlobalVarible.h"
#include <QtNetwork>
#include <QDateTime>
#include "unistd.h"

typedef struct {
    quint8  mHeaderSize[4];
    quint8  mHeaderMID[4];
    quint8  mHeaderRev[3];
    quint8  mHeaderOther[9];
    quint8  mCellID[6];                     //1    20
    quint8  mChannelID[4];                  //2
    quint8  mTorqueControllerName[27];      //3
    quint8  mVINNumber[27];                 //4
    quint8  mLinkingGroupID[6];             //5
    quint8  mApplicationID[5];              //6    90
    quint8  mStrategy[4];                   //7
    quint8  mStrategyOption[7];             //8
    quint8  mBatchSize[6];                  //9     112
    quint8  mBatchCounter[3];               //10
    quint8  mTighteningStatus[3];           //11
    quint8  mBatchStatus[3];                //12
    quint8  mTorqueStatus[3];               //13
    quint8  mAngleStatus[3];                //14
    quint8  mRunDownAngleStatus[3];         //15
    quint8  mCurMonitorStatus[3];
    quint8  mSelftapStatus[3];
    quint8  mPreTorqueMonitorStatus[3];
    quint8  mPreTorqueCompenStatus[3];
    quint8  mTightenErrStatus[12];           //20
    quint8  mTorqueMinLimit[8];
    quint8  mTorqueMaxLimit[8];
    quint8  mTorqueFinalTarget[8];
    quint8  mTorque[8];
    quint8  mAngleMin[7];                   //25
    quint8  mAngleMax[7];
    quint8  mFinalAngle[7];
    quint8  mAngle[7];
    quint8  mRundownAngleMin[7];
    quint8  mRundownAngleMax[7];           //30
    quint8  mRundownAngle[7];
    quint8  mCurMonitorMin[5];
    quint8  mCurMonitorMax[5];
    quint8  mCurMonitorValue[5];
    quint8  mSelfTapMin[8];                  //35
    quint8  mSelfTapMax[8];
    quint8  mSelfTapTorque[8];
    quint8  mPreTorqueMonitorMin[8];
    quint8  mPreTorqueMonitorMax[8];
    quint8  mPreTorque[8];                  //40
    quint8  mTightenID[12];
    quint8  mLinkGroupSeqNum[7];
    quint8  mSyncTightenID[7];
    quint8  mToolSerialNum[16];
    quint8  mTimeStamp[21];                 //45
    quint8  mDateTimeOfLast[21];            //46
    quint8  mAppName[27];
    quint8  mTorqueValueUnit[3];
    quint8  mResultType[4];
    quint8  mIDResultPart2[27];             //50
    quint8  mIDResultPart3[27];
    quint8  mIDResultPart4[27];             //52
    quint8  m53[6];
    quint8  m54[8];
    quint8  m55[12];
    quint8  mGraphType[3]; //56
    quint8  mTorqueScaleFactor[12];
    quint8  mAngleOffset[8];
    quint8  mTimeScale[6];
    quint8  mBinaryOffset[12];
    quint8  mTraceLength[6];
    quint8  mDataBlob[9002];
} T_MID61_DATA_V1;

typedef struct {
    quint8  mHeaderSize[4];
    quint8  mHeaderMID[4];
    quint8  mHeaderRev[3];
    quint8  mHeaderOther[9];
    quint8  mCellID[6];                     //1    20
    quint8  mChannelID[4];                  //2
    quint8  mTorqueControllerName[27];      //3
    quint8  mVINNumber[27];                 //4
    quint8  mLinkingGroupID[6];             //5
    quint8  mApplicationID[5];              //6    90
    quint8  mStrategy[4];                   //7
    quint8  mStrategyOption[7];             //8
    quint8  mBatchSize[6];                  //9     112
    quint8  mBatchCounter[6];               //10 ;;;5 6
    quint8  mTighteningStatus[3];           //11 ;;;4 3
    quint8  mBatchStatus[3];                //12
    quint8  mTorqueStatus[3];               //13
    quint8  mAngleStatus[3];                //14
    quint8  mRunDownAngleStatus[3];         //15
    quint8  mCurMonitorStatus[3];
    quint8  mSelftapStatus[3];
    quint8  mPreTorqueMonitorStatus[3];
    quint8  mPreTorqueCompenStatus[3];
    quint8  mTightenErrStatus[12];           //20
    quint8  mTorqueMinLimit[8];
    quint8  mTorqueMaxLimit[8];
    quint8  mTorqueFinalTarget[8];
    quint8  mTorque[8];
    quint8  mAngleMin[7];                   //25
    quint8  mAngleMax[7];
    quint8  mFinalAngle[7];
    quint8  mAngle[7];
    quint8  mRundownAngleMin[7];
    quint8  mRundownAngleMax[7];           //30
    quint8  mRundownAngle[7];
    quint8  mCurMonitorMin[5];
    quint8  mCurMonitorMax[5];
    quint8  mCurMonitorValue[5];
    quint8  mSelfTapMin[8];                  //35
    quint8  mSelfTapMax[8];
    quint8  mSelfTapTorque[8];
    quint8  mPreTorqueMonitorMin[8];
    quint8  mPreTorqueMonitorMax[8];
    quint8  mPreTorque[8];                  //40
    quint8  mTightenID[12];
    quint8  mLinkGroupSeqNum[7];
    quint8  mSyncTightenID[7];
    quint8  mToolSerialNum[16];
    quint8  mTimeStamp[21];                 //45
    quint8  mDateTimeOfLast[21];            //46
    quint8  mAppName[27];
    quint8  mTorqueValueUnit[3];
    quint8  mResultType[4];
    quint8  mIDResultPart2[27];             //50
    quint8  mIDResultPart3[27];
    quint8  mIDResultPart4[27];             //52
    quint8  m53[6];
    quint8  m54[8];
    quint8  m55[12];
    quint8  mGraphType[3]; //56
    quint8  mTorqueScaleFactor[12];
    quint8  mAngleOffset[8];
    quint8  mTimeScale[6];
    quint8  mBinaryOffset[12];
    quint8  mTraceLength[6];
    quint8  mDataBlob[9002];
} T_MID61_DATA_V2;

class TightenOpNetCurve : public QObject
{
    Q_OBJECT
public:
    explicit TightenOpNetCurve(QObject *parent = 0);

signals:
    void sendfromworkthread(QVariant);
    void send_mainwindow(QString,QString,QString,int,int);
    void IsTigntenReady(bool);

private:
    QThread m_thread;
    QTcpSocket *tcpSocket;
    QTimer *timer5000ms;

    int  nexoHandleFlag;             // NC/Time set/set time/system error system/system error system ACK/old data upload/data ACK/data subscribe
                                    // Alive/Job info subscribe/Job ACK/select Job/NC/NC/NC/vin download
                                    // curve Angle subscribe/curve Torque subscribe/curve ACK/71 alarm ack/curve unsubscribe/enable tool/disable tool/tool data
                                    // Tightening program numbers/NC/NC/commucation stop/reset batch counter/set batch size/selected program/commucaiton start
    short programNO;
    short boltCount;
    short boltNokCount;
    int   intReadTighteningID;
    //    int   intNexoTighteningID;
    int   intNexoMaxTighteningID;

    int   nexoAliveCount;
    bool  nexoLinkOk;
    bool  readOldDataFlag;
    bool  nexoStartFlag;
    bool  readMaxTightenIDFlag;
    short nexoReadyFlag;
    bool  powerOnFlag;
    int   revDataLen;
    int   revFinishLen;
    int   curveFirstFlag;
    int   errCount;
    int   errType;
    int   angleSubFlag;
    int   curveDataFlag;
    int   sendVinFlag;
    int   sendNextEnableFlag;
    int   sendDisableFlag;
    QByteArray curveBuf;
    QString curveType;
    //QString curveBufTemp;
    QString idCode;
    QByteArray vinBuf;
    QByteArray boltNumberBuf;

    QByteArray nexoProNO;
    QByteArray nexoResult;
    QByteArray torqueValue;
    QByteArray angleValue;
    QByteArray tighenTime;
    QByteArray tighteningID;
    QByteArray nexoCycleID;
    QByteArray nexoVin;
    QByteArray Channel;

    QByteArray nexoOldTigheningID;
//    QString data_model[16];
    QByteArray strJobID;

    int testNum;
    QString Factory;

    QByteArray recBuf;
    QByteArray recBufBk;
    DATA_STRUCT demo;


private slots:
    void tightenStart();
    void disConnectDO();
    void newConnects();
    void ctlNexoHandle();
    void revNexoData();
    void timerFunc5000ms();
    void sendReadOperate(bool,int);





};
#endif

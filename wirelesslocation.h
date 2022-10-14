#ifndef WIRELESSLOCATION_H
#define WIRELESSLOCATION_H

#include <QObject>
#include <QTimer>
//#include <QDialog>
#include <QThread>
#include <QtNetwork>
#include <qtcpsocket.h>
//#define _MSL_STDINT_H
#include <stdint.h>
#include "analyzelocationdata.h"
#include "locationclient.h"
#include "locationparsejson.h"

#include "./json/parser.h"
#include "./json/serializer.h"
//#include <Eigen/Geometry>
//#include <Eigen/Dense>
////#include <dns_sd.h>
//#include "position.h"
class QTcpServer;
class QTcpSocket;
//class AnalyzeLocationData;
//class LocationClient;

//using namespace Eigen;

class WirelessLocation : public QObject
{
    Q_OBJECT
public:
    explicit WirelessLocation(QObject *parent = 0);

signals:
	void SendCalibrationsatus(int);
    void sendoffsetlist(QVariantList);
private:
    QThread m_thread;
    QTimer *timer;
    QTimer *getStatusTimer;
    QTimer *syncTimer;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QList<uint16_t> anchor_no;
    int anchorNums;
    QList<QList<AnalyzeLocationData*> >analyzeList;
    QMap<int,QMap<QString,int> > coordsMap;
    QList<QList<int> > checkedList;
    QList<QStringList> tag_mac;
//    bool currentIsInside;
    int locationTimes;

//    typedef struct mesgStruct
//    {
//        quint64 mesgTimer;
//        //        uint64_t messMac;
//        int mesgCycle;
//        uint16_t mesgAncNos;
//    }MesgStructs,;

//    QVector<MesgStructs> mesgVector;

    QList<uint16_t> replyList;
    QList<uint16_t> completeList;

//    void analyzeData();
//    void getData(QVector<MesgStructs>);
    void readLocation(QVariantMap);
//    void sendLocation(bool);

    typedef enum
    {
        OSTR_CMD_INFORM,//OSTR 同步信号告知
        OSTR_CMD_REPLY,//OSTR 命令回复
        OSTR_CMD_COMPLETED,//OSTR 同步完成
        OSTR_CMD_EMIT,//OSTR SYNC 同步信号发射 仅对同步器有作用
        OSTR_CMD_REQUEST,//OSTR 同步信号告知
        CMD_HEARTBEAT,//同步器心跳包
        TAG_INFO_UPLOAD,//标签数据上传
        NET_CMD_TWR_START,//目前没用到
        NET_CMD_TWR_REPORT,//...
        NET_CMD_WL_SYNC_REPORT,//无线同步数据上传
        NET_CMD_START_WL_SYNC//开始无线同步,第一版单工位没有用到，全场和后面单工位需要
    }LOC_CMD;
#pragma pack(push,1)
    typedef union{
        struct{
            uint64_t tag_mac; //标签物理地址编号
            uint8_t timestamp[5]; //基站接收到数据的时间戳，40bit 单位：1/ (128*499.2×106)秒.
            uint8_t frame_seq; //sequence number
            uint8_t status; // 标签状态 : bit0-3 （0 - 16） 表示电量级别 0 对应 不支持 电量报告
            //  1 - 12 3.1V - 4.2V
        }TDOA_LOC_INFO;

        struct {
            uint64_t srcAddr;
            uint64_t dstAddr;
            uint16_t distance;
        }TWR_REPORT;
        uint64_t TWR_DST_ADDR;
        struct {
            uint32_t wl_sync_time_span;
            uint16_t anc_no;
        }WL_SYNC_START;
        struct {
            uint16_t srcAddr;
            uint8_t send_ticks[5]; //基站接收到数据的时间戳，40bit 单位：1/ (128*499.2×106)秒.
            uint8_t recv_ticks[5];
            uint8_t seqNum;
        }WL_SYNC_REPORT;
    }NETFRAME_DATA;

    typedef struct _NETWORK_FRAME{
        uint8_t header[2];//固定字
        //	uint8_t header[1] = 0x55//固定字
        uint8_t version; //协议版本
        uint8_t subversion; //协议子版本
        uint32_t cmd;//命令字 0 是数据 1是ostr回应
        uint8_t length; //帧长度
        uint16_t anchor_no; //基站编号
        NETFRAME_DATA data;
        uint32_t sum_Check; //校验和
    }NETWORK_FRAME;
#pragma pack(pop)
    void sendCmd(LOC_CMD,uint16_t);

//    int64_t Offset[5];
    QList<int64_t> Offset;
//    QTimer *calibrationTimer;

    qint64 calibrationArray[10][10][256];
//    QMap<QString,QMap<QString,TimestampArray> > calibrationMap;

    void sendData(NETWORK_FRAME *);
//    int coords[5][3];
//    int area[4][2];

    int64_t averaging(int64_t *delay, int length);
    uint64_t distance(int x1, int y1, int z1, int x2, int y2, int z2);
    int currentAnchor;
    bool isSync;
    int cal_fail_cnt;//add lw 2017/10/30  标定失败次数统计
//    int tagBattery[4];
    QStringList DataIP;
    QList<LocationClient*> locationClients;
    void saveOffSet(QVariantList);
	bool Cell_connectsuccess;
    QVariantList offsetList;
private slots:
    void receiveConnect();
    void disConnectDo();
    void readMessage();
    void serverSendHeart();
    void calibration();
    void sendStartSync();
    void getTagStatus();
    void reSync();
	void Calibration_cell();
public slots:
    void initLocation();

};

#endif // WIRELESSLOCATION_H

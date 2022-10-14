#ifndef ANALYZELOCATIONDATA_H
#define ANALYZELOCATIONDATA_H

#include <QObject>
#include <stdint.h>
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <QTimer>
#include <QTime>
//#include <dns_sd.h>
#include "position.h"
#include "CurrentStatus.h"
#include "kalman.h"

using namespace Eigen;

class MesgStructs
{
public:
    quint64 mesgTimer;
    //        uint64_t messMac;
    int mesgCycle;
    uint16_t mesgAncNos;
    uint8_t status;
};

class AnalyzeLocationData : public QObject
{
    Q_OBJECT
public:
    explicit AnalyzeLocationData(QObject *parent = 0);
    void setOffset(QList<int64_t>);
    void setInit(QList<uint16_t>,int,QMap<int,QMap<QString,int> >,int,int);
    int area[4][2];
    typedef struct _DIFFTIME
    {
        MatrixXf dftm;
        time_t t;
    }_DIFFTIME;
    void revStruct(MesgStructs);
    CurrentStatus getcurrentStatus();
signals:
//    void tagconnect(bool);
private slots:
    void Losedata();
private:
    QVector<MesgStructs> mesgVector;
    void analyzeData();
    void getData(QVector<MesgStructs>);
    void sendLocation(bool );
//    int coords[5][3];
//    uint16_t anchor_no[5];
    QList<int64_t> Offset;

    QList<uint16_t> anchor_no;
    int anchorNums;
    QMap<int,QMap<QString,int> > coordsMap;
    int locationTimes;
    bool currentIsInside;
    QPointF co_ordinate;//add lw 2017/8/21
    QPointF co_ordinate2;//add lw 2017/8/21
    QPointF co_ordinate3;//add lw 2017/8/21
    QPointF CorA;
    QPointF CorB;
    QPointF CorC;
    QPointF CorD;
    uint8_t status;
    int leastNum;
    QTimer *timertag;//add lw 2017/9/19
    uint32_t tag_timeout;
    int cycle_no;
    int tag_incount;
    int tag_outcount;
    bool tagconnect_flag ;
    double th ;
    double th2 ;
    double th3 ;
    int cnt ;
    int isInside1count;
	bool isInside;
    KalmanInfo *kalmanInfox;
    KalmanInfo *kalmanInfoy;


    int smoothnum;
    QPointF smoothbuffer20[20];
    bool startsmoothflag;
    QPointF smooth20(QPointF a, int points);
    //QPointF smooth201(QPointF a,bool inside2,bool last_inside);
    QPointF smooth201(QPointF a,int points,bool inside2);
    MatrixXf smoothdifftime(MatrixXf  a);

};

#endif // ANALYZELOCATIONDATA_H

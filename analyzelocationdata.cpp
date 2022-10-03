#include "analyzelocationdata.h"
#include "GlobalVarible.h"
#include "qmath.h"
#include "time.h"
#include <QFile>
#include <QString>
AnalyzeLocationData::AnalyzeLocationData(QObject *parent) :
    QObject(parent)
{
    cycle_no=25;
    anchorNums = 0;
    status = 0;
    leastNum = 5;    //至少是4，最好是5，以后会在界面上配置 add lw 2017/8/21
    locationTimes = 0;
    smoothnum = 0;
    currentIsInside = true;
    tagconnect_flag = false;
	isInside = false;
    startsmoothflag = false;
    timertag = new QTimer(this);//add lw 2017/9/19 标签接收数据超时定时器
    tag_timeout =300;//标签超时时间
    tag_incount = 1;//标签进入区域的有效次数
    tag_outcount = 5;//标签出区域的有效次数
    th = 4000;      //add lw 2017/10/30有效区域
    th2 = 5000;      //add lw 2017/10/30第一次跳动阈值
    th3 = 3000;      //add lw 2017/10/30数据集中小于阈值保留数据显示
    cnt = 2 ;
    isInside1count = 0;//超出有效区域次数
    //卡尔曼滤波初始化
    kalmanInfox = new KalmanInfo;
    kalmanInfoy = new KalmanInfo;
    Init_KalmanInfo(kalmanInfox,0.5,0.5);
    Init_KalmanInfo(kalmanInfoy,0.5,0.5);

    connect(timertag,SIGNAL(timeout()),this,SLOT(Losedata()));

}

void AnalyzeLocationData::setOffset(QList<int64_t> tempOffsetList)
{
    Offset = tempOffsetList;
}

void AnalyzeLocationData::setInit(QList<uint16_t> a, int b, QMap<int, QMap<QString, int> > c, int d,int e)
{
    anchor_no = a;
    anchorNums = b;
    coordsMap = c;
    if(d!=0)
    leastNum = d;
    if(e!=0)
    tag_timeout =e;
}

void AnalyzeLocationData::revStruct(MesgStructs dataStruct)   //收到一个数据就先存起来
{
    if(timertag->isActive())//add lw 2017/9/19
        timertag->stop();

    status = dataStruct.status;
    mesgVector.push_back(dataStruct);
    analyzeData();
    tagconnect_flag =true;
    timertag->start(tag_timeout*1000);//add lw 2017/9/19
}

void AnalyzeLocationData::analyzeData()   //分析是否已经可以计算坐标
{
    QMap<int,int> cycleMap;

    foreach (MesgStructs dataStruct, mesgVector)
    {
        if(cycleMap.contains(dataStruct.mesgCycle))
            cycleMap[dataStruct.mesgCycle] = cycleMap[dataStruct.mesgCycle]+1;
        else
            cycleMap[dataStruct.mesgCycle] = 1;
    }
    //qDebug()<<"cycleMap"<<cycleMap;

    int cycle = -1;
    QMap<int,int>::iterator i;
    for(i = cycleMap.begin(); i!=cycleMap.end(); ++i)
    {
        if(i.value() >= leastNum)   //如果同一个循环号收到大于等于配置的可以计算坐标基站数，即方程个数，就开始计算
            cycle = i.key();
    }
    //  qDebug()<< "cycle" << cycle;

    if(cycle != -1)
    {
        QVector<MesgStructs> needVector;
        QVector<MesgStructs> restVector;
        foreach (MesgStructs dataStruct, mesgVector)
        {
            if(dataStruct.mesgCycle == cycle)
                needVector.push_back(dataStruct);   //把可以计算的循环号对应的值先存起来，之前收到的循环号的信息可以舍弃了
            else if(cycle-dataStruct.mesgCycle>0 || cycle-dataStruct.mesgCycle < -254)
                ;
            else
                restVector.push_back(dataStruct);
        }
        mesgVector = restVector;
        //        qDebug()<<"mesgVector"<<mesgVector.size();
        getData(needVector);
    }
    else
        if(mesgVector.size()>cycle_no)  //一直不能计算
        {
            mesgVector.remove(0);

           // qDebug()<<"cycleMap"<<cycleMap;//add lw 2017/9/19
            qDebug()<<"***************Tag connect faild!*************** ";//add lw 2017/9/19
//            locationstation = 5;
//            currentIsInside = false;//断开使能不变
//            emit tagconnect(false);
            locationTimes = 0;
        }

}
VectorXf convQPoint2VectorXf(QPointF P )
{
    VectorXf res = VectorXf::Zero(2);
    res[0] = P.x() ;    res[1] = P.y();
    return res;

}
//static time_t last_recv_tick = 0 ,curr_tick = 0 ;
void AnalyzeLocationData::getData(QVector<MesgStructs> needVector)  //开始计算坐标
{
    //bool inSide2;
    qint64 timeStamp[anchorNums];
    //inSide2 = false;
    memset(timeStamp,0, sizeof(timeStamp));

    foreach (MesgStructs dataStruct, needVector)
    {
        //        qDebug()<<dataStruct.mesgAncNos<<dataStruct.mesgCycle<<dataStruct.mesgTimer;
        bool isMyAnchor_no = false;
        int i;
        for(i=0; i<anchorNums; i++)
        {
            if(dataStruct.mesgAncNos == anchor_no[i])
            {
                isMyAnchor_no = true;
                break;
            }
        }
        if(isMyAnchor_no)
        {
            timeStamp[i]=dataStruct.mesgTimer; //每个基站的时间戳
        }
    }
    
    int nums = 0;
    int min = 0;
    for(int i=0;i<anchorNums;i++)
    {
        if(timeStamp[i] != 0)
        {
            if(nums == 0)
                min = i;
            nums++;
        }
        //qDebug()<<"timeStamp"<<i<<timeStamp[i];
    }
    if(nums<leastNum)    //如果时间戳的数量小于设置的最小值
        return;
    MatrixXf l = MatrixXf::Zero(leastNum,3);
    
    int n=0;
    for(int i=0; i<anchorNums; i++)   //返回时间戳的基站对应的坐标组成一个矩阵
    {
        if(timeStamp[i] != 0)
        {
            QMap<QString,int> coords = coordsMap[anchor_no[i]];
            l(n,0) = coords["x"];
            l(n,1) = coords["y"];
            l(n,2) = coords["z"];
            n++;
        }
    }
    //    l<< 10000,	0,	0,
    //            0,	10000,	0,
    //            0,	0,	10000,
    //            10000,	10000,	0,
    //            5000,	5000,	0;
    //    l<< 0,	0,	0,
    //            0,	10000,	0,
    //            10000,	10000,	0,
    //            10000,	0,	0,
    //            5000,	5000,	0;
    //cout <<"l"<< l << endl;

    //    MatrixXf m(5,1);
    //    for(int i=0;i<5;i++)
    //    {
    //        m(i,0) = timeStamp[i];
    //    }

    //    cout << m << endl;
    //    m << 2.219790194736379e+03,
    //         2.018368328234236e+03,
    //         1.139822608672597e+03,
    //         2.700552250116151e+03,
    //         1.492600168456572e+03;
    
    //    MatrixXf diffTime(4,1);
    //    diffTime<<timeStamp[1]-timeStamp[0]-Offset[1],
    //            timeStamp[2]-timeStamp[0]-Offset[2],
    //            timeStamp[3]-timeStamp[0]-Offset[3],
    //            timeStamp[4]-timeStamp[0]-Offset[4];
    MatrixXf diffTime = MatrixXf::Zero(leastNum-1,1);  //每个基站和最小的收到时间戳的基站 两两之间的时间差组成矩阵
    int m = 0;
    for(int i=(min+1);i<anchorNums;i++)
    {
        if(timeStamp[i] != 0)
        {
            int Off = 0;
            for(int k=(min+1);k<i+1;k++)
                Off = Off+ Offset[k];
            diffTime(m,0) = timeStamp[i]-timeStamp[min]-Off;
            m++;
        }
    }
    //    diffTime<<timeStamp[1]-timeStamp[0]-Offset[1],
    //            timeStamp[2]-timeStamp[0]-Offset[2],
    //            timeStamp[3]-timeStamp[0]-Offset[3],
    //            timeStamp[4]-timeStamp[0]-Offset[4];
    //cout <<"diffTime"<< diffTime << endl;

    diffTime = smoothdifftime(diffTime);
    Position position;// = new Position;
    MatrixXf p = MatrixXf::Zero(3,1);
    
    p=position.GetPosition(l,leastNum,diffTime);  //计算坐标
    //cout << "difftime:" <<  diffTime << std::endl ;//test
    //add lw 2017/10/30
	double speed = 299792458 / (128 * 499.2 * 1e6) * 1000;
	MatrixXf p2 = position.Taylor_Solve(l,diffTime*speed , p );//泰勒优化
    p = position.Solve_value(l,leastNum,diffTime*speed,p2,p);

    //cout << "difftime:" << diffTime << "result:" << p << std::endl ;//test

    QPointF P(p(0,0),p(1,0));

    //    QPointF a(l(0,0),l(0,1));
    //    QPointF b(l(1,0),l(1,1));
    //    QPointF c(l(2,0),l(2,1));
    //    QPointF d(l(3,0),l(3,1));

    QPointF a(area[0][0],area[0][1]);
    QPointF b(area[1][0],area[1][1]);
    QPointF c(area[2][0],area[2][1]);
    QPointF d(area[3][0],area[3][1]);

    CorA = a;
    CorB = b;
    CorC = c;
    CorD = d;
 //卡尔曼滤波
    kalmanInfox->filterValue=P.x();
    kalmanInfoy->filterValue=P.y();
    P.setX( KalmanFilter(kalmanInfox, co_ordinate.x()));
    P.setY( KalmanFilter(kalmanInfoy, co_ordinate.y()));

 //限幅滤波
//    //跳动的点数距离大于5米，去掉这个点
//    // co_ordinate3上一次的点           P本次的点          co_ordinate原始点
//    qDebug()<<"	original data:"<<P.x()<<":"<<P.y();
//    //本次点与上次点做比较
//    VectorXf diff = convQPoint2VectorXf( co_ordinate3 - P );
//    float dis = sqrt(diff.transpose()*diff);
//    //本次点与显示点作比较
//    VectorXf diff1 = convQPoint2VectorXf( co_ordinate - P );
//    float dis1 = sqrt(diff1.transpose()*diff1);

//     if((dis1>5000)&& (curr_tick -last_recv_tick < 10 ))
//     {
//         if((co_ordinate.x()!=0)&&(co_ordinate.y()!=0))
//         {
//             if(dis<3000)
//             {
//                 cnt++;
//                 if(cnt>5)
//                 {
//                     cnt = 0;
//                     co_ordinate3  = P;
//                 }
//                 else
//                 {
//                    co_ordinate3  = P;
//                    P = co_ordinate;
//                 }
//             }
//             else
//             {
//                 co_ordinate3  = P;
//                  P = co_ordinate;
//                 cnt =0;
//             }
//         }
//     }
//     else
//     {
//         cnt =0;
//         co_ordinate3  = P;
//     }
//    qDebug()<<"	clear 5m data:"<<P.x()<<":"<<P.y();

 //渐进滤波
//    // Y = Y + (X - Y)/C;
//    P.x()=co_ordinate.x()+(P.x()-co_ordinate.x())/2;
//    P.y()=co_ordinate.y()+(P.y()-co_ordinate.y())/2;


 //开始平滑滤波
//         P = smooth20(P,8,inSide2);
//         P = smooth20(P,inSide2,isInside);
//         P = smooth20(P,5);

         co_ordinate = P;
         qDebug()<<"Display data:"<<P.x()<<":"<<P.y();
        isInside = position.pInQuadrangle(a,b,c,d,P) ;  //判断是否在工位区域内
         sendLocation(isInside);


//         //test
//       QDateTime time = QDateTime::currentDateTime();
//         QString date = time.toString("yyyy-MM-dd hh:mm:ss:zzz ddd");
////         QFile file(QString("/etc/difftime")+date"));
//         QString str ;
//         str +=date+":";
//         str += "difftime:" ;
//         for(int i = 0 ; i < diffTime.rows() ; i++)
//         {
//             str+= QString::number(diffTime(i,0));
//             str += "," ;
//         }
//         str += "result p:" ;
//         for(int i = 0 ; i < p.rows(); i++)
//         {
//             str+=QString::number(p(i,0));
//             str += "," ;
//         }
//         str += "result P:" ;
//         str+=QString::number(P.x());
//         str += "," ;
//         str+=QString::number(P.y());
//         str += "," ;

//         str += "Display P:" ;
//         str+=QString::number(co_ordinate.x());
//         str += "," ;
//         str+=QString::number(co_ordinate.y());
//         str += "," ;
//         str += "\n" ;

//         QFile f("/etc/difftime");
//         f.open(QIODevice::Append);
//         f.write(str.toAscii());
//         f.close();

//    }
//end add lw
}

void AnalyzeLocationData::sendLocation(bool isInside)   //判断是否一定次数内都连续在区域内或者外 add lw 2017/8/21 P
{
    if(isInside)
    {
        if(!currentIsInside)
        {
            locationTimes++;
            if(locationTimes >= tag_incount)    //次数  可以搞个常量，需要调试  == ->  >=  add lw 2017/9/19
            {
                currentIsInside = true;
                locationTimes = 0;
                qDebug() << "tag inside sendEnable success!";
            }
        }
        else
            locationTimes = 0;
    }
    else
    {
        if(currentIsInside)
        {
            locationTimes++;
            tag_outcount = tag_timeout;//Ui _para timeout
            if(locationTimes >= tag_outcount)//== ->  >=  add lw 2017/9/19
            {
                currentIsInside = false;
                locationTimes = 0;
                qDebug() << "tag outside sendEnable false!";
            }
        }
        else
            locationTimes = 0;
    }
}


CurrentStatus AnalyzeLocationData::getcurrentStatus()    //返回标签的状态信息
{
    CurrentStatus currentStatus;
    currentStatus.isInside = currentIsInside;
    currentStatus.powerStatus = status;
    currentStatus.P = co_ordinate;
    currentStatus.A = CorA;
    currentStatus.B = CorB;
    currentStatus.C = CorC;
    currentStatus.D = CorD;
    currentStatus.Label_Time_out = tagconnect_flag;
    return currentStatus;
}

//add lw 2017/9/19

void AnalyzeLocationData::Losedata()
{
    status = 0;
    tagconnect_flag = false;
    currentIsInside = true;//不切断枪使能 add 20180117
    qDebug()<<"+++++++++Tag data timeout,close cor display!+++++++++";
}

QPointF AnalyzeLocationData::smooth20(QPointF a,int points)
{
    float a_x_sum = 0;
    float a_y_sum = 0;
    QPointF temp_a;

//        if((!isInside)&&(fflag_inside))
//        {
//            memset(smoothbuffer20,0,points);
//        }

    if(smoothnum >= points)
    {
       smoothnum = 0 ;
       //开始Points点平滑
       startsmoothflag = true;
    }
    smoothbuffer20[smoothnum]  = a;
    smoothnum++;

    if(startsmoothflag)
    {
        for(int i=0;i<points;i++)
        {
            a_x_sum+=smoothbuffer20[i].x();
            a_y_sum+=smoothbuffer20[i].y();
        }
        a_x_sum = a_x_sum/points;
        temp_a.setX(a_x_sum);
        a_y_sum = a_y_sum/points;
        temp_a.setY(a_y_sum);
    }
    else
    {
        for(int i=0;i<smoothnum;i++)
        {
            a_x_sum+=smoothbuffer20[i].x();
            a_y_sum+=smoothbuffer20[i].y();
        }
        a_x_sum = a_x_sum/smoothnum;
        temp_a.setX(a_x_sum);
        a_y_sum = a_y_sum/smoothnum;
        temp_a.setY(a_y_sum);
    }
    return temp_a;
}

QVector< AnalyzeLocationData::_DIFFTIME> pp;

MatrixXf AnalyzeLocationData::smoothdifftime(MatrixXf  a)
{
    _DIFFTIME dt ;
    time_t t ;
    time(&t);
    dt.dftm = a ;
    dt.t = t ;
    MatrixXf res = MatrixXf::Zero(a.rows(),1);
    float weight_sum = 0 ;
    float weight = 0;
    if(pp.size()>20)
        pp.pop_front();
    pp.push_back(dt);
    for(int i = 0 ; i< pp.size() ; i++ )
    {
        if(t - pp[i].t < 10)
            weight = 1.0f;
        else
            weight = 1.0f / (t - pp[i].t+1);
        res += dt.dftm*weight ;
        weight_sum  += weight ;
    }
    res /= weight_sum;

    return res;
}


//typedef struct smooth_para{
//    QPointF a;
//    int inside;
//}_smooth_para;
//QVector <smooth_para> smooth20_para;
//QPointF AnalyzeLocationData::smooth201(QPointF a,bool inside2,bool last_inside)
//{
//    float a_x_sum = 0;
//    float a_y_sum = 0;
//    QPointF temp_a;
//    smooth_para sp ;
////    if(!currentIsInside)//一定次数内
////    {
////        smoothnum = 0 ;
////        //开始Points点平滑
////        startsmoothflag = false;
////    }
//    sp.a = a ;
//    sp.inside = inside2 ;

//    if(smoothnum >= 20)
//    {
//       smoothnum = 0 ;
//       //开始Points点平滑
//       startsmoothflag = true;
//        smooth20_para.pop_front();
//    }
//    smooth20_para.push_back(sp);
//    smoothnum++;

//    if(startsmoothflag)
//    {
//        for(int i=0;i<smooth20_para.size();i++)
//        {
//            a_x_sum+=smooth20_para[i].a.x();
//            a_y_sum+=smooth20_para[i].a.y();
//        }
//        a_x_sum = a_x_sum/20;
//        temp_a.setX(a_x_sum);
//        a_y_sum = a_y_sum/20;
//        temp_a.setY(a_y_sum);
//    }
//    else
//    {
//        for(int i=0;i<smoothnum;i++)
//        {
//            a_x_sum+=smooth20_para[i].a.x();
//            a_y_sum+=smooth20_para[i].a.y();
//        }
//        a_x_sum = a_x_sum/smoothnum;
//        temp_a.setX(a_x_sum);
//        a_y_sum = a_y_sum/smoothnum;
//        temp_a.setY(a_y_sum);
//    }
//    if(!last_inside&&inside2)
//    {
//         //for (std::vector<smooth_para *>::iterator i = smooth20_para.begin(); i != smooth20_para.end(); i++)
//        for(int i = smooth20_para.size()-1;i>=0 ; i--)
//        {
//            if(!smooth20_para[i].inside)
//                 smooth20_para.remove(i);
//        }
//    }
//    return temp_a;
//}


typedef struct smooth_para{
    QPointF a;
    int inside;
}_smooth_para;
QVector <smooth_para> smooth20_para;
QPointF AnalyzeLocationData::smooth201(QPointF a,int points,bool inside2)
{
    float a_x_sum = 0;
    float a_y_sum = 0;
    QPointF temp_a;
    smooth_para sp ;

    sp.a = a ;
    sp.inside = inside2 ;

    if(smoothnum >= points)
    {
       smoothnum = 0 ;
       //开始Points点平滑
       startsmoothflag = true;
       smooth20_para.pop_front();
    }
    smooth20_para.push_back(sp);
    smoothnum++;

    // if(startsmoothflag)
    // {
        int weight_sum = 0,weight = 0;
        for(int i=0;i<smooth20_para.size();i++)
        {
            if(smooth20_para[i].inside)
            {
                weight = 1;
            }
            else
            {
                weight = 1;
            }
            a_x_sum+=smooth20_para[i].a.x()*weight;
            a_y_sum+=smooth20_para[i].a.y()*weight;
            weight_sum+=weight;
         }
        a_x_sum = a_x_sum/weight_sum;
        temp_a.setX(a_x_sum);
        a_y_sum = a_y_sum/weight_sum;
        temp_a.setY(a_y_sum);
    // }
    // else
    // {
        // for(int i=0;i<smoothnum;i++)
        // {
            // a_x_sum+=smooth20_para[i].a.x();
            // a_y_sum+=smooth20_para[i].a.y();
        // }
        // a_x_sum = a_x_sum/smoothnum;
        // temp_a.setX(a_x_sum);
        // a_y_sum = a_y_sum/smoothnum;
        // temp_a.setY(a_y_sum);
    // }
    // if(!last_inside&&inside2)
    // {
         // for (std::vector<smooth_para *>::iterator i = smooth20_para.begin(); i != smooth20_para.end(); i++)
        // for(int i = smooth20_para.size()-1;i>=0 ; i--)
        // {
            // if(!smooth20_para[i].inside)
                 // smooth20_para.remove(i);
        // }
    // }
    return temp_a;
}

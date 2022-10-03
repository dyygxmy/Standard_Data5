#include "wirelesslocation.h"
//const double lightspeed = 299792458 ;
const double speed = 299792458 / (128 * 499.2 * 1e6) * 1000;
int locationstation = 0;//add lw 2017/10/30 0：网络断开  1：基站连通  2：标定成功  3：标定失败   4.正常定位数据  5.标签离开工位超时
WirelessLocation::WirelessLocation(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(&m_thread);
    m_thread.start();
}

void WirelessLocation::initLocation()   //初始化，读配置
{
    anchorNums = 0;
 	Cell_connectsuccess = false;//基站是否连接
    isSync = false;   //是否已经计算标定过
    //    memset(tagBattery,0,sizeof(tagBattery));
    //    memset(Offset, 0, sizeof(Offset));
    memset(calibrationArray,0,sizeof(calibrationArray));
    LocationParseJson * locationParseJson = new LocationParseJson;
    QVariantMap currentLocation = locationParseJson->parseJson();
    delete locationParseJson;
    if(currentLocation.value("errorCode")!=1)
        readLocation(currentLocation);
    //    initCoord();
    //    currentIsInside = true;
    locationTimes = 0;
        locationstation = 0;//add lw 2017/10/30
    cal_fail_cnt = 0;//add lw 2017/10/30
    timer = new QTimer(this);
    //    calibrationTimer = new QTimer(this);
    tcpServer = new QTcpServer(this);

    if (!tcpServer->listen(QHostAddress::Any, 5000)) {
        qDebug() << tcpServer->errorString();
        qDebug()<<"Cell listen fail";
    }
    else
        qDebug()<<"Cell listen success";
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(receiveConnect()));
    connect(timer,SIGNAL(timeout()),this,SLOT(serverSendHeart()));   //定时发心跳
    getStatusTimer = new QTimer(this);
    connect(getStatusTimer,SIGNAL(timeout()),this,SLOT(getTagStatus()));    //定时把数据交给客户端，客户端再转交给对应的Data的服务端
    syncTimer = new QTimer(this);
    connect(syncTimer,SIGNAL(timeout()),this,SLOT(reSync()));
}


void WirelessLocation::readLocation(QVariantMap currentLocation)   //把读到的配置存到相应变量中
{
    int i = 0;
    int leastNum = currentLocation["leastNum"].toInt();
    uint32_t tag_timeout = currentLocation["tag_timeout"].toInt();
    foreach (QVariant anchor, currentLocation["anchors"].toList()) {
        QVariantMap anchorMap = anchor.toMap();   //所有基站编号
        bool ok;
        anchor_no.append(anchorMap["anchor_no"].toByteArray().toInt(&ok,16));
        QMap<QString,int> coords;
        coords.insert("x",anchorMap["x"].toInt());
        coords.insert("y",anchorMap["y"].toInt());
        coords.insert("z",anchorMap["z"].toInt());
        coordsMap.insert(anchor_no[i],coords);   //所有基站坐标
        QStringList checked = anchorMap["checked"].toString().split(":",QString::SkipEmptyParts);
        //        qDebug()<<"checked"<<checked;
        QList<int> checkedTmp;
        for(int j=0;j<checked.length();j++)
        {
            checkedTmp.append(checked[j].toInt());
        }
        //        qDebug()<<"checkedTmp"<<checkedTmp;
        checkedList.append(checkedTmp);   //所有基站标定时用到的其他基站  eg 0:3:5
        i++;
    }
    //    qDebug()<<"checkedList"<<checkedList<<checkedList.size();
    anchorNums = anchor_no.size();   //基站个数
    i=0;
    foreach (QVariant data, currentLocation["datas"].toList()) {
        QVariantMap dataMap = data.toMap();
        // qDebug()<<"LW add !";
        // qDebug()<<"dataMAP"<<dataMap<<dataMap.size();//add lw 2017/8/21
        DataIP.append(dataMap["IP"].toString());   //Data Ip
        LocationClient *locationClient = new LocationClient;
        locationClient->setIP(DataIP[i]);
        locationClient->Init();
        locationClients.append(locationClient);   //有几台Data建立几个客户端
        int area[4][2];   //定位区域坐标
        for(int k=0;k<4;k++)
        {
            area[k][0] = dataMap["x"+QString::number(k)].toInt();
            area[k][1] = dataMap["y"+QString::number(k)].toInt();
        }
        QPointF a(area[0][0],area[0][1]);//aa
        QPointF b(area[1][0],area[1][1]);
        QPointF c(area[2][0],area[2][1]);
        QPointF d(area[3][0],area[3][1]);
        QStringList tmpTag_mac;
        QList<AnalyzeLocationData*> tmpAnalyzeList;

        foreach (QVariant tag_mac, dataMap["tag_mac"].toList())
        {
            QString tagMac = tag_mac.toString();
            tmpTag_mac.append(tagMac);  //一个标签的MAC
            AnalyzeLocationData *analyzeLocationData = new AnalyzeLocationData;   //每台Data有几个标签new几个计算坐标的对象
            //            connect(analyzeLocationData,SIGNAL(tagconnect(bool)),this,SLOT(ReceiveTimeout(bool)),Qt::AutoConnection);//add lw 2017/9/19
            analyzeLocationData->setInit(anchor_no,anchorNums,coordsMap,leastNum,tag_timeout);
            for(int k=0;k<4;k++)
            {
                analyzeLocationData->area[k][0] = area[k][0];
                analyzeLocationData->area[k][1] = area[k][1];
            }
            tmpAnalyzeList.append(analyzeLocationData);  //一个标签的对象
        }
        tag_mac.append(tmpTag_mac);  //整台Data的标签的MAC
        analyzeList.append(tmpAnalyzeList);  //整台Data的标签的对象
        i++;//add lw 2017/8/21
        qDebug()<<"new CLient count"<<i;
    }

     offsetList = currentLocation["offsetList"].toList();
    if(offsetList.size()==anchorNums)
    {
		 locationstation=1;//已标定
        isSync = true;
        for(int i=0;i<offsetList.size();i++)
		 {
               Offset.append(offsetList[i].toInt());
                qDebug()<<Offset[i];
         }
    }
    else//长度大于有错误
    {
        offsetList.clear();
        locationstation=0;//未标定
    }
       for(int i=0;i<analyzeList.size();i++)
       {
           for(int j=0;j<analyzeList[i].size();j++)
               analyzeList[i][j]->setOffset(Offset);  //标定信息发给每个标签对应的对象
       }
    //    Offset[1]=1001;
    //    Offset[2]=3994;
    //    Offset[3]=4261;
    //    Offset[4]=3751;

}

void WirelessLocation::serverSendHeart()   //send message to client
{
    sendCmd(CMD_HEARTBEAT,anchor_no[0]);   //发送心跳给基站
}

void WirelessLocation::disConnectDo()    //client disconnect0  和基站断开连接
{
    qDebug()<<"5000 is disconnected!!!";
    timer->stop();   //停止发心跳
	Cell_connectsuccess = false;
    //    tcpSocket->deleteLater();
}

void WirelessLocation::receiveConnect()   //receive new connect
{
    qDebug() << "5000 port  ************ Cell connect success";
//    isSync = false;
//    Offset.clear();
//    memset(calibrationArray,0,sizeof(calibrationArray));
	Cell_connectsuccess = true;
    tcpSocket = tcpServer->nextPendingConnection();
    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readMessage()));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disConnectDo()));

//    sendCmd(OSTR_CMD_INFORM,anchor_no[0]);
    if(!syncTimer->isActive())
        syncTimer->start(10000);

    if(!timer->isActive())
        timer->start(500);
}

void WirelessLocation::readMessage()   //hand message  接收到基站发的数据，存到结构体中
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray receiveMessage = socket->readAll();
    //    qDebug()<<"recv:"<<receiveMessage.toHex();
    //    qDebug()<<"recv:"<<receiveMessage.toHex().mid(50,2);

    QByteArray head;
    head.resize(2);
    head[0] = 0xAA;
    head[1] = 0x55;

    while((uint)receiveMessage.size()>=sizeof(NETWORK_FRAME))
    {
        while( (!receiveMessage.isEmpty())&&(!receiveMessage.startsWith(head)) )  //如果不是以aa55开头的，删掉
        {
            //            qDebug()<<"location receive not Start with "<<head.toHex();
            receiveMessage.replace(0,1,"");
        }
        //        if(locationstation == 0)
        //            locationstation = 1;//add lw 2017/10/30

        if((uint)receiveMessage.size()>=sizeof(NETWORK_FRAME))
        {
            NETWORK_FRAME  networkFrame ;
            memcpy(&networkFrame,receiveMessage.data(),sizeof(NETWORK_FRAME));
            receiveMessage = receiveMessage.right(receiveMessage.size()-sizeof(NETWORK_FRAME));   //可能拼帧，剩下的后面继续处理

            if(networkFrame.cmd == OSTR_CMD_REQUEST)
            {
				 isSync = false ;
                 Offset.clear();
                 currentAnchor = 0;
                 memset(calibrationArray,0,sizeof(calibrationArray));
                sendCmd(OSTR_CMD_INFORM,anchor_no[0]);
            }
            else if(networkFrame.cmd == OSTR_CMD_REPLY)
            {
                if(!replyList.contains(networkFrame.anchor_no))
                    replyList.append(networkFrame.anchor_no);
                if(replyList.size() >= anchorNums)   //收到所有基站回复才给它发命令
                {
                    sendCmd(OSTR_CMD_EMIT,anchor_no[0]);
                    replyList.clear();
                }
            }
            else if(networkFrame.cmd == OSTR_CMD_COMPLETED)
            {
                if(isSync)
                    return;    //如果已经标定过了返回
                if(!completeList.contains(networkFrame.anchor_no))
                    completeList.append(networkFrame.anchor_no);
                if(completeList.size() >= anchorNums)
                {
                    completeList.clear();                           //宁波旧基站需要把下面四行注掉
                    currentAnchor = 0;
                  //  isSync = true;     //?????????????????
                    sendStartSync();  //开始标定，发送开始同步信号


                    //    Offset[1]=1001;
                    //    Offset[2]=3994;
                    //    Offset[3]=4261;
                    //    Offset[4]=3751;
                    //                    Offset.clear();                  //宁波旧基站把这七行注释打开，这是上次标定的，不知道对不对
                    //                    Offset.append(0);
                    //                    Offset.append(1001);
                    //                    Offset.append(2993);
                    //                    Offset.append(267);
                    //                    Offset.append(-510);
                    //                    syncTimer->stop();
                }
            }
            else if(networkFrame.cmd == NET_CMD_WL_SYNC_REPORT)   //接收到标定数据，都先存到数组中，之后一起计算
            {
                union mesg
                {
                    quint64 Stime;
                    uint8_t StimeTemp[5];
                }recv = {0};

                for(int i = 0;i<5;i++)
                {
                    recv.StimeTemp[i] = networkFrame.data.WL_SYNC_REPORT.recv_ticks[i];
                    //                    send.StimeTemp[i] = networkFrame.data.WL_SYNC_REPORT.send_ticks[i];
                }

                bool isMysrcAddr = false;
                bool isMyAnchor_no = false;
                int i,j;
                for(i=0; i<anchorNums; i++)   //发送标定信号的基站
                {
                    if(networkFrame.data.WL_SYNC_REPORT.srcAddr == anchor_no[i])
                    {
                        isMysrcAddr = true;
                        break;
                    }
                }
                for(j=0; j<anchorNums; j++)   //接收标定信号的基站
                {
                    if(networkFrame.anchor_no == anchor_no[j])
                    {
                        isMyAnchor_no= true;
                        break;
                    }
                }
                if(isMysrcAddr && isMyAnchor_no)   //都是已经配置过的基站
                {
                    //                    qDebug()<<"calibrationArray"<<i<<j<<networkFrame.data.WL_SYNC_REPORT.seqNum<<recv.Stime;
                    calibrationArray[i][j][networkFrame.data.WL_SYNC_REPORT.seqNum] = recv.Stime; //   对应的循环号里存接收时间戳
                    //                    qDebug()<<calibrationArray[i][j][networkFrame.data.WL_SYNC_REPORT.seqNum];
                }
                else
                {
                    //                    qDebug()<<"receive WL_SYNC_REPORT wrong "<<networkFrame.data.WL_SYNC_REPORT.srcAddr<<networkFrame.anchor_no<<networkFrame.data.WL_SYNC_REPORT.seqNum;
                }
            }
            else if(networkFrame.cmd == TAG_INFO_UPLOAD)   //标签的信息
            {
                if(Offset.size()== 0)  //没标定时舍弃
                    return;

                QByteArray tempMac = QByteArray::fromRawData((char *)&networkFrame.data.TDOA_LOC_INFO.tag_mac,sizeof(networkFrame.data.TDOA_LOC_INFO.tag_mac));
                QByteArray mac = "";
                for(int i=(tempMac.size()-1);i>=0;i--)
                    mac = mac + tempMac[i];   //解析出来的MAC地址
                //                bool isTag = false;
                //                qDebug()<<"mac"<<mac.toHex();
                

                for(int j=0;j<tag_mac.size();j++)
                {
                    for(int k=0;k<tag_mac[j].size();k++)
                    {
                        if( mac.toHex() == tag_mac[j][k].toLower())  //判断是否是配合过的标签
                        {
                            //                            qDebug()<<"coming in******";

                            union mesg
                            {
                                quint64 Stime;
                                uint8_t StimeTemp[5];
                            }no={0};

                            for(int i = 0;i<5;i++)
                            {
                                no.StimeTemp[i] = networkFrame.data.TDOA_LOC_INFO.timestamp[i];
                            }

                            MesgStructs dataStruct;
                            dataStruct.mesgAncNos = networkFrame.anchor_no;  //收到的基站编号
                            dataStruct.mesgCycle = networkFrame.data.TDOA_LOC_INFO.frame_seq;  //序列号
                            dataStruct.mesgTimer = no.Stime;  //基站收到数据的时间戳
                            dataStruct.status = networkFrame.data.TDOA_LOC_INFO.status;   //电量
                            //                            qDebug()<<"tagPower"<<dataStruct.status;

                            analyzeList[j][k]->revStruct(dataStruct);   //发给对应的标签的new出来的对象里去计算坐标
                        }
                    }
                }
            }
        }
    }
}

void WirelessLocation::sendStartSync()
{
    if(currentAnchor<anchorNums)
    {
        //        qDebug()<<"$$$$$$$$$$$$$$$"<<currentAnchor<<anchorNums;
        sendCmd(NET_CMD_START_WL_SYNC,anchor_no[currentAnchor]);  //让所有基站分别发同步信号
        currentAnchor++;
        QTimer::singleShot(2000,this,SLOT(sendStartSync()));
    }
    else   //所有基站都发过后开始计算标定值
    {
        //        qDebug()<<"$$$$$$$$$$$$$$$1"<<currentAnchor;
        currentAnchor = 0;
        calibration();
    }
}

void WirelessLocation::calibration()     //计算标定值
{   
    //    for(int i=0;i<10;i++)
    //    {
    //        for(int j=0;j<10;j++)
    //        {
    //            for(int k=0;k<256;k++)
    //            {
    //                qDebug()<<i<<j<<k<<calibrationArray[i][j][k];
    //            }
    //        }
    //    }
    int64_t mean[10][10][10];
    memset(mean, 0 ,sizeof(mean));
    for(int i = 0; i < anchorNums; i++)
    {
        QList<int> checkedTmp = checkedList[i];
        for(int j = 1; j< anchorNums;j++)
        {
            if(checkedTmp.contains(j) && j!= i)
            {
                for(int k = 0; k<anchorNums-1; k++)
                {
                    if( j>k && checkedTmp.contains(k) && k != i)
                    {
                        QMap<QString,int> coordsI = coordsMap[anchor_no[i]];
                        QMap<QString,int> coordsJ = coordsMap[anchor_no[j]];
                        QMap<QString,int> coordsK = coordsMap[anchor_no[k]];
                        int64_t R2 = distance(coordsI["x"],coordsI["y"],coordsI["z"],coordsJ["x"],coordsJ["y"],coordsJ["z"]);
                        int64_t R1 = distance(coordsI["x"],coordsI["y"],coordsI["z"],coordsK["x"],coordsK["y"],coordsK["z"]);
                        int64_t delay[256];
                        memset(delay, 0, sizeof(delay));
                        for(int m = 0; m<256; m++)
                        {
                            if( calibrationArray[i][j][m] != 0 && calibrationArray[i][k][m] != 0 )
                            {
                                //                        qDebug()<<"@@@"<<i<<j<<k<<calibrationArray[i][j][k]<<calibrationArray[i][0][k]
                                //                                  <<calibrationArray[i][j][k] - calibrationArray[i][0][k]<<(R2 - R1)/speed;
                                int64_t diff = qRound64(((double)(R2 - R1))/speed);
                                delay[m] = calibrationArray[i][j][m] - calibrationArray[i][k][m] - diff;
                                //                        qDebug()<<"delay[k]"<<delay[k];
                            }
                        }
                        mean[i][j][k] = averaging(delay,256);   //计算出所有标定信息的时间差   基站i发送，j到k的时间差，j>k
                    }
                }
            }
        }
    }

    int64_t OffsetAll[anchorNums][anchorNums-1];
    memset(OffsetAll, 0 ,sizeof(OffsetAll));
    
    for(int j=1;j<anchorNums;j++)
    {
        for(int k=0; k<anchorNums-1; k++)
        {
            if(j>k)
            {
                int64_t meanTmp = 0;
                int num = 0;
                for(int i=0;i<anchorNums;i++)
                {
                    if(mean[i][j][k]!=0 && i!=j && k!= i)
                    {
                        meanTmp = meanTmp + mean[i][j][k];
                        num++;
                    }
                }
                if(num != 0)
                    OffsetAll[j][k] = meanTmp/num;    //所有时间差求平均
            }
        }
    }

    //    for(int i=0;i<anchorNums;i++)
    //    {
    //        for(int j=0;j<anchorNums-1;j++)
    //        {
    //            qDebug()<<i<<j<<OffsetAll[i][j];
    //        }
    //    }
    
    bool isAvailable = true;
    QList<int64_t> tempOffsetList;   //求相邻两个基站的时间差   tempOffsetList[k]即是基站k到基站k-1的时间差，基站从0开始
    tempOffsetList.append(0);
    for(int j=1; j<anchorNums; j++)
    {
        if(OffsetAll[j][j-1] != 0 )
            tempOffsetList.append(OffsetAll[j][j-1]);
        else
        {
            int64_t tempOffset = 0;
            for(int i=0;i<anchorNums;i++)
            {
                if(i<j-1)
                {
                    if( OffsetAll[j][i]!=0 && OffsetAll[j-1][i]!=0)
                    {
                        tempOffset = OffsetAll[j][i]-OffsetAll[j-1][i];
                        break;
                    }
                }
                else if(i>j)
                {
                    if( OffsetAll[i][j-1]!=0 && OffsetAll[i][j]!=0)
                    {
                        tempOffset = OffsetAll[i][j-1]-OffsetAll[i][j];
                        break;
                    }
                }
            }
            if(tempOffset !=0)
                tempOffsetList.append(tempOffset);
            else
            {
                tempOffsetList.append(0);
                isAvailable = false;  //有相邻基站求不出来时间差的情况
                //                break;
            }
        }
    }

    if(isAvailable)  //标定成功
    {
        Offset = tempOffsetList;
        for(int i=0;i<analyzeList.size();i++)
        {
            for(int j=0;j<analyzeList[i].size();j++)
                analyzeList[i][j]->setOffset(tempOffsetList);  //标定信息发给每个标签对应的对象
        }
        syncTimer->stop();
        getStatusTimer->start(1000);  //开始把标签信息发送给客户端
        locationstation = 2;//add lw 2017/10/30
//        cal_fail_cnt = 0;//add lw 2017/10/30
        qDebug()<<"###########Basestation calibration success!###########！";
		isSync = true;
        offsetList.clear();
        for(int i= 0;i<Offset.size();i++)
        {
            qDebug()<<"Offset"<<i<<Offset[i];

            //        QVariantList offsetList;
            //        for(int i =0;i<Offset.size();i++)
//            offsetList.append(Offset[i]);     todo 20190716
        }
		sendoffsetlist(offsetList);
        saveOffSet(offsetList);
    }
    else
    {
        qDebug()<<"Basestation calibrate fail"<<tempOffsetList.size()<<tempOffsetList;
        locationstation = 3;//add lw 2017/10/30
		 offsetList.clear();
         sendoffsetlist(offsetList);
		 saveOffSet(offsetList);
//        if(cal_fail_cnt<3)
//        {
//            currentAnchor = 0;
//            Offset.clear();
//           memset(calibrationArray,0,sizeof(calibrationArray));
//            sendStartSync();
//        }
//        cal_fail_cnt++;
    }
	 SendCalibrationsatus(locationstation);
}

void WirelessLocation::saveOffSet(QVariantList offsetList)
{
    LocationParseJson * locationParseJson = new LocationParseJson;
    QVariantMap currentLocation = locationParseJson->parseJson();
    delete locationParseJson;
    currentLocation.insert("offsetList",offsetList);

    QJson::Serializer serializer;
    bool ok;
    QByteArray json = serializer.serialize(currentLocation, &ok);

    if (ok)
    {
        qDebug() << json;
        QFile locationJson("/location.json");
        if(locationJson.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            QTextStream text_stream(&locationJson);
            text_stream << json << endl;
            locationJson.flush();
            locationJson.close();
        }
        else
            qDebug()<<"locationJson.json write open fail";
    }
    else
        qCritical() << "Something went wrong:" << serializer.errorMessage();
}

int64_t WirelessLocation::averaging(int64_t *delay, int length)  //求标定时收到时间差的平均值
{
    int64_t sum = 0, mean = 0;
    int64_t max = 0;
    int64_t min = 0;
    int count = 0;
    for(int m=0; m<length; m++)
    {
        if(delay[m] != 0)
        {
            //            qDebug()<<"!!!delay[m]"<<delay[m];
            sum = sum + delay[m];
            count++;
            if(max==0)
            {
                max = delay[m];
                min = delay[m];
            }
            else
            {
                if(delay[m]>max)
                    max = delay[m];
                else if(delay[m]<min)
                    min = delay[m];
            }
        }
    }
    //    qDebug()<<"count"<<count<<sum<<max<<min;
    if(count>2)
        mean = (sum - max - min)/(count - 2);  //去掉最大值和最小值再求平均
    else if(count<2 && count >0)
        mean = sum / count;
    //    qDebug()<<"!!!mean"<<mean;
    return mean;
}

uint64_t WirelessLocation::distance(int x1,int y1,int z1,int x2,int y2,int z2)  //求两点之间距离
{
    uint64_t d = qSqrt(qPow((x1-x2),2)+qPow((y1-y2),2)+qPow((z1-z2),2));
    return d;
}

void WirelessLocation::sendCmd(LOC_CMD sendCmd, uint16_t anchor)  //给基站发送命令
{
    NETWORK_FRAME networkFrame;
    networkFrame.header[0] = 0xaa;
    networkFrame.header[1] = 0x55;
    networkFrame.version = 0x00;
    networkFrame.subversion = 0x00;
    networkFrame.cmd = sendCmd;   //命令号
    networkFrame.length = sizeof(NETWORK_FRAME);
    networkFrame.anchor_no = 0x00;

    memset(&networkFrame.data,0, sizeof(networkFrame.data));
    memset(&networkFrame.sum_Check,0, sizeof(networkFrame.sum_Check));
    if(sendCmd == NET_CMD_START_WL_SYNC)
    {
        networkFrame.data.WL_SYNC_START.wl_sync_time_span = 500;
        networkFrame.data.WL_SYNC_START.anc_no = anchor;

        sendData(&networkFrame);
    }
    else
    {
        sendData(&networkFrame);
    }
}

void WirelessLocation::sendData(NETWORK_FRAME *networkFrame)
{
    volatile unsigned char *ptr = (unsigned char*)networkFrame;
    for(uint i = 0;i<sizeof(NETWORK_FRAME)-sizeof(networkFrame->sum_Check);i++)
    {
        networkFrame->sum_Check += *(ptr++);   //求校验和
    }

    QByteArray dataArray = QByteArray::fromRawData((char*)networkFrame,sizeof(NETWORK_FRAME));
    tcpSocket->write(dataArray,dataArray.size());
    //    qDebug()<<"send:"<<dataArray.toHex();
}

void WirelessLocation::getTagStatus()   //定时读Tag对应的位置信息和电量，发送给对应的Data,若有多个标签，有一个在区域内就给使能
{
    //    if(locationstation !=4 )
    //        locationstation = 4;//add lw 2017/10/30
    for(int i=0;i<tag_mac.size();i++)
    {
        CurrentStatus currentStatus;
        currentStatus.isInside = false;
        currentStatus.powerStatus = 0;
        for(int j=0;j<tag_mac[i].size();j++)
        {
            CurrentStatus currentstatus = analyzeList[i][j]->getcurrentStatus();
            if(currentstatus.powerStatus != 0)
            {
                currentStatus = currentstatus;
                if(currentstatus.isInside)
                {
                    break;
                }
            }
        }
        locationClients[i]->setCurrentStatus(currentStatus);
    }
}

void WirelessLocation::reSync()
{
 if(isSync)
    {
         syncTimer->stop();
         getStatusTimer->start(1000);  //开始把标签信息发送给客户端
    }
    else
    {
//        sendCmd(OSTR_CMD_INFORM,anchor_no[0]);
    }
}

void WirelessLocation::Calibration_cell()
{
    if(Cell_connectsuccess)//如果网络连通，去标定，否则不进行标定
    {
        isSync = false;
        currentAnchor = 0;
        Offset.clear();
        memset(calibrationArray,0,sizeof(calibrationArray));
//        sendStartSync();

        sendCmd(OSTR_CMD_INFORM,0);
    }
    else
    {
        locationstation = 4;
        SendCalibrationsatus(locationstation);
    }
}

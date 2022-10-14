#include "iobox.h"

ioBox::ioBox(QObject *parent) : QObject(parent)
{
    this->moveToThread(&thread);
    thread.start();

    mOldIoBoxInput = 0;
    system("echo 1 > /root/gpio/COM0");

    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);
    QString tmpIoBoxSerial = configIniRead->value("baseinfo/IoBoxSerial").toString();
    configIniRead->deleteLater();
    DTdebug()<< "IoBoxSerial" << tmpIoBoxSerial;

    QString dev_name = tmpIoBoxSerial;
    mSerialPort = new QextSerialPort(dev_name);
    connect(mSerialPort, SIGNAL(readyRead()), this, SLOT(slotReadyRead()), Qt::QueuedConnection);
    //设置波特率
    mSerialPort->setBaudRate(BAUD115200);
    //设置数据位
    mSerialPort->setDataBits(DATA_8);
    //设置校验
    mSerialPort->setParity(PAR_NONE);
    //设置停止位
    mSerialPort->setStopBits(STOP_1);
    //设置数据流控制
    mSerialPort->setFlowControl(FLOW_OFF);
    //设置延时
    mSerialPort->setTimeout(10);
    if(mSerialPort->open(QIODevice::ReadWrite)){
        DTdebug()<<QObject::tr("serial open ok ") << dev_name;
    }else{
        DTdebug()<<QObject::tr("serial open fail ") << dev_name;
    }

    QTimer::singleShot(1000, this, SLOT(slotAutoTest()));
}

void ioBox::slotSetIoBox(int pIndex,bool power)
{
    sendFunc(6,pIndex,power);
    if(power){
        QtSleep(10);
        sendFunc(6,pIndex,power);
//        QtSleep(50);
//        sendFunc(6,pIndex,power);
//        QtSleep(50);
    }
//    for(int i=0;i<8;i++)
//    {
//        if(pIndex == i)
//            sendFunc(6,i,true);
//        else
//            sendFunc(6,i,false);
//        QtSleep(50);
//    }
}

void ioBox::slotReadyRead()
{
    QByteArray tmpData = mSerialPort->readAll();
    if(tmpData != recvBuf_temp){
        qDebug() << "recvNewIoBox:"<<tmpData.toHex();
        recvBuf_temp = tmpData;
    }
    //63 01 02 08 00 47 F4
    if(tmpData.count() ==  7)
    {
        if(tmpData.at(0) != 0x63)
            return;
        if(tmpData.at(1) != 0x01)
            return;
        if(tmpData.at(2) != 0x02)
            return;
        quint16 crc = checkCRC(tmpData.mid(0, 5));
        quint8 tmpCrcL = (quint8)tmpData.at(5);
        quint8 tmpCrcH = (quint8)tmpData.at(6);
        quint16 tmpCrc = (tmpCrcL) + (((quint16)tmpCrcH)<<8);
        if(tmpCrc != crc)
        {
            DTdebug() << "checkCRC" << tmpCrcL << tmpCrcH << tmpCrc << crc;
            return;
        }
        int tmpIoInput = ((int)tmpData.at(3));// + (((int)tmpData.at(4))<<8);
        if(mOldIoBoxInput != tmpIoInput)
        {
            mOldIoBoxInput = tmpIoInput;
            DTdebug() << "IoBoxInput" << tmpIoInput;
            emit signalIoBoxInput(tmpIoInput);
        }
    }
}

int ioBox::checkCRC(QByteArray buf)
{
    QString str = buf.toHex();
    int crc = 0XFFFF;
    for(int i = 0;i<buf.size();i++){
        crc ^= str.mid(2*i,2).toInt(0,16);
        for(int j =0;j<8;j++){
            if(1 == (crc & 1)){
                crc = (crc >> 1) ^ 0xA001;
            }else{
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

//类型（type）
//1 读2个板
//2 读上板
//3 读下板
//4 写2个板
//5 写上板
//6 写下板
//7 查看输入输出
void ioBox::sendFunc(int type,int ioNum,bool ONOFF)
{
    writeStruct va;
    va.deviceNum = 0x63;//99, 设备码目前手动输入，以后用配置文件读入
    if(2 == type)//读单板
    {
        va.ctrlNum = 0x01;//读的功能码都是0x01
        va.address_h = 0;
        va.data_h = 0;
        va.address_l = ioNum;//
        va.data_l = 0x10;//0x10 = 16 读16个IO
    }
    else if(6 == type)//写单板
    {
        va.ctrlNum = 0x05;//写的功能码都是0x05
        va.address_h = 0;
        va.address_l = 8 + ioNum;//目前是从8开始
        if(ONOFF){
            va.data_h = 0xff;//亮灯
        }else{
            va.data_h = 0;//灭灯
        }
        va.data_l = 0;
    }
    else if(7 == type)//查输入/输出状态
    {
        va.ctrlNum = 0x03;
        va.address_h = 0;
        va.address_l = 0;
        va.data_h = 0;
        va.data_l = 0x01;
    }

    QByteArray buf;
    buf[0] = va.deviceNum;//设备地址
    buf[1] = va.ctrlNum;//功能码
    buf[2] = va.address_h;//高地址
    buf[3] = va.address_l;//低地址
    buf[4] = va.data_h;//寄存器高
    buf[5] = va.data_l;//寄存器低
    int crc = checkCRC(buf);
    buf[6] = crc & 0xff;//CRC低
    buf[7] = crc >> 8;//CRC高
    qint64 tmpRet = mSerialPort->write(buf);
    if(buf != sendBuf_temp){
        qDebug() <<"sendNewIoBox:"<< tmpRet << buf.toHex();
        sendBuf_temp = buf;
    }
}

void ioBox::slotAutoTest()
{
    sendFunc(2,0,true);
    QTimer::singleShot(500, this, SLOT(slotAutoTest()));
}


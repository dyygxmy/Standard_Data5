#include "serialthread.h"
#include <QtGui>

serialThread::serialThread(QObject *parent) :
    QObject(parent)
{
    whichindex = 0;
    serial_thread.start();
    this->moveToThread(&serial_thread);

    //QTimer::singleShot(1000, this, SLOT(slotTest()));
    //qDebug() << "wxume";
}


//打开串口
void serialThread::initSerial()
{
    FUNC() ;
    seriallist.clear();
    QString portName = "ttymxc0" ;   //获取串口名
    if(factory == "Haima")
    {
        portName = "ttymxc3" ;
    }
    myCom = new QextSerialPort("/dev/" + portName);
    connect(myCom, SIGNAL(readyRead()), this, SLOT(readMyCom()));

    //设置波特率
    myCom->setBaudRate((BaudRateType)9600);

    //设置数据位
    myCom->setDataBits((DataBitsType)8);

    //设置校验
    myCom->setParity(PAR_NONE);

    //设置停止位
    myCom->setStopBits(STOP_1);

    //设置数据流控制
    myCom->setFlowControl(FLOW_OFF);
    //设置延时
    myCom->setTimeout(10);

    if(myCom->open(QIODevice::ReadWrite)){
        DTdebug()<<QObject::tr("serial open ok ") << portName;

    }else{
        DTdebug()<<QObject::tr("serial open fail ") << portName;
    }
}

void serialThread::slotTest()
{
    emit signalQRCodeFromSerial(QString("C6201964956718"));
}

//读取数据
void serialThread::readMyCom()
{
    FUNC();



    if(factory == "SVW2")
    {
        QByteArray temp = myCom->readAll();

        if(!temp.isEmpty())
        {
            serialNums = serialNums+temp;
            DTdebug() << serialNums  << serialNums.right(2);

            //回车等清除
            while(!serialNums.startsWith("#") && !serialNums.startsWith("2") &&!serialNums.isEmpty())
            {
                serialNums.replace(0,1,"");
            }
            while (serialNums.right(2) == "*=")
            {
                DTdebug() << serialNums ;
                Q_EMIT signalQRCodeFromSerial(serialNums);
                serialNums = "" ;
            }

        }
    }
    else if(factory == "Haima")
    {
        usleep(50000) ;
        QByteArray temp = myCom->readAll();
        serialNums = temp ;

        if(CsIsConnect && !ISmaintenance  && SYSS!="ING"&& isBarCode)
        {
            if((ControlType_1 == "SB356_PLC")&&(!PLCIsConnect))
            {}
            else
            {
                if(SerialGunMode)
                {
                    system("echo 1 > /sys/class/leds/control_uart1/brightness");
                }

                isEqual = false;
                if(seriallist.isEmpty())
                {
                    seriallist.push_back(serialNums);
                    whichindex++;
                }
                else
                {
                    QList<QString>::iterator i;
                    for (i = seriallist.begin(); i != seriallist.end(); ++i)
                    {
                        if(!(QString::compare(serialNums, *i, Qt::CaseSensitive)))
                        {
                            isEqual = true;
                            break;
                        }
                    }
                    if(factory == "GZBAIC")
                    {
                        isEqual = false ;
                    }

                    if(!isEqual)
                    {
                        if(seriallist.size() == 10)
                        {
                            seriallist[whichindex] = serialNums;
                        }
                        else
                        {
                            seriallist.push_back(serialNums);
                        }
                        whichindex++;
                        if(whichindex == 10)
                            whichindex = 0;
                    }
                }
                DTdebug()<<"send SerialNumbs "<<serialNums<<isEqual;
                emit signalSerialCom(serialNums,isEqual,"BarCode");
            }
        }
        else {
            DTdebug()<<CsIsConnect << ISmaintenance << SYSS << isBarCode ;
        }
    }
}





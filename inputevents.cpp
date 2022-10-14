#include "inputevents.h"
#include "GlobalVarible.h"
#include <QDebug>
InputEvents::InputEvents(QObject *parent) :
    QObject(parent)
{
    numcount = 0;
    issingle = false;
    keyvalue = 0;
    this->moveToThread(&m_thread);
    m_thread.start();

}

#if 1
void InputEvents::InputEventStart()
{
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(mtimerarrve3()));
    m_timer.start(100);
}
#else
void InputEvents::InputEventStart()
{
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(mtimerarrve3()));
//    const char *dev_name;
//    dev_name = "/root/gpio/IN0";
//    while(1)
//    {
//        keys_fd = open(dev_name, O_RDWR, 0);
//        if (keys_fd < 0)
//        {
//            continue;
//        }
//        else
//        {
//            break;
//        }
//    }
//    if (fcntl(keys_fd, F_SETFL, O_NONBLOCK) < 0)
//        printf("Unable set to NONBLOCK mode");


    m_timer.start(100);
}
#endif

bool gKeyLedFlag = false;

void InputEvents::mtimerarrve3()
{
    if(!ISWARNING)
    {
        QFile input1("/root/gpio/IN0");  //IN0
        input1.open(QIODevice::ReadOnly);
        QTextStream in1(&input1);
        in1 >> keyvalue;
        input1.close();

        if(ControlType_1 == "SB356_PLC")
        {
            return;
        }

//        qDebug()<<"keyvalue:"<<keyvalue;

        if(keyvalue)
        {
            numcount++;
            if(numcount == 20)
            {
//                qDebug()<<"skip all";
                if(factory == "KFCHERY"){
                    emit sendconfigwarning(false);
                }else{
                    if(SYSS == "ING"){
                        emit sendconfigwarning(false);
                    }
                }
                numcount = 0;
                keyvalue = 0;
            }
//            if(!gKeyLedFlag)
//            {
//                gKeyLedFlag = true;
//                DTdebug() << "nok_led(1)";
//                system("echo 1 > /root/gpio/OUT0 &");
//            }
        }
        else
        {
            if(numcount < 20 && numcount != 0)
            {
                Queuelock.lockForRead();
//                qDebug()<<"skip one";
                if(isBarCode)//???????????????????????
                {
                    if(SYSS == "ING")
                    {
                        emit sendconfigwarning(true);
                    }
                }
                else if(isRFID||isQueue)
                {
                    if(SYSS == "ING")              //???????????????
                        emit sendconfigwarning(true);
                }
                Queuelock.unlock();
             }
             numcount = 0 ;
//             if(gKeyLedFlag)
//             {
//                 gKeyLedFlag = false;
//                 DTdebug() << "nok_led(0)";
//                 system("echo 0 > /root/gpio/OUT0 &");
//             }

        }


//        if(t.value==0 || t.value==1)
//        {
//            keyvalue = t.value;

//            if(t.value == 1)
//            {
//                if(DebugMode)
//                {
//                    system("echo 1 > /root/gpio/OUT0");
//                }
//                numcount++;
//                if(numcount == 20)
//                {
//                    if(SYSS == "ING")
//                        emit sendconfigwarning(false);
//                    numcount = 0;
//                }

//            }
//            else if(t.value == 0)
//            {
//                if(DebugMode)
//                {
//                    system("echo 0 > /root/gpio/OUT0");
//                }
//                numcount = 0;
//                Queuelock.lockForRead();
//                if(isBarCode)//???????????????????????
//                {
//                    if(SYSS == "ING")
//                    {
//                        emit sendconfigwarning(true);
//                    }
//                }
//                else if(isRFID||isQueue)
//                {
//                    if(SYSS == "ING")              //???????????????
//                        emit sendconfigwarning(true);
//                }
//                Queuelock.unlock();
//            }
//        }
    }
    else
    {
        DTdebug() << "is warning" ;
    }

}

#if 0
void InputEvents::InputEventStart()
{
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(mtimerarrve3()));
    const char *dev_name;
    dev_name = "/dev/input/event0";
    while(1)
    {
        keys_fd = open(dev_name, O_RDWR, 0);
        if (keys_fd < 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    if (fcntl(keys_fd, F_SETFL, O_NONBLOCK) < 0)
        printf("Unable set to NONBLOCK mode");
    m_timer.start(100);
}

void InputEvents::mtimerarrve3()
{
    if(!ISWARNING)
    {
        if(read(keys_fd,&t,sizeof(t))==sizeof(t))
        {
            if(ControlType_1 == "SB356_PLC")
            {
                return;
            }
            if(t.type == EV_KEY)
                if(t.value==0 || t.value==1)
                {
                    switch(t.code)
                    {
                    case 114:   //钥匙
                        keyvalue = t.value;

                        if(t.value == 1)
                        {
                            if(DebugMode)
                            {
                                system("echo 1 > /root/gpio/OUT0");
                            }
                            numcount++;
                            if(numcount == 20)
                            {
                                if(SYSS == "ING")
                                    emit sendconfigwarning(false);
                                numcount = 0;
                            }

                        }
                        else if(t.value == 0)
                        {
                            if(DebugMode)
                            {
                                system("echo 0 > /root/gpio/OUT0");
                            }
                            numcount = 0;
                            Queuelock.lockForRead();
                            if(isBarCode)//???????????????????????
							{
								if(SYSS == "ING")
                            	{
                                	emit sendconfigwarning(true);
                            	}
							}
                            else if(isRFID||isQueue)
							{
                                if(SYSS == "ING")              //???????????????
                                    emit sendconfigwarning(true);
							}
                            Queuelock.unlock();
                        }
                        break;

                    case 115:   //电池
                        if(!battery)
                        {
                            if(t.value == 0)
                                emit sendbatterysta(true);
                            else
                                emit sendbatterysta(false);
                            break;
                        }
                    case 116:
                        break;
                    default:
                        break;
                    }
                }
        }
        else
        {
            if(keyvalue)
            {
                numcount++;
                if(numcount == 20)
                {
                    if(SYSS == "ING")
                        emit sendconfigwarning(false);
                    numcount = 0;
                    keyvalue = 0;
                }

            }
        }
    }
    else
    {
        read(keys_fd,&t,sizeof(t));
    }
}
#endif

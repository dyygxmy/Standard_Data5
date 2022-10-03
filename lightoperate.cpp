#include "lightoperate.h"
#include <QDebug>

LightOperate::LightOperate(QObject *parent) : QObject(parent)
{
    connect(&mRedBlinkTimer,SIGNAL(timeout()),this,SLOT(slot_RedBlink()));
    connect(&mYellowBlinkTimer,SIGNAL(timeout()),this,SLOT(slot_YellowBlink()));
    connect(&mGreenBlinkTimer,SIGNAL(timeout()),this,SLOT(slot_GreenBlink()));
    connect(&mWhiteBlinkTimer,SIGNAL(timeout()),this,SLOT(slot_WhiteBlink()));
    mRedBlinkTimer.setInterval(500);
    mYellowBlinkTimer.setInterval(500);
    mGreenBlinkTimer.setInterval(500);
    mWhiteBlinkTimer.setInterval(500);
    mRed = 0;
    mRedBlinkTime = -1;
    mWhiteBlinkTime = -1;
    mGreenBlinkTime = -1;
    mYellowBlinkTime = -1;
    mGreen = 0;
    mYellow = 0;
    mWhite = 0;

    slot_LightLogicOperate(e_WaitForScan);
}

void LightOperate::setRedState(E_LIGHT_STATE pState)
{
    if(pState == eLightOFF)
    {
        mRedBlinkTimer.stop();
        red_led(0);
    }
    else if(pState == eLightON)
    {
        mRedBlinkTimer.stop();
        red_led(1);
    }
    else if(pState == eLightBlinkOnce)
    {
        if(!mRedBlinkTimer.isActive())
        {
            red_led(0);
            mRedBlinkTime = 2;
            mRedBlinkTimer.start();
        }
    }
    else if(pState == eLightBlink)
    {
        if(!mRedBlinkTimer.isActive())
        {
            red_led(0);
            mRedBlinkTimer.start();
        }
    }
//    else if(pState == eLightBlink1)
//    {
//        if(!mRedBlinkTimer.isActive())
//        {
//            red_led(0);
//            mRedBlinkTimer.start();
//            mRedBlinkTimer.
//        }
//    }
}

void LightOperate::setGreenState(E_LIGHT_STATE pState)
{
    if(pState == eLightOFF)
    {
        mGreenBlinkTimer.stop();
        green_led(0);
    }
    else if(pState == eLightON)
    {
        mGreenBlinkTimer.stop();
        green_led(1);
    }
    else if(pState == eLightBlink)
    {
        if(!mGreenBlinkTimer.isActive())
        {
            green_led(0);
            mGreenBlinkTimer.start();
        }
    }
}

void LightOperate::setYellowState(E_LIGHT_STATE pState)
{
    if(pState == eLightOFF)
    {
        mYellowBlinkTimer.stop();
        yellow_led(0);
    }
    else if(pState == eLightON)
    {
        mYellowBlinkTimer.stop();
        yellow_led(1);
    }
    else if(pState == eLightBlink)
    {
        if(!mYellowBlinkTimer.isActive())
        {
            yellow_led(0);
            mYellowBlinkTimer.start();
        }
    }
}

void LightOperate::setWhiteState(E_LIGHT_STATE pState)
{
    if(pState == eLightOFF)
    {
        mWhiteBlinkTimer.stop();
        white_led(0);
    }
    else if(pState == eLightON)
    {
        mWhiteBlinkTimer.stop();
        white_led(1);
    }
    else if(pState == eLightBlinkOnce)
    {
        if(!mWhiteBlinkTimer.isActive())
        {
            red_led(0);
            mWhiteBlinkTime = 2;
            mWhiteBlinkTimer.start();
        }
    }
    else if(pState == eLightBlink)
    {
        if(!mWhiteBlinkTimer.isActive())
        {
            white_led(0);
            mWhiteBlinkTimer.start();
        }
    }
}

void LightOperate::setkeyState(E_LIGHT_STATE pState)
{
    if(pState == eLightOFF)
    {
        nok_led(0);
    }
    else if(pState == eLightON)
    {
        nok_led(1);
    }
}

void LightOperate::LightOperateFromString(QString pValue)
{
    QStringList tmpList = pValue.split("#");
    foreach(QString tmpSubValue,tmpList)
    {
        if(tmpSubValue.isEmpty())
            continue;
        else
        {
            QStringList tmpSubList = tmpSubValue.split("_");
            if(tmpSubList.length()!=2)
                continue;
            else
            {
                QString  tmpLightColor= tmpSubList.at(0);
                QString  tmpLightState = tmpSubList.at(1);

                E_LIGHT_STATE tmpState;
                if(tmpLightState =="ON")
                {
                    tmpState = eLightON;
                }
                else if(tmpLightState =="OFF")
                {
                    tmpState = eLightOFF;
                }
                else if(tmpLightState =="Blink")
                {
                    tmpState = eLightBlink;
                }
                else if(tmpLightState =="BlinkOnce")
                {
                    tmpState = eLightBlinkOnce;
                }
                if((tmpLightColor=="red"))
                {
                    setRedState(tmpState);
                }
                else if((tmpLightColor=="green"))
                {
                    setGreenState(tmpState);
                }
                else if((tmpLightColor=="yellow"))
                {
                    setYellowState(tmpState);
                }
                else if((tmpLightColor=="white"))
                {
                    setWhiteState(tmpState);
                }
                else if((tmpLightColor=="key"))
                {
                    setkeyState(tmpState);
                }
            }
        }
    }
}

void LightOperate::slot_LightLogicOperate(LightLogic pValue)
{
    qDebug()<< "into LightLogic operate";
    QSettings *configIniRead = new QSettings("/config.ini", QSettings::IniFormat);

    switch (pValue) {
        case e_SingleOK:
        {        
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_SingleOK").toString();  // GroupOK="red_blink67#"
            LightOperateFromString(tmpValue);
            break;
        }
        case e_SingleFail:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_SingleFail").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_GroupOK:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_GroupOK").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_GroupFail:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_GroupFail").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_ING:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_ING").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_NotING:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_NotING").toString();
            LightOperateFromString(tmpValue);
            break;
        }

        case e_Enable:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_Enable").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_Disable:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_Disable").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_WaitForScan:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_waitForScan").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_ScanFinishThenWait:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_ScanFinishThenWait").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_TightenDisConnect:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_TightenDisConnect").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_TightenConnect:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_TightenConnect").toString();
            LightOperateFromString(tmpValue);
            break;
        }
//        case e_PlcConnect:
//        {
//            QString tmpValue = configIniRead->value("baseinfo/LightLogic_PlcConnect").toString();
//            LightOperateFromString(tmpValue);
//            break;
//        }
//        case e_PlcDisConnect:
//        {
//            QString tmpValue = configIniRead->value("baseinfo/LightLogic_PlcDisConnect").toString();
//            LightOperateFromString(tmpValue);
//            break;
//        }
//        case e_PlcConnectAndReadError:
//        {
//            QString tmpValue = configIniRead->value("baseinfo/LightLogic_PlcConnectAndReadError").toString();
//            LightOperateFromString(tmpValue);
//            break;
//        }
        case e_NetWorkConnect:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_NetWorkConnect").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_NetWorkDisConnect:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_NetWorkDisConnect").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        case e_SingleSkip:
        {
            QString tmpValue = configIniRead->value("baseinfo/LightLogic_SingleSkip").toString();
            LightOperateFromString(tmpValue);
            break;
        }
        default:
            break;
    }
    configIniRead->deleteLater();
}

void LightOperate::slot_RedBlink()
{
    red_led(!mRed);

    if(mRedBlinkTime > 0)
        mRedBlinkTime--;
    if(mRedBlinkTime == 0)
    {
        mRedBlinkTime = -1;
        mRedBlinkTimer.stop();
    }
}

void LightOperate::slot_YellowBlink()
{
    yellow_led(!mYellow);
    if(mYellowBlinkTime > 0)
        mYellowBlinkTime--;
    if(mYellowBlinkTime == 0)
    {
        mYellowBlinkTime = -1;
        mYellowBlinkTimer.stop();
    }
}

void LightOperate::slot_GreenBlink()
{
    green_led(!mGreen);
    if(mGreenBlinkTime > 0)
        mGreenBlinkTime--;
    if(mGreenBlinkTime == 0)
    {
        mGreenBlinkTime = -1;
        mGreenBlinkTimer.stop();
    }
}

void LightOperate::slot_WhiteBlink()
{
    white_led(!mWhite);
    if(mWhiteBlinkTime > 0)
        mWhiteBlinkTime--;
    if(mWhiteBlinkTime == 0)
    {
        mWhiteBlinkTime = -1;
        mWhiteBlinkTimer.stop();
    }
}

void LightOperate::red_led(int leds)
{
    mRed = leds;
    if(leds)
        system("echo 1 > /root/gpio/OUT4 &");
    else
        system("echo 0 > /root/gpio/OUT4 &");

}
void LightOperate::green_led(int leds)
{
    mGreen = leds;
    if(leds)
    {
        //Q_EMIT signalRedTwinkle(false);
        system("echo 1 > /root/gpio/OUT3 &");
    }
    else
        system("echo 0 > /root/gpio/OUT3 &");
}
void LightOperate::yellow_led(int leds)
{
    mYellow = leds;
    if(leds)
        system("echo 1 > /root/gpio/OUT2 &");
    else
        system("echo 0 > /root/gpio/OUT2 &");
}
void LightOperate::white_led(int leds)
{
    mWhite = leds;
    if(factory != "AQCHERY" && factory != "KFCHERY")
    {
        if(leds)
            system("echo 1 > /root/gpio/OUT1 &");
        else
            system("echo 0 >  /root/gpio/OUT1 &");
    }
}
void LightOperate::nok_led(int leds)  //KEY
{
//    if(!TaoTongState)
//    {
    DTdebug() << "nok_led" << leds;
        if(leds)
            system("echo 1 > /root/gpio/OUT0 &");
        else
            system("echo 0 > /root/gpio/OUT0 &");
//    }

}

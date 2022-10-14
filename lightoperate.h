#ifndef LIGHTOPERATE_H
#define LIGHTOPERATE_H

#include <QObject>
#include  "mainwindow.h"
#include  <QTimer>

typedef enum{
    eLightOFF = 0,
    eLightON,
    eLightBlinkOnce,
    eLightBlink
} E_LIGHT_STATE;
// GroupOK="red_blink67#"

class LightOperate : public QObject
{
    Q_OBJECT
public:
    explicit LightOperate(QObject *parent = 0);

    int mRed;
    int mGreen;
    int mYellow;
    int mWhite;
    int     mRedBlinkTime;
    int     mWhiteBlinkTime;
    int     mGreenBlinkTime;
    int     mYellowBlinkTime;
    QTimer  mRedBlinkTimer;
    QTimer  mYellowBlinkTimer;
    QTimer  mGreenBlinkTimer;
    QTimer  mWhiteBlinkTimer;

public:
    void red_led(int leds);
    void green_led(int leds);
    void yellow_led(int leds);
    void white_led(int leds);
    void nok_led(int leds);

    void setRedState(E_LIGHT_STATE pState);
    void setGreenState(E_LIGHT_STATE pState);
    void setYellowState(E_LIGHT_STATE pState);
    void setWhiteState(E_LIGHT_STATE pState);
    void setkeyState(E_LIGHT_STATE pState);

    void LightOperateFromString(QString pValue);

signals:

public slots:
    void slot_LightLogicOperate(LightLogic pValue);
    void slot_RedBlink();
    void slot_YellowBlink();
    void slot_GreenBlink();
    void slot_WhiteBlink();
};

#endif // LIGHTOPERATE_H

#ifndef CARDREAD_H
#define CARDREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <termios.h>

class CardRead : public QObject
{
    Q_OBJECT
public:
    explicit CardRead(QObject *parent = 0);

signals:
    void sendCardNumber(int);

public slots:
    void comInit();                      //初始化条码枪串口

private:
    QThread m_thread;
    char buff[20];
    int dev_fd;               //串口文件描述符

};

#endif // CARDREAD_H

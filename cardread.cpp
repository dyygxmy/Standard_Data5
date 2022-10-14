#include "cardread.h"

CardRead::CardRead(QObject *parent) :
    QObject(parent)
{
//    whichindex = 0;
    bzero(&buff,sizeof(buff));
    this->moveToThread(&m_thread);
    m_thread.start();
}

void CardRead::comInit()
{
    qDebug() <<"card read thread start!!";
//    seriallist.clear();
    if(true)
    {
        const char *dev_name;
        dev_name = "/dev/ttymxc4";
//        dev_name = "/dev/ttyUSB1";
        struct termios tty_attr;
        int baudrate;
        int byte_bits ;
        baudrate = B9600;
        byte_bits = CS8;
        fd_set rset;
        dev_fd = open(dev_name, O_RDWR, 0);
        if (dev_fd < 0)
        {
            qDebug() << "card read serial open fail "+QString(dev_name);
        }
        else
        {
            qDebug() <<"card read open serial success "+QString(dev_name);
        }

        if (fcntl(dev_fd, F_SETFL, O_NONBLOCK) < 0)
            qDebug() << "unable to set serial port";

        memset(&tty_attr, 0, sizeof(struct termios));
        tty_attr.c_iflag = IGNPAR;
        tty_attr.c_cflag = baudrate | HUPCL | byte_bits | CREAD | CLOCAL;
        tty_attr.c_cc[VTIME] = 0;
        tty_attr.c_cc[VMIN] = 1;
        tcsetattr(dev_fd, TCSANOW, &tty_attr);
        QByteArray buff;//[20];
        int select_fd ;
        while(1)
        {
            FD_ZERO(&rset);
            FD_SET(dev_fd, &rset);

            select_fd = select(dev_fd+1,&rset,NULL,NULL,NULL);
            if (select_fd < 0)
            {
                qDebug() << "select fail";
            }
            if (FD_ISSET(dev_fd, &rset))
            {
                int  nread = 0;
                char data1[1];
                while ((nread = (read(dev_fd, &data1,sizeof(data1)))) > 0)
                {
                    buff.append(data1[0]);
                }
               if(buff.length()==12)
               {
//                   qDebug("0x%02x ", (unsigned char)buff[10]);
//                   qDebug("0x%02x ", (unsigned char)buff[9]);
//                   qDebug("0x%02x ", (unsigned char)buff[8]);
//                   qDebug("0x%02x ", (unsigned char)buff[7]);
                   uint CardNumber = (unsigned char)buff[10];
                   CardNumber <<= 8;
                   CardNumber |= (unsigned char)buff[9];
                   CardNumber <<= 8;
                   CardNumber |= (unsigned char)buff[8];
                   CardNumber <<= 8;
                   CardNumber |= (unsigned char)buff[7];
                   qDebug()<<CardNumber;
                   emit sendCardNumber(CardNumber);
                   buff.clear();
               }
               else if(buff.length()>12)
               {
                   qDebug()<<"wrong buff";
                   buff.clear();
               }
            }
        }
    }
}


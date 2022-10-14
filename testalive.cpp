#include "testalive.h"

Testalive::Testalive(QObject *parent) :
    QObject(parent)
{
    test_alive5s_thread.start();
        this->moveToThread(&test_alive5s_thread);
}


void Testalive::start_timer5s()
{
    qDebug()<<"PFC test_alive5s_thread thread start !!!";
    test_alive5s=new QTimer(this);
     connect(test_alive5s,SIGNAL(timeout()),this,SLOT(timer5stimeout()));
     test_alive5s->start(5000);
}
 void Testalive::timer5stimeout()
 {

     emit sendalive5s();
     qDebug()<<"PFC timer5stimeout start alive!";
 }

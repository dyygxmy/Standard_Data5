#ifndef TESTALIVE_H
#define TESTALIVE_H

#include <QObject>
#include <QThread>
#include <qtimer.h>
#include <qdebug.h>
#include "GlobalVarible.h"

class Testalive : public QObject
{
    Q_OBJECT
public:
    explicit Testalive(QObject *parent = 0);

signals:
    void sendalive5s();
public slots:
    void start_timer5s();
    void timer5stimeout();
private:
    QThread test_alive5s_thread;
    QTimer *test_alive5s;
};

#endif // TESTALIVE_H

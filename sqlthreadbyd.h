#ifndef SQLTHREADBYD_H
#define SQLTHREADBYD_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include "GlobalVarible.h"
#include <QThread>

class SqlThreadBYD : public QObject
{
    Q_OBJECT
public:
    explicit SqlThreadBYD(QObject *parent = 0);

signals:
    void send_mysqlerror();

public slots:
    void sqlinit();
    void mysqlopen();
    void sqlclose();
    void sqlinsert(QVariant);
    void receiveNokAll(int);
    void configOne(QString,QString,QString,int);
public:
    QThread m_thread;
    QSqlDatabase db2;
    //    QSqlDatabase db3;
    //    QSqlQuery query1;
    QSqlQuery query2;
    bool isFirst;
    void setRepair(bool);

private:
    bool isRepair;
    bool NOKflag;
};

#endif // SQLTHREADBYD_H

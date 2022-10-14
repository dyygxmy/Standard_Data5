#ifndef SQLTHREADBAIC_H
#define SQLTHREADBAIC_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include "GlobalVarible.h"
#include <QThread>
#include "newconfiginfo.h"

class SqlThreadBAIC : public QObject
{
    Q_OBJECT
public:
    explicit SqlThreadBAIC(QObject *parent = 0);

public slots:
    void sqlinit();
    void mysqlopen();
    void sqlclose();
    void sqlinsert(QVariant);
    void receiveNokAll(int);
    void configOne(QString,QString,QString,int);
    void configOneGroup(QString,QString *,QString *);
public:
    QThread m_thread;
    QSqlDatabase db2;
    //    QSqlDatabase db3;
    //    QSqlQuery query1;
    QSqlQuery query2;
    bool isFirst;
    bool NOKflag;

signals:
    void send_mysqlerror();

};

#endif // SQLTHREADBAIC_H

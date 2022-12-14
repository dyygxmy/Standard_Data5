#ifndef SQLTHREAD_H
#define SQLTHREAD_H
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QThread>
#include "GlobalVarible.h"
class SqlThread : public QObject
{
    Q_OBJECT
public:
    explicit SqlThread(QObject *parent = 0);

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
    bool isFirstError;
signals:
    void send_mysqlerror();


};

#endif // SQLTHREAD_H

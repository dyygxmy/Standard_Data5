#ifndef SQLTHREADSVW2_H
#define SQLTHREADSVW2_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include "GlobalVarible.h"
#include <QThread>

class SqlThreadSVW2 : public QObject
{
    Q_OBJECT
public:
    explicit SqlThreadSVW2(QObject *parent = 0);

public slots:
    void sqlinit();
    void mysqlopen();
    void sqlclose();
    void sqlinsert(QVariant);
    void sqlinserterror(QVariant DataVar);
    void receiveNokAll(int,int);
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
    QString Factory;
    bool NOKflag;

    void changeColumnName( ) ;

private:
    QString addzero(int m, QString s);

signals:
    void send_mysqlerror();


};

#endif // SQLTHREADSVW2_H

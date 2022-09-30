#ifndef SQLTHREADBENZ_H
#define SQLTHREADBENZ_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include "GlobalVarible.h"
#include <QThread>

class SqlThreadBenz : public QObject
{
    Q_OBJECT
public:
    explicit SqlThreadBenz(QObject *parent = 0);

signals:
    void send_mysqlerror();

public slots:
    void sqlinit();
    void mysqlopen();
    void sqlclose();
    void Useropen();
    void Userclose();
    void sqlinsert(QVariant);
    void receiveNokAll(int);
    void configOne(QString,QString,QString,int);

public:
    QThread m_thread;
    QSqlDatabase db1;
    QSqlQuery query1;
    QSqlDatabase db2;
    QSqlQuery query2;
    QString controllerIp1;
    QString controllerIp2;
    bool isFirst;
    bool NOKflag;

};

#endif // SQLTHREADBENZ_H

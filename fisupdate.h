#ifndef FISUPDATE_H
#define FISUPDATE_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include "GlobalVarible.h"
#include "unistd.h"
class FisUpdate : public QObject
{
    Q_OBJECT
public:
    explicit FisUpdate(QObject *parent = 0);

signals:
    void time_error(bool);
    void sendTime(QString);
    
public slots:
    void myfistimer();
    void fisupdateFunc();
    void update_column(QString);
    void QueryTime();
private:
    QTimer *fisupdataTimer;
    QThread fis_thread;
    QString Factory;
};

#endif // FISUPDATE_H

#ifndef FTPDATAPARSE_H
#define FTPDATAPARSE_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "GlobalVarible.h"

#include "./json/parser.h"
#include "./json/serializer.h"

class ftpDataParse : public QObject
{
    Q_OBJECT
public:
    explicit ftpDataParse(QObject *parent = 0);
//    bool isMaster;

signals:
    void send_mainwindow(QString,QString,QString,int);
    void sendfromjsonthread(QVariant);

private:
    QThread parseJsonData_thread;
    QTimer  ftpupload;
    QMap<QString,int> wrongMap;
    QString systemRunStatus;
    int StartBolt;

public slots:
    void ftpStart();
    void FindFile(QString);
    void parseFile(QString ,QString);
    void ftptimeouts();
    void revRunStatus(QString);    
    void sendReadOperate(bool enable, int n);
};

#endif // FTPDATAPARSE_H

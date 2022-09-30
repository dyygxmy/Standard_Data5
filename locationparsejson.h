#ifndef LOCATIONPARSEJSON_H
#define LOCATIONPARSEJSON_H

#include <QObject>
#include <QVariant>
#include <QFile>
#include <QDebug>
#include "./json/parser.h"

class LocationParseJson : public QObject
{
    Q_OBJECT
public:
    explicit LocationParseJson(QObject *parent = 0);
    QVariantMap parseJson();

signals:

public slots:

};

#endif // LOCATIONPARSEJSON_H

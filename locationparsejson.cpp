#include "locationparsejson.h"

LocationParseJson::LocationParseJson(QObject *parent) :
    QObject(parent)
{
}

QVariantMap LocationParseJson::parseJson()
{
    QFile locationJson("/location.json");
    if(locationJson.open(QIODevice::ReadOnly))
    {
        QByteArray json = locationJson.readAll();

        QJson::Parser parser;
        bool ok;

        QVariantMap currentLocation = parser.parse(json, &ok).toMap();
        if (!ok)
        {
            qDebug()<<"location.json An error occurred during parsing";
        }
        else
        {
            return currentLocation;
        }
    }
    else
    {
        qDebug()<<"location.json locationJson read open fail";
    }
    QVariantMap errorMap;
    errorMap["errorCode"] = 1;
    return errorMap;
}

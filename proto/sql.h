#ifndef SQL_H
#define SQL_H

#include <QObject>

#define SQL_TABLE_FisSVW2 SVW2FisPreview

class SQL : public QObject
{
    Q_OBJECT
public:
    explicit SQL(QObject *parent = 0);

signals:

public slots:

};

extern SQL DTSql;

#endif // SQL_H

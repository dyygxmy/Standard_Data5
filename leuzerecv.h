#ifndef LEUZERECV_H
#define LEUZERECV_H

#include <QObject>
#include <QThread>
class QUdpSocket;

class LeuzeRecv : public QObject
{
    Q_OBJECT
public:
    explicit LeuzeRecv(QObject *parent = 0);

signals:
    void sendNewCode(QString);

public slots:
    void init();

private:
    QThread m_thread;
    QUdpSocket *receiver;
    QString currentCode;

private slots:
    void processPendingDatagram();

};

#endif // LEUZERECV_H

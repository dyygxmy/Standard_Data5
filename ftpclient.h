#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QObject>

class QFtp;
#include <QFtp>
#include <QFile>
#include <QHash>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QDir>
#include "unistd.h"
class QFile;

class FtpClient : public QObject
{
    Q_OBJECT
public:
    explicit FtpClient(QObject *parent = 0);
    QString ftpServer;

signals:
//    void listDone();
    void sendFtpConnected(bool);

private:
    QThread ftp_thread;
    QFtp *ftp;

    // 用来表示下载的文件
    QFile *file;
    QTimer connectTimer;
    QTimer uploadTimer;
    QTimer heartTimer;
    QString sPath;		//多文件增加用
    QString m_sCurrentPath;
    bool add;			//是否启动多文件上传
    QVector<QString> m_fold;                   //只存放文件夹目录
    QVector<QString>::iterator iter_fold;
    QHash<QString , QString> m_record;		//对应目录下有文件则存放在这
    QHash<QString , QString>::iterator iter_record;
    QString pcPath , ftpPath;                     //在对象内的全局上存放本地路径 和 FTP路径
    QString gbkCodeToFtpCode(const QString &gbkCode);
    int FindFileLocal(const QString &sComFilePath);
    bool uploadFile(QString pcPath);

    void processNextDirectory();
    QStringList allFilePaths;
    QString currentDir;
//    QString currentLocalDir;
    QStringList pendingDirs;
    QStringList allDirs;
    QMap<int,QString> deleteMap;

private slots:
    void ftpCommandStarted(int);
    void ftpCommandFinished(int, bool);
//    void ftpDone();

    void initServer();
    void connectServer();
    void dirList();
    void upload();
    void connectTimeout();
    void heartTimeout();
    void ftpListInfo(const QUrlInfo &urlInfo);
    void stateChanged(int state);
};

#endif // FTPCLIENT_H

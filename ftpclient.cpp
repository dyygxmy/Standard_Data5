#include "ftpclient.h"

FtpClient::FtpClient(QObject *parent) :
    QObject(parent)
{
    ftp_thread.start();
    this->moveToThread(&ftp_thread);
    connect (&connectTimer,SIGNAL(timeout()),this,SLOT(connectTimeout()));
    connect (&uploadTimer,SIGNAL(timeout()),this,SLOT(dirList()));
    connect (&heartTimer,SIGNAL(timeout()),this,SLOT(heartTimeout()));
}

void FtpClient::initServer()
{
    connectTimer.start(15000);
    connectServer();
}

void FtpClient::stateChanged(int state)
{
    switch(state)
    {
    case QFtp::Unconnected:
        qDebug("Ftp:Unconnected...");
        break;
    case QFtp::HostLookup:
        qDebug("Ftp:HostLookup...");
        break;
    case QFtp::Connecting:
        qDebug("Ftp:Connecting...");
        break;
    case QFtp::Connected:
        qDebug("Ftp:Connected...");
        break;
    case QFtp::LoggedIn:
        qDebug("Ftp:LoggedIn...");
        break;
    case QFtp::Closing:
        qDebug("Ftp:Closing...");
        break;
    }
}

void FtpClient::ftpCommandStarted(int)
{
    int id = ftp->currentCommand();
    switch (id)
    {
    case QFtp::ConnectToHost :
        qDebug()<<tr("正在连接到服务器…");
        break;
    case QFtp::Login :
        qDebug()<<tr("正在登录…");
        break;
    case QFtp::Close :
        qDebug()<<"正在关闭连接…";
        break;
    case QFtp::Connecting:
        qDebug()<<(tr("连接中"));
        break;
        //    case QFtp::Get:
        //        qDebug()<<tr("下载中");
        //        break;
    default:
        break;
    }
}

void FtpClient::ftpCommandFinished(int id, bool error)
{
    if(heartTimer.isActive())
        heartTimer.stop();
    if(ftp->currentCommand() == QFtp::ConnectToHost)
    {
        if (error)
            qDebug()<<tr("连接服务器出现错误：%1").arg(ftp->errorString());
        else
            qDebug()<<tr("连接到服务器成功");
    }
    else if (ftp->currentCommand() == QFtp::Login)
    {
        if (error)
            qDebug()<<tr("登录出现错误：%1").arg(ftp->errorString());
        else
        {
            qDebug()<<tr("登录成功");
        }
        if(connectTimer.isActive())
            connectTimer.stop();
        uploadTimer.start(5000);
        emit sendFtpConnected(true);
    }
    else if (ftp->currentCommand() == QFtp::Close)
    {
        qDebug()<<tr("已经关闭连接");
    }
    else if (ftp->currentCommand() == QFtp::Put)
    {
        if(error)
            qDebug()<<tr("put error!：%1").arg(ftp->errorString())<<id;
        else
        {
            qDebug()<<"put success recv put id"<<id;
            if(deleteMap.contains(id))
            {
                unlink(deleteMap.value(id).toLocal8Bit().data());
                deleteMap.remove(id);
            }
        }
    }
    else if(ftp->currentCommand() == QFtp::Mkdir)
    {
        if (error)
            qDebug()<<tr("mkdir fail:%1").arg(ftp->errorString());
        else
            qDebug()<<tr("mkdir success");
        //        if (!add)            //克服闪烁
        //        {
        //            //            m_ui.treeIssue->clear();
        //        }
        //        //        m_Ftp->list();
        //        if (add)
        //        {
        //            QString createFoldPath;
        //            iter_fold++;
        //            if (iter_fold != m_fold.end())
        //            {
        //                createFoldPath = *iter_fold;
        //                ftp->mkdir(gbkCodeToFtpCode(createFoldPath));
        //            }
        //            else
        //            {
        //                if (m_record.isEmpty())
        //                {+
        //                    //                    m_ui.treeIssue->clear();
        //                }
        //                m_fold.clear();                  //文件夹创建完毕则清空存放文件夹路径的数据区
        //                if ( !m_record.isEmpty() )
        //                {
        //                    //上传文件
        //                    pcPath = iter_record.key();
        //                    ftpPath = iter_record.value();        //所在FTP目录体系中哪个目录下
        //                    iter_record++;
        //                    ftp->cd(gbkCodeToFtpCode(ftpPath));//包含文件的文件夹中有文件时先打开对应于要上传到的FTP所在目录级
        //                }
        //                else
        //                {
        //                    add = false;
        //                    //QMessageBox::information(this, tr("信息提示"), tr("上传成功！"));
        //                }
        //            }
        //        }
    }
    else if (ftp->currentCommand() == QFtp::Cd)
    {
        if (error)
            qDebug()<<tr("cd fail:%1").arg(ftp->errorString());
        //        else
        //            qDebug()<<tr("cd success");
        //        if (true == add && false == m_record.isEmpty())    //包含文件的文件夹中有文件时先打开对应于要上传到的FTP所在目录级
        //        {
        //            uploadFile(pcPath);                                               //在所有文件夹创建完毕后提交的第一个文件在这里执行,执行全部文件数次
        //        }
        //        else if (true == add && true == m_record.isEmpty())
        //        {
        //            add = false;
        //            //            btnReflashClicked();
        //            //	QMessageBox::information(this, tr("信息提示"), tr("上传成功！"));
        //        }
    }
    else if (ftp->currentCommand() == QFtp::List)
    {
        if (error) {
            qDebug() << "list Error: " << qPrintable(ftp->errorString());
        } else {
            qDebug() << "list success";
            //            qDebug() << "Downloaded " << qPrintable(currentDir) << " to "
            //                      << qPrintable(QDir::toNativeSeparators(
            //                                        QDir(currentLocalDir).canonicalPath()));
        }

        //        qDeleteAll(openedFiles);
        //        openedFiles.clear();

        processNextDirectory();
        //        if (isDirectory.isEmpty())
        //        {
        //            ui->fileList->addTopLevelItem(
        //                        new QTreeWidgetItem(QStringList()<< tr("<empty>")));
        //            ui->fileList->setEnabled(false);
        //            ui->label->setText(tr("该目录为空"));
        //        }
    }
    //    if(ftp->currentCommand() == QFtp::Put)
    //    {
    //        if(error)
    //            qDebug()<<"put error!";
    //        if(!error)
    //        {
    //            qDebug()<<"recv put id"<<id;
    //            if(id == ftpID)
    //                unlink(deleteFile.toLocal8Bit().data());
    //            //            if (!add)
    //            //            {
    //            //                //				m_ui.treeIssue->clear();
    //            //                //				m_Ftp->list();
    //            //            }

    //            //            if (add)
    //            //            {
    //            //                if (iter_record != m_record.end())     //当存放文件项的数据区未被遍历至最后时
    //            //                {
    //            //                    pcPath = iter_record.key();
    //            //                    ftpPath = iter_record.value();        //所在FTP目录体系中哪个目录下
    //            //                    iter_record++;
    //            //                    ftp->cd(gbkCodeToFtpCode(ftpPath));
    //            //                }
    //            //                else
    //            //                {
    //            //                    m_record.clear();                         //清空文件项为下次上传文件夹做准备
    //            //                    iter_record = m_record.begin();
    //            //                    //                    bool a = m_record.isEmpty();
    //            //                    ftp->cd(gbkCodeToFtpCode(m_sCurrentPath));
    //            //                }
    //            //            }
    //        }
    //    }
    if(error)
        QTimer::singleShot(5000,this,SLOT(heartTimeout()));
    else
        heartTimer.start(10000);
}

//void FtpClient::ftpDone()
//{
//    if(error)
//    {
//        qDebug()<<"ftp error";
//    }
//    else
//    {
//        qDebug()<<"list success";
//    }
//    if (error) {
//        qDebug() << "Error: " << qPrintable(ftp->errorString());
//    } else {
//        qDebug() << "Downloaded " << qPrintable(currentDir) << " to "
//                  << qPrintable(QDir::toNativeSeparators(
//                                    QDir(currentLocalDir).canonicalPath()));
//    }

//    qDeleteAll(openedFiles);
//    openedFiles.clear();

//    processNextDirectory();
//    qDebug()<<"allDirs"<<allDirs;
//}

// 连接按钮
void FtpClient::connectServer()
{
    //    add = false;
    ftp = new QFtp(this);
    connect(ftp, SIGNAL(commandStarted(int)), this, SLOT(ftpCommandStarted(int)));
    connect(ftp, SIGNAL(commandFinished(int, bool)),
            this, SLOT(ftpCommandFinished(int, bool)));
    //    connect(this, SIGNAL(listDone()), this, SLOT(ftpDone()));
    connect(ftp, SIGNAL(listInfo(const QUrlInfo &)), this, SLOT(ftpListInfo(const QUrlInfo &)));
    //    ftpServer = slave;//ui->ftpServerLineEdit->text();
    qDebug()<<"ftpclient connect to ftpServer"<<ftpServer;
    ftp->connectToHost(ftpServer, 21);
    ftp->login("root", "myzr");
}

void FtpClient::dirList()
{
    allDirs.clear();
    allFilePaths.clear();
    deleteMap.clear();
    //    ftp->cd("/tmp/Curve");
    //    ftp->list();
    QString path = "/tmp/Curve/FO";
    pendingDirs.append(path);
    allDirs.append(path);
    processNextDirectory();
}

void FtpClient::upload()
{
    //        int i = strFilePath.lastIndexOf('/');
    //        QString strTempName = strFilePath.right(strFilePath.length() - i - 1);

    m_sCurrentPath = "/tmp/Curve/FO";
    sPath = m_sCurrentPath;

    QString strFilePath = "/tmp/Curve/FO";
    QDir dir(strFilePath);
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);

    //文件夹优先
    dir.setSorting(QDir::DirsFirst);
    //转化成一个list
    QFileInfoList list = dir.entryInfoList();

    for(int i=0;i<list.size();i++)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileNm = fileInfo.fileName();
        if(fileNm.right(fileNm.size()-4).toInt()<11)
            continue;
        else
        {
            QString currentPath = gbkCodeToFtpCode(fileInfo.filePath());
            //            qDebug()<<"currentPath"<<currentPath;
            bool isExist = false;
            foreach (QString singledir, allDirs) {
                //                qDebug()<<"foreach1"<<singledir<<currentPath;
                if(singledir == currentPath)
                {
                    isExist = true;
                    break;
                }
            }
            if(!isExist)
                ftp->mkdir(currentPath);
            ftp->cd(currentPath);
            FindFileLocal(currentPath);
            ftp->cd(gbkCodeToFtpCode(strFilePath));
        }
    }


    //    QString sCurrentPath = m_sCurrentPath + "/" + strTempName;
    //    m_fold.append(sCurrentPath);        //将所选文件的FTP路径存放在相应数据区


    //    add = true;
    //    iter_fold = m_fold.begin();
    //    iter_record = m_record.begin();
    //    QString foldPath;
    //    if (NULL != iter_fold)
    //    {
    //        foldPath = *iter_fold;
    //    }
    //    else
    //        return;
    //    ftp->mkdir(gbkCodeToFtpCode(foldPath));

    //    m_sCurrentPath = sPath;
}

void FtpClient::connectTimeout()
{
    if(uploadTimer.isActive())
        uploadTimer.stop();
    ftp->disconnect();
    delete ftp;
    qDebug()<<"FTP connect timeout";
    connectServer();
}

void FtpClient::heartTimeout()
{
    if(heartTimer.isActive())
        heartTimer.stop();
    if(uploadTimer.isActive())
        uploadTimer.stop();
    emit sendFtpConnected(false);
    qDebug()<<"FTP heart Timeout";
    ftp->disconnect();
    delete ftp;
    connectTimer.start(15000);
    connectServer();
}

void FtpClient::ftpListInfo(const QUrlInfo &urlInfo)
{
    qDebug()<<"urlInfo"<<urlInfo.name();
    //    QTreeWidgetItem *item = new QTreeWidgetItem;
    //    item->setText(0, urlInfo.name());
    //    item->setText(1, QString::number(urlInfo.size()));
    //    item->setText(2, urlInfo.owner());
    //    item->setText(3, urlInfo.group());
    //    item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));
    //    QPixmap pixmap(urlInfo.isDir() ? "../myFTP/dir.png" : "../myFTP/file.png");
    //    item->setIcon(0, pixmap);
    //    isDirectory[urlInfo.name()] = urlInfo.isDir();
    //    ui->fileList->addTopLevelItem(item);
    //    if (!ui->fileList->currentItem()) {
    //        ui->fileList->setCurrentItem(ui->fileList->topLevelItem(0));
    //        ui->fileList->setEnabled(true);
    //    }
    if (urlInfo.isFile()) {
        allFilePaths.append(currentDir + "/" + urlInfo.name());
        qDebug()<<"list"<<allFilePaths.last();
        //            QFile *file = new QFile(currentLocalDir + "/"
        //                                    + urlInfo.name());

        //            if (!file->open(QIODevice::WriteOnly)) {
        //                qDebug() << "Warning: Cannot write file "
        //                          << qPrintable(QDir::toNativeSeparators(
        //                                            file->fileName()))
        //                          << ": " << qPrintable(file->errorString());
        //                return;
        //            }

        //ftp.get(urlInfo.name(), file);
        //            openedFiles.append(file);

    } else if (urlInfo.isDir() && !urlInfo.isSymLink()) {
        pendingDirs.append(currentDir + "/" + urlInfo.name());
        allDirs.append(currentDir + "/" + urlInfo.name());
        qDebug()<<"list"<<allDirs.last();
    }
}



void FtpClient::processNextDirectory()
{
    qDebug()<<"pendingDirs"<<pendingDirs;
    if (!pendingDirs.isEmpty()) {
        currentDir = pendingDirs.takeFirst();
        qDebug()<<"cd currentDir"<<currentDir;
        //        currentLocalDir = "downloads/" + currentDir;
        //        QDir(".").mkpath(currentLocalDir);

        ftp->cd(currentDir);
        ftp->list();
    } else {
        //        emit listDone();
        qDebug()<<"allDirs"<<allDirs;
        qDebug()<<"allFilePaths"<<allFilePaths;
        upload();
    }
}


bool FtpClient::uploadFile(QString pcPath)
{
    if(pcPath.length() == 0)
    {
        return false;
    }
    QFile *m_pFile = new QFile(pcPath);
    if (!m_pFile->open(QFile::ReadOnly))
    {
        m_pFile->close();
        return false;
    }
    int i = pcPath.lastIndexOf('/');
    QString sFileName = pcPath.right(pcPath.length() - i - 1);
    sFileName = gbkCodeToFtpCode(sFileName);
    int ftpID = ftp->put(m_pFile, sFileName);
    qDebug()<<"ftp put:"<<pcPath<<ftpID;
    deleteMap.insert(ftpID,pcPath);
    return true;
}

int FtpClient::FindFileLocal(const QString & sComFilePath)
{
    QDir dir(sComFilePath);
    if (!dir.exists())
    {
        return -1;
    }
    //取到所有的文件和文件名，但是去掉.和..的文件夹（这是QT默认有的）
    dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);

    //文件夹优先
    dir.setSorting(QDir::DirsFirst);
    //转化成一个list
    QFileInfoList list = dir.entryInfoList();
    if(list.size()< 1)
    {
        return -1;
    }
    int i = 0 ;
    //递归算法的核心部分
    do
    {
        QFileInfo fileInfo = list.at(i);
        //        QString fileNm = fileInfo.fileName();
        //如果是文件夹，递归
        bool bisDir = fileInfo.isDir();
        if(bisDir)
        {
            //添加各个文件夹
            QString currentPath = gbkCodeToFtpCode(fileInfo.filePath());
            //            qDebug()<<"currentPath"<<currentPath;
            //            ftp->list(currentPath);
            bool isExist = false;
            foreach (QString singledir, allDirs) {
                //                qDebug()<<"foreach2"<<singledir<<currentPath;
                if(singledir == currentPath)
                {
                    isExist = true;
                    break;
                }
            }
            if(!isExist)
                ftp->mkdir(currentPath);
            //            ftp->mkdir(currentPath);
            ftp->cd(currentPath);
            FindFileLocal(currentPath);
            ftp->cd(gbkCodeToFtpCode(sComFilePath));
        }
        else
        {
            //            QString str = fileNm.right(4);

            //            int i = fileNm.lastIndexOf('/');
            //            QString strName = fileNm.right(fileNm.length() - i - 1);
            //            QString strName = gbkCodeToFtpCode(fileNm);
            //            m_sCurrentPath = sCurrentPath;            //对应于FTP中的组织中的某个分支路径
            //            i = sComFilePath.lastIndexOf('/');
            //            QString name = sComFilePath.right(sComFilePath.length() - i - 1);
            //            ftp->cd(gbkCodeToFtpCode(name));
            //            QString tempPath = sComFilePath + "/" + fileNm;           //直到文件的全路径
            //            m_record[tempPath] = sComFilePath;                        //PC上文件路径做key 因为一个文件夹下可能有多个文件
            QString currentPath = gbkCodeToFtpCode(fileInfo.filePath());
            bool isExist = false;
            foreach (QString path, allFilePaths) {
                //                qDebug()<<"foreach3"<<path<<currentPath;
                if(path == currentPath)
                {
                    isExist = true;
                    break;
                }
            }
            if(!isExist)
                uploadFile(fileInfo.filePath());
        }
        i++;
    } while(i < list.size());
    return 0;
}

/*************************************************************************
*处理内容：将GBK编码格式转化为FTP编码格式
*@param：gbkCode		待转化字符串
*@return：
*@exception
**************************************************************************/
QString FtpClient::gbkCodeToFtpCode(const QString &gbkCode)
{
    QString TempResult;
    //1.提取数据
    size_t InStrBufSize = gbkCode.toAscii().size();
    char* InStrBuf = (char *)malloc(InStrBufSize);//= InputStr.toAscii().data();
    memset(InStrBuf, 0, InStrBufSize);
    if (InStrBuf == NULL)
    {
        return TempResult;
    }
    memcpy(InStrBuf, gbkCode.toAscii().constData(), InStrBufSize);
    //2.建立输出缓冲
    //输出缓冲大小要比Ascii字节*2后多两个字节 用于标识输出字符串结尾
    size_t OutStrBufSize = (InStrBufSize + 1) * sizeof(QChar);
    quint16* OutStrBuf = (quint16 *)malloc(OutStrBufSize);
    if (OutStrBuf == NULL)
    {
        return TempResult;
    }
    memset(OutStrBuf, 0, OutStrBufSize);
    //3.重新编码
    size_t i = 0;
    for(i = 0; i < InStrBufSize; i++)
    {
        OutStrBuf[i] = (quint8)InStrBuf[i];
    }
    //4.数据重新组装
    TempResult = QString::fromUtf16(OutStrBuf);
    free(InStrBuf);
    free(OutStrBuf);
    return TempResult;
}

#ifndef LOCATIONCONFIG_H
#define LOCATIONCONFIG_H

#include <QDialog>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include "tagmac.h"

namespace Ui {
class LocationConfig;
}

class LocationConfig : public QDialog
{
    Q_OBJECT

public:
    explicit LocationConfig(QWidget *parent = 0);
    ~LocationConfig();
    void initUI(QVariantMap);

    void initIp();
signals:
    void sendParameter(QVariantMap);
    void closeDiglag();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_tagMac_append_clicked();

    void on_pushButton_tagMac_remove_clicked();

    void on_pushButton_tagMac_update_clicked();


private:
    Ui::LocationConfig *ui;
    bool judgment();
    QVariantList tagMacList;
    enum tagMacOperate{tagMacAdd,tagMacUpdate};
    int currenttagMacOperate;
    int tagMacInsertRow;
    void newTagMac();
    QGraphicsOpacityEffect *e3;
    TagMac * tagMac;

public slots:
    void receiveTagMac(QString);
    void closeTagMac();

};

#endif // LOCATIONCONFIG_H

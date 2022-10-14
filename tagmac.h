#ifndef TAGMAC_H
#define TAGMAC_H

#include <QDialog>

namespace Ui {
class TagMac;
}

class TagMac : public QDialog
{
    Q_OBJECT

public:
    explicit TagMac(QWidget *parent = 0);
    ~TagMac();

    void initUI(QString);
signals:
    void sendParameter(QString);
    void closeDiglag();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::TagMac *ui;
    bool judgment();
};

#endif // TAGMAC_H

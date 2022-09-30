#ifndef BASESTATION_H
#define BASESTATION_H

#include <QDialog>
#include <QDebug>

namespace Ui {
class BaseStation;
}

class BaseStation : public QDialog
{
    Q_OBJECT

public:
    explicit BaseStation(QWidget *parent = 0);
    ~BaseStation();

    void initUI(QVariantMap);

signals:
    void sendParameter(QVariantMap);
    void closeDiglag();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::BaseStation *ui;
    bool judgment();
};

#endif // BASESTATION_H

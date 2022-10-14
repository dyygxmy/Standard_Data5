#ifndef WORKSHIFT_H
#define WORKSHIFT_H

#include <QDialog>
#include <QTime>
#include <QDebug>

namespace Ui {
class WorkShift;
}

class WorkShift : public QDialog
{
    Q_OBJECT

public:
    explicit WorkShift(QWidget *parent = 0);
    ~WorkShift();
    void getWorkShiftNames(QStringList);
    void WorkShiftInit(QStringList,int,QString,QTime,QTime,QString);

signals:
    void send_workShift(int,QString,QTime,QTime,QString);
    void send_Close();
    void delete_workShift(int);

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_cancel_clicked();

    void on_pushButton_StartHourAdd_clicked();

    void on_pushButton_StartHourSubstract_clicked();

    void on_pushButton_StartMinuteAdd_clicked();

    void on_pushButton_StartMinuteSubstract_clicked();

    void on_pushButton_EndHourAdd_clicked();

    void on_pushButton_EndHourSubstract_clicked();

    void on_pushButton_EndMinuteAdd_clicked();

    void on_pushButton_EndMinuteSubstract_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_delete_clicked();

private:
    Ui::WorkShift *ui;
    QStringList workShiftNames;
    int WorkShiftID;
    QString currentName;
};

#endif // WORKSHIFT_H


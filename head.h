#ifndef HEAD_H
#define HEAD_H

#include <QDialog>
#include <QDir>
#include <QDebug>
#include <QListWidgetItem>

namespace Ui {
class Head;
}

class Head : public QDialog
{
    Q_OBJECT

public:
    explicit Head(QWidget *parent = 0);
    ~Head();
    void headinit();

signals:
    void sendHead(QString);
    void sendClose();

private slots:
    void headselect(QListWidgetItem*);
    void on_pushButton_cancel_clicked();

private:
    Ui::Head *ui;
};

#endif // HEAD_H

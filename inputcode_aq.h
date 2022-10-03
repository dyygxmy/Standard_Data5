#ifndef INPUTCODE_AQ_H
#define INPUTCODE_AQ_H

#include <QDialog>

namespace Ui {
class Inputcode_AQ;
}

class Inputcode_AQ : public QDialog
{
    Q_OBJECT

public:
    explicit Inputcode_AQ(QWidget *parent = 0);
    ~Inputcode_AQ();

signals:
    void sendClose();
    void sendCode(QString);

private slots:
    void on_pushButton_cancel_clicked();

    void on_pushButton_clicked();

private:
    Ui::Inputcode_AQ *ui;
};

#endif // INPUTCODE_AQ_H

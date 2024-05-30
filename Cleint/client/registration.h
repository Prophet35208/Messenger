#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class Registration;
}

class Registration : public QDialog
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    ~Registration();

private slots:

    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();

public slots:
    void slotReadyRead();
    void slotConnectionEstablished();
private:
    Ui::Registration *ui;
    QTcpSocket* socket;
    QByteArray data;
    void SendToServer(QString str);
};

#endif // REGISTRATION_H

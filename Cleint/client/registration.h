#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QDialog>
#include <QTcpSocket>
#include <client.h>

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

    void on_pushButton_signin_clicked();
    void on_pushButton_register_clicked();
    void on_pushButton_connect_clicked();

public slots:
    void slotReadyRead();
    void slotConnectionEstablished();
private:
    Ui::Registration *ui;
    QTcpSocket* socket;
    QByteArray data;
    QString connection_address;
    // Пока что форма клиента будет открываться из окна регистрации.
    Client* w;

    // Режим работы для кнопок. От него зависит их название
    bool registry_mod=false;

    // Отправка на сервер строки
    void SendToServer(QString str);

    // Обработчики сообщений от сервера
    void ProcessLoginRespond(QStringList& str_list);
    void ProcessRegistrationRespond(QStringList& str_list);

    // Передача данных клиенту из окна регистрации
    void TransferClientInfo(QStringList &str_list, Client* client);

    // Вход и выход в режим регистрации
    void RegistryOn();
    void RegistryOff();


};

#endif // REGISTRATION_H

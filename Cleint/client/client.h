#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class Client;
}
QT_END_NAMESPACE


struct message {
    int message_num;
    int user_id_sender;
    int user_id_receiver;
    //int size_of_message;
    //QChar text[300];
    QString str;
};

// Для каждого контакта отведён минимум 1 чат, где текущий пользователь общается с контактом.
// Данная структура содержит сведения о контакте и ваших с ним сообщениях в чате
struct Contact
{
    QString login;
    qint64 chat_id;
    QList <message> message_list;

};


class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private:
    Ui::Client *ui;
    // Определяем сокет и массив данных для него
    QByteArray data;

    QList <Contact*> contact_list;
    // Запихиваем кол-во сообщений в чате, после чего все сообщения
    void ChatSerialization(QDataStream& stream, message* mas_message, int num_of_messages);
    // Достаём данные из бинарного потока
    int ChatUnSerialization(QDataStream& stream, message* mas_message);

    void QStringToQChar_ (QString& str,QChar *mas_char, int size);
        QTcpSocket *socket;
public:
    QString connection_address;
    QString login;

    void ReconnectToServer();
public slots:
    void slotReadyRead();
private slots:
    void on_pushButton_clicked();
    void on_pushButton_get_contact_clicked();
    void ProcessAddContactRespond(QStringList& str_list);
};
#endif // CLIENT_H

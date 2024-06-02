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
    // Запихиваем кол-во сообщений в чате, после чего все сообщения
    void ChatSerialization(QDataStream& stream, message* mas_message, int num_of_messages);
    // Достаём данные из бинарного потока
    int ChatUnSerialization(QDataStream& stream, message* mas_message);

    void QStringToQChar_ (QString& str,QChar *mas_char, int size);
public:
    QTcpSocket *socket;
private slots:
    void on_pushButton_clicked();
};
#endif // CLIENT_H

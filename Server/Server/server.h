#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>

// Заготовка к сообщениям
struct message {
    int message_num;
    int user_id_sender;
    int user_id_receiver;
    //int size_of_message;
    //QChar text[300];
    QString str;
};

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(const QString& path);
    ~Server();
// Буфер для сокета
    QTcpSocket *socket;

private:
// Вектор хранимых сокетов
    QVector <QTcpSocket*> sockets;
// Данные передаём в типе QByteArray,
    QByteArray data;
// Переменная для базы данных
    QSqlDatabase m_db;

// Передача данных клиенту (строки)
    void SentToClient(QString str);
// Обработчик регистрации. Вызвать при коде 2, передать массив строк - параметров
    void ProcessRegistry(QStringList& str_list);
// Обработчик регистрации. Вызвать при коде 2, передать массив строк - параметров
    void ProcessLogin (QStringList& str_list);
// Запихиваем кол-во сообщений в чате, после чего все сообщения
    void ChatSerialization(QDataStream& stream, message* mas_message, int num_of_messages);
// Достаём данные из бинарного потока
    int ChatUnSerialization(QDataStream& stream, message* mas_message);

public slots:
// Обрабтчик входящих подключений
    void incomingConnection(qintptr socket_descriptor);
// Обработчик сообщений
    void slotReadyRead();
};

#endif // SERVER_H

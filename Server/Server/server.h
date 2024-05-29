#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server();
// Буфер для сокета
    QTcpSocket *socket;

private:
// Вектор хранимых сокетов
    QVector <QTcpSocket*> sockets;
// Данные передаём в типе QByteArray,
    QByteArray data;
// Передача данных клиенту (строки)
    void SentToClient(QString str);

public slots:
// Обрабтчик входящих подключений
    void incomingConnection(qintptr socket_descriptor);
// Обработчик сообщений
    void slotReadyRead();
};

#endif // SERVER_H

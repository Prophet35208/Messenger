#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>

struct SubscriptedSocket{
    QString login;
    QTcpSocket* socket;
};

// Заготовка к сообщениям
struct message {
    int message_num; // id
    int user_id_sender;
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
// Список для сокетов, готовых получать уведомления
    QList <SubscriptedSocket*> list_subscribed_sockets;
// Вектор хранимых сокетов
    QVector <QTcpSocket*> sockets;
// Данные передаём в типе QByteArray,
    QByteArray data;
// Переменная для базы данных
    QSqlDatabase m_db;
// Лист для передачи параметров
    QStringList str_list;
// Передача данных клиенту (строки)
    void SentToClient(QStringList& str_list, int num_of_strings);
// Обработчик регистрации. Вызвать при коде 2, передать массив строк - параметров
    void ProcessRegistry(QStringList& str_list);
// Обработчик регистрации. Вызвать при коде 2, передать массив строк - параметров
    void ProcessLogin (QStringList& str_list);
// Обработчик получения контакта. Вызывать при коде 4.
    void ProcessGetContact (QStringList& str_list);
// Запихиваем кол-во сообщений в чате, после чего все сообщения
    void ChatSerialization(QDataStream& stream, message* mas_message, int num_of_messages);
// Достаём данные из бинарного потока
    int ChatUnSerialization(QDataStream& stream, message* mas_message);
// Инициализируем список парамтров, которые нужно передать клиенту для инициализации его контактов и чатов
    int InitializeParamsForClientStartUp (QStringList &str_list, QString str_user_login);
// Добавляет клиентов в список, в котором они считаются онлайн и получают обновления.
// Главным образом позволяет связать сокет и логин.
    void ProcessSubscriptionForUpdates(QTcpSocket* socket, QString str);
// Получаем сообщение от клиента
void ProcessMessageFromClient(QStringList& str_list);
// Отправка всем подписавшемся о том, что их чат изменён (добавлено сообщение)
    void NotifyAboutNewMessage(int message_id);
public slots:
// Обрабтчик входящих подключений
    void incomingConnection(qintptr socket_descriptor);
// Обработчик сообщений
    void slotReadyRead();
// Позволяет удалить невалидный сокет из списка + удалить подписавшегося пользователя
    void slotDisconnectUser();
};

#endif // SERVER_H

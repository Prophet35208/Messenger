#include "server.h"

Server::Server(const QString& path)
{
    // Подключаем БД
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    m_db.open();

    // Проверяем работоспособность БД
    if (!m_db.open())
    {
        qDebug() << "Error: connection with database failed" << m_db.lastError().text();
    }
    else
    {
        qDebug() << "Database: connection ok";
    }




    // Начинаем прослушивать порт Tcp/Udp по всем сетевым интерфейсам.
    if (this->listen(QHostAddress::Any,2323)){
        qDebug() << "Server started";
    }
    else
    {
        qDebug() << "Server error";
    }

}

Server::~Server()
{
    m_db.close();
}

// Принимаем подключение
void Server::incomingConnection(qintptr socket_descriptor){
    // Создаём сокет и устанвливаем его дескриптор
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socket_descriptor);
    // Соединяем сигналы
    // Сигнал readyRead сокета (инофмрация готова к отправке) с нашей ф-ией обработки
    // Сигнал disconnected сокета со слотом на удаление сокета
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    //Сохраняем сокет в векторе
    sockets.push_back(socket);
}

// Обработка сообщений из сокета
void Server::slotReadyRead()
{
    // Получим ссылку на сокет, который вызвал данный слот
    socket = (QTcpSocket*)sender();

    // Используем QDataStream для обработки ввода
    // (Возможно стоит установить конкретную версию)
    QDataStream in(socket);
    in.setVersion(QDataStream::Version::Qt_6_7);
    // Смотрим, правильно ли инициализировался объект
    if(in.status()== QDataStream::Ok)
    {
        qDebug() << "Read stream ok";
        //Теперь выведем данные из потока в консоль
        QString str;
        in >> str;

        //Тест
        if(str == "4"){
            // Получаем структуру
            message mas_mes[2];
            int num_of_messages;

           num_of_messages = ChatUnSerialization(in,mas_mes);
        }

        // Строку получили. Теперь её нужно обработать. Для этого смотрим на первое число - флаг.
        QStringList str_list;
        //and password = (:password)
        str_list = str.split("$");
        if (str_list[0]=="1"){
            ProcessLogin (str_list);
        }
        if (str_list[0]=="2"){
            ProcessRegistry (str_list);
        }



    }
    else
    {
        qDebug() << "Problem with read stream";
    }
}

void Server::SentToClient(QString str)
{
    data.clear();
    // Инициализируем поток на вывод. С его помощью запишем str в data
    QDataStream out(&data, QIODevice::WriteOnly);
    out << str;
    socket->write(data);
}
// Обрабатываем регистрацию. Смотрим есть ли такой человек, если нет - то одобряем.
void Server::ProcessRegistry(QStringList &str_list)
{
    QSqlQuery query;
    query.prepare("SELECT login FROM user WHERE login = :login");
    query.bindValue(":login", str_list[1]);

    if (query.exec())
    {

        if (query.next())
        {
            // Пользователь существует, значит аккаунт создать нельзя. Передаём код 2.
            SentToClient("2");

        }
        else
        {
            // Такого пользователя нет, аккаунт создать можно. Передаём код 3
            // Но сначала создаём запись в БД.

            query.prepare("INSERT INTO user (login, password) VALUES (:login, :password)");
            query.bindValue(":login", str_list[1]);
            query.bindValue(":password", str_list[2]);

            if(query.exec())
            {
                 SentToClient("3");
            }
            else
            {
                qDebug() << "AddUser error:"
                         << query.lastError();
            }

        }
    }
    else
    {
        qDebug() << "Ошибка при выполнении query-запроса";
    }

}

void Server::ProcessLogin(QStringList &str_list)
{
    QSqlQuery query;
    query.prepare("SELECT login FROM user WHERE login = :login and password = :password");
    query.bindValue(":login", str_list[1]);
    query.bindValue(":password", str_list[2]);
    if (query.exec())
    {

        if (query.next())
        {
            // Пользователь существует и пароль подходит. Теперь нужно передать, что он подключился
            // todo: В будущем нужно сделать передачу некоторогот значения, которое бы сгенерировалось
            // сервером и свидетельствовало о том, что пользователь авторизован. Пока что возвратим код 1
            SentToClient("1");

        }
        else
        {
            // Такого пользователя нет или пароль не верный. Возвращаем код ошибки
            SentToClient("-1");
        }
    }
    else
    {
        qDebug() << "Ошибка при выполнении query-запроса";
    }
}

void Server::ChatSerialization(QDataStream &stream, message *mas_message, int num_of_messages)
{
    stream << num_of_messages;
    for (int i = 0; i < num_of_messages; ++i) {
        stream << (*(mas_message+i)).message_num;
        stream << (*(mas_message+i)).user_id_sender;
        stream << (*(mas_message+i)).user_id_receiver;
        stream << (*(mas_message+i)).str;
    }
}

int Server::ChatUnSerialization(QDataStream &stream, message *mas_message)
{
    int num_of_messages;
    stream >> num_of_messages;
    for (int i = 0; i < num_of_messages; ++i) {
        stream >> (*(mas_message+i)).message_num;
        stream >> (*(mas_message+i)).user_id_sender;
        stream >> (*(mas_message+i)).user_id_receiver;
        stream >> (*(mas_message+i)).str;
    }
    return num_of_messages;
}

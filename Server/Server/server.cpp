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

        // Строку получили. Теперь её нужно обработать. Для этого смотрим на первое число - флаг.
        QStringList str_list;
        QString str;
        //and password = (:password)
        in >> str;
        str_list.append(str);

        if (str_list[0]=="4"){
            in >> str;
            str_list.append(str);
            ProcessGetContact (str_list);
        }

        if (str_list[0]=="1"){
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessLogin (str_list);
        }

        if (str_list[0]=="2"){
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessRegistry (str_list);
        }

    }
    else
    {
        qDebug() << "Problem with read stream";
    }
}
// Передаём совокупность строк клиенту
void Server::SentToClient(QStringList& str_list, int num_of_strings)
{
    data.clear();
    // Инициализируем поток на вывод. С его помощью запишем str в data
    QDataStream out(&data, QIODevice::WriteOnly);
    for (int i = 0; i < num_of_strings; ++i) {
        out << str_list[i];
    }
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
            str_list.clear();
            str_list.append("2");
            SentToClient(str_list,1);

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
                str_list.clear();
                str_list.append("3");
                SentToClient(str_list,1);
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
            // В будущем можно сделать передачу некоторого значения, которое бы сгенерировалось
            // сервером и свидетельствовало о том, что пользователь авторизован. Пока что возвратим код 1
            str_list.clear();
            str_list.append("1");
            SentToClient(str_list,1);

        }
        else
        {
            // Такого пользователя нет или пароль не верный. Возвращаем код ошибки
            str_list.clear();
            str_list.append("-1");
            SentToClient(str_list,1);
        }
    }
    else
    {
        qDebug() << "Ошибка при выполнении query-запроса";
    }
}
// Обрабатываем получение контакта.
void Server::ProcessGetContact(QStringList &str_list)
{
    // Нужно получить логин, чекнуть бд. Если нет, то говорим, что нет такого. Если да, то смотрим,
    QString id_chat;
    // Первый это отправитель, второй - получатель (тот, кого передали в login)
    QString id_first_user;
    QString id_second_user;

    QSqlQuery query;
    query.prepare("SELECT login FROM user WHERE login = :login and password = :password");
    query.bindValue(":login", str_list[1]);
    if (query.exec())
    {

        if (query.next())
        {
            // Пользователь существует. Теперь нужно создать соответствующий чат
            // Создаём пустой чат
            query.prepare("INSERT INTO chat (id_last_message) VALUES (-1, 2)");
            query.exec();

            id_chat =  query.lastInsertId().toString();

            // Пользователь отправитель
            query.prepare("Select pk_user IN user Where login = :login");
            query.bindValue(":login", str_list[1]);
            query.exec();
            id_first_user = query.value(0).toString();

            // Пользователь получатель
            query.prepare("Select pk_user IN user Where login = :login");
            query.bindValue(":login", str_list[2]);
             query.exec();
            id_second_user = query.value(0).toString();

             if (id_first_user != "" && id_second_user != ""){
                // В таблице user_in_chat привязываем пользователей к нему
                query.prepare("INSERT INTO user_in_chat (pk_user, pk_chat) VALUES (:id_first_user, :id_chat), (:id_second_user, :id_chat)");
                query.bindValue(":id_second_user", id_second_user);
                query.bindValue(":id_first_user", id_first_user);
                query.bindValue(":id_chat", id_chat);
                query.exec();

                // Отправляем пользователю сообщение о том, что данные добавлены в БД (код 5) и клиент может запустить чат у себя.
                // Так же отправим чат id, чтобы клиент понимал что это за чат и логин того, для кого чат создавался.
                str_list.clear();
                str_list.append("5");
                str_list.append(id_chat);
                str_list.append(id_second_user);
                SentToClient(str_list,3);
             }
             else
             {
                 if(id_first_user == ""){
                     // Возвращаем код ошибки. Не найден пользователь
                     str_list.clear();
                     str_list.append("-2");
                     SentToClient(str_list,1);
                 }
                 else{
                     // Возвращаем код ошибки. Другая ошибка
                     str_list.clear();
                     str_list.append("-1");
                     SentToClient(str_list,1);
                 }
             }

        }
        else
        {
            //  Маловероятная ошибка. Возвращаем код ошибки
            str_list.clear();
            str_list.append("-1");
            SentToClient(str_list,1);
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

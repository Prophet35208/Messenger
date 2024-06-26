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
    this->listen(QHostAddress::Any,2323);

}

Server::~Server()
{
    m_db.close();
}

// Принимаем подключение
void Server::incomingConnection(qintptr socket_descriptor){
    // Создаём сокет и устанавливаем его дескриптор
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socket_descriptor);
    // Соединяем сигналы
    // Сигнал readyRead сокета (инофмрация готова к отправке) с нашей ф-ией обработки
    // Сигнал disconnected сокета со слотом на удаление сокета
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::slotDisconnectUser);

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
        in >> str;
        str_list.append(str);



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

        if (str_list[0]=="4"){
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessGetContact (str_list);
        }

        if (str_list[0]=="5")
        {
            in >> str;
            str_list.append(str);
            ProcessSubscriptionForUpdates((QTcpSocket*)sender(),str_list[1]);
        }


        if (str_list[0]=="6")
        {
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessMessageFromClient(str_list);
        }

        if (str_list[0]=="7")
        {
            in >> str;
            str_list.append(str);

            int num = str_list[1].toInt();

            for (int i = 0; i < num; ++i) {
                in >> str;
                str_list.append(str);
            }
            ProcessCreateGroupChat(str_list);
        }

        if (str_list[0]=="8")
        {
            in >> str;
            str_list.append(str);

            DeleteMessage(str_list);
        }


    }
    else
    {
        qDebug() << "Problem with read stream";
    }
}

void Server::slotDisconnectUser()
{
    // Удаляем пользователя из подписавшихся, после удаляем сокет
    QTcpSocket* socket = (QTcpSocket*)sender();

    int f=0;
    auto it = list_subscribed_sockets.begin();
    for (it; it != list_subscribed_sockets.end(); it++) {
        if ((*it)->socket == socket)
        {
            f=1;
            break;
        }
    }
    if (f)
        list_subscribed_sockets.erase(it);

    socket->deleteLater();

}

// Получаем сокет и логин, на их основе организуем объект структуры и добавляем в список
void Server::ProcessSubscriptionForUpdates(QTcpSocket* socket, QString str)
{
    SubscriptedSocket* sub_socket = new SubscriptedSocket();
    sub_socket->socket = socket;
    sub_socket->login = str;
    list_subscribed_sockets.append(sub_socket);
}

void Server::ProcessMessageFromClient(QStringList &str_list)
{
    // Получаем 4 параметра: код, логин отправителя, id чата, текст сообщения.
    // Нужно занести сообщение в базу и отправить всем подписанным юзерам (которые имеют чаты для обновления) уведомление
    QString id_last_message;
    QString id_user_sender;
    QString id_inserted_message;
    // Добавление в БД
    // Получаем последнее сообщение чата
     QSqlQuery query;
    query.prepare("Select id_last_message From chat Where pk_chat = :id_chat");
    query.bindValue(":id_chat", str_list[2]);
    query.exec();


    if (query.next())
        id_last_message = query.value(0).toString();

    // Учитываем, что чат может быть пустой
    if (id_last_message == "")
        id_last_message = "-1";

    // Получаем id юзера
    query.prepare("Select pk_user From user Where login = :login");
    query.bindValue(":login", str_list[1]);
    query.exec();
    if (query.next())
        id_user_sender = query.value(0).toString();

    // Теперь добавим новое, нужно чтобы оно указывало на предыдущее, а чат указывал на новое как на последнее
    query.prepare("Insert Into message (pk_previous, text, pk_user_sender, pk_chat) VALUES (:id_last_message, :text, :sender,:id_chat)");
    query.bindValue(":id_last_message", id_last_message);
    query.bindValue(":text", str_list[3]);
    query.bindValue(":sender", id_user_sender);
    query.bindValue(":id_chat", str_list[2]);
    // (Сюда можно добавить обработчик в случае оишбки)
    query.exec();
    id_inserted_message = query.lastInsertId().toString();

    // Обновляем параметр чата, чтобы он указывал на новое сообщение как последнее
    query.prepare("Update chat Set id_last_message = :message_id Where pk_chat = :id_chat");
    query.bindValue(":message_id", id_inserted_message);
    query.bindValue(":id_chat", str_list[2]);
    query.exec();

    // Теперь нужно отослать уведомление тем подписавшимся юзерам, которые находятся в данном чате
    // Уведомление о новом сообщении, параметры: код (6),  логин юзера отправителя, id сообщения, id чата, текст сообщения

    // Подготовка параметров
    QStringList str_params;
    str_params.append("6");
    str_params.append(str_list[1]);
    str_params.append(id_inserted_message);
    str_params.append(str_list[2]);
    str_params.append(str_list[3]);

    // Получение списка юзеров чата
    QStringList str_logins;
    query.prepare("Select login From user Where pk_user In (Select pk_user From user_in_chat Where pk_chat = :id_chat)");
    query.bindValue(":id_chat", str_list[2]);
    query.exec();
    while (query.next()){
        str_logins.append(query.value(0).toString());
    }

    int num_of_users;
    num_of_users = str_logins.size();
    int num_of_subscribed_sockets;
    num_of_subscribed_sockets = list_subscribed_sockets.size();

    // Инициализация потока для передачи
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    for (int i = 0; i < 5; ++i) {
        out << str_params[i];
    }

    // Передача параметров всем подписавшимся
    for (int i = 0; i < num_of_users; ++i) {
        for (int j = 0; j < num_of_subscribed_sockets; ++j) {
            if (str_logins[i] == list_subscribed_sockets[j]->login)
            {
                list_subscribed_sockets[j]->socket->write(data);
                break;
            }
        }
    }


}

// Пока что не используется
void Server::NotifyAboutNewMessage(int message_id)
{
    // Имеем id сообщения, нужно получить из бд чат, в котором оно находится. После этого список пользователей чата.
    // После этого для всех тех кто подписался, отсылваем данные о новом сообщении: id сообщения, id чата, логин отправителя.

    // Получение id чата
    QString id_chat;
    QSqlQuery query;
    query.prepare("Select pk_chat From message Where pk_message = :id_message");
    query.bindValue(":id_message", message_id);
    query.exec();
    if (query.next())
        id_chat = query.value(0).toString();

    // Получение списка id юзеров
    QStringList user_id_list;
    query.prepare("Select pk_user From user_in_chat Where pk_chat = :id_chat");
    query.bindValue(":id_chat", id_chat);
    query.exec();
    while (query.next()){
        user_id_list.append(query.value(0).toString());
    }

    int num_of_users = user_id_list.size();
    QStringList user_login_list;

    // Получение списка логинов пользователей
    for (int i = 0; i < num_of_users; ++i) {

        query.prepare("Select login From user Where pk_user = :id_user");
        query.bindValue(":id_user", user_id_list[i]);
        query.exec();
        if (query.next())
            user_login_list.append(query.value(0).toString());

    }
    // todo: тут нужна функция отправки оповещений

}

// Передаём совокупность строк клиенту, передача идёт на сокет в буфере!
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
    QString login  = str_list[1];
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
            int num_of_params;
            // Передаём клиенту данные о чатах
            num_of_params = InitializeParamsForClientStartUp(str_list,login);
            SentToClient(str_list,num_of_params);

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
    QString buf_login =str_list[2] ;
    QString id_chat("");
    // Первый это отправитель, второй - получатель (тот, кого передали для добавления в контакты)
    QString id_first_user("");
    QString id_second_user("");

    QSqlQuery query;
    query.prepare("SELECT login FROM user WHERE login = :login");
    query.bindValue(":login", str_list[1]);
    if (query.exec())
    {

        if (query.next())
        {
            // Пользователь существует. Можем работать

            // Пользователь отправитель
            query.prepare("Select pk_user FROM user Where login = :login");
            query.bindValue(":login", str_list[1]);
            query.exec();
            if (query.next())
                id_first_user = query.value(0).toString();

            // Пользователь получатель
            query.prepare("Select pk_user FROM user Where login = :login");
            query.bindValue(":login", str_list[2]);
            query.exec();
            if (query.next())
                id_second_user = query.value(0).toString();

            // Если оба пользователя в БД, тогда можем создавать и привязывать чат. Так же не допускаем добавления себя же
             if (id_first_user != "" && id_second_user != "" && id_first_user != id_second_user){

                // Создаём пустой чат
                query.prepare("INSERT INTO chat (id_last_message,num_of_users) VALUES (-1, 2)");
                if(query.exec()){}
                    else {
                        qDebug() << "Ошбика при запросе вставке чата";
                    }
                id_chat =  query.lastInsertId().toString();

                // В таблице user_in_chat привязываем пользователей к нему
                query.prepare("INSERT INTO user_in_chat (pk_user, pk_chat) VALUES (:id_first_user, :id_chat), (:id_second_user, :id_chat)");
                query.bindValue(":id_second_user", id_second_user);
                query.bindValue(":id_first_user", id_first_user);
                query.bindValue(":id_chat", id_chat);
                if(query.exec()){}
                else {
                    qDebug() << "Ошбика при запросе на связывание чата";
                }


                // Отправляем пользователю сообщение о том, что данные добавлены в БД (код 5) и клиент может запустить чат у себя.
                // Так же отправим чат id, чтобы клиент понимал что это за чат и логин того, для кого чат создавался.
                str_list.clear();
                str_list.append("5");
                str_list.append(id_chat);
                str_list.append(buf_login);
                SentToClient(str_list,3);
             }
             else
             {
                 // Можно дополнить дополнительным условиями для определения конкретики ошибки
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

void Server::ProcessCreateGroupChat(QStringList &str_list)
{
    // Получаем 2+n параметров, код, кол-во пользователей и сами пользователи
    // Нужно чекнуть, можем ли вообще создать чат. Если да, то создаём и отправляем оповещение об этом всем подписавшимся, кто состоит в чате

    int num_of_users = str_list[1].toInt();
    QStringList str_logins;

    // Перенос логинов в отдельный список, чтобы не запутаться
    for (int i = 0; i < num_of_users; ++i) {
        str_logins.append(str_list[2+i]);
    }
    // Получение всем id-ков юзеров
    QStringList str_ids;
    QSqlQuery query;
    int f = 0; // Предполаем, что все логины есть в базе
    for (int i = 0; i < num_of_users; ++i) {
        query.prepare("Select pk_user From user WHere login = :login");
        query.bindValue(":login", str_logins[i]);
        query.exec();
        if (query.next()){
            str_ids.append(query.value(0).toString());
            if (str_ids[i] == "")
                f = 1;
        }
    }

    if (f){
        // Не все логины в базе
        qDebug() << "Cant find login";
    }
    else
    {
        // Нужно создать чат на нужно кол-во пользователей и связать юзеров с этим чатом

        // Создание чата

        query.prepare("Insert Into chat(id_last_message, num_of_users) VALUES (-1, :num_of_users )");
        query.bindValue(":num_of_users", num_of_users);
        query.exec();
        QString id_chat = query.lastInsertId().toString();

        // Связывание пользователей
        for (int i = 0; i < num_of_users; ++i) {

            query.prepare("Insert Into user_in_chat(pk_user, pk_chat) VALUES (:id_user,:id_chat)");
            query.bindValue(":id_user", str_ids[i]);
            query.bindValue(":id_chat", id_chat);
            query.exec();
        }

        // Теперь нужно уведомить юзеров о созданном чате
        // Отсылаем клиенту код 7, id чата, кол-во юзеров вместе с ним, список юзеров вместе с ним
        QStringList str_params;
        QString str;
        // Подготовка параметров
        str = "7";
        str_params.append(str);

        str_params.append(id_chat);
        str = QString::number(num_of_users);
        str_params.append(str);

        for (int i = 0; i < num_of_users; ++i) {
            str_params.append(str_logins[i]);
        }


        // Инициализация потока для передачи
        data.clear();
        QDataStream out(&data, QIODevice::WriteOnly);
        for (int i = 0; i < 3+num_of_users; ++i) {
            out << str_params[i];
        }

        int num_of_subscribed_sockets = list_subscribed_sockets.size();


        // Передача параметров всем подписавшимся
        for (int i = 0; i < num_of_users; ++i) {
            for (int j = 0; j < num_of_subscribed_sockets; ++j) {
                if (str_logins[i] == list_subscribed_sockets[j]->login)
                {
                    list_subscribed_sockets[j]->socket->write(data);
                    break;
                }
            }
        }

    }




}

void Server::DeleteMessage(QStringList &str_list)
{
    // 2 параметра, второй - id сообщения

    // Находим сообщение, получаем id прошлого и последующего
    QSqlQuery query;

    QString id_previous;
    QString id_next;

    query.prepare("Select pk_previous From message Where pk_message =:id_message");
    query.bindValue(":id_message", str_list[1]);
    query.exec();
    if (query.next())
        id_previous = (query.value(0).toString());

    query.prepare("Select pk_message From message Where pk_previous = :id_message");
    query.bindValue(":id_message", str_list[1]);
    query.exec();
    if (query.next())
        id_next = (query.value(0).toString());
    // 3 ситуации: в конце, середине, начале


    // Если по середине
    if (id_previous != "-1"   && id_next != "")
    {
        query.prepare("Update message Set pk_previous =:id_previous Where pk_message =:id_next");
        query.bindValue(":id_previous", id_previous);
        query.bindValue(":id_next", id_next);
        if(query.exec());
        else
        {
            qDebug() << "Some problem with query";
        }
    }

    // Если в конце, меняем значение chat
    if (id_previous != "-1" && id_next == "")
    {
        query.prepare("Update chat Set id_last_message = :id_previous Where pk_chat = (Select pk_chat From message Where pk_message =:id_previous )");
        query.bindValue(":id_previous", id_previous);
        if(query.exec());
        else
        {
            qDebug() << "Some problem with query";
        }
    }

    // Если в начале, то у следующего ставим -1 в previous
    if (id_previous == "-1" && id_next != "")
    {
        query.prepare("Update message Set pk_previous = :minus Where pk_message = :id_next ");
        query.bindValue(":id_next", id_next);
        query.bindValue(":minus", "-1");
        if(query.exec());
        else
        {
            qDebug() << "Some problem with query";
        }
    }
    // Если вообще последнее, то у чата ставим -1
    if (id_previous == "-1" && id_next == "")
    {
        query.prepare("Update chat Set id_last_message = :minus Where pk_chat = (Select pk_chat From message Where pk_message = :id_message)");
        query.bindValue(":id_message", str_list[1]);
        query.bindValue(":minus", "-1");
        if(query.exec());
        else
        {
            qDebug() << "Some problem with query";
        }
    }

    // Находим юзеров, связанных с ним
    QStringList user_logins;
    query.prepare("Select login From user Where pk_user In (Select pk_user From user_in_chat Where pk_chat = (Select pk_chat From message Where pk_message = :id_message))");
    query.bindValue(":id_message", str_list[1]);
    query.exec();
    while (query.next())
        user_logins.append((query.value(0).toString()));

    // Вспоминаем чат, к которому относится сообщение
    QString id_chat;
    query.prepare("Select pk_chat From message Where pk_message = :id_message");
    query.bindValue(":id_message", str_list[1]);
    query.exec();
    if (query.next())
        id_chat = (query.value(0).toString());

    // Теперь удаляем
    query.prepare("Delete From message Where pk_message =:id_message");
    query.bindValue(":id_message", str_list[1]);
    query.exec();


    // Говорим подписавшимся, что удалили сообщение с кодом n в чате m
    // Отсылаем клиенту код 8, id чата, id сообщения
    QStringList str_params;
    QString str;
    // Подготовка параметров
    str = "8";
    str_params.append(str);
    str_params.append(id_chat);
    str_params.append(str_list[1]);

    // Инициализация потока для передачи
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    for (int i = 0; i < 3; ++i) {
        out << str_params[i];
    }

    int num_of_subscribed_sockets = list_subscribed_sockets.size();
    int num_of_users = user_logins.size();

    // Передача параметров всем подписавшимся
    for (int i = 0; i < num_of_users; ++i) {
        for (int j = 0; j < num_of_subscribed_sockets; ++j) {
            if (user_logins[i] == list_subscribed_sockets[j]->login)
            {
                list_subscribed_sockets[j]->socket->write(data);
                break;
            }
        }
    }
}

// Кладёт в сокет параметры: 1, кол-во контактов, (логин контакта1, id чата с ним, сам чат (ф-ией ChatSerialization) n раз
int Server::InitializeParamsForClientStartUp(QStringList &str_list, QString str_user_login)
{
    // Здесь мы создаём список строк, передающий переменной число параметров.
    // Нужно передать:
    // код операции (код 1), кол-во контактов, (логин контакта1, id чата с ним, сам чат (ф-ией ChatSerialization), ..., логин контактаX, id чата сним, сам чат)
    // Можем даже не применять ф-ию ChatSerialization, чтобы не делать лишние операции. Для каждого сообщения важен отправитель,текст и (в будущем можно добавить id)
    QSqlQuery query, query2;
    QString id_user;
    int num_of_params;
    // Данные 2 списка связаны. Псоле выборки под одними и теми же индексами будут находится id чата и id соответствующего контакта
    // Так например если юзер имел чат с пользователем с id = 1 в чате с id = 2, то попав в слот 1 списков будет следующее:
    // list_of_chats_id[1] = 2; list_of_logins_id[1] = 1
    // Так же и со списком list_of_logins
    QStringList list_of_chats_id;
    QStringList list_of_logins_id;
    QStringList list_of_logins;
    int buf;
    // Первый параметр
    str_list.clear();
    str_list.append("1");

    // Смотрим id пользователя
    query.prepare("SELECT pk_user FROM user WHERE login = :login");
    query.bindValue(":login", str_user_login);
    query.exec();
    if (query.next())
       id_user = query.value(0).toString();

    // Определяем его чаты
    query.prepare("Select pk_chat From chat Where num_of_users = 2 AND pk_chat IN (Select pk_chat From user_in_chat Where pk_user = :user_id)");
    query.bindValue(":user_id", id_user);
    query.exec();
    while (query.next()){
        list_of_chats_id.append(query.value(0).toString());
    }

    // Имеем список всех 1 на 1 чатов. Их кол-во будет равно кол-ву контактов. Можем сразу подготовить параметр
    buf = list_of_chats_id.size();
    QString s = QString::number(buf);
    str_list.append(s);


    // Смотрим число контактов и создаём их список, нам важны их id для следующих выборок и login для отправки клиенту
    query.prepare("Select pk_user  From user_in_chat Where pk_chat = :id_chat and pk_user != :id_current_user");
    query.bindValue(":id_current_user", id_user);

    query2.prepare("Select login From user Where pk_user = :user_id");

    for (int i = 0; i < buf; ++i) {
        query.bindValue(":id_chat", list_of_chats_id[i]);
        query.exec();
        if (query.next())
            list_of_logins_id.append(query.value(0).toString());

        query2.bindValue(":user_id", list_of_logins_id[list_of_logins_id.size()-1]);
        query2.exec();
        if (query2.next())
            list_of_logins.append(query2.value(0).toString());
    }

    // После прошлой операции у нас массив id чатов, логины и id с кем они связаны.
    // Остаётся для каждого чата найти все его сообщения
    // Следует понимать, что на сервере сообщения реализованы в виде односвязного списка.
    // На клиенте реализация в виде списка, через сокет мы передаём совокупность сообщений.
    // Заполняем list_of_chats_id.size() троек параметров (логин контакта1, id чата с ним, сам чат)
    QString last_message_id;
    QString num_of_messages;
    QString buf_str;
    QString buf_mes_text;
    for (int i = 0; i < list_of_chats_id.size(); ++i) {

        str_list.append(list_of_logins[i]);
        str_list.append(list_of_chats_id[i]);


        // Получим номер последнего сообщения
        query.prepare("Select id_last_message From chat Where pk_chat = :id_chat");
        query.bindValue(":id_chat", list_of_chats_id[i]);
        query.exec();
        if (query.next()){
            buf_str =  query.value(0).toString();
            last_message_id = buf_str;
        }
        if (buf_str == "-1")
        {
            // Сообщений нет
            str_list.append("0");

        }
        else
        {
            // Сообщения есть

            // Получим общее число сообщений
            query.prepare("Select count() From message Where pk_chat = :id_chat");
            query.bindValue(":id_chat", list_of_chats_id[i]);
            query.exec();
            if (query.next()){
                buf =  query.value(0).toInt();
                str_list.append(query.value(0).toString());
            }

            QString buf_next_message;
            // По очереди вытаскиваем сообщения и заносим их данные в сокет
            for (int i = 0; i < buf; ++i) {
                query.prepare("Select pk_previous, text, pk_user_sender, pk_previous  From message Where pk_message = :last_message_id");
                query.bindValue(":last_message_id", last_message_id);
                query.exec();
                if (query.next()){
                    buf_str  =  query.value(2).toString();
                    buf_mes_text = query.value(1).toString();
                    buf_next_message = query.value(3).toString();

                }

                // Найдём логин отправителя и сразу заносим 2 оставшихся параметра сообщения
                query.prepare("Select login From user Where pk_user = :user_id");
                query.bindValue(":user_id", buf_str);
                query.exec();
                if (query.next()){
                    buf_str  =  query.value(0).toString();
                    str_list.append(buf_str);
                    str_list.append(buf_mes_text);
                    str_list.append(last_message_id);
                }
                // Идём к следующему
                last_message_id = buf_next_message;

            }
        }
    }

    // Дополнение: нужно дополнить сведениями о групповых чатах.
    // Структура кол-во чатов, (id чата, кол-во участников, логины участников (n штук), кол-во сообщений и сами сообщения)
    // Сообщения передадим так же как и выше

    QStringList list_of_chats_id_group;
    QStringList list_of_num_users_in_chat_group;
    // Нам нужно хранить переменное число пользователей для каждого чата, чьё кол-во тоже переменное
    QList <QStringList> list_of_lists_of_users_id;
    QList <QStringList> list_of_lists_of_users_logins;

    // Ищем чаты, где кол-во участников больше 2
    // Записываем их id и кол-во участников в списки
    query.prepare("Select pk_chat, num_of_users From chat Where num_of_users > 2 AND pk_chat IN (Select pk_chat From user_in_chat Where pk_user = :id_user)");
    query.bindValue(":id_user", id_user);
    query.exec();
    while (query.next()){
        list_of_chats_id_group.append(query.value(0).toString());
        list_of_num_users_in_chat_group.append(query.value(1).toString());
    }

    // Записываем число чатов
    str_list.append(QString::number(list_of_chats_id_group.size()));



    // Инициализируем списки юзеров для чатов
    int num = list_of_chats_id_group.size();
    for (int i = 0; i < num; ++i) {
        list_of_lists_of_users_id.append(QStringList());
        list_of_lists_of_users_logins.append(QStringList());
    }


    // Смотрим число юзеров в каждом чате, записываем их логины и id
    query.prepare("Select pk_user  From user_in_chat Where pk_chat = :id_chat");

    query2.prepare("Select login From user Where pk_user = :user_id");

    for (int i = 0; i < num; ++i) {
        query.bindValue(":id_chat", list_of_chats_id_group[i]);
        query.exec();
        while (query.next()){
            // Добавляем id юзеров чата
            list_of_lists_of_users_id[i].append(query.value(0).toString());

            query2.bindValue(":user_id", list_of_lists_of_users_id[i][list_of_lists_of_users_id[i].size()-1]);
            query2.exec();
            if (query2.next())
                list_of_lists_of_users_logins[i].append(query2.value(0).toString());
        }
    }

    // Имеем все чаты и их юзеров (логин и id). Теперь для каждого чата нужно найти сообщения
    // И запаковать всё в поток

    for (int i = 0; i < num; ++i) {
        // id чата
        str_list.append(list_of_chats_id_group[i]);
        // Кол-во юзеров
        str_list.append(list_of_num_users_in_chat_group[i]);
        // Логины юзеров
        for (int j = 0; j < list_of_num_users_in_chat_group[i].toInt(); ++j) {
            str_list.append(list_of_lists_of_users_logins[i][j]);
        }

        // Получим номер последнего сообщения
        query.prepare("Select id_last_message From chat Where pk_chat = :id_chat");
        query.bindValue(":id_chat", list_of_chats_id_group[i]);
        query.exec();
        if (query.next()){
            buf_str =  query.value(0).toString();
            last_message_id = buf_str;
        }
        if (buf_str == "-1")
        {
            // Сообщений нет
            str_list.append("0");

        }
        else
        {
            // Сообщения есть

            // Получим общее число сообщений
            query.prepare("Select count() From message Where pk_chat = :id_chat");
            query.bindValue(":id_chat", list_of_chats_id_group[i]);
            query.exec();
            if (query.next()){
                buf =  query.value(0).toInt();
                str_list.append(query.value(0).toString());
            }

            QString buf_next_message_group;
            // По очереди вытаскиваем сообщения и заносим их данные в сокет
            for (int i = 0; i < buf; ++i) {
                query.prepare("Select pk_previous, text, pk_user_sender, pk_previous  From message Where pk_message = :last_message_id");
                query.bindValue(":last_message_id", last_message_id);
                query.exec();
                if (query.next()){
                    buf_str  =  query.value(2).toString();
                    buf_mes_text = query.value(1).toString();
                    buf_next_message_group = query.value(3).toString();

                }

                // Найдём логин отправителя и сразу заносим 3 оставшихся параметра сообщения
                query.prepare("Select login From user Where pk_user = :user_id");
                query.bindValue(":user_id", buf_str);
                query.exec();
                if (query.next()){
                    buf_str  =  query.value(0).toString();
                    str_list.append(buf_str);
                    str_list.append(buf_mes_text);
                    str_list.append(last_message_id);
                }
                // Идём к следующему
                last_message_id = buf_next_message_group;

            }
        }
    }

    num_of_params = str_list.size();
    return num_of_params;

}

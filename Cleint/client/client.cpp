#include "client.h"
#include "ui_client.h"
#include "qlistmodel.h"
#include <QFile>

Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);

    //qlistmodel *model = new qlistmodel(this);
    //ui->listView_contacts->setModel(model);
    // ?
    //ui->listView_contacts->setTextElideMode(Qt::ElideNone);
    //ui->listView_contacts->setResizeMode(QListView::Adjust);

    // Принимаем сигнал сокета о том, что можно читать с него
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    connect (socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

}

Client::~Client()
{
    delete ui;
}
void Client::ChatSerialization(QDataStream &stream, message* mas_message, int num_of_messages)
{
    stream << num_of_messages;
    for (int i = 0; i < num_of_messages; ++i) {
        stream << (*(mas_message+i)).user_login_sender;
        stream << (*(mas_message+i)).user_login_receiver;
        stream << (*(mas_message+i)).str_text;
    }

}

int Client::ChatUnSerialization(QDataStream &stream, message *mas_message)
{

    int num_of_messages;
    stream >> num_of_messages;
    for (int i = 0; i < num_of_messages; ++i) {
        stream >> (*(mas_message+i)).user_login_sender;
        stream >> (*(mas_message+i)).user_login_receiver;
        stream >> (*(mas_message+i)).str_text;
    }
    return num_of_messages;
}

void Client::SentToServerStrings(QStringList &str_list, int num_of_strings)
{
    data.clear();
    // Инициализируем поток на вывод. С его помощью запишем str в data
    QDataStream out(&data, QIODevice::WriteOnly);
    for (int i = 0; i < num_of_strings; ++i) {
        out << str_list[i];
    }
    socket->write(data);
}

void Client::ProcessNewMessageFromServer(QStringList &str_list)
{
    // 5 параметров: код (6), логин юзера отправителя, id сообщения, id чата, текст сообщения
    // Нужно добавить новое сообщение в соответствующий чат
    // Ищем среди контактов и групповых чатов нужный чат по id.

    int f = 0; // Флаг, 0 - не найден, 1 - среди контактов, 2 - среди групповых чатов

    int num_in_list;

    //Поиск среди контактов
    int num_of_contacts = contact_list.size();
    for (int i = 0; i < num_of_contacts; ++i) {
        if (contact_list[i].chat_id == str_list[3].toInt())
        {
            f =1;
            num_in_list = i;
            break;
        }
    }
    // Поиск среди групповых чатов
    int num_of_group_chats = group_chat_list.size();
    if (f == 0)
    {
        for (int i = 0; i < num_of_group_chats; ++i)
        {
            if (group_chat_list[i].chat_id == str_list[3].toInt())
            {
                f = 2;
                num_in_list = i;
                break;
            }
        }
    }

    // todo: так же добавить обработчики для группового чата

    if (f==1)
    {
         // Добавляем сообщение в список сообщений

        message message;
        message.message_id = str_list[2].toInt();
        message.str_text = str_list[4];
        message.user_login_sender = str_list[1];

        contact_list[num_in_list].message_list.append(message);

        //  Теперь смотрим какой чат активный, если этот, то обновляем его

        if (current_chat_id == str_list[3].toInt())
        {
            // Обновляем чат в соответствии со списком сообщений
            RefreshChat(contact_list[num_in_list].message_list);
        }
    }
}

void Client::RefreshChat(QList <message> message_list)
{
    // Полностью отрисовывает чат в соответствии со списком сообщений
    // Одна из тех функций, которую можно оптимизировать, если качественно смотреть на место изменения (добавления сообщения в конец, изменение в середине, удаление из середины и т.п)
    // Пока что просто заново выводит все сообщения

    int num = message_list.size();


    //?
    ui->listWidget_chat->clear();
    for (int i = 0; i <num; ++i) {
        ui->listWidget_chat->addItem(message_list[i].user_login_sender + ": " + message_list[i].str_text);
    }
}

void Client::ProcessNewGroupChatFromServer(QStringList &str_list)
{
    // Принимаем 3 + n параметров
    // Нужно создать объект GroupChat и отобразить его
    // По факту нам не нужно много знать о чате, только его id для получения уведомлений от сервера и отправки сообщений
    // Список юзеров можем использовать в качестве названия

    int num_of_users = str_list[2].toInt();

    // Заполняем структуру
    GroupChat g_chat;
    QString next_user;
    // В список юзеров всех, кроме себя
    for (int i = 0; i < num_of_users; ++i) {
        next_user = str_list[3+i];
        if (next_user != this->login)
            g_chat.list_users.append(next_user);
    }

    g_chat.chat_id = str_list[1].toInt();

    group_chat_list.append(g_chat);

    // После этого выводим объект в gui
    QString text = "Чат " + QString::number(g_chat.chat_id) + " : ";
    for (int i = 0; i < num_of_users-1; ++i) {
        text.append(g_chat.list_users[i]);
        text.append(", ");
    }

    ui->listWidget_group->addItem(text);


}

// Отправит запрос на сервер на подключение уведомлений. Код 5. Передаём логин
void Client::SubscribeForUpdates()
{
    QStringList str_list;
    str_list.append("5");
    str_list.append(this->login);
    SentToServerStrings(str_list,2);
}

void Client::ReconnectToServer()
{
    socket->connectToHost(connection_address,2323);
    if(socket->waitForConnected(3000)){
        qDebug() << "Reconnection is Good";
    }
    else {
        qDebug() << "Reconnection failed";
    }
}

void Client::slotReadyRead()
{
    QDataStream in(socket);
    // Смотрим, правильно ли инициализировался объект
    if(in.status()== QDataStream::Ok)
    {

        qDebug() << "Read stream ok"; // Можно потом убрать
        // Строку получили. Теперь её нужно обработать. Смотрим на первый аргумент.
        QStringList str_list;
        QString str;
        in >> str;
        str_list.append(str);
        // Здесь смотрим код сообщения и вызываем соответствующую функцию обработчик.

        // Получилось добавить контакт
        if (str_list[0]=="5")
        {
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessAddContactRespond(str_list);
        }

        // Обработка нового сообщения
        // Параметры: код (6),  логин юзера отправителя, id сообщения, id чата, текст сообщения
        if (str_list[0]=="6")
        {
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessNewMessageFromServer(str_list);
        }

        if (str_list[0]=="7")
        {
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);

            int num = str_list[2].toInt();

            for (int i = 0; i < num; ++i) {
                in >> str;
                str_list.append(str);
            }

            ProcessNewGroupChatFromServer(str_list);
        }








    }
    else
    {
        qDebug() << "Problem with read stream";
    }
}

void Client::ApplyContactsInfo()
{
    int num_of_contacts = contact_list.size();

    for (int i = 0; i < num_of_contacts; ++i) {
        ui->listWidget_contact->addItem(contact_list[i].login);
    }
}
// Отправляем сообщение. Эта функция будет отправляеть сообщение на сервер
void Client::on_pushButton_clicked()
{
    // Отправляем сообщение на сервер. Код 6, параметры - логин отправителя, id чата, текст сообщения.

    QStringList str_list;
    str_list.append("6");
    str_list.append(login);
    str_list.append(QString::number(current_chat_id));
    str_list.append(ui->textEdit_message_box->toPlainText());
    SentToServerStrings(str_list,4);

    ui->textEdit_message_box->clear();

}

void Client::on_pushButton_get_contact_clicked()
{
    // Читаем логин контакта, отправляем запрос на сервер, получаем ответ.
    // Здесь реализуем отправку.
    // todo: Нужно так же чекнуть имеется ли уже этот человек в контактах.

    // Считаем, что такого контакта ещё нет
    int f = 1;
    int num_of_contacts = contact_list.size();
    QString wanted_contact = ui->textEdit_contact->toPlainText();


    for (int i = 0; i < num_of_contacts; ++i) {
        if(wanted_contact == contact_list[i].login)
        {
            f=0;
            break;
        }
    }

    QString str;

    if (f)
    {

    // Инициализируем поток на вывод. Передаем  код 4, свой логин и логин желаемого контакта.
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    str = "4";
    out << str;

    str = this->login;
    out << str;
    str = ui->textEdit_contact->toPlainText();
    out << str;
    socket->write(data);
    }
    else
    {
        // Такой контакт уже есть
    }

}

void Client::ProcessAddContactRespond(QStringList& str_list)
{
    // Сервер нам отправит либо -1 - ошибка, либо 3 параметра (успех) - 5, id-чата и login того, кого добавить
    if(str_list[0] == "5")
    {
        // Нужно добавить объект в list.
        ui->listWidget_contact->addItem(str_list[2]);
        // Инициализация контакта и чата
        Contact* cont = new Contact();
        cont->login = str_list[2];
        cont->chat_id = str_list[1].toInt();
        contact_list.append(*cont);

        // На этот момент новый контакт имеет базовые параметры но не имеет сообщений.
        // Теперь следует обработать исключения от сервера: пользователь не найден или неизвестная ошибка (просто -1).

    }
    else
        if(str_list[0]== "-2"){
        qDebug() << "Пользователь не найден";
    }
}

// Отрисовываем чат с контактом
void Client::on_listWidget_contact_itemDoubleClicked(QListWidgetItem *item)
{
    ui->listWidget_chat->clear();
    // Определяем логин контакта
    QString log = item->text();
    int num;
    for (int i = 0; i < contact_list.size(); ++i) {
        if (contact_list[i].login == log){
            num = i;
        }
    }
    // Выводим соответствующий чат
    for (int i = 0; i <contact_list[num].message_list.size(); ++i) {
        ui->listWidget_chat->addItem(contact_list[num].message_list[i].user_login_sender + ": " + contact_list[num].message_list[i].str_text);
    }
    // Заполняем данные о текущем чате
    current_chat_id = contact_list[num].chat_id;

}

// При клике вызваем окно для добавление нового чата.
void Client::on_pushButton_get_group_chat_clicked()
{
    d = new DialogGroupChat();

    int size = contact_list.size();
    QStringList str_list;

    str_list.append(this->login);
    for (int i = 0; i < size; ++i) {
        str_list.append(contact_list[i].login);
    }
    d->InitializeList(str_list);
    d->login = this->login;
    // Связываем сигнал с функцией на создание чата
    connect (d, &DialogGroupChat::ContactsReadyToUse, this, &Client::CreateGroupChat);

    d->show();
}

void Client::CreateGroupChat(QStringList str_list)
{
    // Получаем список контактов, из которых нужно создать чат.
    // Отправляем запрос на сервер для создания чата
    // Код 7, кол-во контактов, контакты по порядку. Итого 2+ n параметров, где n - второй параметр

    QString str;
    QStringList str_l;
    str = "7";
    str_l.append(str);

    int size = str_list.size();
    str = QString::number(size);
    str_l.append(str);

    for (int i = 0; i < size; ++i) {
        str_l.append(str_list[i]);
    }

    SentToServerStrings(str_l,2+size);

    d->close();
    d->deleteLater();


}


void Client::on_listWidget_group_itemDoubleClicked(QListWidgetItem *item)
{
    // Выводим соответствующий групповой чат



    ui->listWidget_chat->clear();
    // Определяем id чата
    QString name = item->text();
    QStringList str_list = name.split(" ");
    int id = str_list[1].toInt();

    int num;
    // Ищем среди групповых чатов
    for (int i = 0; i < group_chat_list.size(); ++i) {
        if (group_chat_list[i].chat_id == id){
            num = i;
            break;
        }
    }
    // Выводим соответствующий чат
    for (int i = 0; i <group_chat_list[num].message_list.size(); ++i) {
        ui->listWidget_chat->addItem(contact_list[num].message_list[i].user_login_sender + ": " + group_chat_list[num].message_list[i].str_text);
    }
    // Заполняем данные о текущем чате
    current_chat_id = group_chat_list[num].chat_id;
}


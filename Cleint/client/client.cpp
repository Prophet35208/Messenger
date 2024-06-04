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
// Пока не используется
void Client::QStringToQChar_(QString &str, QChar *mas_char, int size)
{
    int i;
    for ( i = 0; i < size-1; ++i) {
        *(mas_char+i) = str[i];
    }
    *(mas_char+i) = '\0';
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
        // Регистрация прошла
        if (str_list[0]=="5")
        {
            in >> str;
            str_list.append(str);
            in >> str;
            str_list.append(str);
            ProcessAddContactRespond(str_list);
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

void Client::on_pushButton_clicked()
{
    // Отправялем сообщение
    QString text;
    text = ui->lineEdit->text();

    ui->listWidget_chat->addItem(login +": "+text);

}



void Client::on_pushButton_get_contact_clicked()
{
    // Читаем логин контакта, отправляем запрос на сервер, получаем ответ.
    // Здесь реализуем отправку.
    // todo: Нужно так же чекнуть имеется ли уже этот человек в контактах.

    QString str;


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
    for (int i = 0; i < contact_list[num].message_list.size(); ++i) {
        ui->listWidget_chat->addItem(contact_list[num].message_list[i].user_login_sender + ": " + contact_list[num].message_list[i].str_text);
    }

}


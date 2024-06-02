#include "client.h"
#include "ui_client.h"
#include "qlistmodel.h"
#include <QFile>

Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);

    qlistmodel *model = new qlistmodel(this);
    ui->listView->setModel(model);
    ui->listView_3->setModel(model);
    ui->listView->setTextElideMode(Qt::ElideNone);
    ui->listView->setResizeMode(QListView::Adjust);


}

Client::~Client()
{
    delete ui;
}
void Client::ChatSerialization(QDataStream &stream, message* mas_message, int num_of_messages)
{
    stream << num_of_messages;
    for (int i = 0; i < num_of_messages; ++i) {
        stream << (*(mas_message+i)).message_num;
        stream << (*(mas_message+i)).user_id_sender;
        stream << (*(mas_message+i)).user_id_receiver;
        stream << (*(mas_message+i)).str;
    }

}

int Client::ChatUnSerialization(QDataStream &stream, message *mas_message)
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

void Client::QStringToQChar_(QString &str, QChar *mas_char, int size)
{
    int i;
    for ( i = 0; i < size-1; ++i) {
        *(mas_char+i) = str[i];
    }
    *(mas_char+i) = '\0';
}

void Client::on_pushButton_clicked()
{
    //Тест
    message mas_mes[2];
    int num_of_messages;

    mas_mes[0].message_num=1;
    mas_mes[0].user_id_receiver = 1;
    mas_mes[0].user_id_sender = 2;
    //QString str1("Hello");
    //QStringToQChar_(str1,mas_mes[0].text,str1.size());
    mas_mes[0].str = QString("Hello");

    mas_mes[1].message_num = 2;
    mas_mes[1].user_id_receiver = 3;
    mas_mes[1].user_id_sender = 4;
    //QString str2("Hi");
    //QStringToQChar_(str2,mas_mes[1].text,str2.size());
    mas_mes[1].str = QString("Hi");

    // Запихиваем в поток
    data.clear();
    // Инициализируем поток на вывод. С его помощью запишем str в data
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Version::Qt_6_7);
    out << QString("4");

    ChatSerialization(out,mas_mes,2);
    socket->write(data);



}


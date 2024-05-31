#include "registration.h"
#include "ui_registration.h"


Registration::Registration(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Registration)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
// Принимаем сигнал сокета о том, что можно читать с него
    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
// Удалем сокет, который задисконектился
    connect (socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
// Не работает, нужно понять почему
    connect (socket, SIGNAL(connected()), this, SLOT(slotConnectionEstablished()));
}

Registration::~Registration()
{
    delete ui;
}



void Registration::on_pushButton_3_clicked()
{
    socket->connectToHost("127.0.0.1",2323);
    if(socket->waitForConnected(3000))
        qDebug() << "All is ok";
}

void Registration::slotReadyRead()
{

    QDataStream in(socket);
    // Смотрим, правильно ли инициализировался объект
    if(in.status()== QDataStream::Ok)
    {

        qDebug() << "Read stream ok"; // Можно потом убрать
        QString str;
        in >> str;
        // Строку получили. Теперь её нужно обработать. Смотрим на первый аргумент.
        QStringList str_list;
        str_list = str.split(" ");
        // Регистрация прошла
        if (str_list[0]=="1"){
            this->ui->reg_status->setText("Reg Ok");
        }
        // Регистрация не прошла
        if (str_list[0]=="-1"){
            this->ui->reg_status->setText("Reg Not Ok");
        }



    }
    else
    {
        qDebug() << "Problem with read stream";
    }
}

void Registration::slotConnectionEstablished()
{
    qDebug() << "Connection OK";
}

void Registration::SendToServer(QString str)
{
    data.clear();
    // Инициализируем поток на вывод. С его помощью запишем str в data
    QDataStream out(&data, QIODevice::WriteOnly);
    out << str;
    socket->write(data);
}

// Передаём информацию для входа. В начале код 1, чтобы сервер понял, что ему присылают данные на вход.
void Registration::on_pushButton_2_clicked()
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    QString str;
    str = "1 " + this->ui->text_login->toPlainText() +" "+ this->ui->text_pass->toPlainText() ;
    out << str;
    socket->write(data);
}


void Registration::on_pushButton_clicked()
{
}


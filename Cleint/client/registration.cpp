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


// Скрываем ненужные элементы
    ui->text_pass_confirmation->setVisible(false);
    ui->label_password_confirmation->setVisible(false);
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
            this->ui->label_reg_status->setText("Reg Ok");
        }
        // Регистрация не прошла
        if (str_list[0]=="-1"){
            this->ui->label_reg_status->setText("Reg Not Ok");
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



void Registration::on_pushButton_register_clicked()
{
    if (registry_mod){
        // Если мод уже стоит, значит кнопка должна выйти из режима
        registry_mod = false;
        ui->pushButton_register->setText("Зарегистрироваться");
        ui->pushButton_signin->setText("Войти");
        ui->label_up->setText("Вход");
        //Скрываем лишние элементы
        ui->text_pass_confirmation->setVisible(false);
        ui->text_pass_confirmation->setEnabled(false);
        ui->label_password_confirmation->setVisible(false);
        ui->label_password_confirmation->setEnabled(false);
    }
    else
    // Если мод не стоит, значит входим в режим регистрации
    {
        registry_mod = true;
        ui->pushButton_register->setText("Отмена");
        ui->pushButton_signin->setText("Подтвердить");
        ui->label_up->setText("Регистрация");
        //Снова показываем элементы
        ui->text_pass_confirmation->setVisible(true);
        ui->text_pass_confirmation->setEnabled(true);
        ui->label_password_confirmation->setVisible(true);
        ui->label_password_confirmation->setEnabled(true);
    }
}


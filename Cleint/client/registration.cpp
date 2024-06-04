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
// Удалем сокет, который задисконектился.
    connect (socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
//Дебаг
    connect (socket, SIGNAL(connected()), this, SLOT(slotConnectionEstablished()));


// Скрываем ненужные элементы
    ui->text_pass_confirmation->setVisible(false);
    ui->label_password_confirmation->setVisible(false);
    ui->label_registry_status->setVisible(false);

    w = new Client();

}

Registration::~Registration()
{
    delete ui;
}

void Registration::slotReadyRead()
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
        // Передача в лист переменного числа параметров
        if (str_list[0] == "1"){
            int num_of_contacts;
            QList <int> list_num_of_messages;
            int j=0;

            in >> str;
            num_of_contacts = str.toInt();
            str_list.append(str);

            for (int i = 0; i < num_of_contacts; ++i) {
                in >> str;
                str_list.append(str);

                in >> str;
                str_list.append(str);

                in >> str;
                str_list.append(str);
                list_num_of_messages.append(str.toInt());

                for (int k = 0; k < list_num_of_messages[j]; ++k) {
                    in >> str;
                    str_list.append(str);

                    in >> str;
                    str_list.append(str);
                }
                j++;
            }
        }
        // Здесь смотрим код сообщения и вызываем соответствующую функцию обработчик.
        // Регистрация прошла
        if (str_list[0]=="1" || str_list[0]=="-1")
            ProcessLoginRespond(str_list);
        if (str_list[0]=="2" || str_list[0]=="3")
            ProcessRegistrationRespond(str_list);



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

void Registration::ProcessLoginRespond(QStringList &str_list)
{
    if (str_list[0]=="1"){
        ui->label_status->setText("Удалось войти");
        w->connection_address = connection_address;
        w->login = ui->text_login->toPlainText();
        w->ReconnectToServer();

        TransferClientInfo(str_list,w);
        w->ApplyContactsInfo();

        w->show();
    }
    if (str_list[0]=="-1"){
        ui->label_status->setText("Не удалось войти");
    }
}

void Registration::ProcessRegistrationRespond(QStringList &str_list)
{
    if (str_list[0]=="2"){
        // Не удалось зарегистрироваться
        ui->label_registry_status->setVisible(true);
        ui->label_registry_status->setText("Такой пользователь уже существует");
    }
    if (str_list[0]=="3"){
        // Удалось
        ui->label_status->setText("Регистрация успешна");

        //Теперь выходим из режима регистрации, очищаем text-боксы.
        RegistryOff();
    }

}

void Registration::TransferClientInfo(QStringList &str_list, Client *client)
{
    // Нужно проанализировать все параметры str_list
    int num_of_contacts;
    Contact buf;
    message buf_mesage;
    int buf_message_count;

    int j=2;

    num_of_contacts = str_list[1].toInt();
    for (int i = 0; i < num_of_contacts; ++i)
    {
        buf.login = str_list[j];
        j++;
        buf.chat_id = str_list[j].toInt();
        j++;

        buf_message_count = str_list[j].toInt();
        j++;
        for (int k = 0; k < buf_message_count; ++k) {
            buf_mesage.user_login_sender = str_list[j];
            j++;
            buf_mesage.str_text = str_list[j];
            j++;
            buf.message_list.append(buf_mesage);
        }
        client->contact_list.append(buf);
        buf.message_list.clear();
    }


}

void Registration::RegistryOn()
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

    ui->text_login->setText("");
    ui->text_pass->setText("");
    ui->text_pass_confirmation->setText("");
}

void Registration::RegistryOff()
{
    registry_mod = false;
    ui->pushButton_register->setText("Зарегистрироваться");
    ui->pushButton_signin->setText("Войти");
    ui->label_up->setText("Вход");
    //Скрываем лишние элементы
    ui->text_pass_confirmation->setVisible(false);
    ui->text_pass_confirmation->setEnabled(false);
    ui->label_password_confirmation->setVisible(false);
    ui->label_password_confirmation->setEnabled(false);
    ui->label_registry_status->setVisible(false);

    ui->text_login->setText("");
    ui->text_pass->setText("");
    ui->text_pass_confirmation->setText("");
}

// Передаём информацию для входа. В начале код 1, чтобы сервер понял, что ему присылают данные на вход.
// Если режим регистрации, то передаём код 2
void Registration::on_pushButton_signin_clicked()
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    QString str;
    if (registry_mod)
    {
        if (ui->text_pass->toPlainText() == ui->text_pass_confirmation->toPlainText())
        {
            str = "2";
            out << str;
            str = this->ui->text_login->toPlainText();
            out << str;
            str = this->ui->text_pass->toPlainText();
            out << str;
            socket->write(data);
        }
        else
        {
            ui->label_registry_status->setText("Пароли не совпадают");
            ui->label_registry_status->setVisible(true);
        }
    }
    else
    {
        str = "1";
        out << str;
        str = this->ui->text_login->toPlainText();
        out << str;
        str = this->ui->text_pass->toPlainText();
        out << str;
        socket->write(data);
    }

}



void Registration::on_pushButton_register_clicked()
{
    if (registry_mod){
        // Если мод уже стоит, значит кнопка должна выйти из режима
        RegistryOff();

    }
    else
    // Если мод не стоит, значит входим в режим регистрации
    {
        RegistryOn();
    }
}

// Кнопка соединения
void Registration::on_pushButton_connect_clicked()
{
    socket->connectToHost(ui->text_ip->toPlainText(),2323);
    if(socket->waitForConnected(3000)){
        qDebug() << "All is ok";
        connection_address = ui->text_ip->toPlainText();
        ui->pushButton_register->setEnabled(true);
        ui->pushButton_signin->setEnabled(true);
    }

}


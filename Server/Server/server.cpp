#include "server.h"

Server::Server()
{
    // Начинаем прослушивать порт Tcp/Udp по всем сетевым интерфейсам.
    if (this->listen(QHostAddress::Any,2323)){
        qDebug() << "Server started";
    }
    else
    {
        qDebug() << "Server error";
    }
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
    // Смотрим, правильно ли инициализировался объект
    if(in.status()== QDataStream::Ok)
    {
        qDebug() << "Read stream ok";
        //Теперь выведем данные из потока в консоль
        QString str;
        in >> str;
        qDebug() << str;

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

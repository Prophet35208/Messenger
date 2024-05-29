#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class Client;
}
QT_END_NAMESPACE


class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private:
    Ui::Client *ui;
    // Определяем сокет и массив данныхз для него
    QTcpSocket *socket;
    QByteArray data;
};
#endif // CLIENT_H

#include <QCoreApplication>
#include "server.h"
#include <QtSql>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // В параметрах конструктора путь к БД
    Server s("users.db");

    return a.exec();
}

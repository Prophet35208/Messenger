#include "client.h"
#include "registration.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Client w;
    //w.show();
    Registration r;
    r.show();

    return a.exec();
}

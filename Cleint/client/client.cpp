#include "client.h"
#include "ui_client.h"
#include "qlistmodel.h"

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

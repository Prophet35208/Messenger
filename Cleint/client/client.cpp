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
    ui->listView->scrollToBottom();
    ui->listView->setAutoScroll(true);
}

Client::~Client()
{
    delete ui;
}

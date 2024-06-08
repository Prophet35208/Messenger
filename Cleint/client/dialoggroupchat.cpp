#include "dialoggroupchat.h"
#include "ui_dialoggroupchat.h"

DialogGroupChat::DialogGroupChat(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogGroupChat)
{
    ui->setupUi(this);
}

DialogGroupChat::~DialogGroupChat()
{
    delete ui;
}

void DialogGroupChat::InitializeList(QStringList str_list)
{
    // Инициализируем список из данного списка

    int size = str_list.size();

    for (int i = 1; i < size; ++i) {
        QListWidgetItem *listItem = new QListWidgetItem(str_list[i]);
        listItem->setCheckState(Qt::Unchecked);
        ui->listWidget->addItem(listItem);
    }

}

void DialogGroupChat::on_pushButton_clicked()
{
    // Кнопку нажали, нужно посмотреть какие выбраны контакты. Если 0, то отклоняем
    // Выбранные отсылаем, послыая сигнал
    int size = ui->listWidget->count();
    QStringList str_list;

    str_list.append(login);
    for (int i = 0; i < size; ++i) {
        if (ui->listWidget->item(i)->checkState() == Qt::Checked)
            str_list.append( ui->listWidget->item(i)->text());
    }
    emit ContactsReadyToUse(str_list);

}


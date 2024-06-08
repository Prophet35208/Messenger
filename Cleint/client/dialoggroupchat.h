#ifndef DIALOGGROUPCHAT_H
#define DIALOGGROUPCHAT_H

#include <QDialog>

namespace Ui {
class DialogGroupChat;
}

class DialogGroupChat : public QDialog
{
    Q_OBJECT

public:
    explicit DialogGroupChat(QWidget *parent = nullptr);
    ~DialogGroupChat();
    QString login;
    void InitializeList(QStringList str_list);

private slots:
    void on_pushButton_clicked();

private:
    Ui::DialogGroupChat *ui;
signals:
    // Отправляем, когда хотим передать контакт для создания группового чата в клиент
    void ContactsReadyToUse(QStringList str_list);
};

#endif // DIALOGGROUPCHAT_H

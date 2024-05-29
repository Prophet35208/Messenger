#ifndef QLISTMODEL_H
#define QLISTMODEL_H

#include <QAbstractListModel>
//#include <QSize>
//#include <QBrush>
//#include <QFont>
//#include <QStringList>



class qlistmodel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit qlistmodel(QObject *parent = 0);
    // Для создания новой модели нужно определить 4 функции
    //Возвращяет кол-во элементов списка
    int rowCount (const QModelIndex &parent = QModelIndex()) const;
    // Возвращает значение элемента
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    //Сохраняет новое значение элемента
    bool setData(const QModelIndex &index, const QVariant & value, int role = Qt::EditRole);

    //Возвращает параметры редактируемости элемента
    Qt::ItemFlags flags (const QModelIndex & index) const;
private:
    QList <QString> list;


};

#endif // QLISTMODEL_H

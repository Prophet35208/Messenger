#include "qlistmodel.h"

// Тестовая модель.


qlistmodel::qlistmodel(QObject *parent) :
    QAbstractListModel (parent)
{

    // Здесь нужно инициализировать список сообщений. Пока что тут статика

    // Юзаем WordWrap:true для автоматического переноса слов! Не будет работать при одном большом слове, появится слайдер
    list << "Nick: Hello"<<"Dan: HiGGGG GGGGGGGGG GGGGGGGGGGGGGGG GGGGGGGGGGGGG GGGGGGGGGGGGGG GGGGGGGGGG GGGGGGGGGf";
}

int qlistmodel::rowCount(const QModelIndex &parent) const
{
    return list.count();
}

QVariant qlistmodel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role){
    case Qt::DisplayRole:
        return list.at(index.row());
    case Qt::TextAlignmentRole:
        return Qt::AlignRight;  // Добавление условия позволит некоторый текст уводить вправо, другой-влево!
    default:
        return QVariant();
    }
}

bool qlistmodel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    list.replace(index.row(),value.toString());
}

Qt::ItemFlags qlistmodel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled| Qt::ItemIsSelectable ;
}

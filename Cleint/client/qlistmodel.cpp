#include "qlistmodel.h"

qlistmodel::qlistmodel(QObject *parent) :
    QAbstractListModel (parent)
{

    // Здесь нужно инициализировать список сообщений. Пока что тут статика


    list << "Nick: Hello"<<"Dan: Hi";
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

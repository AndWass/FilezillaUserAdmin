#include "filezillausermodel.h"
#include <iostream>

FilezillaUserModel::FilezillaUserModel(std::vector<FilezillaUser> *users, QObject *parent) :
    pUsers(users),
    QAbstractItemModel(parent)
{
}

int FilezillaUserModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        return pUsers->size();
    }

    else if(parent.internalId() == 0)
    {
        return (*pUsers)[parent.row()].directories.size();
    }

    return 0;
}

int FilezillaUserModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex FilezillaUserModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        return createIndex(row, 0);
    }

    if(column > 0)
    {
        return QModelIndex();
    }

    return createIndex(row, column, parent.row()+1);
}

QModelIndex FilezillaUserModel::parent(const QModelIndex &child) const
{
    if(!child.isValid())
    {
        return QModelIndex();
    }

    if(child.internalPointer() != 0)
    {
        return createIndex(child.internalId()-1, 0);
    }

    return QModelIndex();
}

QVariant FilezillaUserModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    if(index.row() > pUsers->size()) return QVariant();
    if(role != Qt::DisplayRole) return QVariant();

    if(index.internalId() != 0)
    {
        int parentIndex = index.internalId() - 1;
        return (*pUsers)[parentIndex].directories[index.row()].dir;
    }

    return (*pUsers)[index.row()].username;
}

QVariant FilezillaUserModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return QVariant("Users");
    }

    return QVariant();
}

Qt::ItemFlags FilezillaUserModel::flags(const QModelIndex &index) const
{
    if(index.internalId() == 0)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled;
}

void FilezillaUserModel::pushBack(const FilezillaUser &toAdd)
{
    beginInsertRows(QModelIndex(), pUsers->size(), pUsers->size());
    pUsers->push_back(toAdd);
    endInsertRows();

    /*QModelIndex parent = createIndex(pUsers->size()-1, 0, static_cast<void*>(&(*pUsers)[pUsers->size()-1]));
    beginInsertRows(parent, 0, toAdd.directories.size()-1);
    endInsertRows();*/
}

void FilezillaUserModel::clear()
{
    if(pUsers->size() == 0)
        return;

    beginRemoveRows(QModelIndex(), 0, pUsers->size()-1);
    pUsers->clear();
    endRemoveRows();
}

void FilezillaUserModel::removeAt(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    std::vector<FilezillaUser>::const_iterator iter = pUsers->begin();
    for(int i=0; i<index; i++)
    {
        iter++;
    }
    pUsers->erase(iter);
    endRemoveRows();
}

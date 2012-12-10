#ifndef FILEZILLAUSERMODEL_H
#define FILEZILLAUSERMODEL_H

#include <QAbstractItemModel>
#include <vector>
#include "filezillaaccounts.h"

class FilezillaUserModel : public QAbstractItemModel
{
    Q_OBJECT

    std::vector<FilezillaUser> *pUsers;
public:
    explicit FilezillaUserModel(std::vector<FilezillaUser>* users, QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    void pushBack(const FilezillaUser &toAdd);
    void clear();
    void removeAt(int index);
    
signals:
    
public slots:
    
};

#endif // FILEZILLAUSERMODEL_H

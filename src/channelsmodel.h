#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QAbstractItemModel>

#include <QHash>

#include <memory>
#include "m3ulist.h"

class QNetworkAccessManager;
class AbstractChannelTreeItem;
class RootTreeItem;

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ChannelsModel(QObject *parent = nullptr);
    void AddList(M3UList list);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    enum ChannelRoles
    {
           NameRole = Qt::UserRole + 1,
           UriRole
    };
    void AddToFavourites(AbstractChannelTreeItem* item);
    void RemoveFromFavourites(AbstractChannelTreeItem* item);
private slots:
    void aquiredIcon(AbstractChannelTreeItem*);
private:
    QModelIndex indexFromItem(AbstractChannelTreeItem* item);
    void loadChannels();
private:
    QNetworkAccessManager* networkManager;
    std::unique_ptr<RootTreeItem> rootItem;
};

#endif // CHANNELSMODEL_H

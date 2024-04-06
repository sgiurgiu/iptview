#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QThread>

#include "channeltreeitem.h"
#include "m3ulist.h"
#include "xstreaminfo.h"
#include <memory>

class AbstractChannelTreeItem;
class RootTreeItem;
class GroupTreeItem;
class LoadingChannelsThread;
class LoadingChannelIconsWorker;

class ChannelsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ChannelsModel(QObject *parent = nullptr);
    ~ChannelsModel();
    M3UList GetM3UList() const;
    void AddList(M3UList list);
    void AddList(XStreamCollectedInfo list);
    void AddChild(AbstractChannelTreeItem *child, const QModelIndex &parent);
    void RemoveChild(AbstractChannelTreeItem *child, const QModelIndex &index);
    void ReloadChannels();
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index,
                          int role = Qt::DisplayRole) const override;
    virtual QModelIndex
    index(int row,
          int column,
          const QModelIndex &parent = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    enum ChannelRoles
    {
        NameRole = Qt::UserRole + 1,
        UriRole,
        IdRole
    };
    void AddToFavourites(AbstractChannelTreeItem *item);
    void RemoveFromFavourites(AbstractChannelTreeItem *item);

signals:
    void cancelImportChannels();
    void groupLoaded(GroupTreeItem *group);
    void loadChannelIcon(ChannelTreeItem *channel);
    void updateImportedChannelIndex(qint64);
    void channelsImported();

private slots:
    void cancelImportChannelsSlot();
    void onGroupLoaded(GroupTreeItem *group);
    void onGroupsCount(int count);
    void onFavouriteChannels(std::vector<ChannelTreeItem *>);
    void onChannelIconReady(ChannelTreeItem *channel);

private:
    QModelIndex indexFromItem(AbstractChannelTreeItem *item);
    void loadChannels();
    void loadGroupChannelsIcons(GroupTreeItem *group);
    void stopAndClearThreads();

private:
    std::unique_ptr<RootTreeItem> rootItem;
    LoadingChannelsThread *loadingChannelsThread = nullptr;
    QThread *loadingChannelIconsThread = nullptr;
    LoadingChannelIconsWorker *channelIconsWorker = nullptr;
    std::atomic_bool cancelImportingChannels = false;
    int xstreamGroupImportingCount = 0;
};

#endif // CHANNELSMODEL_H

#include "channelsmodel.h"
#include "abstractchanneltreeitem.h"
#include "roottreeitem.h"
#include "channeltreeitem.h"

#include "databaseprovider.h"
#include "database.h"
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QThread>

ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractItemModel{parent},
      networkManager{new QNetworkAccessManager{this}},
      rootItem{std::make_unique<RootTreeItem>(networkManager)}
{
    connect(rootItem.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SLOT(aquiredIcon(AbstractChannelTreeItem*)), Qt::QueuedConnection);
    loadChannels();
}
ChannelsModel::~ChannelsModel()
{
    rootItem->setCancelOgoingOperations(true);
    if(loadingIconsThread)
    {
        loadingIconsThread->wait();
    }
}
void ChannelsModel::loadChannels()
{
    auto db = DatabaseProvider::GetDatabase();
    db->LoadChannelsAndGroups(rootItem.get());
    if(loadingIconsThread)
    {
        loadingIconsThread->wait();
        delete loadingIconsThread;
    }
    loadingIconsThread = QThread::create([this](){
        rootItem->loadChannelsIcons();
    });
    loadingIconsThread->start();
}

void ChannelsModel::AddList(M3UList list)
{
    beginResetModel();
    auto db = DatabaseProvider::GetDatabase();
    db->WithTransaction([&list, db, this](){
        for(qsizetype i =0; i< list.GetSegmentsCount(); i++)
        {
            auto channelPtr = rootItem->addMediaSegment(list.GetSegmentAt(i));
            db->AddChannelAndGroup(channelPtr);
            rootItem->updateMaps(channelPtr);
        }
    });
    endResetModel();
}
void ChannelsModel::AddToFavourites(AbstractChannelTreeItem* item)
{
    auto [parent, child] = rootItem->addToFavourites(item);
    auto db = DatabaseProvider::GetDatabase();
    db->SetFavourite(child->getID(), true);
    auto parentIndex = indexFromItem(parent);
    beginInsertRows(parentIndex, child->row(), child->row());
    endInsertRows();
}
void ChannelsModel::RemoveFromFavourites(AbstractChannelTreeItem* item)
{
    auto id = item->getID();
    auto parentIndex = indexFromItem(item->getParent());
    beginRemoveRows(parentIndex, item->row(), item->row());
    rootItem->removeFromFavourites(item);
    auto db = DatabaseProvider::GetDatabase();
    db->SetFavourite(id, false);
    endRemoveRows();
}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[ChannelRoles::NameRole] = "name";
    roles[ChannelRoles::UriRole] = "uri";
    roles[ChannelRoles::IdRole] = "id";
    return roles;
}

QVariant ChannelsModel::headerData(int section, Qt::Orientation orientation,
                            int role) const
{
    Q_UNUSED(section);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return tr("Channels");
    }

    return QVariant();
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
    AbstractChannelTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem.get();
    else
        parentItem = static_cast<AbstractChannelTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}
int ChannelsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}
QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    AbstractChannelTreeItem* item = static_cast<AbstractChannelTreeItem*>(index.internalPointer());
    switch(role)
    {
    case Qt::DisplayRole:
    case NameRole:
        return item->getName();
    case Qt::DecorationRole:
        return item->getIcon();
    case UriRole:
        if(item->getType() == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem* channel = dynamic_cast<ChannelTreeItem*>(item);
            if(channel)
            {
                return channel->getUri();
            }
            else
            {
                return QVariant{};
            }
        }
        else
        {
            return QVariant{};
        }
    case IdRole:
        if(item->getType() == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem* channel = dynamic_cast<ChannelTreeItem*>(item);
            if(channel)
            {
                return QVariant{(qlonglong)channel->getID()};
            }
            else
            {
                return QVariant{};
            }
        }
        return QVariant{};
    default:
        return QVariant{};
    }
}
QModelIndex ChannelsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractChannelTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem.get();
    else
        parentItem = static_cast<AbstractChannelTreeItem*>(parent.internalPointer());

    AbstractChannelTreeItem *childItem = parentItem->child(row);
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}
Qt::ItemFlags ChannelsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex ChannelsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractChannelTreeItem *childItem = static_cast<AbstractChannelTreeItem*>(index.internalPointer());
    AbstractChannelTreeItem *parentItem = childItem->getParent();

    if (parentItem == rootItem.get())
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}
QModelIndex ChannelsModel::indexFromItem(AbstractChannelTreeItem* item)
{
    if(item == rootItem.get() || item == nullptr)
    {
        return QModelIndex();
    }

    return createIndex(item->row(), 0, item);
}
void ChannelsModel::aquiredIcon(AbstractChannelTreeItem* item)
{
    if(item->getType() == ChannelTreeItemType::Channel)
    {
        auto channel = dynamic_cast<ChannelTreeItem*>(item);
        if(channel)
        {
            auto db = DatabaseProvider::GetDatabase();
            db->SetChannelLogo(channel);
        }
    }

    auto index = indexFromItem(item);
    emit dataChanged(index,index);
}

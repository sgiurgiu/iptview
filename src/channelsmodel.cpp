#include "channelsmodel.h"
#include "abstractchanneltreeitem.h"
#include "roottreeitem.h"
#include "grouptreeitem.h"
#include "favouritestreeitem.h"
#include "channeltreeitem.h"

#include "databaseprovider.h"
#include "database.h"
#include <QtDebug>


#include "loadingchannelsthread.h"
#include "loadingchanneliconsworker.h"
#include "loadxstreamchannelsworker.h"


ChannelsModel::ChannelsModel(QObject *parent)
    : QAbstractItemModel{parent},
      rootItem{std::make_unique<RootTreeItem>()}
{
    loadChannels();
    connect(this, SIGNAL(cancelImportChannels()),this, SLOT(cancelImportChannelsSlot()));
}
ChannelsModel::~ChannelsModel()
{
    stopAndClearThreads();
}
void ChannelsModel::stopAndClearThreads()
{
    cancelImportChannelsSlot();
    if(loadingChannelsThread)
    {
        loadingChannelsThread->cancelOperation();
        loadingChannelsThread->wait();
        delete loadingChannelsThread;
        loadingChannelsThread = nullptr;
    }
    if(loadingChannelIconsThread)
    {
        loadingChannelIconsThread->quit();
        loadingChannelIconsThread->wait();
        delete loadingChannelIconsThread;
        loadingChannelIconsThread = nullptr;
    }
}
void ChannelsModel::loadChannels()
{
    loadingChannelIconsThread = new QThread(this);
    channelIconsWorker  = new LoadingChannelIconsWorker();
    channelIconsWorker->moveToThread(loadingChannelIconsThread);
    connect(loadingChannelIconsThread, &QThread::finished, channelIconsWorker, &QObject::deleteLater);
    connect(this, &ChannelsModel::loadChannelIcon, channelIconsWorker, &LoadingChannelIconsWorker::loadChannelIcon);
    connect(channelIconsWorker, &LoadingChannelIconsWorker::channelIconReady, this, &ChannelsModel::onChannelIconReady);
    loadingChannelIconsThread->start();

    loadingChannelsThread = new LoadingChannelsThread(QThread::currentThread(), this);
    connect(loadingChannelsThread, &LoadingChannelsThread::groupLoaded,this, &ChannelsModel::onGroupLoaded);
    connect(loadingChannelsThread, &LoadingChannelsThread::groupsCount, this,  &ChannelsModel::onGroupsCount);
    connect(loadingChannelsThread, &LoadingChannelsThread::favouriteChannels, this,  &ChannelsModel::onFavouriteChannels);
    loadingChannelsThread->start();
}
M3UList ChannelsModel::GetM3UList() const
{
    return rootItem->GetM3UList();
}
void ChannelsModel::ReloadChannels()
{
    stopAndClearThreads();
    beginResetModel();
    rootItem->clear();
    endResetModel();
    loadChannels();
}
void ChannelsModel::onGroupsCount(int)
{

}
void ChannelsModel::onGroupLoaded(GroupTreeItem* group)
{
    beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
    rootItem->addGroup(group);
    endInsertRows();
    loadGroupChannelsIcons(group);
}
void ChannelsModel::loadGroupChannelsIcons(GroupTreeItem* group)
{
    int count = group->childCount();
    for(int i=0;i< count; i++)
    {
        auto child = group->child(i);
        if(child->getType() == ChannelTreeItemType::Channel)
        {
            auto channel = dynamic_cast<ChannelTreeItem*>(child);
            emit loadChannelIcon(channel);
        }
        else if(child->getType() == ChannelTreeItemType::Group)
        {
            auto childGroup = dynamic_cast<GroupTreeItem*>(child);
            loadGroupChannelsIcons(childGroup);
        }
    }
}
void ChannelsModel::onFavouriteChannels(std::vector<ChannelTreeItem*> channels)
{
    if(channels.empty()) return;
    auto favourites = rootItem->getFavourites();
    auto parentIndex = indexFromItem(favourites);
    beginInsertRows(parentIndex, 0, channels.size()-1);
    for(auto c : channels)
    {
        favourites->addChannel(c);
    }
    endInsertRows();

    for(auto c : channels)
    {
        emit loadChannelIcon(c);
    }
}
void ChannelsModel::AddList(M3UList list)
{
    beginResetModel();
    auto db = DatabaseProvider::GetDatabase();
    auto dbptr = db.get();
    db->WithTransaction([&list, dbptr, this](){
        for(qsizetype i =0; i< list.GetSegmentsCount() && !cancelImportingChannels; i++)
        {            
            auto channelPtr = rootItem->addMediaSegment(list.GetSegmentAt(i));
            dbptr->AddChannelAndGroup(channelPtr);
            rootItem->updateMaps(channelPtr);
            emit updateImportedChannelIndex(i);
            emit loadChannelIcon(channelPtr);
        }
    });
    endResetModel();
    emit channelsImported();
}
void ChannelsModel::AddList(CollectedInfo list)
{
    QThread* thread = new QThread();
    auto worker = new LoadXstreamChannelsWorker(std::move(list));
    xstreamGroupImportingCount = 0;
    worker->moveToThread(thread);
    worker->setDestinationThread(this->thread());
    connect( thread, &QThread::started, worker, &LoadXstreamChannelsWorker::importChannels);
    connect( thread, &QThread::finished, thread, &QThread::deleteLater);
    connect( worker, &LoadXstreamChannelsWorker::finished, this, &ChannelsModel::channelsImported);
    connect( worker, &LoadXstreamChannelsWorker::finished, thread, &QThread::quit);
    connect( worker, &LoadXstreamChannelsWorker::finished, worker, &LoadXstreamChannelsWorker::deleteLater);    
    connect( worker, &LoadXstreamChannelsWorker::loadedGroup, this, [this](GroupTreeItem* group){
        beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
        rootItem->addGroup(group);
        endInsertRows();
        xstreamGroupImportingCount++;
        emit updateImportedChannelIndex(xstreamGroupImportingCount);
    });
    connect(this, SIGNAL(cancelImportChannels()), worker, SLOT(cancelImportChannels()));
    thread->start();
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

    //qDebug() << "parentItem->childCount():"<<parentItem->getName() <<" ," << parentItem->childCount();
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
void ChannelsModel::AddChild(AbstractChannelTreeItem* child, const QModelIndex &parent)
{    
    AbstractChannelTreeItem *parentItem = nullptr;
    if(!parent.isValid())
    {
        parentItem = rootItem.get();
    }
    else
    {
        parentItem = static_cast<AbstractChannelTreeItem*>(parent.internalPointer());
    }
    beginInsertRows(parent,parentItem->childCount(),parentItem->childCount());
    auto parentType = parentItem->getType();
    auto childType = child->getType();
    switch(parentType)
    {
    case ChannelTreeItemType::Group:
        if(childType == ChannelTreeItemType::Channel)
        {
            (static_cast<GroupTreeItem*>(parentItem))->addChannel(static_cast<ChannelTreeItem*>(child));
        }
        if(childType == ChannelTreeItemType::Group)
        {
            (static_cast<GroupTreeItem*>(parentItem))->addGroup(static_cast<GroupTreeItem*>(child));
        }
    break;
    case ChannelTreeItemType::Root:
        if(childType == ChannelTreeItemType::Channel)
        {
            rootItem->addChannel(static_cast<ChannelTreeItem*>(child));
        }
        if(childType == ChannelTreeItemType::Group)
        {
            rootItem->addGroup(static_cast<GroupTreeItem*>(child));
        }
        break;
    default:
        break;
    }

    endInsertRows();
}
void ChannelsModel::RemoveChild(AbstractChannelTreeItem* child, const QModelIndex &index)
{
    auto parent = child->getParent();
    if(parent == nullptr) return;
    beginRemoveRows(index.parent(), index.row(),index.row());
    parent->removeChild(child);
    endRemoveRows();
}

void ChannelsModel::onChannelIconReady(ChannelTreeItem* channel)
{
    if(cancelImportingChannels || !channel)
    {
        return;
    }
    auto index = indexFromItem(channel);
    emit dataChanged(index,index);
}
void ChannelsModel::cancelImportChannelsSlot()
{
    cancelImportingChannels = true;
    if(loadingChannelsThread)
    {
        loadingChannelsThread->cancelOperation();
    }
    if(channelIconsWorker)
    {
        channelIconsWorker->CancelIconsLoading();
    }
    if(loadingChannelIconsThread)
    {
        loadingChannelIconsThread->quit();
        loadingChannelIconsThread->wait();
    }
}

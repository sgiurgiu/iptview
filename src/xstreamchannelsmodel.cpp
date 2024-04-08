#include "xstreamchannelsmodel.h"

#include "abstractchanneltreeitem.h"
#include "channeltreeitem.h"
#include "favouritestreeitem.h"
#include "grouptreeitem.h"
#include "roottreeitem.h"
#include "servertreeitem.h"

#include "database.h"
#include "databaseprovider.h"
#include <QNetworkAccessManager>
#include <QtDebug>

XStreamChannelsModel::XStreamChannelsModel(QNetworkAccessManager *networkManager,
                                           QObject *parent)
: QAbstractItemModel{ parent }
, rootItem{ std::make_unique<RootTreeItem>() }
, networkManager{ networkManager }
{
    auto db = DatabaseProvider::GetDatabase();
    auto xstreamServers = db->GetAllXStreamServers();
    for (const auto &server : xstreamServers)
    {
        ServerTreeItem *treeItem = new ServerTreeItem(server);
        connect(treeItem, SIGNAL(childrenLoaded(ServerTreeItem *)), this,
                SLOT(onServerChildrenLoaded(ServerTreeItem *)));
        connect(treeItem, SIGNAL(channelsLoaded(GroupTreeItem *)), this,
                SLOT(onChannelsLoaded(GroupTreeItem *)));
        rootItem->appendChild(treeItem);
    }
}

XStreamChannelsModel::~XStreamChannelsModel()
{
}

QHash<int, QByteArray> XStreamChannelsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[ChannelRoles::NameRole] = "name";
    roles[ChannelRoles::UriRole] = "uri";
    roles[ChannelRoles::IdRole] = "id";
    roles[ChannelRoles::DataRole] = "data";
    return roles;
}

void XStreamChannelsModel::onServerChildrenLoaded(ServerTreeItem *treeItem)
{
    auto index = indexFromItem(treeItem);
    // for the Loading item
    beginRemoveRows(index, 0, treeItem->childCount() - 1);
    endRemoveRows();
    beginInsertRows(index, 0, treeItem->childCount() - 1);
    endInsertRows();
}

void XStreamChannelsModel::onChannelsLoaded(GroupTreeItem *treeItem)
{
    auto index = indexFromItem(treeItem);
    // for the Loading item
    beginRemoveRows(index, 0, treeItem->childCount() - 1);
    endRemoveRows();
    beginInsertRows(index, 0, treeItem->childCount() - 1);
    endInsertRows();
}
void XStreamChannelsModel::refreshItemsChildren(const QVariant &itemVariant)
{
    auto treeItem = itemVariant.value<AbstractChannelTreeItem *>();
    if (treeItem->getType() == ChannelTreeItemType::Server)
    {
        ServerTreeItem *serverTreeItem = dynamic_cast<ServerTreeItem *>(treeItem);
        serverTreeItem->loadChildren(networkManager);
    }
    else if (treeItem->getType() == ChannelTreeItemType::Group)
    {
        GroupTreeItem *groupTreeItem = dynamic_cast<GroupTreeItem *>(treeItem);
        groupTreeItem->loadChannels(networkManager);
    }
}
QVariant XStreamChannelsModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const
{
    Q_UNUSED(section);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return tr("Channels");
    }

    return QVariant();
}

int XStreamChannelsModel::rowCount(const QModelIndex &parent) const
{
    AbstractChannelTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem.get();
    else
        parentItem =
            static_cast<AbstractChannelTreeItem *>(parent.internalPointer());

    if (parentItem->getType() == ChannelTreeItemType::Server)
    {
        ServerTreeItem *serverTreeItem =
            dynamic_cast<ServerTreeItem *>(parentItem);

        if (!serverTreeItem->areChildrenLoaded())
        {
            serverTreeItem->loadChildren(networkManager);
        }
    }
    else if (parentItem->getType() == ChannelTreeItemType::Group)
    {
        GroupTreeItem *groupTreeItem = dynamic_cast<GroupTreeItem *>(parentItem);

        if (!groupTreeItem->areChannelsLoaded())
        {
            groupTreeItem->loadChannels(networkManager);
        }
    }
    return parentItem->childCount();
}
int XStreamChannelsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}
QVariant XStreamChannelsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    AbstractChannelTreeItem *item =
        static_cast<AbstractChannelTreeItem *>(index.internalPointer());
    switch (role)
    {
    case Qt::DisplayRole:
    case NameRole:
        return item->getName();
    case Qt::DecorationRole:
        return item->getIcon();
    case UriRole:
        if (item->getType() == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem *channel = dynamic_cast<ChannelTreeItem *>(item);
            if (channel)
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
        if (item->getType() == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem *channel = dynamic_cast<ChannelTreeItem *>(item);
            if (channel)
            {
                return QVariant{ (qlonglong)channel->getID() };
            }
            else
            {
                return QVariant{};
            }
        }
        return QVariant{};
    case DataRole:
        if (item->getType() == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem *channel = dynamic_cast<ChannelTreeItem *>(item);
            if (channel)
            {
                return QVariant::fromValue(channel);
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
QModelIndex XStreamChannelsModel::index(int row,
                                        int column,
                                        const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AbstractChannelTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem.get();
    else
        parentItem =
            static_cast<AbstractChannelTreeItem *>(parent.internalPointer());

    AbstractChannelTreeItem *childItem = parentItem->child(row);
    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}
Qt::ItemFlags XStreamChannelsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex XStreamChannelsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractChannelTreeItem *childItem =
        static_cast<AbstractChannelTreeItem *>(index.internalPointer());
    AbstractChannelTreeItem *parentItem = childItem->getParent();

    if (parentItem == rootItem.get())
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}
QModelIndex XStreamChannelsModel::indexFromItem(AbstractChannelTreeItem *item)
{
    if (item == rootItem.get() || item == nullptr)
    {
        return QModelIndex();
    }

    return createIndex(item->row(), 0, item);
}

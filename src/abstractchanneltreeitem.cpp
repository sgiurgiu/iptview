#include "abstractchanneltreeitem.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QImage>
#include <QPixmap>
#include <QString>

AbstractChannelTreeItem::AbstractChannelTreeItem(QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent):
    QObject{parent},
    parent{parent},
    networkManager{networkManager}
{
}
void AbstractChannelTreeItem::appendChild(std::unique_ptr<AbstractChannelTreeItem> child)
{
    children.push_back(std::move(child));
}
int AbstractChannelTreeItem::childCount() const
{
    return children.size();
}
int AbstractChannelTreeItem::row() const
{
    if(parent)
    {
        for(size_t i=0; i<parent->children.size(); i++)
        {
            if(parent->children.at(i).get() == this)
            {
                return i;
            }
        }
        return -1;
    }
    return -1;
}
AbstractChannelTreeItem* AbstractChannelTreeItem::child(int index) const
{
    if(index < 0 || index >= static_cast<int>(children.size())) return nullptr;
    return children.at(index).get();
}
AbstractChannelTreeItem* AbstractChannelTreeItem::getParent() const
{
    return parent;
}
QIcon AbstractChannelTreeItem::getIcon() const
{
    return icon;
}
int64_t AbstractChannelTreeItem::getID() const
{
    return id;
}
void AbstractChannelTreeItem::setID(int64_t id)
{
    this->id = id;
}
QNetworkAccessManager* AbstractChannelTreeItem::getNetworkManager() const
{
    return networkManager;
}
RootTreeItem::RootTreeItem(QNetworkAccessManager* networkManager) : AbstractChannelTreeItem(networkManager, nullptr)
{
    auto favourites = std::make_unique<FavouritesTreeItem>(networkManager, this);
    connect(favourites.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    appendChild(std::move(favourites));
}

ChannelTreeItem* RootTreeItem::addMediaSegment(const MediaSegment& segment)
{
    auto groupTitle = segment.GetAttributeValue("group-title");
    if(groupTitle)
    {
        auto it = groupsMap.find(groupTitle.value());
        if(it != groupsMap.end())
        {
            return it.value()->addMediaSegment(segment);
        }
        else
        {
            auto group = std::make_unique<GroupTreeItem>(groupTitle.value(),networkManager, this);
            auto channelPtr = group->addMediaSegment(segment);
            groupsMap.insert(groupTitle.value(), group.get());
            connect(group.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
            appendChild(std::move(group));
            return channelPtr;
        }
    }
    else
    {
        auto channel = std::make_unique<ChannelTreeItem>(segment, networkManager, this);
        connect(channel.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
        channel->loadLogo();
        auto channelPtr = channel.get();
        appendChild(std::move(channel));
        return channelPtr;
    }
}
void RootTreeItem::addChannel(std::unique_ptr<ChannelTreeItem> channel)
{
    connect(channel.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    channel->loadLogo();
    appendChild(std::move(channel));
}
void RootTreeItem::updateMaps(ChannelTreeItem* channel)
{
    auto parent = channel->getParent();
    AbstractChannelTreeItem* secondToLastParent = nullptr;
    while(parent != nullptr && parent != this && parent->getType() == ChannelTreeItemType::Group)
    {
        secondToLastParent = parent;
        parent = parent->getParent();
    }
    if(secondToLastParent && secondToLastParent->getID() >=0 && secondToLastParent->getType() == ChannelTreeItemType::Group)
    {
        auto group = dynamic_cast<GroupTreeItem*>(secondToLastParent);
        groupsIdMap.insert(group->getID(), group);
    }
}
void RootTreeItem::addGroup(std::unique_ptr<GroupTreeItem> group)
{
    groupsMap.insert(group->getName(), group.get());
    if(group->getID() >= 0)
    {
        groupsIdMap.insert(group->getID(), group.get());
    }
    connect(group.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    appendChild(std::move(group));
}
GroupTreeItem* RootTreeItem::getGroup(int64_t id) const
{
    auto it = groupsIdMap.find(id);
    if(it != groupsIdMap.end())
    {
        return it.value();
    }
    else
    {
        for(const auto& childGroup : groupsIdMap)
        {
            auto group = childGroup->getGroup(id);
            if(group != nullptr)
            {
                return group;
            }
        }
    }
    return nullptr;
}

GroupTreeItem* GroupTreeItem::getGroup(int64_t id) const
{
    auto it = groupsIdMap.find(id);
    if(it != groupsIdMap.end())
    {
        return it.value();
    }
    else
    {
        for(const auto& childGroup : groupsIdMap)
        {
            auto group = childGroup->getGroup(id);
            if(group != nullptr)
            {
                return group;
            }
        }
    }
    return nullptr;
}
ChannelTreeItem* GroupTreeItem::getChannel(int64_t id) const
{
    auto it = channelsIdMap.find(id);
    if(it != channelsIdMap.end())
    {
        return it.value();
    }
    else
    {
        for(const auto& childGroup : groupsIdMap)
        {
            auto channel = childGroup->getChannel(id);
            if(channel != nullptr)
            {
                return channel;
            }
        }
    }
    return nullptr;
}
ChannelTreeItem* GroupTreeItem::addMediaSegment(const MediaSegment& segment)
{
    auto channel = std::make_unique<ChannelTreeItem>(segment, networkManager, this);
    connect(channel.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    channel->loadLogo();
    auto channelPtr = channel.get();
    appendChild(std::move(channel));
    return channelPtr;
}
void GroupTreeItem::addGroup(std::unique_ptr<GroupTreeItem> group)
{
    if(group->getID() >= 0)
    {
        groupsIdMap.insert(group->getID(), group.get());
    }
    connect(group.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    appendChild(std::move(group));
}
void GroupTreeItem::addChannel(std::unique_ptr<ChannelTreeItem> channel)
{
    if(channel->getID() >= 0)
    {
        channelsIdMap.insert(channel->getID(), channel.get());
    }
    connect(channel.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    channel->loadLogo();
    appendChild(std::move(channel));
}
GroupTreeItem::GroupTreeItem(QString name,QNetworkAccessManager* networkManager, RootTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)}
{
}
GroupTreeItem::GroupTreeItem(QString name, QNetworkAccessManager* networkManager, GroupTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)}
{
}

ChannelTreeItem::ChannelTreeItem(QString name, QString uri, QString logoUri, QByteArray logo, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)},
      uri(std::move(uri)), logoUri{std::move(logoUri)}, logo{std::move(logo)}
{
    if(!this->logo.isEmpty())
    {
        QImage image = QImage::fromData(this->logo);
        icon = QIcon{QPixmap::fromImage(image)};
    }
}
ChannelTreeItem::ChannelTreeItem(const MediaSegment& segment, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent)
{
    name = segment.GetTitle();
    uri = segment.GetUri();
    auto logo = segment.GetAttributeValue("tvg-logo");
    if(logo && !logo.value().isEmpty())
    {
        logoUri = logo.value();
    }
}

void ChannelTreeItem::loadLogo()
{
    if(!logo.isEmpty()) return;

    if(logoUri.startsWith("data:"))
    {
        auto base64Index = logoUri.indexOf("base64,");
        if(base64Index > 0)
        {
            QString base64Data = logoUri.right(logoUri.size() - base64Index - 7);
            logo = QByteArray::fromBase64(base64Data.toUtf8());
            QImage image = QImage::fromData(logo);
            icon = QIcon{QPixmap::fromImage(image)};
            emit aquiredIcon(this);
        }
    }
    else
    {
        QNetworkRequest request;
        request.setUrl(QUrl{logoUri});
        request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
        auto reply = networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]()
        {
            logo = reply->readAll();
            QImage image = QImage::fromData(logo);
            icon = QIcon{QPixmap::fromImage(image)};
            reply->deleteLater();
            emit aquiredIcon(this);
        });
    }
}
FavouritesTreeItem::FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent) : GroupTreeItem{"Favourites", networkManager, parent}
{
    icon = QIcon(":/icons/star.svg");
}


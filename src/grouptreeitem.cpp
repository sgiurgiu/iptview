#include "grouptreeitem.h"
#include "channeltreeitem.h"
#include "mediasegment.h"
#include "roottreeitem.h"
#include "servertreeitem.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <QColor>
#include <QGraphicsColorizeEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>

namespace
{
Q_GLOBAL_STATIC(QIcon, defaultGroupIcon);
}
GroupTreeItem::GroupTreeItem(QString name,
                             QString categoryId,
                             XStreamAuthenticationInfo server,
                             ServerTreeItem* parent)
: AbstractChannelTreeItem(parent)
, name{ std::move(name) }
, loadedRemoteChannels{ false }
, categoryId{ std::move(categoryId) }
, server{ std::move(server) }
{
    if (defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
    {
        auto sizes = icon.availableSizes();

        for (const auto& size : sizes)
        {
            // prepare graphics scene and pixmap
            QGraphicsScene scene;
            QGraphicsPixmapItem item;
            auto pixmap = icon.pixmap(size);
            item.setPixmap(pixmap);

            QGraphicsColorizeEffect effect;
            effect.setColor(QColor(Qt::GlobalColor::lightGray));
            effect.setStrength(0.7);
            item.setGraphicsEffect(&effect);
            scene.addItem(&item);
            QPainter ptr(&pixmap);
            scene.render(&ptr, QRectF(),
                         QRectF(0, 0, size.width(), size.height()));
            unloadedIcon.addPixmap(pixmap);
        }
    }

    appendChild(new LoadingTreeItem(this));
}

GroupTreeItem::GroupTreeItem(QString name, RootTreeItem* parent)
: AbstractChannelTreeItem(parent), name{ std::move(name) }
{
    if (defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
}
GroupTreeItem::GroupTreeItem(QString name, GroupTreeItem* parent)
: AbstractChannelTreeItem(parent), name{ std::move(name) }
{
    if (defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
}
GroupTreeItem::GroupTreeItem(QString name)
: AbstractChannelTreeItem(nullptr), name{ std::move(name) }
{
    if (defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
}
QIcon GroupTreeItem::getIcon() const
{
    return loadedRemoteChannels ? icon : unloadedIcon;
}
QList<MediaSegment> GroupTreeItem::GetMediaSegments() const
{
    QList<MediaSegment> list;
    for (auto child : children)
    {
        auto type = child->getType();
        if (type == ChannelTreeItemType::Group)
        {
            auto group = dynamic_cast<GroupTreeItem*>(child);
            list.append(group->GetMediaSegments());
        }
        else if (type == ChannelTreeItemType::Channel)
        {
            auto channel = dynamic_cast<ChannelTreeItem*>(child);
            MediaSegment segment;
            segment.SetDuration(-1);
            segment.SetTitle(channel->getName());
            segment.SetUri(channel->getUri());
            segment.AddAttribute("tvg-logo", channel->getLogoUri());
            segment.AddAttribute("tvg-name", channel->getName());
            segment.AddAttribute("tvg-id", channel->getEpgChannelId());
            segment.AddAttribute("group-title", this->getName());

            list.append(std::move(segment));
        }
    }
    return list;
}
/*void GroupTreeItem::loadChannelsIcons()
{
    if(cancelOngoingOperations) return;

    for(auto&& child : children)
    {
        if(cancelOngoingOperations) return;
        auto type = child->getType();
        if(type == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem* channel =
dynamic_cast<ChannelTreeItem*>(child.get()); channel->loadIcon();
        }
        else if(type == ChannelTreeItemType::Group || type ==
ChannelTreeItemType::Favourite)
        {
            GroupTreeItem* group  = dynamic_cast<GroupTreeItem*>(child.get());
            group->loadChannelsIcons();
        }
    }
}*/
GroupTreeItem* GroupTreeItem::getGroup(int64_t id) const
{
    auto it = groupsIdMap.find(id);
    if (it != groupsIdMap.end())
    {
        return it.value();
    }
    else
    {
        for (const auto& childGroup : groupsIdMap)
        {
            auto group = childGroup->getGroup(id);
            if (group != nullptr)
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
    if (it != channelsIdMap.end())
    {
        return it.value();
    }
    else
    {
        for (const auto& childGroup : groupsIdMap)
        {
            auto channel = childGroup->getChannel(id);
            if (channel != nullptr)
            {
                return channel;
            }
        }
    }
    return nullptr;
}
ChannelTreeItem* GroupTreeItem::addMediaSegment(const MediaSegment& segment)
{
    ChannelTreeItem* channel = new ChannelTreeItem(segment, this);
    // channel->loadLogo();
    appendChild(channel);
    return channel;
}
void GroupTreeItem::addGroup(GroupTreeItem* group)
{
    if (groupsIdMap.contains(group->getID()))
        return;

    if (group->getID() >= 0)
    {
        groupsIdMap.insert(group->getID(), group);
    }
    appendChild(group);
}
void GroupTreeItem::addChannel(ChannelTreeItem* channel)
{
    if (channel->getID() >= 0)
    {
        channelsIdMap.insert(channel->getID(), channel);
    }
    appendChild(channel);
}
void GroupTreeItem::clear()
{
    AbstractChannelTreeItem::clear();
    groupsIdMap.clear();
    channelsIdMap.clear();
}
void GroupTreeItem::loadChannels(QNetworkAccessManager* networkManager)
{
    QNetworkRequest request;
    QUrl url;
    url.setHost(server.serverUrl);
    url.setScheme(server.serverSchema);
    url.setPort(server.serverPort.toInt());
    url.setPath("/player_api.php");
    QUrlQuery query;
    query.addQueryItem("username", server.username);
    query.addQueryItem("password", server.password);
    query.addQueryItem("action", "get_live_streams");
    query.addQueryItem("category_id", categoryId);
    url.setQuery(query);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    auto reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this,
            [reply, this]()
            {
                loadedRemoteChannels = true;
                clear();
                reply->deleteLater();
                if (reply->error())
                {
                    ErrorLoadingTreeItem* errorChild =
                        new ErrorLoadingTreeItem(reply->errorString(), this);
                    appendChild(errorChild);
                    emit channelsLoaded(this);
                    return;
                }

                auto response = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(response);
                auto channels = doc.array();

                for (const auto& ch : channels)
                {
                    auto channelObject = ch.toObject();
                    auto streamId = channelObject.value("stream_id").toInt();
                    if (streamId == 0)
                        continue;

                    auto categoryType =
                        channelObject.value("stream_type").toString("");
                    QUrl streamUrl;
                    streamUrl.setHost(server.serverUrl);
                    streamUrl.setScheme(server.serverSchema);
                    streamUrl.setPort(server.serverPort.toInt());
                    streamUrl.setPath(
                        QString("/%1/%2/%3/%4.ts")
                            .arg(categoryType, server.username, server.password)
                            .arg(streamId));
                    QUrl epgUrl;
                    epgUrl.setHost(server.serverUrl);
                    epgUrl.setScheme(server.serverSchema);
                    epgUrl.setPort(server.serverPort.toInt());
                    epgUrl.setPath("/player_api.php");
                    QUrlQuery epgQuery;
                    epgQuery.addQueryItem("username", server.username);
                    epgQuery.addQueryItem("password", server.password);
                    epgQuery.addQueryItem("action", "get_short_epg");
                    epgQuery.addQueryItem("stream_id", QString::number(streamId));
                    epgUrl.setQuery(epgQuery);
                    ChannelTreeItem* channelTreeItem = new ChannelTreeItem(
                        channelObject.value("name").toString(""),
                        streamUrl.toString(),
                        channelObject.value("stream_icon").toString(""), {},
                        this);
                    channelTreeItem->setEpgChannelId(
                        channelObject.value("epg_channel_id").toString(""));
                    channelTreeItem->setEpgChannelUri(epgUrl.toString());
                    channelTreeItem->setXStreamServerId(server.id);
                    addChannel(channelTreeItem);
                }

                emit channelsLoaded(this);
            });
}

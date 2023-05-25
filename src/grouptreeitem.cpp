#include "grouptreeitem.h"
#include "channeltreeitem.h"
#include "mediasegment.h"
#include "roottreeitem.h"

GroupTreeItem::GroupTreeItem(QString name,QNetworkAccessManager* networkManager, RootTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)}
{
}
GroupTreeItem::GroupTreeItem(QString name, QNetworkAccessManager* networkManager, GroupTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)}
{
}

void GroupTreeItem::loadChannelsIcons()
{
    for(auto&& child : children)
    {
        auto type = child->getType();
        if(type == ChannelTreeItemType::Channel)
        {
            ChannelTreeItem* channel = dynamic_cast<ChannelTreeItem*>(child.get());
            channel->loadIcon();
        }
        else if(type == ChannelTreeItemType::Group || type == ChannelTreeItemType::Favourite)
        {
            GroupTreeItem* group  = dynamic_cast<GroupTreeItem*>(child.get());
            group->loadChannelsIcons();
        }
    }
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

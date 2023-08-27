#include "grouptreeitem.h"
#include "channeltreeitem.h"
#include "mediasegment.h"
#include "roottreeitem.h"

namespace
{
    Q_GLOBAL_STATIC(QIcon, defaultGroupIcon);
}

GroupTreeItem::GroupTreeItem(QString name,RootTreeItem* parent)
    : AbstractChannelTreeItem(parent), name{std::move(name)}
{
    if(defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
}
GroupTreeItem::GroupTreeItem(QString name, GroupTreeItem* parent)
    : AbstractChannelTreeItem(parent), name{std::move(name)}
{
    if(defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
}
GroupTreeItem::GroupTreeItem(QString name)
    : AbstractChannelTreeItem(nullptr), name{std::move(name)}
{
    if(defaultGroupIcon->isNull())
    {
        defaultGroupIcon->addFile(":/icons/folder-open.png");
    }
    icon = *defaultGroupIcon;
}
QList<MediaSegment> GroupTreeItem::GetMediaSegments() const
{
    QList<MediaSegment> list;
    for(auto child:children)
    {
        auto type = child->getType();
        if(type == ChannelTreeItemType::Group)
        {
            auto group = dynamic_cast<GroupTreeItem*>(child);
            list.append(group->GetMediaSegments());
        }
        else if(type == ChannelTreeItemType::Channel)
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
            ChannelTreeItem* channel = dynamic_cast<ChannelTreeItem*>(child.get());
            channel->loadIcon();
        }
        else if(type == ChannelTreeItemType::Group || type == ChannelTreeItemType::Favourite)
        {
            GroupTreeItem* group  = dynamic_cast<GroupTreeItem*>(child.get());
            group->loadChannelsIcons();
        }
    }
}*/
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
    ChannelTreeItem* channel = new ChannelTreeItem(segment, this);
    //channel->loadLogo();    
    appendChild(channel);
    return channel;
}
void GroupTreeItem::addGroup(GroupTreeItem* group)
{
    if(groupsIdMap.contains(group->getID())) return;

    if(group->getID() >= 0)
    {
        groupsIdMap.insert(group->getID(), group);
    }
    appendChild(group);
}
void GroupTreeItem::addChannel(ChannelTreeItem* channel)
{
    if(channel->getID() >= 0)
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

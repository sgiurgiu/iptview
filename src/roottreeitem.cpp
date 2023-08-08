#include "roottreeitem.h"

#include "grouptreeitem.h"
#include "channeltreeitem.h"
#include "mediasegment.h"
#include "favouritestreeitem.h"

RootTreeItem::RootTreeItem() : AbstractChannelTreeItem(nullptr)
{
    favourites = new FavouritesTreeItem(this);
    connect(favourites, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    appendChild(favourites);
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
            GroupTreeItem* group = new GroupTreeItem(groupTitle.value(),this);
            auto channelPtr = group->addMediaSegment(segment);
            groupsMap.insert(groupTitle.value(), group);
            appendChild(group);
            return channelPtr;
        }
    }
    else
    {
        ChannelTreeItem* channel = new ChannelTreeItem(segment, this);
        appendChild(channel);
        return channel;
    }
}
void RootTreeItem::addChannel(ChannelTreeItem* channel)
{
    appendChild(channel);
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
void RootTreeItem::addGroup(GroupTreeItem* group)
{
    if(groupsIdMap.contains(group->getID())) return;

    groupsMap.insert(group->getName(), group);
    if(group->getID() >= 0)
    {
        groupsIdMap.insert(group->getID(), group);
    }
    appendChild(group);
}
/*void RootTreeItem::loadChannelsIcons()
{
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

void RootTreeItem::addToFavourites(ChannelTreeItem* channel)
{
    favourites->addChannel(channel);
}

std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> RootTreeItem::addToFavourites(AbstractChannelTreeItem* item)
{
    if(item == nullptr) return std::make_pair(nullptr,nullptr);

    ChannelTreeItem* newChild = nullptr;
    if(item->getType() == ChannelTreeItemType::Channel)
    {
        ChannelTreeItem* channelPtr = dynamic_cast<ChannelTreeItem*>(item);
        newChild = channelPtr->clone(favourites);
        favourites->addChannel(newChild);
    }
    return std::make_pair(favourites,newChild);
}
std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> RootTreeItem::removeFromFavourites(AbstractChannelTreeItem* item)
{
    auto lastChild = favourites->removeFavouriteChild(item);
    return std::make_pair(favourites,lastChild);
}

void RootTreeItem::clear()
{
    AbstractChannelTreeItem::clear();
    groupsIdMap.clear();
    groupsIdMap.clear();
    favourites = new FavouritesTreeItem(this);
    connect(favourites, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    appendChild(favourites);
}

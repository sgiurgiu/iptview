#include "roottreeitem.h"

#include "grouptreeitem.h"
#include "channeltreeitem.h"
#include "mediasegment.h"
#include "favouritestreeitem.h"

RootTreeItem::RootTreeItem(QNetworkAccessManager* networkManager) : AbstractChannelTreeItem(networkManager, nullptr)
{
    auto favourites = std::make_unique<FavouritesTreeItem>(networkManager, this);
    this->favourites = favourites.get();
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
void RootTreeItem::loadChannelsIcons()
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

void RootTreeItem::addToFavourites(std::unique_ptr<ChannelTreeItem> channel)
{
    favourites->addChannel(std::move(channel));
}

std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> RootTreeItem::addToFavourites(AbstractChannelTreeItem* item)
{
    if(item == nullptr) return std::make_pair(nullptr,nullptr);

    ChannelTreeItem* newChildPtr = nullptr;
    if(item->getType() == ChannelTreeItemType::Channel)
    {
        ChannelTreeItem* channelPtr = dynamic_cast<ChannelTreeItem*>(item);
        auto newChild = channelPtr->clone(favourites);
        newChildPtr = newChild.get();
        favourites->addChannel(std::move(newChild));
    }
    return std::make_pair(favourites,newChildPtr);
}
std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> RootTreeItem::removeFromFavourites(AbstractChannelTreeItem* item)
{
    auto lastChild = favourites->removeChild(item);
    return std::make_pair(favourites,lastChild);
}

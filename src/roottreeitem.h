#ifndef ROOTTREEITEM_H
#define ROOTTREEITEM_H

#include "abstractchanneltreeitem.h"
#include "m3ulist.h"

class ChannelTreeItem;
class GroupTreeItem;
class MediaSegment;
class FavouritesTreeItem;

class RootTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    RootTreeItem();
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Root;
    }
    virtual QString getName() const  override
    {
        return "";
    }
    FavouritesTreeItem* getFavourites() const
    {
        return favourites;
    }
    ChannelTreeItem* addMediaSegment(const MediaSegment& segment);
    void addChannel(ChannelTreeItem* channel);
    void updateMaps(ChannelTreeItem* channel);
    void addGroup(GroupTreeItem* group);
    GroupTreeItem* getGroup(int64_t id) const;
    void addToFavourites(ChannelTreeItem* channel);
    std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> addToFavourites(AbstractChannelTreeItem* item);
    std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> removeFromFavourites(AbstractChannelTreeItem* item);
    virtual void clear() override;
    M3UList GetM3UList() const;
private:
    QHash<QString, GroupTreeItem*> groupsMap;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
    FavouritesTreeItem* favourites;
};

#endif // ROOTTREEITEM_H

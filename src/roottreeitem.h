#ifndef ROOTTREEITEM_H
#define ROOTTREEITEM_H

#include "abstractchanneltreeitem.h"

class ChannelTreeItem;
class GroupTreeItem;
class MediaSegment;
class FavouritesTreeItem;

class RootTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    RootTreeItem(QNetworkAccessManager* networkManager);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Root;
    }
    virtual QString getName() const  override
    {
        return "";
    }
    ChannelTreeItem* addMediaSegment(const MediaSegment& segment);
    void addChannel(std::unique_ptr<ChannelTreeItem> channel);
    void updateMaps(ChannelTreeItem* channel);
    void addGroup(std::unique_ptr<GroupTreeItem> group);
    GroupTreeItem* getGroup(int64_t id) const;
    void loadChannelsIcons();
    void addToFavourites(std::unique_ptr<ChannelTreeItem> channel);
    std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> addToFavourites(AbstractChannelTreeItem* item);
    std::pair<AbstractChannelTreeItem*,AbstractChannelTreeItem*> removeFromFavourites(AbstractChannelTreeItem* item);
private:
    QHash<QString, GroupTreeItem*> groupsMap;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
    FavouritesTreeItem* favourites;
};

#endif // ROOTTREEITEM_H

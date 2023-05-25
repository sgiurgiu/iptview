#ifndef GROUPTREEITEM_H
#define GROUPTREEITEM_H

#include "abstractchanneltreeitem.h"

class RootTreeItem;
class ChannelTreeItem;
class MediaSegment;

class GroupTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit GroupTreeItem(QString name, QNetworkAccessManager* networkManager, RootTreeItem* parent);
    explicit GroupTreeItem(QString name, QNetworkAccessManager* networkManager, GroupTreeItem* parent);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Group;
    }
    virtual QString getName() const override
    {
        return name;
    }
    ChannelTreeItem* addMediaSegment(const MediaSegment& segment);
    void addChannel(std::unique_ptr<ChannelTreeItem> channel);
    void addGroup(std::unique_ptr<GroupTreeItem> group);
    GroupTreeItem* getGroup(int64_t id) const;
    ChannelTreeItem* getChannel(int64_t id) const;
    void loadChannelsIcons();
private:
    QString name;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
    QHash<int64_t, ChannelTreeItem*> channelsIdMap;
};


#endif // GROUPTREEITEM_H

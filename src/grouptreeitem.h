#ifndef GROUPTREEITEM_H
#define GROUPTREEITEM_H

#include "abstractchanneltreeitem.h"

class RootTreeItem;
class ChannelTreeItem;
class MediaSegment;

class TestingItem
{
public:
    explicit TestingItem(QString name):name{name}
    {
    }
    QString getName() const
    {
        return name;
    }
private:
    QString name;
};

class GroupTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit GroupTreeItem(QString name,RootTreeItem* parent);
    explicit GroupTreeItem(QString name,GroupTreeItem* parent);
    explicit GroupTreeItem(QString name);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Group;
    }
    virtual QString getName() const override
    {
        return name;
    }
    ChannelTreeItem* addMediaSegment(const MediaSegment& segment);
    void addChannel(ChannelTreeItem* channel);
    void addGroup(GroupTreeItem* group);
    GroupTreeItem* getGroup(int64_t id) const;
    ChannelTreeItem* getChannel(int64_t id) const;
    virtual void clear() override;
private:
    QString name;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
    QHash<int64_t, ChannelTreeItem*> channelsIdMap;
};


#endif // GROUPTREEITEM_H

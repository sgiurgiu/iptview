#ifndef GROUPTREEITEM_H
#define GROUPTREEITEM_H

#include "abstractchanneltreeitem.h"
#include "mediasegment.h"
#include "xstreaminfo.h"
#include <QList>
class RootTreeItem;
class ChannelTreeItem;
class ServerTreeItem;
class QNetworkAccessManager;

class TestingItem
{
public:
    explicit TestingItem(QString name) : name{ name }
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
    explicit GroupTreeItem(QString name, RootTreeItem* parent);
    explicit GroupTreeItem(QString name, GroupTreeItem* parent);
    explicit GroupTreeItem(QString name,
                           QString categoryId,
                           XStreamAuthenticationInfo server,
                           ServerTreeItem* parent);
    explicit GroupTreeItem(QString name);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Group;
    }
    virtual QString getName() const override
    {
        return name;
    }
    bool areChannelsLoaded() const
    {
        return loadedRemoteChannels;
    }
    QList<MediaSegment> GetMediaSegments() const;
    ChannelTreeItem* addMediaSegment(const MediaSegment& segment);
    void addChannel(ChannelTreeItem* channel);
    void addGroup(GroupTreeItem* group);
    GroupTreeItem* getGroup(int64_t id) const;
    ChannelTreeItem* getChannel(int64_t id) const;
    virtual void clear() override;
    void loadChannels(QNetworkAccessManager* networkManager);
signals:
    void channelsLoaded(GroupTreeItem*);

private:
    QString name;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
    QHash<int64_t, ChannelTreeItem*> channelsIdMap;
    bool loadedRemoteChannels = false;
    QString categoryId;
    XStreamAuthenticationInfo server;
};

#endif // GROUPTREEITEM_H

#ifndef ABSTRACTCHANNELTREEITEM_H
#define ABSTRACTCHANNELTREEITEM_H

#include <QObject>
#include <QIcon>
#include <vector>
#include <QHash>

#include "mediasegment.h"

class QNetworkAccessManager;

enum class ChannelTreeItemType
{
    Root = 0,
    Group,
    Channel,
    Favourite
};

class AbstractChannelTreeItem : public QObject
{
    Q_OBJECT
public:
    explicit AbstractChannelTreeItem(QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent = nullptr);
    virtual ~AbstractChannelTreeItem() = default;
    virtual ChannelTreeItemType getType() const = 0;
    virtual QString getName() const = 0;
    virtual QIcon getIcon() const;
    virtual void appendChild(std::unique_ptr<AbstractChannelTreeItem> child);
    virtual int childCount() const;
    virtual int row() const;
    virtual AbstractChannelTreeItem* child(int index) const;
    virtual AbstractChannelTreeItem* getParent() const;
    int64_t getID() const;
    void setID(int64_t id);
    QNetworkAccessManager* getNetworkManager() const;
signals:
    void aquiredIcon(AbstractChannelTreeItem*);
protected:
    int64_t id = -1;
    std::vector<std::unique_ptr<AbstractChannelTreeItem>> children;
    AbstractChannelTreeItem* parent = nullptr;
    QIcon icon;
    QNetworkAccessManager* networkManager;
};

class RootTreeItem;
class ChannelTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit ChannelTreeItem(const MediaSegment& segment, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent);
    explicit ChannelTreeItem(QString name, QString uri, QString logoUri, QByteArray logo, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Channel;
    }
    virtual QString getName() const override
    {
        return name;
    }
    virtual QString getUri() const
    {
        return uri;
    }
    virtual QString getLogoUri() const
    {
        return logoUri;
    }
    virtual QByteArray const& getLogo() const
    {
        return logo;
    }
    void loadLogo();
private:
    QString name;
    QString uri;
    QString logoUri;
    QByteArray logo;
};

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
private:
    QString name;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
    QHash<int64_t, ChannelTreeItem*> channelsIdMap;
};

class FavouritesTreeItem : public GroupTreeItem
{
    Q_OBJECT
public:
    FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Favourite;
    }
};

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
private:
    QHash<QString, GroupTreeItem*> groupsMap;
    QHash<int64_t, GroupTreeItem*> groupsIdMap;
};


#endif // ABSTRACTCHANNELTREEITEM_H

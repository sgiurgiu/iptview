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
    Channel
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
signals:
    void aquiredIcon(AbstractChannelTreeItem*);
protected:
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
    void loadLogo();
private:
    QString name;
    QString uri;
    QString logoUri;
};

class GroupTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit GroupTreeItem(QString name, QNetworkAccessManager* networkManager, RootTreeItem* parent);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Group;
    }
    virtual QString getName() const override
    {
        return name;
    }
    void addMediaSegment(const MediaSegment& segment);
private:
    QString name;
};

class FavouritesTreeItem : public GroupTreeItem
{
    Q_OBJECT
public:
    FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent);
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
    void addMediaSegment(const MediaSegment& segment);
private:
    QHash<QString, GroupTreeItem*> groupsMap;
};


#endif // ABSTRACTCHANNELTREEITEM_H

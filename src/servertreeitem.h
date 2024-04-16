#ifndef SERVER_TREE_ITEM_H
#define SERVER_TREE_ITEM_H

#include "abstractchanneltreeitem.h"
#include "xstreaminfo.h"

class QNetworkAccessManager;
class GroupTreeItem;

class LoadingTreeItem : public AbstractChannelTreeItem
{
public:
    explicit LoadingTreeItem(AbstractChannelTreeItem* parent = nullptr)
    : AbstractChannelTreeItem(parent)
    {
    }
    virtual ChannelTreeItemType getType() const
    {
        return ChannelTreeItemType::Loading;
    }
    virtual QString getName() const
    {
        return "Loading...";
    }
};

class ErrorLoadingTreeItem : public AbstractChannelTreeItem
{
public:
    explicit ErrorLoadingTreeItem(QString error,
                                  AbstractChannelTreeItem* parent = nullptr)
    : AbstractChannelTreeItem(parent), error{ std::move(error) }
    {
        icon = QIcon(":/icons/remove-circle.png");
    }
    virtual ChannelTreeItemType getType() const
    {
        return ChannelTreeItemType::Error;
    }
    virtual QString getName() const
    {
        return error;
    }

private:
    QString error;
};

class ServerTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit ServerTreeItem(XStreamAuthenticationInfo server,
                            AbstractChannelTreeItem* parent = nullptr);
    virtual ChannelTreeItemType getType() const
    {
        return ChannelTreeItemType::Server;
    }
    virtual QString getName() const
    {
        return server.serverUrl;
    }

    bool areChildrenLoaded() const
    {
        return loadedChildren;
    }

    void loadChildren(QNetworkAccessManager* networkManager);
    virtual QIcon getIcon() const;
signals:
    void childrenLoaded(ServerTreeItem*);
    void channelsLoaded(GroupTreeItem*);

private:
    void emitChildrenLoaded();

private:
    XStreamAuthenticationInfo server;
    bool loadedChildren = false;
    QIcon unloadedIcon;
};
#endif
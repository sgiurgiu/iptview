#ifndef ABSTRACTCHANNELTREEITEM_H
#define ABSTRACTCHANNELTREEITEM_H

#include <QObject>
#include <QIcon>
#include <vector>
#include <QHash>

#include <shared_mutex>
#include <atomic>

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
    virtual void setCancelOgoingOperations(bool flag);
signals:
    void aquiredIcon(AbstractChannelTreeItem*);
protected:
    int64_t id = -1;
    std::vector<std::unique_ptr<AbstractChannelTreeItem>> children;
    AbstractChannelTreeItem* parent = nullptr;
    QIcon icon;
    QNetworkAccessManager* networkManager;
    mutable std::shared_mutex iconMutex;
    std::atomic_bool cancelOngoingOperations = {false};
};

#endif // ABSTRACTCHANNELTREEITEM_H

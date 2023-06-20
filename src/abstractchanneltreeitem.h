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
    explicit AbstractChannelTreeItem(AbstractChannelTreeItem* parent = nullptr);
    virtual ~AbstractChannelTreeItem() = default;
    virtual ChannelTreeItemType getType() const = 0;
    virtual QString getName() const = 0;
    virtual QIcon getIcon() const;
    virtual void setIcon(QIcon icon);
    virtual void appendChild(AbstractChannelTreeItem* child);
    virtual void removeChild(AbstractChannelTreeItem* child);
    virtual int childCount() const;
    virtual int row() const;
    virtual AbstractChannelTreeItem* child(int index) const;
    virtual AbstractChannelTreeItem* getParent() const;
    int64_t getID() const;
    void setID(int64_t id);
signals:
    void aquiredIcon(AbstractChannelTreeItem*);
protected:
    int64_t id = -1;
    std::vector<AbstractChannelTreeItem*> children;
    AbstractChannelTreeItem* parent = nullptr;
    QIcon icon;
    mutable std::shared_mutex iconMutex;
};

#endif // ABSTRACTCHANNELTREEITEM_H

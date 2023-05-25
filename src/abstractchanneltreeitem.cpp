#include "abstractchanneltreeitem.h"



AbstractChannelTreeItem::AbstractChannelTreeItem(QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent):
    QObject{parent},
    parent{parent},
    networkManager{networkManager}
{
}

void AbstractChannelTreeItem::appendChild(std::unique_ptr<AbstractChannelTreeItem> child)
{
    children.push_back(std::move(child));
}
int AbstractChannelTreeItem::childCount() const
{
    return children.size();
}
int AbstractChannelTreeItem::row() const
{
    if(parent)
    {
        for(size_t i=0; i<parent->children.size(); i++)
        {
            if(parent->children.at(i).get() == this)
            {
                return i;
            }
        }
        return -1;
    }
    return -1;
}
AbstractChannelTreeItem* AbstractChannelTreeItem::child(int index) const
{
    if(index < 0 || index >= static_cast<int>(children.size())) return nullptr;
    return children.at(index).get();
}
AbstractChannelTreeItem* AbstractChannelTreeItem::getParent() const
{
    return parent;
}
QIcon AbstractChannelTreeItem::getIcon() const
{
    std::shared_lock lock(iconMutex);
    return icon;
}
int64_t AbstractChannelTreeItem::getID() const
{
    return id;
}
void AbstractChannelTreeItem::setID(int64_t id)
{
    this->id = id;
}
QNetworkAccessManager* AbstractChannelTreeItem::getNetworkManager() const
{
    return networkManager;
}



#include "abstractchanneltreeitem.h"
#include <mutex>

AbstractChannelTreeItem::AbstractChannelTreeItem(AbstractChannelTreeItem* parent):
    QObject{parent},
    parent{parent}
{
}

void AbstractChannelTreeItem::appendChild(AbstractChannelTreeItem* child)
{
    child->setParent(this);
    child->parent = this;
    children.push_back(child);
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
            if(parent->children.at(i) == this)
            {
                return i;
            }
        }
        return -1;
    }
    return -1;
}
void AbstractChannelTreeItem::removeChild(AbstractChannelTreeItem* child)
{
    std::erase_if(children, [child](const auto& c){return c == child;});
}
AbstractChannelTreeItem* AbstractChannelTreeItem::child(int index) const
{
    if(index < 0 || index >= static_cast<int>(children.size())) return nullptr;
    return children.at(index);
}
AbstractChannelTreeItem* AbstractChannelTreeItem::getParent() const
{
    return parent;
}
void AbstractChannelTreeItem::setIcon(QIcon icon)
{
    std::unique_lock lock(iconMutex);
    this->icon = std::move(icon);
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


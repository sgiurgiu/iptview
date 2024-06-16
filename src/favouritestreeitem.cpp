#include "favouritestreeitem.h"

FavouritesTreeItem::FavouritesTreeItem(RootTreeItem* parent)
: GroupTreeItem{ "Favourites", parent }
{
    icon = QIcon(":/icons/star.png");
}

AbstractChannelTreeItem*
FavouritesTreeItem::removeFavouriteChild(AbstractChannelTreeItem* item)
{
    removeChild(item);
    if (children.empty())
    {
        return nullptr;
    }
    return children.back();
}
void FavouritesTreeItem::appendChild(AbstractChannelTreeItem* child)
{
    GroupTreeItem::appendChild(child);
    std::sort(children.begin(), children.end(),
              [](AbstractChannelTreeItem* first, AbstractChannelTreeItem* second)
              { return first->getName().compare(second->getName()) < 0; });
}
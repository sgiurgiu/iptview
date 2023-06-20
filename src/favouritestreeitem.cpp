#include "favouritestreeitem.h"


FavouritesTreeItem::FavouritesTreeItem(RootTreeItem* parent) : GroupTreeItem{"Favourites", parent}
{
    icon = QIcon(":/icons/star.png");
}

AbstractChannelTreeItem* FavouritesTreeItem::removeFavouriteChild(AbstractChannelTreeItem* item)
{
    removeChild(item);
    if(children.empty())
    {
        return nullptr;
    }
    return children.back();
}

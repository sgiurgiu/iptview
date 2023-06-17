#include "favouritestreeitem.h"


FavouritesTreeItem::FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent) : GroupTreeItem{"Favourites", networkManager, parent}
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
    return children.back().get();
}

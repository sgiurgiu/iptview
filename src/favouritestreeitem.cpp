#include "favouritestreeitem.h"


FavouritesTreeItem::FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent) : GroupTreeItem{"Favourites", networkManager, parent}
{
    icon = QIcon(":/icons/star.svg");
}

AbstractChannelTreeItem* FavouritesTreeItem::removeChild(AbstractChannelTreeItem* item)
{
    children.erase(std::remove_if(children.begin(), children.end(),[item](auto& child){
        return item == child.get();
    }), children.end());
    if(children.empty())
    {
        return nullptr;
    }
    return children.back().get();
}

#ifndef FAVOURITESTREEITEM_H
#define FAVOURITESTREEITEM_H

#include "grouptreeitem.h"

class RootTreeItem;

class FavouritesTreeItem : public GroupTreeItem
{
    Q_OBJECT
public:
    FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent);
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Favourite;
    }

    //returns the last child or nullptr if none
    AbstractChannelTreeItem* removeFavouriteChild(AbstractChannelTreeItem* item);
};

#endif // FAVOURITESTREEITEM_H

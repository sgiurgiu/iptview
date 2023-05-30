#include "channelswidget.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QDebug>

#include "channelsfilteringmodel.h"
#include "channelsmodel.h"
#include "abstractchanneltreeitem.h"

ChannelsWidget::ChannelsWidget(QWidget *parent)
    : QWidget{parent}
{
    channels = new QTreeView(this);
    model = new ChannelsModel(this);
    proxyModel = new ChannelsFilteringModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(ChannelsModel::NameRole);
    channels->setSelectionBehavior(QAbstractItemView::SelectItems);
    channels->setModel(proxyModel);
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search");

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(searchField, 0);
    layout->addWidget(channels, 1);
    setLayout(layout);

    connect(searchField, SIGNAL(textChanged(QString)), this, SLOT(searchTextChanged(QString)));
    connect(channels,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onDoubleClickedTreeItem(QModelIndex)));
    connect(channels->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this, SLOT(itemsSelectionChanged(QItemSelection,QItemSelection)));
    contextMenu = new QMenu(this);
    addToFavouritesAction = new QAction("Add to Favourites", this);
    removeFromFavouritesAction = new QAction("Remove from Favourites", this);
    channels->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(channels, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenu(QPoint)));
    connect(addToFavouritesAction, SIGNAL(triggered(bool)), this, SLOT(onAddToFavourites()));
    connect(removeFromFavouritesAction, SIGNAL(triggered(bool)), this, SLOT(onRemoveFromFavourites()));
}
void ChannelsWidget::searchTextChanged(const QString& text)
{
    proxyModel->setFilterWildcard(text);
}
void ChannelsWidget::ImportPlaylist(M3UList list)
{
    model->AddList(std::move(list));
}
void ChannelsWidget::onDoubleClickedTreeItem(const QModelIndex &index)
{
    if(index.isValid())
    {
        auto uri = index.data(ChannelsModel::ChannelRoles::UriRole);
        auto name = index.data(ChannelsModel::ChannelRoles::NameRole);
        if(uri.isValid())
        {
            emit playChannel(name.isValid()?name.toString():"", uri.toString());
        }
    }
}
void ChannelsWidget::itemsSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if(selected.isEmpty()) return;
    auto firstSelectedProxyIndex = selected.indexes().front();
    if(!firstSelectedProxyIndex.isValid())
    {
        return;
    }
    auto firstSelected = proxyModel->mapToSource(firstSelectedProxyIndex);
    if(!firstSelected.isValid())
    {
        return;
    }

    auto uri = firstSelected.data(ChannelsModel::ChannelRoles::UriRole);
    auto name = firstSelected.data(ChannelsModel::ChannelRoles::NameRole);
    if(uri.isValid())
    {
        emit selectChannel(name.isValid()?name.toString():"", uri.toString());
    }
}
void ChannelsWidget::onCustomContextMenu(const QPoint &point)
{
    QModelIndex proxyIndex = channels->indexAt(point);
    if(!proxyIndex.isValid())
    {
        return;
    }
    auto index = proxyModel->mapToSource(proxyIndex);
    if(!index.isValid())
    {
        return;
    }

    auto treeItem = static_cast<AbstractChannelTreeItem*>(index.internalPointer());
    if(treeItem && treeItem->getType() == ChannelTreeItemType::Channel)
    {
        contextMenu->clear();
        auto parentItem = treeItem->getParent();
        if(parentItem && parentItem->getType() == ChannelTreeItemType::Favourite)
        {
            removeFromFavouritesAction->setData(QVariant::fromValue(treeItem));
            contextMenu->addAction(removeFromFavouritesAction);
            contextMenu->exec(channels->viewport()->mapToGlobal(point));
        }
        else if(parentItem && (parentItem->getType() == ChannelTreeItemType::Group || parentItem->getType() == ChannelTreeItemType::Root))
        {
            addToFavouritesAction->setData(QVariant::fromValue(treeItem));
            contextMenu->addAction(addToFavouritesAction);
            contextMenu->exec(channels->viewport()->mapToGlobal(point));
        }
    }

}

void ChannelsWidget::onAddToFavourites()
{
    auto treeItem = addToFavouritesAction->data().value<AbstractChannelTreeItem*>();
    model->AddToFavourites(treeItem);
    proxyModel->invalidate();
}
void ChannelsWidget::onRemoveFromFavourites()
{
    auto treeItem = removeFromFavouritesAction->data().value<AbstractChannelTreeItem*>();
    model->RemoveFromFavourites(treeItem);
    proxyModel->invalidate();
}

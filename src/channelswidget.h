#ifndef CHANNELSWIDGET_H
#define CHANNELSWIDGET_H

#include <QWidget>
#include "m3ulist.h"

class QTreeView;
class ChannelsModel;
class QMenu;
class QAction;
class QItemSelection;
class QLineEdit;
class ChannelsFilteringModel;

class ChannelsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelsWidget(QWidget *parent = nullptr);
    void ImportPlaylist(M3UList list);
signals:
    void playChannel(const QString& name, const QString& uri);
    void selectChannel(const QString& name, const QString& uri);
private slots:
    void onDoubleClickedTreeItem(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &point);
    void onAddToFavourites();
    void onRemoveFromFavourites();
    void itemsSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void searchTextChanged(const QString& text);
private:
    QTreeView* channels;
    ChannelsModel* model;
    QMenu* contextMenu;
    QAction* addToFavouritesAction;
    QAction* removeFromFavouritesAction;
    QLineEdit* searchField;
    ChannelsFilteringModel* proxyModel;
};

#endif // CHANNELSWIDGET_H

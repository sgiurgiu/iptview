#ifndef CHANNELSWIDGET_H
#define CHANNELSWIDGET_H

#include <QWidget>
#include "m3ulist.h"
#include "xstreaminfo.h"
#include <atomic>

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
    void ImportPlaylist(CollectedInfo list);
    M3UList GetM3UList() const;
signals:
    void cancelImportChannels();
    void playChannel(int64_t id);
    void selectChannel(int64_t id);
    void updateImportedChannelIndex(qint64);
    void channelsImported();
    void enableSkipForward(bool);
    void enableSkipBack(bool);

public slots:
    void SkipForward();
    void SkipBack();
private slots:
    void onDoubleClickedTreeItem(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &point);
    void onAddToFavourites();
    void onRemoveFromFavourites();
    void itemsSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void searchTextChanged(const QString& text);
    void onAddNewChannel();
    void onRemoveChannelGroup();
    void onAddNewChannelGroup();
private:

private:
    QTreeView* channels;
    ChannelsModel* model;
    QMenu* contextMenu;
    QAction* addToFavouritesAction;
    QAction* removeFromFavouritesAction;
    QAction* addNewChannelAction;
    QAction* addNewChannelGroupAction;
    QAction* removeChannelGroupAction;
    QLineEdit* searchField;
    ChannelsFilteringModel* proxyModel;
};

#endif // CHANNELSWIDGET_H

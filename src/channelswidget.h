#ifndef CHANNELSWIDGET_H
#define CHANNELSWIDGET_H

#include "m3ulist.h"
#include "xstreaminfo.h"
#include <QTabWidget>
#include <atomic>

class QTreeView;
class ChannelsModel;
class QMenu;
class QAction;
class QItemSelection;
class QLineEdit;
class ChannelsFilteringModel;
class QNetworkAccessManager;
class XStreamChannelsModel;
class ChannelTreeItem;

class ChannelsWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit ChannelsWidget(QNetworkAccessManager* networkManager,
                            QWidget* parent = nullptr);
    void ImportPlaylist(M3UList list);
    void ImportPlaylist(XStreamCollectedInfo list);
    M3UList GetM3UList() const;
signals:
    void cancelImportChannels();
    void playChannel(int64_t id);
    void playChannel(ChannelTreeItem*);
    void selectChannel(int64_t id);
    void selectChannel(ChannelTreeItem*);
    void updateImportedChannelIndex(qint64);
    void channelsImported();
    void enableSkipForward(bool);
    void enableSkipBack(bool);

public slots:
    void SkipForward();
    void SkipBack();
private slots:
    void onDoubleClickedXStreamTreeItem(const QModelIndex& index);
    void onDoubleClickedTreeItem(const QModelIndex& index);
    void onCustomContextMenu(const QPoint& point);
    void onAddToFavourites();
    void onRemoveFromFavourites();
    void itemsSelectionChanged(const QItemSelection& selected,
                               const QItemSelection& deselected);
    void xstreamItemsSelectionChanged(const QItemSelection& selected,
                                      const QItemSelection& deselected);
    void searchTextChanged(const QString& text);
    void onAddNewChannel();
    void onRemoveChannelGroup();
    void onAddNewChannelGroup();

private:
    void skipForwardLocalChannels();
    void skipForwardXStreamChannels();
    void skipBackLocalChannels();
    void skipBackXStreamChannels();
    void skipLocalChannels(int amount);
    void skipXStreamChannels(int amount);

private:
    QTreeView* localChannels;
    ChannelsModel* localChannelsModel;
    QMenu* localChannelsContextMenu;
    QAction* addToFavouritesAction;
    QAction* removeFromFavouritesAction;
    QAction* addNewChannelAction;
    QAction* addNewChannelGroupAction;
    QAction* removeChannelGroupAction;
    QLineEdit* localChannelsSearchField;
    ChannelsFilteringModel* localChannelsProxyModel;
    QTreeView* xstreamChannels;
    XStreamChannelsModel* xstreamChannelModel;
    ChannelsFilteringModel* xstreamChannelsProxyModel;
    QLineEdit* xstreamChannelsSearchField;
    QNetworkAccessManager* networkManager;
};

#endif // CHANNELSWIDGET_H

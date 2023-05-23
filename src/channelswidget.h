#ifndef CHANNELSWIDGET_H
#define CHANNELSWIDGET_H

#include <QWidget>
#include "m3ulist.h"

class QTreeView;
class ChannelsModel;

class ChannelsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelsWidget(QWidget *parent = nullptr);
    void ImportPlaylist(M3UList list);
signals:
    void playChannel(QString uri);
private slots:
    void onDoubleClickedTreeItem(const QModelIndex &index);
private:
    QTreeView* channels;
    ChannelsModel* model;
};

#endif // CHANNELSWIDGET_H

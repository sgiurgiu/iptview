#include "channelswidget.h"

#include <QTreeView>
#include <QVBoxLayout>

#include "channelsmodel.h"

ChannelsWidget::ChannelsWidget(QWidget *parent)
    : QWidget{parent}
{
    channels = new QTreeView(this);
    model = new ChannelsModel(this);
    channels->setSelectionBehavior(QAbstractItemView::SelectItems);

    channels->setModel(model);
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(channels, 1);
    setLayout(layout);

    connect(channels,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(onDoubleClickedTreeItem(const QModelIndex &)));
}
void ChannelsWidget::ImportPlaylist(M3UList list)
{
    model->AddList(std::move(list));
}
void ChannelsWidget::onDoubleClickedTreeItem(const QModelIndex &index)
{
    if(index.isValid())
    {
        auto data = index.data(ChannelsModel::ChannelRoles::UriRole);
        if(data.isValid())
        {
            emit playChannel(data.toString());
        }
    }
}

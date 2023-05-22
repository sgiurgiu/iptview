#include "channelswidget.h"

#include <QTreeView>

ChannelsWidget::ChannelsWidget(QWidget *parent)
    : QWidget{parent}
{
    channels = new QTreeView(this);

}

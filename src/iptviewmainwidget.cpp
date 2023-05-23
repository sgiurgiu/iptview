#include "iptviewmainwidget.h"

#include "channelswidget.h"
#include "mediawidget.h"

IPTViewMainWidget::IPTViewMainWidget(QWidget *parent): QSplitter{Qt::Horizontal, parent}
{
    channelsWidget = new ChannelsWidget(this);

    mediaWidget = new MediaWidget(this);

    addWidget(channelsWidget);
    addWidget(mediaWidget);

    QSizePolicy mediaExpandingPolicy{QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding};
    mediaExpandingPolicy.setHorizontalStretch(1);
    mediaExpandingPolicy.setVerticalStretch(1);
    mediaWidget->setSizePolicy(mediaExpandingPolicy);

    QSizePolicy channelsExpandingPolicy{QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding};
    channelsExpandingPolicy.setHorizontalStretch(0);
    channelsExpandingPolicy.setVerticalStretch(1);
    channelsWidget->setSizePolicy(channelsExpandingPolicy);

    connect(channelsWidget, SIGNAL(playChannel(QString)), mediaWidget, SLOT(PlayChannel(QString)));
}

void IPTViewMainWidget::ImportPlaylist(M3UList list)
{
    channelsWidget->ImportPlaylist(std::move(list));
}

#include "iptviewmainwidget.h"

#include "channelswidget.h"
#include "mediawidget.h"

IPTViewMainWidget::IPTViewMainWidget(QWidget *parent): QSplitter{Qt::Horizontal, parent}
{
    channelsWidget = new ChannelsWidget(this);

    mediaWidget = new MediaWidget(this);
    connect(mediaWidget, SIGNAL(showingFullScreen(bool)), this, SLOT(fullScreen(bool)));
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

    connect(channelsWidget, SIGNAL(playChannel(QString,QString)), mediaWidget, SLOT(PlayChannel(QString,QString)));
    connect(channelsWidget, SIGNAL(selectChannel(QString,QString)), mediaWidget, SLOT(SelectChannel(QString,QString)));
}

void IPTViewMainWidget::ImportPlaylist(M3UList list)
{
    channelsWidget->ImportPlaylist(std::move(list));
}

void IPTViewMainWidget::fullScreen(bool flag)
{
    emit showingFullScreen(flag);
    if(flag)
    {
        contentMargins = this->contentsMargins();
        this->setContentsMargins(0,0,0,0);
        channelsWidget->hide();
    }
    else
    {
        channelsWidget->show();
        this->setContentsMargins(contentMargins);
    }
}

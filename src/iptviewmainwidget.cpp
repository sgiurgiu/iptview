#include "iptviewmainwidget.h"

#include "channelswidget.h"
#include "mediawidget.h"

#ifdef IPTVIEW_DBUS
#include "mprisdbus.h"
#endif

IPTViewMainWidget::IPTViewMainWidget(QWidget *parent): QSplitter{Qt::Horizontal, parent}
{
    channelsWidget = new ChannelsWidget(this);

    mediaWidget = new MediaWidget(this);
    connect(mediaWidget, SIGNAL(showingFullScreen(bool)), this, SLOT(fullScreen(bool)));
    connect(mediaWidget, SIGNAL(skipForward()), channelsWidget, SLOT(SkipForward()));
    connect(mediaWidget, SIGNAL(skipBack()), channelsWidget, SLOT(SkipBack()));

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

    connect(channelsWidget, SIGNAL(playChannel(int64_t)), mediaWidget, SLOT(PlayChannel(int64_t)));
    connect(channelsWidget, SIGNAL(selectChannel(int64_t)), mediaWidget, SLOT(SelectChannel(int64_t)));
    connect(channelsWidget, SIGNAL(updateImportedChannelIndex(qint64)),this, SIGNAL(updateImportedChannelIndex(qint64)));
    connect(channelsWidget, SIGNAL(channelsImported()),this, SIGNAL(channelsImported()));
    connect(channelsWidget, SIGNAL(enableSkipForward(bool)), mediaWidget, SLOT(EnableSkipForward(bool)));
    connect(channelsWidget, SIGNAL(enableSkipBack(bool)), mediaWidget, SLOT(EnableSkipBack(bool)));
    connect(this, SIGNAL(cancelImportChannels()), channelsWidget, SIGNAL(cancelImportChannels()));
#ifdef IPTVIEW_DBUS
    setupMprisDBus();
#endif

}

void IPTViewMainWidget::ImportPlaylist(M3UList list)
{
    channelsWidget->ImportPlaylist(std::move(list));
}

void IPTViewMainWidget::ImportPlaylist(CollectedInfo list)
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

void IPTViewMainWidget::SkipForward()
{
    channelsWidget->SkipForward();
}
void IPTViewMainWidget::SkipBack()
{
    channelsWidget->SkipBack();
}

#ifdef IPTVIEW_DBUS
void IPTViewMainWidget::setupMprisDBus()
{
    dbus = new MPRISDBus(this);
    dbus->setObjectName("MPRISDBus");
    dbus->SetInitialVolume(mediaWidget->GetVolume());
    connect(mediaWidget, SIGNAL(showingFullScreen(bool)), dbus, SLOT(SetFullscreen(bool)));
    connect(mediaWidget, SIGNAL(playingTrack(int64_t)), dbus, SLOT(PlayingChannel(int64_t)));    
    connect(mediaWidget, SIGNAL(volumeToggledSignal(bool)), dbus, SLOT(VolumeToggledExternal(bool)));
    connect(mediaWidget, SIGNAL(volumeChangedSignal(int)), dbus, SLOT(VolumeChangedExternal(int)));

    connect(dbus, SIGNAL(skipForward()), channelsWidget, SLOT(SkipForward()));
    connect(dbus, SIGNAL(skipBack()), channelsWidget, SLOT(SkipBack()));
    connect(channelsWidget, SIGNAL(selectChannel(int64_t)), dbus, SLOT(SelectedChannel(int64_t)));

    connect(channelsWidget, SIGNAL(enableSkipForward(bool)), dbus, SLOT(EnableSkipForward(bool)));
    connect(channelsWidget, SIGNAL(enableSkipBack(bool)), dbus, SLOT(EnableSkipBack(bool)));


    connect(dbus, SIGNAL(playSelectedChannel()), mediaWidget, SLOT(PlaySelected()));
    connect(dbus, SIGNAL(pausePlayingSelectedChannel()), mediaWidget, SLOT(Pause()));
    connect(dbus, SIGNAL(stopPlayingSelectedChannel()), mediaWidget, SLOT(Stop()));
    connect(dbus, SIGNAL(playPauseSelectedChannel()), mediaWidget, SLOT(PlayPause()));
    connect(dbus, SIGNAL(volumeChanged(int)), mediaWidget, SLOT(VolumeChanged(int)));
    //connect(dbus, SIGNAL(volumeToggledSignal(bool)), mediaWidget, SLOT(VolumeToggled(bool)));
}
#endif

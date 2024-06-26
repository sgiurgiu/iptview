#include "mediawidget.h"
#include <QAction>
#include <QActionGroup>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "database.h"
#include "databaseprovider.h"
#include "epgwidget.h"
#include "mpvwidget.h"

namespace
{
constexpr int VOLUME_OVERLAY_ID = 0;
// TODO: make it a preference
constexpr int MAX_FILE_LOAD_RETRY_TIMES = 10;
} // namespace

MediaWidget::MediaWidget(QNetworkAccessManager* networkManager, QWidget* parent)
: QWidget{ parent }
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mpvWidget = new MpvWidget(this);
    connect(mpvWidget, SIGNAL(wheelScrolled(QPoint)), this,
            SLOT(mediaWheelEvent(QPoint)));
    connect(mpvWidget, SIGNAL(fileLoaded()), this, SLOT(fileLoaded()));
    connect(mpvWidget, SIGNAL(doubleClicked()), this, SLOT(mpvDoubleClicked()));
    connect(mpvWidget, SIGNAL(mouseMoved(QMouseEvent*)), this,
            SLOT(mpvMouseMoved(QMouseEvent*)));

    connect(mpvWidget, SIGNAL(fileLoadingError(QString)), this,
            SLOT(fileLoadingError(QString)));
    connect(mpvWidget, SIGNAL(fileUnknownFormatError(QString)), this,
            SLOT(fileUnknownFormatError(QString)));
    connect(mpvWidget, SIGNAL(unsupportedSystemError(QString)), this,
            SLOT(unsupportedSystemError(QString)));
    connect(mpvWidget, SIGNAL(outputInitializationError(QString)), this,
            SLOT(outputInitializationError(QString)));

    layout->addWidget(mpvWidget, 1);

    volumeOsdTimer = new QTimer(this);
    volumeOsdTimer->setSingleShot(true);
    volumeOsdTimer->setInterval(1000);
    connect(volumeOsdTimer, SIGNAL(timeout()), this,
            SLOT(volumeOsdTimerTimeout()));

    cursorBlankTimer = new QTimer(this);
    cursorBlankTimer->setSingleShot(true);
    cursorBlankTimer->setInterval(1000);
    connect(cursorBlankTimer, SIGNAL(timeout()), this,
            SLOT(cursorBlankTimerTimeout()));

    controlsWidget = createControlsWidget(networkManager);
    layout->addWidget(controlsWidget, 0);

    setLayout(layout);
}

QWidget* MediaWidget::createControlsWidget(QNetworkAccessManager* networkManager)
{
    QSettings settings;
    stopAction = new QAction(stopIcon, "", this);
    stopAction->setCheckable(false);
    stopAction->setEnabled(false);
    stopAction->setToolTip("Stop");
    connect(stopAction, SIGNAL(triggered(bool)), this, SLOT(Stop()));

    playPauseAction = new QAction(playIcon, "", this);
    playPauseAction->setCheckable(false);
    playPauseAction->setEnabled(false);
    playPauseAction->setShortcut(
        QKeySequence{ settings.value("player/pause", Qt::Key_Space).toInt() });
    connect(playPauseAction, SIGNAL(triggered(bool)), this, SLOT(PlayPause()));

    skipForwardAction = new QAction(playSkipForwardIcon, "", this);
    skipForwardAction->setCheckable(false);
    skipForwardAction->setEnabled(false);
    skipForwardAction->setToolTip("Next Channel");
    connect(skipForwardAction, SIGNAL(triggered(bool)), this,
            SLOT(skipForwardTriggered()));

    skipBackAction = new QAction(playSkipBackIcon, "", this);
    skipBackAction->setCheckable(false);
    skipBackAction->setEnabled(false);
    skipBackAction->setToolTip("Previous Channel");
    connect(skipBackAction, SIGNAL(triggered(bool)), this,
            SLOT(skipBackTriggered()));

    volumeAction = new QAction(volumeMediumIcon, "", this);
    volumeAction->setCheckable(true);
    volumeAction->setEnabled(true);
    volumeAction->setObjectName("Volume Mute toggle");
    volumeAction->setShortcut(
        QKeySequence{ settings.value("player/mute", Qt::Key_M).toInt() });
    connect(volumeAction, SIGNAL(toggled(bool)), this, SLOT(VolumeToggled(bool)));
    connect(volumeAction, SIGNAL(toggled(bool)), this,
            SIGNAL(volumeToggledSignal(bool)));

    fullScreenAction = new QAction(fullScreenIcon, "", this);
    fullScreenAction->setCheckable(true);
    fullScreenAction->setEnabled(false);
    fullScreenAction->setToolTip("Full Screen");
    fullScreenAction->setShortcuts(
        QList<QKeySequence>()
        << QKeySequence{ settings.value("player/fullscreen", Qt::Key_F).toInt() }
        << QKeySequence::FullScreen);
    connect(fullScreenAction, SIGNAL(triggered(bool)), this,
            SLOT(fullScreenActionToggled(bool)));

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(150);
    volumeSlider->setSingleStep(5);
    volumeSlider->setObjectName("Volume Slider");
    volumeSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(volumeSlider, SIGNAL(valueChanged(int)), this,
            SLOT(VolumeChanged(int)));
    connect(volumeSlider, SIGNAL(valueChanged(int)), this,
            SIGNAL(volumeChangedSignal(int)));
    volumeSlider->setValue(settings.value("player/volume", 100.0).toDouble());
    mpvWidget->setProperty("volume",
                           settings.value("player/volume", 100.0).toDouble());

    subtitlesChoicesButton = new QToolButton(this);
    subtitlesChoicesButton->setIcon(
        QIcon(":/icons/chatbox-ellipses-outline.png"));
    subtitlesChoicesButton->setPopupMode(QToolButton::MenuButtonPopup);
    subtitlesChoicesButton->setEnabled(false);
    subtitlesChoicesButton->setCheckable(true);
    subtitlesChoicesButton->setChecked(false);
    subtitlesChoicesButton->setToolTip("Subtitles");
    connect(subtitlesChoicesButton, SIGNAL(clicked(bool)), this,
            SLOT(subtitlesToggled(bool)));
    subtitlesChoicesActionGroup = new QActionGroup(this);
    subtitlesMenu = new QMenu(this);
    subtitlesChoicesButton->setMenu(subtitlesMenu);

    mediaTitleLabel = new QLabel(this);
    mediaTitleLabel->setMargin(10);

    epgWidget = new EPGWidget(networkManager, this);

    QToolBar* widget = new QToolBar(this);
    widget->addAction(skipBackAction);
    widget->addAction(playPauseAction);
    widget->addAction(stopAction);
    widget->addAction(skipForwardAction);
    widget->addWidget(subtitlesChoicesButton);
    widget->addAction(fullScreenAction);
    widget->addWidget(mediaTitleLabel);
    widget->addWidget(epgWidget);

    QWidget* empty = new QWidget(this);
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    widget->addWidget(empty);

    widget->addAction(volumeAction);
    widget->addWidget(volumeSlider);

    widget->setFloatable(false);
    widget->setOrientation(Qt::Orientation::Horizontal);
    return widget;
}
int MediaWidget::GetVolume() const
{
    return volumeSlider->value();
}
void MediaWidget::PlayPause()
{
    if (stopped && selectedChannel)
    {
        playChannel(std::move(selectedChannel));
        return;
    }
    auto pausedVariant = mpvWidget->getProperty("pause");
    bool paused = pausedVariant.toBool();
    qInfo() << "paused property is:" << paused;
    playPauseAction->setToolTip(paused ? "Pause" : "Play");
    playPauseAction->setIcon(paused ? pauseIcon : playIcon);
    mpvWidget->setProperty("pause", QVariant{ !paused });
}

void MediaWidget::Stop()
{
    mpvWidget->command(QStringList() << "stop");
    mpvWidget->stopRenderingMedia();
    stopped = true;
    playPauseAction->setIcon(playIcon);
    playPauseAction->setToolTip("Play");
    toggleSystemSleep();
}
void MediaWidget::skipBackTriggered()
{
    emit skipBack();
}
void MediaWidget::skipForwardTriggered()
{
    emit skipForward();
}
void MediaWidget::playChannel(std::unique_ptr<ChannelTreeItem> channel)
{
    if (!channel)
        return;
    selectedChannel = std::move(channel);
    subtitles.clear();
    epgWidget->ClearChannel();
    mpvWidget->command(QStringList() << "stop");
    mpvWidget->stopRenderingMedia();
    //    mpvWidget->command(QStringList() << "apply-profile" << "gpu-hq");
    mpvWidget->command(QStringList() << "loadfile" << selectedChannel->getUri());
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(pauseIcon);
    playPauseAction->setToolTip("Pause");
    stopAction->setEnabled(true);
    fullScreenAction->setEnabled(true);
    mpvWidget->setProperty("pause", QVariant{ false });
    mpvWidget->setProperty("sid", QVariant{ "no" });
    mpvWidget->setProperty("loop-playlist", QVariant{ "inf" });
    mediaTitleLabel->setStyleSheet("");
    mediaTitleLabel->setText(
        QString("Loading %1 ...").arg(selectedChannel->getName()));

    stopped = false;
    toggleSystemSleep();
}
void MediaWidget::PlayChannel(int64_t id)
{
    fileLoadRetryTimes = 0;
    playChannel(std::unique_ptr<ChannelTreeItem>{
        DatabaseProvider::GetDatabase()->GetChannel(id) });
}
void MediaWidget::PlayChannel(ChannelTreeItem* channel)
{
    fileLoadRetryTimes = 0;
    playChannel(std::unique_ptr<ChannelTreeItem>{ channel->clone(nullptr) });
}
void MediaWidget::SelectChannel(int64_t id)
{
    if (selectedChannel && !stopped)
        return;
    auto channel = DatabaseProvider::GetDatabase()->GetChannel(id);
    if (!channel)
        return;
    selectedChannel.reset(channel);
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(playIcon);
    playPauseAction->setToolTip("Play");
    mediaTitleLabel->setText(selectedChannel->getName());
}
void MediaWidget::SelectChannel(ChannelTreeItem* channel)
{
    if (!channel)
        return;
    if (selectedChannel && !stopped)
        return;
    selectedChannel.reset(channel->clone(nullptr));
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(playIcon);
    playPauseAction->setToolTip("Play");
    mediaTitleLabel->setText(selectedChannel->getName());
}

void MediaWidget::VolumeToggled(bool checked)
{
    mpvWidget->setProperty("mute", QVariant{ checked });
    volumeAction->setIcon(checked ? volumeMuteIcon : getVolumeIcon());
    QVariantMap map;
    map["name"] = "osd-overlay";
    map["id"] = VOLUME_OVERLAY_ID;
    map["format"] = "ass-events";
    map["data"] = QString(R"({\an9\fs36}Mute %1)").arg(checked ? "on" : "off");
    map["res_x"] = width();
    map["res_y"] = height();
    mpvWidget->command(map);
    volumeOsdTimer->start();
}
QIcon MediaWidget::getVolumeIcon()
{
    int volume = volumeSlider->value();
    if (volume < 10)
    {
        return volumeOffIcon;
    }
    else if (volume < 50)
    {
        return volumeLowIcon;
    }
    else if (volume < 85)
    {
        return volumeMediumIcon;
    }
    else
    {
        return volumeHighIcon;
    }
}
void MediaWidget::VolumeChanged(int volume)
{
    qDebug() << "volume changed:" << volume;
    if (sender() != volumeSlider)
    {
        QSignalBlocker volumeActionBlocker(volumeAction);
        QSignalBlocker volumeSliderBlocker(volumeSlider);
        volumeAction->setChecked(false);
        volumeAction->setIcon(getVolumeIcon());
        volumeSlider->setSliderPosition(volume);
    }
    double vol = static_cast<double>(volume);
    mpvWidget->setProperty("volume", QVariant{ vol });
    volumeAction->setIcon(getVolumeIcon());
    volumeAction->setChecked(false);
    QSettings settings;
    settings.setValue("player/volume", vol);
    if (!selectedChannel)
        return;

    QVariantMap map;
    map["name"] = "osd-overlay";
    map["id"] = VOLUME_OVERLAY_ID;
    map["format"] = "ass-events";
    map["data"] = QString(R"({\an9\fs36}Volume %1)").arg(volume);
    map["res_x"] = width();
    map["res_y"] = height();
    mpvWidget->command(map);
    volumeOsdTimer->start();
}
void MediaWidget::volumeOsdTimerTimeout()
{
    QVariantMap map;
    map["name"] = "osd-overlay";
    map["id"] = VOLUME_OVERLAY_ID;
    map["format"] = "none";
    map["data"] = "";
    mpvWidget->command(map);
}

void MediaWidget::mediaWheelEvent(QPoint delta)
{
    int volume = volumeSlider->value();
    int step = volumeSlider->singleStep() * (delta.y() < 0 ? -1 : 1);
    volumeSlider->setValue(volume + step);
}

void MediaWidget::fileLoaded()
{
    mediaTitleLabel->setText(selectedChannel->getName());
    fileLoadRetryTimes = 0;
    mpvWidget->startRenderingMedia();
    subtitles.clear();
    int tracksCount = mpvWidget->getProperty("track-list/count").toInt();
    qDebug() << "track count " << tracksCount;
    Subtitle offSub = { "no", "Off", "" };
    subtitles.append(std::move(offSub));
    for (int i = 0; i < tracksCount; i++)
    {
        QString type =
            mpvWidget->getProperty(QString("track-list/%1/type").arg(i)).toString();
        QString id =
            mpvWidget->getProperty(QString("track-list/%1/id").arg(i)).toString();
        QString title =
            mpvWidget->getProperty(QString("track-list/%1/title").arg(i)).toString();
        QString lang =
            mpvWidget->getProperty(QString("track-list/%1/lang").arg(i)).toString();
        qDebug() << "track " << i << " type " << type << ", id " << id
                 << ", title " << title << ", lang " << lang;
        if (type == "sub")
        {
            if (title.isEmpty())
            {
                title = QString("Subtitle %1").arg(id);
            }
            if (!lang.isEmpty())
            {
                title.append(" (" + lang + ")");
            }
            Subtitle sub = { id, title, lang };
            subtitles.append(std::move(sub));
        }
    }
    setupSubtitlesMenu();

    epgWidget->SetChannel(selectedChannel->clone(nullptr));

    emit playingTrack(selectedChannel.get());
}

void MediaWidget::setupSubtitlesMenu()
{
    subtitlesMenu->clear();
    auto actions = subtitlesChoicesActionGroup->actions();
    for (auto action : actions)
    {
        subtitlesChoicesActionGroup->removeAction(action);
        delete action;
    }

    for (const auto& sub : subtitles)
    {
        auto action = subtitlesChoicesActionGroup->addAction(sub.title);
        action->setData(sub.id);
        action->setCheckable(true);
        connect(action, SIGNAL(triggered(bool)), this,
                SLOT(subtitleChanged(bool)));
    }
    if (!subtitles.empty())
    {
        setSubtitle(subtitles.front().id);
        subtitlesChoicesActionGroup->actions().constFirst()->setChecked(true);
    }

    subtitlesMenu->addActions(subtitlesChoicesActionGroup->actions());
    subtitlesChoicesButton->setEnabled(subtitles.size() > 1);
    subtitlesChoicesButton->setChecked(false);
}
void MediaWidget::setSubtitle(const QString& id)
{
    mpvWidget->setProperty("sid", id);
}
void MediaWidget::subtitlesToggled(bool toggled)
{
    // the first action is Off.
    // and if we're toggled, we select the first subtitle, if it's available
    auto actions = subtitlesChoicesActionGroup->actions();
    auto actionIndex = toggled ? 1 : 0;
    if (actionIndex >= actions.size())
        return;

    actions.at(actionIndex)->setChecked(true);
    setSubtitle(actions.at(actionIndex)->data().toString());
}
void MediaWidget::subtitleChanged(bool checked)
{
    if (!checked)
        return;
    auto checkedAction = subtitlesChoicesActionGroup->checkedAction();
    if (checkedAction)
    {
        setSubtitle(checkedAction->data().toString());
        auto index =
            subtitlesChoicesActionGroup->actions().indexOf(checkedAction);
        subtitlesChoicesButton->setChecked(index != 0);
    }
}
void MediaWidget::toggleSystemSleep()
{
#ifdef Q_OS_LINUX
    const int MAX_SERVICES = 2;

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (bus.isConnected())
    {
        QString services[MAX_SERVICES] = { "org.freedesktop.ScreenSaver",
                                           "org.gnome.SessionManager" };
        QString paths[MAX_SERVICES] = { "/org/freedesktop/ScreenSaver",
                                        "/org/gnome/SessionManager" };

        static uint cookies[2];

        for (int i = 0; i < MAX_SERVICES; i++)
        {
            QDBusInterface screenSaverInterface(services[i], paths[i],
                                                services[i], bus);

            if (!screenSaverInterface.isValid())
                continue;

            QDBusReply<uint> reply;

            if (!stopped)
            {
                reply = screenSaverInterface.call("Inhibit", "iptview",
                                                  "playing video");
            }
            else
            {
                reply = screenSaverInterface.call("UnInhibit", cookies[i]);
            }

            if (reply.isValid())
            {
                cookies[i] = reply.value();

                // qDebug()<<"succesful: " << reply;
            }
            else
            {
                // QDBusError error =reply.error();
                // qDebug()<<error.message()<<error.name();
            }
        }
    }

#elif defined Q_OS_WIN

    EXECUTION_STATE result;

    if (!stopped)
        result = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED |
                                         ES_DISPLAY_REQUIRED);
    else
        result = SetThreadExecutionState(ES_CONTINUOUS);

    if (result == NULL)
        qDebug() << "EXECUTION_STATE failed";

#endif
}
void MediaWidget::toggleFullScreen()
{
    if (stopped)
        return;
    fullScreen = !fullScreen;
    emit showingFullScreen(fullScreen);
    if (fullScreen)
    {
        cursorBlankTimer->start();
        contentMargins = this->contentsMargins();
        this->setContentsMargins(0, 0, 0, 0);
        controlsWidget->hide();
        window()->showFullScreen();
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        cursorBlankTimer->stop();
        this->setContentsMargins(contentMargins);
        controlsWidget->show();
        window()->showNormal();
    }
    fullScreenAction->setChecked(fullScreen);
}
void MediaWidget::mpvDoubleClicked()
{
    toggleFullScreen();
}
void MediaWidget::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "MediaWidget::keyPressEvent:" << event->key();
    bool matchesFullScreenAction = false;
    for (const auto& shortcut : fullScreenAction->shortcuts())
    {
        matchesFullScreenAction |=
            (shortcut == (event->key() | event->modifiers()));
    }
    bool matchesPlayPauseAction = false;
    for (const auto& shortcut : playPauseAction->shortcuts())
    {
        matchesPlayPauseAction |=
            (shortcut == (event->key() | event->modifiers()));
    }

    if (fullScreen)
    {
        if (event->matches(QKeySequence::Cancel) || matchesFullScreenAction)
        {
            toggleFullScreen();
            event->accept();
        }
        else if (matchesPlayPauseAction)
        {
            PlayPause();
            event->accept();
        }
    }

    QWidget::keyPressEvent(event);
}
void MediaWidget::fullScreenActionToggled(bool)
{
    toggleFullScreen();
}

void MediaWidget::fileLoadingError(QString message)
{
    QString errorMessage = QString("%1. Retrying (attempt %2 of %3)... ")
                               .arg(message)
                               .arg(fileLoadRetryTimes + 1)
                               .arg(MAX_FILE_LOAD_RETRY_TIMES);
    if (fileLoadRetryTimes >= MAX_FILE_LOAD_RETRY_TIMES || stopped)
    {
        errorMessage = QString("%1. No more retries ").arg(message);
    }

    mediaTitleLabel->setText(errorMessage);
    mediaTitleLabel->setStyleSheet("QLabel { color : red; }");

    if (fileLoadRetryTimes < MAX_FILE_LOAD_RETRY_TIMES && !stopped)
    {
        QTimer::singleShot(2000, this,
                           [this]()
                           {
                               playChannel(std::move(selectedChannel));
                               ++fileLoadRetryTimes;
                           });
    }
}
void MediaWidget::fileUnknownFormatError(QString message)
{
    mediaTitleLabel->setText(message);
    mediaTitleLabel->setStyleSheet("QLabel { color : red; }");
}
void MediaWidget::unsupportedSystemError(QString message)
{
    mediaTitleLabel->setText(message);
    mediaTitleLabel->setStyleSheet("QLabel { color : red; }");
}
void MediaWidget::outputInitializationError(QString message)
{
    mediaTitleLabel->setText(message);
    mediaTitleLabel->setStyleSheet("QLabel { color : red; }");
}
void MediaWidget::EnableSkipForward(bool flag)
{
    skipForwardAction->setEnabled(flag);
}
void MediaWidget::EnableSkipBack(bool flag)
{
    skipBackAction->setEnabled(flag);
}

void MediaWidget::Pause()
{
}
void MediaWidget::PlaySelected()
{
}
void MediaWidget::mpvMouseMoved(QMouseEvent* event)
{
    if (fullScreen)
    {
        setCursor(Qt::ArrowCursor);
        cursorBlankTimer->start();
    }
}

void MediaWidget::cursorBlankTimerTimeout()
{
    setCursor(Qt::BlankCursor);
}
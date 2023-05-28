#include "mediawidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QDebug>
#include <QTimer>
#include <QToolButton>
#include <QMenu>
#include <QActionGroup>
#include <QLabel>
#include <QSettings>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include "mpvwidget.h"

namespace
{
    constexpr int VOLUME_OVERLAY_ID = 0;
}

MediaWidget::MediaWidget(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    mpvWidget = new MpvWidget(this);
    connect(mpvWidget,SIGNAL(wheelScrolled(QPoint)), this, SLOT(mediaWheelEvent(QPoint)));
    connect(mpvWidget,SIGNAL(fileLoaded()), this, SLOT(fileLoaded()));
    layout->addWidget(mpvWidget, 1);

    volumeOsdTimer = new QTimer(this);
    volumeOsdTimer->setSingleShot(true);
    volumeOsdTimer->setInterval(1000);
    connect(volumeOsdTimer, SIGNAL(timeout()), this, SLOT(volumeOsdTimerTimeout()));

    auto controlsWidget = createControlsWidget();
    layout->addWidget(controlsWidget, 0);

    setLayout(layout);
}

QWidget* MediaWidget::createControlsWidget()
{
    stopAction = new QAction(stopIcon, "", this);
    stopAction->setCheckable(false);
    stopAction->setEnabled(false);
    connect(stopAction, SIGNAL(triggered(bool)), this, SLOT(stopTriggered()));

    playPauseAction = new QAction(playIcon, "", this);
    playPauseAction->setCheckable(false);
    playPauseAction->setEnabled(false);
    connect(playPauseAction, SIGNAL(triggered(bool)), this, SLOT(playPauseTriggered()));

    skipForwardAction = new QAction(playSkipForwardIcon, "", this);
    skipForwardAction->setCheckable(false);
    skipForwardAction->setEnabled(false);
    connect(skipForwardAction, SIGNAL(triggered(bool)), this, SLOT(skipForwardTriggered()));

    skipBackAction = new QAction(playSkipBackIcon, "", this);
    skipBackAction->setCheckable(false);
    skipBackAction->setEnabled(false);
    connect(skipBackAction, SIGNAL(triggered(bool)), this, SLOT(skipBackTriggered()));

    volumeAction = new QAction(volumeMediumIcon, "", this);
    volumeAction->setCheckable(true);
    volumeAction->setEnabled(true);
    connect(volumeAction, SIGNAL(toggled(bool)), this, SLOT(volumeToggled(bool)));

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(150);    
    volumeSlider->setSingleStep(5);
    volumeSlider->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    connect(volumeSlider,SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));
    QSettings settings;
    volumeSlider->setValue(settings.value("player/volume", 100.0).toDouble());

    subtitlesChoicesButton = new QToolButton(this);
    subtitlesChoicesButton->setIcon(QIcon(":/icons/text.svg"));
    subtitlesChoicesButton->setPopupMode(QToolButton::MenuButtonPopup);
    subtitlesChoicesButton->setEnabled(false);
    subtitlesChoicesActionGroup = new QActionGroup(this);
    subtitlesMenu = new QMenu(this);
    subtitlesChoicesButton->setMenu(subtitlesMenu);

    mediaTitleLabel = new QLabel(this);
    mediaTitleLabel->setMargin(10);
    QToolBar* widget = new QToolBar(this);
    widget->addAction(skipBackAction);
    widget->addAction(playPauseAction);
    widget->addAction(stopAction);
    widget->addAction(skipForwardAction);
    widget->addWidget(subtitlesChoicesButton);
    widget->addWidget(mediaTitleLabel);

    QWidget* empty = new QWidget(this);
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    widget->addWidget(empty);

    widget->addAction(volumeAction);
    widget->addWidget(volumeSlider);


    widget->setFloatable(false);
    widget->setOrientation(Qt::Orientation::Horizontal);
    return widget;
}
void MediaWidget::playPauseTriggered()
{
    if(stopped && !selectedUri.isEmpty())
    {
        PlayChannel(selectedName, selectedUri);
        return;
    }
    auto pausedVariant = mpvWidget->getProperty("pause");
    bool paused = pausedVariant.toBool();
    qInfo() << "paused property is:"<<paused;
    playPauseAction->setToolTip(paused ? "Pause" : "Play");
    playPauseAction->setIcon(paused ? pauseIcon : playIcon);
    mpvWidget->setProperty("pause", QVariant{!paused});
}

void MediaWidget::stopTriggered()
{
    mpvWidget->command(QStringList() << "stop");
    stopped = true;
    playPauseAction->setIcon(playIcon);
    playPauseAction->setToolTip("Play");
    toggleSystemSleep();
}
void MediaWidget::skipBackTriggered()
{

}
void MediaWidget::skipForwardTriggered()
{

}

void MediaWidget::PlayChannel(const QString& name, const QString& uri)
{
    if(uri.isEmpty()) return;
    subtitles.clear();
    selectedUri = uri;
    selectedName = name;
    mpvWidget->command(QStringList() << "loadfile" << uri);
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(pauseIcon);
    playPauseAction->setToolTip("Pause");
    stopAction->setEnabled(true);
    mpvWidget->setProperty("pause", QVariant{false});
    mpvWidget->setProperty("sid", QVariant{"no"});
    mediaTitleLabel->setText(name);
    stopped = false;
    toggleSystemSleep();
}
void MediaWidget::SelectChannel(const QString& name, const QString& uri)
{
    if(uri.isEmpty() || !stopped) return;

    selectedUri = uri;
    selectedName = name;
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(playIcon);
    playPauseAction->setToolTip("Play");
    mediaTitleLabel->setText(name);
}
void MediaWidget::volumeToggled(bool checked)
{
    mpvWidget->setProperty("mute", QVariant{checked});
    volumeAction->setIcon(checked ? volumeMuteIcon: getVolumeIcon());
    QVariantMap map;
    map["name"] = "osd-overlay";
    map["id"] = VOLUME_OVERLAY_ID;
    map["format"] = "ass-events";
    map["data"] = QString(R"({\an9\fs36}Mute %1)").arg(checked?"on":"off");
    map["res_x"] = width();
    map["res_y"] = height();
    mpvWidget->command(map);
    volumeOsdTimer->start();
}
QIcon MediaWidget::getVolumeIcon()
{
    int volume = volumeSlider->value();
    if(volume < 10)
    {
        return volumeOffIcon;
    }
    else if(volume < 50)
    {
        return volumeLowIcon;
    }
    else if(volume < 85)
    {
        return volumeMediumIcon;
    }
    else
    {
        return volumeHighIcon;
    }
}
void MediaWidget::volumeChanged(int volume)
{
    double vol = static_cast<double>(volume);
    mpvWidget->setProperty("volume", QVariant{vol});
    volumeAction->setIcon(getVolumeIcon());
    volumeAction->setChecked(false);
    QSettings settings;
    settings.setValue("player/volume", vol);
    if(selectedUri.isEmpty()) return;

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
    volumeSlider->setValue(volume+step);
}

void MediaWidget::fileLoaded()
{
    subtitles.clear();
    int tracksCount = mpvWidget->getProperty("track-list/count").toInt();
    qDebug() << "track count "<<tracksCount;
    Subtitle offSub = {"no", "Off", ""};
    subtitles.append(std::move(offSub));
    for(int i=0;i<tracksCount;i++)
    {
        QString type = mpvWidget->getProperty(QString("track-list/%1/type").arg(i)).toString();
        QString id = mpvWidget->getProperty(QString("track-list/%1/id").arg(i)).toString();
        QString title = mpvWidget->getProperty(QString("track-list/%1/title").arg(i)).toString();
        QString lang = mpvWidget->getProperty(QString("track-list/%1/lang").arg(i)).toString();
        qDebug() << "track "<<i<<" type "<<type<<", id "<<id << ", title "<<title <<", lang "<<lang;
        if(type == "sub")
        {
            if(title.isEmpty())
            {
                title = QString("Subtitle %1").arg(id);
            }
            if(!lang.isEmpty())
            {
                title.append(" ("+lang+")");
            }
            Subtitle sub = {id, title, lang};
            subtitles.append(std::move(sub));
        }
    }
    setupSubtitlesMenu();
}
void MediaWidget::setupSubtitlesMenu()
{
    subtitlesMenu->clear();
    auto actions = subtitlesChoicesActionGroup->actions();
    for(auto action : actions)
    {
        subtitlesChoicesActionGroup->removeAction(action);
        delete action;
    }

    for(const auto& sub : subtitles)
    {
        auto action = subtitlesChoicesActionGroup->addAction(sub.title);
        action->setData(sub.id);
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(subtitleChanged(bool)));
    }
    if(!subtitles.empty())
    {
        setSubtitle(subtitles.front().id);
        subtitlesChoicesActionGroup->actions().constFirst()->setChecked(true);
    }

    subtitlesMenu->addActions(subtitlesChoicesActionGroup->actions());
    subtitlesChoicesButton->setEnabled(subtitles.size() > 1);
}
void MediaWidget::setSubtitle(const QString& id)
{
    mpvWidget->setProperty("sid", id);
}
void MediaWidget::subtitleChanged(bool checked)
{
    if(!checked) return;
    auto checkedAction = subtitlesChoicesActionGroup->checkedAction();
    if(checkedAction)
    {
        setSubtitle(checkedAction->data().toString());
    }
}
void MediaWidget::toggleSystemSleep()
{
#ifdef Q_OS_LINUX
    const int MAX_SERVICES = 2;

    QDBusConnection bus = QDBusConnection::sessionBus();
    if(bus.isConnected())
    {
        QString services[MAX_SERVICES] =
        {
            "org.freedesktop.ScreenSaver",
            "org.gnome.SessionManager"
        };
        QString paths[MAX_SERVICES] =
        {
            "/org/freedesktop/ScreenSaver",
            "/org/gnome/SessionManager"
        };

        static uint cookies[2];

        for(int i = 0; i < MAX_SERVICES ; i++)
        {
            QDBusInterface screenSaverInterface( services[i], paths[i],services[i], bus);

            if (!screenSaverInterface.isValid())
                continue;

            QDBusReply<uint> reply;

            if(!stopped)
            {
                reply = screenSaverInterface.call("Inhibit", "iptview", "playing video");
            }
            else
            {
                reply  = screenSaverInterface.call("UnInhibit", cookies[i]);
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

#elif defined  Q_OS_WIN

    EXECUTION_STATE result;

    if(!stopped)
        result = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
    else
        result = SetThreadExecutionState(ES_CONTINUOUS);

    if(result == NULL)
        qDebug() << "EXECUTION_STATE failed";

#endif
}


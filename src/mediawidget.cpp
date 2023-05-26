#include "mediawidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QDebug>
#include <QTimer>

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
    layout->addWidget(mpvWidget, 1);

    auto controlsWidget = createControlsWidget();
    layout->addWidget(controlsWidget, 0);

    setLayout(layout);

    volumeOsdTimer = new QTimer(this);
    volumeOsdTimer->setSingleShot(true);
    volumeOsdTimer->setInterval(1000);
    connect(volumeOsdTimer, SIGNAL(timeout()), this, SLOT(volumeOsdTimerTimeout()));
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

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(150);
    volumeSlider->setValue(100);
    volumeSlider->setSingleStep(5);
    connect(volumeSlider,SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));

    volumeAction = new QAction(volumeMediumIcon, "", this);
    volumeAction->setCheckable(true);
    volumeAction->setEnabled(true);
    connect(volumeAction, SIGNAL(toggled(bool)), this, SLOT(volumeToggled(bool)));

    QToolBar* widget = new QToolBar(this);
    widget->addAction(skipBackAction);
    widget->addAction(playPauseAction);
    widget->addAction(stopAction);
    widget->addAction(skipForwardAction);

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
        PlayChannel(selectedUri);
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
}
void MediaWidget::skipBackTriggered()
{

}
void MediaWidget::skipForwardTriggered()
{

}

void MediaWidget::PlayChannel(QString uri)
{
    if(uri.isEmpty()) return;

    selectedUri = uri;
    mpvWidget->command(QStringList() << "loadfile" << uri);
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(pauseIcon);
    playPauseAction->setToolTip("Pause");
    stopAction->setEnabled(true);
    mpvWidget->setProperty("pause", QVariant{false});
    stopped = false;
}
void MediaWidget::SelectChannel(QString uri)
{
    if(uri.isEmpty() || !stopped) return;

    selectedUri = uri;
    playPauseAction->setEnabled(true);
    playPauseAction->setIcon(playIcon);
    playPauseAction->setToolTip("Play");

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
    mpvWidget->setProperty("volume", QVariant{(double)volume});
    QVariantMap map;
    map["name"] = "osd-overlay";
    map["id"] = VOLUME_OVERLAY_ID;
    map["format"] = "ass-events";
    map["data"] = QString(R"({\an9\fs36}Volume %1)").arg(volume);
    map["res_x"] = width();
    map["res_y"] = height();
    mpvWidget->command(map);
    volumeAction->setIcon(getVolumeIcon());
    volumeAction->setChecked(false);
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

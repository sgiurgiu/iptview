#include "mediawidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QIcon>
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
    stopAction = new QAction(QIcon(":/icons/stop.svg"), "", this);
    stopAction->setCheckable(false);
    stopAction->setEnabled(false);
    connect(stopAction, SIGNAL(triggered(bool)), this, SLOT(stopTriggered()));

    playPauseAction = new QAction(QIcon(":/icons/play.svg"), "", this);
    playPauseAction->setCheckable(false);
    playPauseAction->setEnabled(false);
    connect(playPauseAction, SIGNAL(triggered(bool)), this, SLOT(playPauseTriggered()));

    skipForwardAction = new QAction(QIcon(":/icons/play-skip-forward.svg"), "", this);
    skipForwardAction->setCheckable(false);
    skipForwardAction->setEnabled(false);
    connect(skipForwardAction, SIGNAL(triggered(bool)), this, SLOT(skipForwardTriggered()));

    skipBackAction = new QAction(QIcon(":/icons/play-skip-back.svg"), "", this);
    skipBackAction->setCheckable(false);
    skipBackAction->setEnabled(false);
    connect(skipBackAction, SIGNAL(triggered(bool)), this, SLOT(skipBackTriggered()));

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(150);
    volumeSlider->setValue(100);
    volumeSlider->setSingleStep(5);
    connect(volumeSlider,SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));

    QToolBar* widget = new QToolBar(this);
    widget->addAction(skipBackAction);
    widget->addAction(playPauseAction);
    widget->addAction(stopAction);
    widget->addAction(skipForwardAction);

    widget->addSeparator();

    widget->addWidget(volumeSlider);

    widget->addSeparator();

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
    playPauseAction->setIcon(paused ? QIcon(":/icons/pause.svg") : QIcon(":/icons/play.svg"));
    mpvWidget->setProperty("pause", QVariant{!paused});
}

void MediaWidget::stopTriggered()
{
    mpvWidget->command(QStringList() << "stop");
    stopped = true;
    playPauseAction->setIcon(QIcon(":/icons/play.svg"));
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
    playPauseAction->setIcon(QIcon(":/icons/pause.svg"));
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
    playPauseAction->setIcon(QIcon(":/icons/play.svg"));
    playPauseAction->setToolTip("Play");

}

void MediaWidget::volumeChanged(int volume)
{
    mpvWidget->setProperty("volume", QVariant{(double)volume});
    QVariantMap map;
    map["name"] = "osd-overlay";
    map["id"] = VOLUME_OVERLAY_ID;
    map["format"] = "ass-events";
    map["data"] = QString("Volume %1").arg(volume);
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

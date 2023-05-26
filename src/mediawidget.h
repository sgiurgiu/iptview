#ifndef MEDIAWIDGET_H
#define MEDIAWIDGET_H

#include <QWidget>
#include <QIcon>

class MpvWidget;
class QAction;
class QSlider;
class QTimer;
class QToolButton;
class QMenu;

class MediaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MediaWidget(QWidget *parent = nullptr);
public slots:
    void PlayChannel(QString uri);
    void SelectChannel(QString uri);
private slots:
    void playPauseTriggered();
    void stopTriggered();
    void skipBackTriggered();
    void skipForwardTriggered();
    void volumeChanged(int);
    void volumeOsdTimerTimeout();
    void volumeToggled(bool);
    void mediaWheelEvent(QPoint delta);
    void fileLoaded();
    void subtitleChanged(bool);
private:
    QWidget* createControlsWidget();
    QIcon getVolumeIcon();
    void setupSubtitlesMenu();
    void setSubtitle(const QString& id);
private:
    struct Subtitle
    {
        QString id;
        QString title;
        QString lang;
    };
    MpvWidget* mpvWidget;
    QAction* playPauseAction;
    QAction* stopAction;
    QAction* skipForwardAction;
    QAction* skipBackAction;
    QSlider* volumeSlider;
    QAction* volumeAction;
    QString selectedUri;
    bool stopped = true;
    QTimer* volumeOsdTimer;
    QIcon stopIcon = QIcon{":/icons/stop.svg"};
    QIcon playIcon = QIcon{":/icons/play.svg"};
    QIcon pauseIcon = QIcon{":/icons/pause.svg"};
    QIcon playSkipForwardIcon = QIcon{":/icons/play-skip-forward.svg"};
    QIcon playSkipBackIcon = QIcon{":/icons/play-skip-back.svg"};
    QIcon volumeMuteIcon = QIcon{":/icons/volume-mute.svg"};
    QIcon volumeOffIcon = QIcon{":/icons/volume-off.svg"};
    QIcon volumeLowIcon = QIcon{":/icons/volume-low.svg"};
    QIcon volumeMediumIcon = QIcon{":/icons/volume-medium.svg"};
    QIcon volumeHighIcon = QIcon{":/icons/volume-high.svg"};
    QList<Subtitle> subtitles;
    QToolButton* subtitlesChoicesButton;
    QActionGroup* subtitlesChoicesActionGroup;
    QMenu* subtitlesMenu;
};

#endif // MEDIAWIDGET_H

#ifndef MEDIAWIDGET_H
#define MEDIAWIDGET_H

#include "channeltreeitem.h"
#include <QIcon>
#include <QWidget>
#include <memory>

class MpvWidget;
class QAction;
class QSlider;
class QToolButton;
class QMenu;
class QLabel;
class QNetworkAccessManager;
class EPGWidget;

class MediaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MediaWidget(QNetworkAccessManager* networkManager,
                         QWidget* parent = nullptr);
    int GetVolume() const;
public slots:
    void PlayChannel(int64_t);
    void PlayChannel(ChannelTreeItem*);
    void SelectChannel(int64_t);
    void SelectChannel(ChannelTreeItem*);
    void EnableSkipForward(bool);
    void EnableSkipBack(bool);
    void Stop();
    void PlayPause();
    void Pause();
    void PlaySelected();
    void VolumeChanged(int);
    void VolumeToggled(bool);
signals:
    void showingFullScreen(bool);
    void skipBack();
    void skipForward();
    void playingTrack(ChannelTreeItem*);
    void volumeChangedSignal(int);
    void volumeToggledSignal(bool);
private slots:

    void skipBackTriggered();
    void skipForwardTriggered();
    void volumeOsdTimerTimeout();
    void cursorBlankTimerTimeout();
    void mediaWheelEvent(QPoint delta);
    void fileLoaded();
    void subtitleChanged(bool);
    void subtitlesToggled(bool);
    void mpvDoubleClicked();
    void mpvMouseMoved(QMouseEvent*);
    void fullScreenActionToggled(bool);

    void fileLoadingError(QString message);
    void fileUnknownFormatError(QString message);
    void unsupportedSystemError(QString message);
    void outputInitializationError(QString message);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    QWidget* createControlsWidget(QNetworkAccessManager* networkManager);
    QIcon getVolumeIcon();
    void setupSubtitlesMenu();
    void setSubtitle(const QString& id);
    void toggleSystemSleep();
    void toggleFullScreen();
    void playChannel(std::unique_ptr<ChannelTreeItem> channel);
    void playChannel(ChannelTreeItem* channel);

private:
    struct Subtitle
    {
        QString id;
        QString title;
        QString lang;
    };
    MpvWidget* mpvWidget = nullptr;
    QAction* playPauseAction = nullptr;
    QAction* stopAction = nullptr;
    QAction* skipForwardAction = nullptr;
    QAction* skipBackAction = nullptr;
    QSlider* volumeSlider = nullptr;
    QAction* volumeAction = nullptr;
    QAction* fullScreenAction = nullptr;
    std::unique_ptr<ChannelTreeItem> selectedChannel = { nullptr };
    bool stopped = true;
    QTimer* volumeOsdTimer = nullptr;
    QIcon stopIcon = QIcon{ ":/icons/stop.png" };
    QIcon playIcon = QIcon{ ":/icons/play.png" };
    QIcon pauseIcon = QIcon{ ":/icons/pause.png" };
    QIcon playSkipForwardIcon = QIcon{ ":/icons/play-skip-forward.png" };
    QIcon playSkipBackIcon = QIcon{ ":/icons/play-skip-back.png" };
    QIcon volumeMuteIcon = QIcon{ ":/icons/volume-mute.png" };
    QIcon volumeOffIcon = QIcon{ ":/icons/volume-off.png" };
    QIcon volumeLowIcon = QIcon{ ":/icons/volume-low.png" };
    QIcon volumeMediumIcon = QIcon{ ":/icons/volume-medium.png" };
    QIcon volumeHighIcon = QIcon{ ":/icons/volume-high.png" };
    QIcon fullScreenIcon = QIcon{ ":/icons/scan.png" };
    QList<Subtitle> subtitles;
    QToolButton* subtitlesChoicesButton = nullptr;
    QActionGroup* subtitlesChoicesActionGroup = nullptr;
    QMenu* subtitlesMenu = nullptr;
    QLabel* mediaTitleLabel = nullptr;
    EPGWidget* epgWidget = nullptr;
    bool fullScreen = false;
    QWidget* controlsWidget = nullptr;
    QMargins contentMargins;
    int fileLoadRetryTimes = 0;
    QTimer* cursorBlankTimer = nullptr;
};

#endif // MEDIAWIDGET_H

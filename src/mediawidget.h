#ifndef MEDIAWIDGET_H
#define MEDIAWIDGET_H

#include <QWidget>

class MpvWidget;
class QAction;
class QSlider;
class QTimer;

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
private:
    QWidget* createControlsWidget();

private:
    MpvWidget* mpvWidget;
    QAction* playPauseAction;
    QAction* stopAction;
    QAction* skipForwardAction;
    QAction* skipBackAction;
    QSlider* volumeSlider;
    QString selectedUri;
    bool stopped = true;
    QTimer* volumeOsdTimer;
};

#endif // MEDIAWIDGET_H

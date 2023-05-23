#ifndef MEDIAWIDGET_H
#define MEDIAWIDGET_H

#include <QWidget>

class MpvWidget;
class QPushButton;

class MediaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MediaWidget(QWidget *parent = nullptr);
public slots:
    void PlayChannel(QString uri);
private slots:
    void play();
private:
    MpvWidget* mpvWidget;
    QPushButton* playButton;

};

#endif // MEDIAWIDGET_H

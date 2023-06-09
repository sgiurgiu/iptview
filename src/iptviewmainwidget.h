#ifndef IPTVIEWMAINWIDGET_H
#define IPTVIEWMAINWIDGET_H

#include <QSplitter>

#include "m3ulist.h"

class ChannelsWidget;
class MediaWidget;

class IPTViewMainWidget : public QSplitter
{
    Q_OBJECT
public:
    IPTViewMainWidget(QWidget *parent = nullptr);
    void ImportPlaylist(M3UList list);
signals:
    void showingFullScreen(bool);
private slots:
    void fullScreen(bool flag);
private:
    ChannelsWidget* channelsWidget;
    MediaWidget* mediaWidget;
    QMargins contentMargins;
};

#endif // IPTVIEWMAINWIDGET_H

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

public slots:
    void ImportPlaylist(M3UList list);
    void CancelImportChannels();
signals:
    void showingFullScreen(bool);
    void updateImportedChannelIndex(qint64);
    void channelsImported();
private slots:
    void fullScreen(bool flag);
private:
    ChannelsWidget* channelsWidget;
    MediaWidget* mediaWidget;
    QMargins contentMargins;
};

#endif // IPTVIEWMAINWIDGET_H

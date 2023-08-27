#ifndef IPTVIEWMAINWIDGET_H
#define IPTVIEWMAINWIDGET_H

#include <QSplitter>

#include "m3ulist.h"
#include "xstreaminfo.h"

class ChannelsWidget;
class MediaWidget;
class QNetworkAccessManager;

#ifdef IPTVIEW_DBUS
class MPRISDBus;
#endif
class IPTViewMainWidget : public QSplitter
{
    Q_OBJECT
public:
    IPTViewMainWidget(QNetworkAccessManager* networkManager,QWidget *parent = nullptr);
    M3UList GetM3UList() const;
public slots:
    void ImportPlaylist(M3UList list);
    void ImportPlaylist(CollectedInfo list);
    void SkipForward();
    void SkipBack();

signals:
    void cancelImportChannels();
    void showingFullScreen(bool);
    void updateImportedChannelIndex(qint64);
    void channelsImported();
private slots:
    void fullScreen(bool flag);
private:
#ifdef IPTVIEW_DBUS
  void setupMprisDBus();
#endif

private:
    ChannelsWidget* channelsWidget;
    MediaWidget* mediaWidget;
    QMargins contentMargins;
    QNetworkAccessManager* networkManager;
#ifdef IPTVIEW_DBUS
  MPRISDBus* dbus = nullptr;
#endif

};

#endif // IPTVIEWMAINWIDGET_H

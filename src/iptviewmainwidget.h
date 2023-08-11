#ifndef IPTVIEWMAINWIDGET_H
#define IPTVIEWMAINWIDGET_H

#include <QSplitter>

#include "m3ulist.h"
#include "xstreaminfo.h"

class ChannelsWidget;
class MediaWidget;
#ifdef IPTVIEW_DBUS
class MPRISDBus;
#endif
class IPTViewMainWidget : public QSplitter
{
    Q_OBJECT
public:
    IPTViewMainWidget(QWidget *parent = nullptr);

public slots:
    void ImportPlaylist(M3UList list);
    void ImportPlaylist(CollectedInfo list);
    void CancelImportChannels();
    void SkipForward();
    void SkipBack();

signals:
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
#ifdef IPTVIEW_DBUS
  MPRISDBus* dbus = nullptr;
#endif

};

#endif // IPTVIEWMAINWIDGET_H

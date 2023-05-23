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

private:
    ChannelsWidget* channelsWidget;
    MediaWidget* mediaWidget;
};

#endif // IPTVIEWMAINWIDGET_H

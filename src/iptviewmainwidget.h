#ifndef IPTVIEWMAINWIDGET_H
#define IPTVIEWMAINWIDGET_H

#include <QSplitter>

class ChannelsWidget;
class MediaWidget;

class IPTViewMainWidget : public QSplitter
{
    Q_OBJECT
public:
    IPTViewMainWidget(QWidget *parent = nullptr);
private:
    ChannelsWidget* channelsWidget;
    MediaWidget* mediaWidget;
};

#endif // IPTVIEWMAINWIDGET_H

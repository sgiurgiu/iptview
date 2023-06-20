#ifndef LOADINGCHANNELICONSWORKER_H
#define LOADINGCHANNELICONSWORKER_H

#include <QObject>
class ChannelTreeItem;
class QNetworkAccessManager;

class LoadingChannelIconsWorker : public QObject
{
    Q_OBJECT
public:
    explicit LoadingChannelIconsWorker(QObject *parent = nullptr);

signals:
    void channelIconReady(ChannelTreeItem* channel);
public slots:
    void loadChannelIcon(ChannelTreeItem* channel);
private:
    QNetworkAccessManager* networkManager;
};

#endif // LOADINGCHANNELICONSWORKER_H

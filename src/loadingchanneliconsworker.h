#ifndef LOADINGCHANNELICONSWORKER_H
#define LOADINGCHANNELICONSWORKER_H

#include <QObject>
#include <atomic>
class ChannelTreeItem;
class QNetworkAccessManager;

class LoadingChannelIconsWorker : public QObject
{
    Q_OBJECT
public:
    explicit LoadingChannelIconsWorker(QObject *parent = nullptr);
    void CancelIconsLoading()
    {
        cancelled = true;
    }
signals:
    void channelIconReady(ChannelTreeItem* channel);
public slots:
    void loadChannelIcon(ChannelTreeItem* channel);
private:
    QNetworkAccessManager* networkManager;
    std::atomic_bool cancelled = {false};
};

#endif // LOADINGCHANNELICONSWORKER_H

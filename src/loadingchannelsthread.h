#ifndef LOADINGCHANNELSTHREAD_H
#define LOADINGCHANNELSTHREAD_H

#include <QThread>
#include <atomic>
#include <memory>
#include "grouptreeitem.h"

class QNetworkAccessManager;
class Database;

class LoadingChannelsThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadingChannelsThread(QThread* uiThread, QObject *parent = nullptr);
    ~LoadingChannelsThread();
    void cancelOperation();
signals:
    void groupsCount(int count);
    void groupLoaded(GroupTreeItem* group);
    void favouriteChannels(std::vector<ChannelTreeItem*>);
protected:
    virtual void run() override;
private:

private:
    std::atomic_bool cancelled = false;
    QThread* uiThread = nullptr;
};

#endif // LOADINGCHANNELSTHREAD_H

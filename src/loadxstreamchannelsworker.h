#ifndef LOADXSTREAMCHANNELSWORKER_H
#define LOADXSTREAMCHANNELSWORKER_H

#include <QObject>
#include "xstreaminfo.h"
#include <atomic>
class QNetworkAccessManager;
class GroupTreeItem;
class QThread;

class LoadXstreamChannelsWorker : public QObject
{
    Q_OBJECT
public:
    explicit LoadXstreamChannelsWorker(XStreamCollectedInfo list, QObject *parent = nullptr);
    void setDestinationThread(QThread *thread)
    {
        this->destinationThread = thread;
    }
public slots:
    void importChannels();
    void cancelImportChannels()
    {
        cancelled = false;
        emit finished();
    }
signals:
    void finished();
    void loadedGroup(GroupTreeItem *);

private:
    void loadGroup(const XStreamCategoryInfo &category,
                   const QString &action);

private:
    QNetworkAccessManager *networkManager;
    qsizetype totalCategories;
    XStreamCollectedInfo list;
    qsizetype processedCategories = 0;
    QThread *destinationThread = nullptr;
    std::atomic_bool cancelled = {false};
};

#endif // LOADXSTREAMCHANNELSWORKER_H

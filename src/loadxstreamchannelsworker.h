#ifndef LOADXSTREAMCHANNELSWORKER_H
#define LOADXSTREAMCHANNELSWORKER_H

#include <QObject>
#include "xstreaminfo.h"

class QNetworkAccessManager;
class GroupTreeItem;
class QThread;

class LoadXstreamChannelsWorker : public QObject
{
    Q_OBJECT
public:
    explicit LoadXstreamChannelsWorker(CollectedInfo list, QObject *parent = nullptr);
    void setDestinationThread(QThread* thread)
    {
        this->destinationThread = thread;
    }
public slots:
    void importChannels();
signals:
    void finished();
    void loadedGroup(GroupTreeItem*);
private:
    void loadGroup(const CategoryInfo& category,
                   const QString& action);
private:
    QNetworkAccessManager* networkManager;
    qsizetype totalCategories;
    CollectedInfo list;
    qsizetype processedCategories = 0;
    QThread* destinationThread = nullptr;
};

#endif // LOADXSTREAMCHANNELSWORKER_H

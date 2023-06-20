#include "loadingchannelsthread.h"

#include "databaseprovider.h"
#include "database.h"
#include "channeltreeitem.h"
#include "grouptreeitem.h"
#include <QtDebug>

LoadingChannelsThread::LoadingChannelsThread(QThread* uiThread, QObject *parent)
    : QThread{parent}, uiThread{uiThread}
{

}
LoadingChannelsThread::~LoadingChannelsThread()
{
    cancelled = true;
    wait();
}
void LoadingChannelsThread::cancelOperation()
{
    cancelled = true;
}

void LoadingChannelsThread::run()
{
    auto db = DatabaseProvider::GetDatabase();
    auto channels = db->GetFavouriteChannels();

    for(auto channel :channels)
    {
        channel->moveToThread(uiThread);
    }

    emit favouriteChannels(channels);
    emit groupsCount(db->GetGroupsCount());
    auto groups = db->GetAllGroups();
    for(auto& group:groups)
    {
        if(cancelled) break;
        db->LoadChannels(group);
        group->moveToThread(uiThread);
        emit groupLoaded(group);
    }
}


#include "loadxstreamchannelsworker.h"

#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include "grouptreeitem.h"
#include "channeltreeitem.h"
#include "databaseprovider.h"
#include "database.h"

LoadXstreamChannelsWorker::LoadXstreamChannelsWorker(CollectedInfo list, QObject *parent)
    : QObject{parent},networkManager{new QNetworkAccessManager{this}},
      totalCategories{list.liveCategories.size() + list.vodCategories.size()},
      list{std::move(list)}
{

}

void LoadXstreamChannelsWorker::importChannels()
{
    for(const auto& category: list.liveCategories)
    {
        if(cancelled) return;
        loadGroup(category,"get_live_streams");
    }
    for(const auto& category: list.vodCategories)
    {
        if(cancelled) return;
        loadGroup(category,"get_vod_streams");
    }
}

void LoadXstreamChannelsWorker::loadGroup(const CategoryInfo& category,
               const QString& action)
{
    int64_t xstreamServerId = 0;
    {
        auto db = DatabaseProvider::GetDatabase();
        xstreamServerId = db->AddRetrieveXStreamServer(list.authInfo);
    }
    QNetworkRequest request;
    QUrl url;
    url.setHost(list.authInfo.serverUrl);
    url.setScheme(list.authInfo.serverSchema);
    url.setPort(list.authInfo.serverPort.toInt());
    url.setPath("/player_api.php");
    QUrlQuery query;
    query.addQueryItem("username",list.authInfo.username);
    query.addQueryItem("password",list.authInfo.password);
    query.addQueryItem("action",action);
    query.addQueryItem("category_id",category.categoryId);
    url.setQuery(query);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    auto reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this,
            [reply,
            category, info=list.authInfo, xstreamServerId, this]()
    {
        if(cancelled) return;
        if(reply->error())
        {
            reply->deleteLater();
            return;
        }
        auto response = reply->readAll();
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        auto channels = doc.array();

        auto group = std::make_unique<GroupTreeItem>(category.categoryName);
        for(const auto& ch: channels)
        {
            if(cancelled) return;
            auto channelObject = ch.toObject();
            auto streamId = channelObject.value("stream_id").toInt();
            auto categoryType = channelObject.value("stream_type").toString("");
            QUrl streamUrl;
            streamUrl.setHost(info.serverUrl);
            streamUrl.setScheme(info.serverSchema);
            streamUrl.setPort(info.serverPort.toInt());
            streamUrl.setPath(QString("/%1/%2/%3/%4.ts").arg(categoryType,info.username,info.password).arg(streamId));
            QUrl epgUrl;
            epgUrl.setHost(info.serverUrl);
            epgUrl.setScheme(info.serverSchema);
            epgUrl.setPort(info.serverPort.toInt());
            epgUrl.setPath("/player_api.php");
            QUrlQuery epgQuery;
            epgQuery.addQueryItem("username",list.authInfo.username);
            epgQuery.addQueryItem("password",list.authInfo.password);
            epgQuery.addQueryItem("action","get_short_epg");
            epgQuery.addQueryItem("stream_id",QString::number(streamId));
            epgUrl.setQuery(epgQuery);

            if(streamId == 0) continue;
            ChannelTreeItem* channelTreeItem = new ChannelTreeItem(
                        channelObject.value("name").toString(""),
                        streamUrl.toString(),
                        channelObject.value("stream_icon").toString(""),
                        {},
                        group.get()
                        );
            if(categoryType == "live")
            {
                channelTreeItem->setEpgChannelId(channelObject.value("epg_channel_id").toString(""));
                channelTreeItem->setEpgChannelUri(epgUrl.toString());
            }
            channelTreeItem->setXStreamServerId(xstreamServerId);

            group->addChannel(channelTreeItem);
        }
        auto db = DatabaseProvider::GetDatabase();
        db->AddGroup(group.get());
        group->moveToThread(destinationThread);
        emit loadedGroup(group.release());
        ++processedCategories;
        if(processedCategories >= totalCategories)
        {
            emit finished();
        }
    });

}

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
        loadGroup(category,"get_live_streams");
    }
    for(const auto& category: list.vodCategories)
    {
        loadGroup(category,"get_vod_streams");
    }
}

void LoadXstreamChannelsWorker::loadGroup(const CategoryInfo& category,
               const QString& action)
{
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
            category, info=list.authInfo, this]()
    {
        if(reply->error())
        {
            reply->deleteLater();
            return;
        }
        auto response = reply->readAll();
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        auto channels = doc.array();

        GroupTreeItem* group = new GroupTreeItem(category.categoryName);
        for(const auto& ch: channels)
        {
            auto channelObject = ch.toObject();
            auto streamId = channelObject.value("stream_id").toInt();
            auto categoryType = channelObject.value("stream_type").toString("");
            QUrl url;
            url.setHost(info.serverUrl);
            url.setScheme(info.serverSchema);
            url.setPort(info.serverPort.toInt());
            url.setPath(QString("/%1/%2/%3/%4.ts").arg(categoryType,info.username,info.password).arg(streamId));

            if(streamId == 0) continue;
            ChannelTreeItem* channelTreeItem = new ChannelTreeItem(
                        channelObject.value("name").toString(""),
                        url.toString(),
                        channelObject.value("stream_icon").toString(""),
                        {},
                        group
                        );
            group->addChannel(channelTreeItem);
        }
        auto db = DatabaseProvider::GetDatabase();
        db->AddGroup(group);
        group->moveToThread(destinationThread);
        emit loadedGroup(group);
        ++processedCategories;
        if(processedCategories >= totalCategories)
        {
            emit finished();
        }
    });

}

#include "servertreeitem.h"

#include "grouptreeitem.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <QColor>
#include <QGraphicsColorizeEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>

ServerTreeItem::ServerTreeItem(XStreamAuthenticationInfo server,
                               AbstractChannelTreeItem* parent)
: AbstractChannelTreeItem(parent), server{ std::move(server) }
{
    icon = QIcon(":/icons/server.png");
    {
        auto sizes = icon.availableSizes();

        for (const auto& size : sizes)
        {
            // prepare graphics scene and pixmap
            QGraphicsScene scene;
            QGraphicsPixmapItem item;
            auto pixmap = icon.pixmap(size);
            item.setPixmap(pixmap);

            QGraphicsColorizeEffect effect;
            effect.setColor(QColor(Qt::GlobalColor::lightGray));
            effect.setStrength(0.7);
            item.setGraphicsEffect(&effect);
            scene.addItem(&item);
            QPainter ptr(&pixmap);
            scene.render(&ptr, QRectF(),
                         QRectF(0, 0, size.width(), size.height()));
            unloadedIcon.addPixmap(pixmap);
        }
    }

    appendChild(new LoadingTreeItem(this));
}
void ServerTreeItem::emitChildrenLoaded()
{
    emit childrenLoaded(this);
}
QIcon ServerTreeItem::getIcon() const
{
    return loadedChildren ? icon : unloadedIcon;
}
void ServerTreeItem::loadChildren(QNetworkAccessManager* networkManager)
{
    QNetworkRequest request;
    QUrl url;
    url.setScheme(server.serverSchema);
    url.setPort(server.serverPort.toInt());
    url.setPath("/player_api.php");
    url.setHost(server.serverUrl);
    QUrlQuery query;
    query.addQueryItem("username", server.username);
    query.addQueryItem("password", server.password);
    query.addQueryItem("action", "get_live_categories");
    url.setQuery(query);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    auto reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished,
            [reply, this]()
            {
                loadedChildren = true;
                reply->deleteLater();
                clear();
                if (reply->error())
                {
                    ErrorLoadingTreeItem* errorChild =
                        new ErrorLoadingTreeItem(reply->errorString(), this);
                    appendChild(errorChild);
                    emitChildrenLoaded();
                    return;
                }
                auto response = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(response);
                auto categories = doc.array();
                for (const auto& cat : categories)
                {
                    if (!cat.isObject())
                        continue;
                    auto catObject = cat.toObject();
                    auto categoryId = catObject.value("category_id").toString("");
                    auto categoryName =
                        catObject.value("category_name").toString("");
                    auto group = new GroupTreeItem(categoryName, categoryId,
                                                   server, this);
                    connect(group, SIGNAL(channelsLoaded(GroupTreeItem*)), this,
                            SIGNAL(channelsLoaded(GroupTreeItem*)));
                    appendChild(group);
                }
                emitChildrenLoaded();
            });
}

#include "loadingchanneliconsworker.h"

#include "databaseprovider.h"
#include "database.h"
#include "channeltreeitem.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

LoadingChannelIconsWorker::LoadingChannelIconsWorker(QObject *parent)
    : QObject{parent}, networkManager{new QNetworkAccessManager{this}}
{

}

void LoadingChannelIconsWorker::loadChannelIcon(ChannelTreeItem* channel)
{
    if(!channel->getLogo().isEmpty() || channel->getLogoUri().isEmpty())
    {
        return;
    }

    if(channel->getLogoUri().startsWith("data:"))
    {
        auto base64Index = channel->getLogoUri().indexOf("base64,");
        if(base64Index > 0)
        {
            QString base64Data = channel->getLogoUri().right(channel->getLogoUri().size() - base64Index - 7);
            auto logo = QByteArray::fromBase64(base64Data.toUtf8());
            QImage image = QImage::fromData(logo);
            QIcon icon = QIcon{QPixmap::fromImage(image)};
            channel->setIcon(icon);
            channel->setLogo(logo);
            auto db = DatabaseProvider::GetDatabase();
            db->SetChannelLogo(channel);
            emit channelIconReady(channel);
        }
    }
    else
    {
        QNetworkRequest request;
        request.setUrl(QUrl{channel->getLogoUri()});
        request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
        auto reply = networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, channel, reply]()
        {

            auto logo = reply->readAll();
            QImage image = QImage::fromData(logo);
            QIcon icon = QIcon{QPixmap::fromImage(image)};
            channel->setIcon(icon);
            channel->setLogo(logo);
            reply->deleteLater();

            auto db = DatabaseProvider::GetDatabase();
            db->SetChannelLogo(channel);
            emit channelIconReady(channel);
        });
    }
}

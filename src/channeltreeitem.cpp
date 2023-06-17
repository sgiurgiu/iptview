#include "channeltreeitem.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <mutex>

#include "mediasegment.h"

namespace
{
    Q_GLOBAL_STATIC(QIcon, defaultChannelIcon);
}

ChannelTreeItem::ChannelTreeItem(QString name, QString uri, QString logoUri, QByteArray logo, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)},
      uri(std::move(uri)), logoUri{std::move(logoUri)}, logo{std::move(logo)}
{
    if(defaultChannelIcon->isNull())
    {
        defaultChannelIcon->addFile(":/icons/film-outline.png");
    }
    icon = *defaultChannelIcon;
}
void ChannelTreeItem::loadIcon()
{
    if(!this->logo.isEmpty() && (icon.isNull() || defaultIcon) && !cancelOngoingOperations)
    {
        QImage image = QImage::fromData(this->logo);
        {
            std::unique_lock lock(iconMutex);
            icon = QIcon{QPixmap::fromImage(image)};
            defaultIcon = false;
        }
        emit aquiredIcon(this);
    }
}
ChannelTreeItem::ChannelTreeItem(const MediaSegment& segment, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem(networkManager, parent)
{
    name = segment.GetTitle();
    uri = segment.GetUri();
    auto logo = segment.GetAttributeValue("tvg-logo");
    if(logo && !logo.value().isEmpty())
    {
        logoUri = logo.value();
    }
}

void ChannelTreeItem::loadLogo()
{
    if(!logo.isEmpty()) return;

    if(logoUri.startsWith("data:"))
    {
        auto base64Index = logoUri.indexOf("base64,");
        if(base64Index > 0)
        {
            QString base64Data = logoUri.right(logoUri.size() - base64Index - 7);
            logo = QByteArray::fromBase64(base64Data.toUtf8());
            loadIcon();
        }
    }
    else
    {
        QNetworkRequest request;
        request.setUrl(QUrl{logoUri});
        request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
        auto reply = networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]()
        {
            logo = reply->readAll();
            loadIcon();
            reply->deleteLater();
        });
    }
}

std::unique_ptr<ChannelTreeItem> ChannelTreeItem::clone(AbstractChannelTreeItem* newParent) const
{
    auto channel = std::make_unique<ChannelTreeItem>(name, uri, logoUri, logo, networkManager, newParent);
    channel->icon = icon;
    channel->id = id;
    return channel;
}

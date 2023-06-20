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

ChannelTreeItem::ChannelTreeItem(QString name, QString uri, QString logoUri, QByteArray logo, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem(parent), name{std::move(name)},
      uri(std::move(uri)), logoUri{std::move(logoUri)}, logo{std::move(logo)}
{
    if(defaultChannelIcon->isNull())
    {
        defaultChannelIcon->addFile(":/icons/film-outline.png");
    }
    if(!this->logo.isEmpty())
    {
        QImage image = QImage::fromData(this->logo);
        setIcon(QIcon{QPixmap::fromImage(image)});
        defaultIcon = false;
    }
    else
    {
        setIcon(*defaultChannelIcon);
    }
}
ChannelTreeItem::ChannelTreeItem(const MediaSegment& segment, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem( parent)
{
    name = segment.GetTitle();
    uri = segment.GetUri();
    auto logo = segment.GetAttributeValue("tvg-logo");
    if(logo && !logo.value().isEmpty())
    {
        logoUri = logo.value();
    }
    if(defaultChannelIcon->isNull())
    {
        defaultChannelIcon->addFile(":/icons/film-outline.png");
    }
    setIcon(*defaultChannelIcon);
}

ChannelTreeItem* ChannelTreeItem::clone(AbstractChannelTreeItem* newParent) const
{
    auto channel = new ChannelTreeItem(name, uri, logoUri, logo, newParent);
    channel->icon = icon;
    channel->id = id;
    return channel;
}

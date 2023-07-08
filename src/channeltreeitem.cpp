#include "channeltreeitem.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <mutex>
#include <QtConcurrent>

#include "mediasegment.h"

namespace
{
    Q_GLOBAL_STATIC(QIcon, defaultChannelIcon);
}

ChannelTreeItem::ChannelTreeItem(QString name, QString uri, QString logoUri, QByteArray logo, QIcon icon, AbstractChannelTreeItem* parent)
    : AbstractChannelTreeItem(parent), name{std::move(name)},
      uri(std::move(uri)), logoUri{std::move(logoUri)}, logo{std::move(logo)}
{
    setIcon(std::move(icon));
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
        iconFutureLoader = QtConcurrent::run([logo = this->logo](){
            QPixmap pixmap;
            pixmap.loadFromData(logo);
            return QIcon{pixmap};
        }).then([this](QIcon icon){
            setIcon(icon);
            defaultIcon = false;
        });
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
ChannelTreeItem::~ChannelTreeItem()
{
    iconFutureLoader.waitForFinished();
}
ChannelTreeItem* ChannelTreeItem::clone(AbstractChannelTreeItem* newParent) const
{
    auto channel = new ChannelTreeItem(name, uri, logoUri, logo, icon, newParent);
    channel->id = id;
    return channel;
}

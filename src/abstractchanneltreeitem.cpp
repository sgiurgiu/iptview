#include "abstractchanneltreeitem.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QImage>
#include <QPixmap>
#include <QString>

AbstractChannelTreeItem::AbstractChannelTreeItem(QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent):
    QObject{parent},
    parent{parent},
    networkManager{networkManager}
{
}
void AbstractChannelTreeItem::appendChild(std::unique_ptr<AbstractChannelTreeItem> child)
{
    children.push_back(std::move(child));
}
int AbstractChannelTreeItem::childCount() const
{
    return children.size();
}
int AbstractChannelTreeItem::row() const
{
    if(parent)
    {
        for(size_t i=0; i<parent->children.size(); i++)
        {
            if(parent->children.at(i).get() == this)
            {
                return i;
            }
        }
        return -1;
    }
    return -1;
}
AbstractChannelTreeItem* AbstractChannelTreeItem::child(int index) const
{
    if(index < 0 || index >= children.size()) return nullptr;
    return children.at(index).get();
}
AbstractChannelTreeItem* AbstractChannelTreeItem::getParent() const
{
    return parent;
}
QIcon AbstractChannelTreeItem::getIcon() const
{
    return icon;
}

RootTreeItem::RootTreeItem(QNetworkAccessManager* networkManager) : AbstractChannelTreeItem(networkManager, nullptr)
{
    auto favourites = std::make_unique<FavouritesTreeItem>(networkManager, this);
    connect(favourites.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    appendChild(std::move(favourites));
}

void RootTreeItem::addMediaSegment(const MediaSegment& segment)
{
    auto groupTitle = segment.GetAttributeValue("group-title");
    if(groupTitle)
    {
        auto it = groupsMap.find(groupTitle.value());
        if(it != groupsMap.end())
        {
            it.value()->addMediaSegment(segment);
        }
        else
        {
            auto group = std::make_unique<GroupTreeItem>(groupTitle.value(),networkManager, this);
            group->addMediaSegment(segment);
            groupsMap.insert(groupTitle.value(), group.get());
            connect(group.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
            appendChild(std::move(group));
        }
    }
    else
    {
        auto channel = std::make_unique<ChannelTreeItem>(segment, networkManager, this);
        connect(channel.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
        channel->loadLogo();
        appendChild(std::move(channel));
    }
}
void GroupTreeItem::addMediaSegment(const MediaSegment& segment)
{
    auto channel = std::make_unique<ChannelTreeItem>(segment, networkManager, this);
    connect(channel.get(), SIGNAL(aquiredIcon(AbstractChannelTreeItem*)), this, SIGNAL(aquiredIcon(AbstractChannelTreeItem*)));
    channel->loadLogo();
    appendChild(std::move(channel));
}
GroupTreeItem::GroupTreeItem(QString name,QNetworkAccessManager* networkManager, RootTreeItem* parent) : AbstractChannelTreeItem(networkManager, parent), name{std::move(name)}
{
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
    if(logoUri.startsWith("data:"))
    {
        auto base64Index = logoUri.indexOf("base64,");
        if(base64Index > 0)
        {
            QString base64Data = logoUri.right(logoUri.size() - base64Index - 7);
            QImage image = QImage::fromData(QByteArray::fromBase64(base64Data.toUtf8()));
            icon = QIcon{QPixmap::fromImage(image)};
            emit aquiredIcon(this);
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
            QImage image = QImage::fromData(reply->readAll());
            icon = QIcon{QPixmap::fromImage(image)};
            reply->deleteLater();
            emit aquiredIcon(this);
        });
    }
}
FavouritesTreeItem::FavouritesTreeItem(QNetworkAccessManager* networkManager, RootTreeItem* parent) : GroupTreeItem{"Favourites", networkManager, parent}
{
    icon = QIcon(":/icons/star.svg");
}


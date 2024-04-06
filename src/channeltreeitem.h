#ifndef CHANNELTREEITEM_H
#define CHANNELTREEITEM_H

#include "abstractchanneltreeitem.h"
#include <QFutureWatcher>

class MediaSegment;

class ChannelTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit ChannelTreeItem(const MediaSegment& segment,
                             AbstractChannelTreeItem* parent);
    explicit ChannelTreeItem(QString name,
                             QString uri,
                             QString logoUri,
                             QByteArray logo,
                             AbstractChannelTreeItem* parent);
    explicit ChannelTreeItem(QString name,
                             QString uri,
                             QString logoUri,
                             QByteArray logo,
                             QIcon icon,
                             AbstractChannelTreeItem* parent);
    virtual ~ChannelTreeItem();
    virtual ChannelTreeItemType getType() const override
    {
        return ChannelTreeItemType::Channel;
    }
    virtual QString getName() const override
    {
        return name;
    }
    virtual QString getUri() const
    {
        return uri;
    }
    virtual QString getLogoUri() const
    {
        return logoUri;
    }
    virtual QByteArray const& getLogo() const
    {
        return logo;
    }
    void setLogo(const QByteArray& logo)
    {
        this->logo = logo;
    }
    bool isFavourite() const
    {
        return favourite;
    }
    void setFavourite(bool flag)
    {
        favourite = flag;
    }

    void setEpgChannelId(const QString& id)
    {
        this->epgChannelId = id;
    }
    QString getEpgChannelId() const
    {
        return epgChannelId;
    }

    void setEpgChannelUri(const QString& uri)
    {
        this->channelEpgUri = uri;
    }
    QString getEpgChannelUri() const
    {
        return channelEpgUri;
    }
    int64_t getXStreamServerId() const
    {
        return xstreamServerId;
    }
    void setXStreamServerId(int64_t id)
    {
        xstreamServerId = id;
    }

    ChannelTreeItem* clone(AbstractChannelTreeItem* parent) const;

private:
    QString name;
    QString uri;
    QString logoUri;
    QByteArray logo;
    QString epgChannelId;
    QString channelEpgUri;
    bool defaultIcon = true;
    bool favourite = false;
    QFuture<void> iconFutureLoader;
    int64_t xstreamServerId = -1;
};
Q_DECLARE_METATYPE(ChannelTreeItem)

#endif // CHANNELTREEITEM_H

#ifndef CHANNELTREEITEM_H
#define CHANNELTREEITEM_H

#include "abstractchanneltreeitem.h"

class MediaSegment;

class ChannelTreeItem : public AbstractChannelTreeItem
{
    Q_OBJECT
public:
    explicit ChannelTreeItem(const MediaSegment& segment, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent);
    explicit ChannelTreeItem(QString name, QString uri, QString logoUri, QByteArray logo, QNetworkAccessManager* networkManager, AbstractChannelTreeItem* parent);
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
    void loadLogo();
    void loadIcon();
    std::unique_ptr<ChannelTreeItem> clone(AbstractChannelTreeItem* parent) const;
private:
    QString name;
    QString uri;
    QString logoUri;
    QByteArray logo;
    bool defaultIcon = true;
};

#endif // CHANNELTREEITEM_H

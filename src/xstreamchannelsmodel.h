#ifndef XSTREAM_CHANNELS_MODEL_H
#define XSTREAM_CHANNELS_MODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QThread>

#include "channeltreeitem.h"
#include "xstreaminfo.h"
#include <memory>

class AbstractChannelTreeItem;
class RootTreeItem;
class ServerTreeItem;
class GroupTreeItem;
class QNetworkAccessManager;

class XStreamChannelsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit XStreamChannelsModel(QNetworkAccessManager *networkManager,
                                  QObject *parent = nullptr);
    ~XStreamChannelsModel();
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index,
                          int role = Qt::DisplayRole) const override;
    virtual QModelIndex
    index(int row,
          int column,
          const QModelIndex &parent = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void refreshItemsChildren(const QVariant &itemVariant);
    enum ChannelRoles
    {
        NameRole = Qt::UserRole + 1,
        UriRole,
        IdRole,
        DataRole
    };
private slots:
    void onServerChildrenLoaded(ServerTreeItem *treeItem);
    void onChannelsLoaded(GroupTreeItem *treeItem);

private:
    QModelIndex indexFromItem(AbstractChannelTreeItem *item);

private:
    std::unique_ptr<RootTreeItem> rootItem;
    QNetworkAccessManager *networkManager;
};

#endif
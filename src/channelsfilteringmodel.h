#ifndef CHANNELSFILTERINGMODEL_H
#define CHANNELSFILTERINGMODEL_H

#include <QSortFilterProxyModel>

class ChannelsFilteringModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ChannelsFilteringModel(QObject *parent = nullptr);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // CHANNELSFILTERINGMODEL_H

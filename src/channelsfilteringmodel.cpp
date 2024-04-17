#include "channelsfilteringmodel.h"

#include "abstractchanneltreeitem.h"

#include <QDebug>

ChannelsFilteringModel::ChannelsFilteringModel(QObject *parent)
: QSortFilterProxyModel{ parent }
{
}

bool ChannelsFilteringModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const
{
    auto filter = filterRegularExpression();
    if (!filter.isValid() || filter.pattern().isEmpty())
    {
        return true;
    }
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    AbstractChannelTreeItem *item =
        static_cast<AbstractChannelTreeItem *>(sourceIndex.internalPointer());
    if (item->getType() == ChannelTreeItemType::Error ||
        item->getType() == ChannelTreeItemType::Loading)
    {
        return false;
    }
    if (item->getType() != ChannelTreeItemType::Channel)
    {
        const int count = sourceModel()->rowCount(sourceIndex);
        // if we have any children that are to be shown
        // then we should be shown as well, otherwise, we should be hidden
        for (int i = 0; i < count; ++i)
        {
            if (filterAcceptsRow(i, sourceIndex))
            {
                return true;
            }
        }
        return false;
    }

    return item->getName().contains(filter);
}

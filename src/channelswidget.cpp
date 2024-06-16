#include "channelswidget.h"

#include <QAction>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QTreeView>
#include <QVBoxLayout>

#include "abstractchanneltreeitem.h"
#include "channelsfilteringmodel.h"
#include "channelsmodel.h"
#include "channeltreeitem.h"
#include "database.h"
#include "databaseprovider.h"
#include "grouptreeitem.h"
#include "xstreamchannelsmodel.h"

namespace
{
struct TreeItemIndex
{
    QModelIndex index;
    AbstractChannelTreeItem* treeItem = nullptr;
};
} // namespace

ChannelsWidget::ChannelsWidget(QNetworkAccessManager* networkManager,
                               QWidget* parent)
: QTabWidget{ parent }, networkManager{ networkManager }
{
    this->setTabPosition(QTabWidget::TabPosition::South);
    QWidget* localChannelsTabWidget = new QWidget();

    localChannels = new QTreeView(localChannelsTabWidget);
    localChannelsModel = new ChannelsModel(localChannelsTabWidget);
    localChannelsProxyModel = new ChannelsFilteringModel(localChannelsTabWidget);
    localChannelsProxyModel->setSourceModel(localChannelsModel);
    localChannelsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    localChannelsProxyModel->setFilterRole(ChannelsModel::NameRole);
    localChannels->setSelectionBehavior(QAbstractItemView::SelectItems);
    localChannels->setSelectionMode(QAbstractItemView::ExtendedSelection);
    localChannels->setModel(localChannelsProxyModel);
    localChannelsSearchField = new QLineEdit(localChannelsTabWidget);
    localChannelsSearchField->setPlaceholderText("Search");

    QVBoxLayout* localLayout = new QVBoxLayout(localChannelsTabWidget);

    localLayout->addWidget(localChannelsSearchField, 0);
    localLayout->addWidget(localChannels, 1);
    localChannelsTabWidget->setLayout(localLayout);

    QWidget* xstreamChannelsTabWidget = new QWidget();
    xstreamChannels = new QTreeView(xstreamChannelsTabWidget);
    xstreamChannelModel =
        new XStreamChannelsModel(networkManager, xstreamChannelsTabWidget);
    xstreamChannelsProxyModel =
        new ChannelsFilteringModel(xstreamChannelsTabWidget);

    xstreamChannelsProxyModel->setSourceModel(xstreamChannelModel);
    xstreamChannelsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    xstreamChannelsProxyModel->setFilterRole(ChannelsModel::NameRole);
    xstreamChannels->setSelectionBehavior(QAbstractItemView::SelectItems);
    xstreamChannels->setSelectionMode(QAbstractItemView::SingleSelection);
    xstreamChannels->setModel(xstreamChannelsProxyModel);

    xstreamChannelsSearchField = new QLineEdit(xstreamChannelsTabWidget);
    xstreamChannelsSearchField->setPlaceholderText("Search");

    QVBoxLayout* xstreamLayout = new QVBoxLayout(xstreamChannelsTabWidget);
    xstreamLayout->addWidget(xstreamChannelsSearchField, 0);
    xstreamLayout->addWidget(xstreamChannels, 1);
    xstreamChannelsTabWidget->setLayout(xstreamLayout);

    addTab(localChannelsTabWidget, "Local");
    addTab(xstreamChannelsTabWidget, "XStream");

    connect(localChannelsSearchField, SIGNAL(textChanged(QString)), this,
            SLOT(searchTextChanged(QString)));
    connect(localChannels, SIGNAL(doubleClicked(QModelIndex)), this,
            SLOT(onDoubleClickedTreeItem(QModelIndex)));
    connect(localChannels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
            SLOT(itemsSelectionChanged(QItemSelection, QItemSelection)));

    connect(xstreamChannelsSearchField, SIGNAL(textChanged(QString)), this,
            SLOT(xstreamSearchTextChanged(QString)));
    connect(xstreamChannels, SIGNAL(doubleClicked(QModelIndex)), this,
            SLOT(onDoubleClickedXStreamTreeItem(QModelIndex)));
    connect(xstreamChannels->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
            SLOT(xstreamItemsSelectionChanged(QItemSelection, QItemSelection)));

    localChannelsContextMenu = new QMenu(localChannelsTabWidget);
    addToFavouritesAction = new QAction("Add to Favourites", this);
    removeFromFavouritesAction = new QAction("Remove from Favourites", this);
    addNewChannelAction = new QAction("Add new channel", this);
    removeChannelGroupAction = new QAction("Remove channel(s)/group(s)", this);
    addNewChannelGroupAction = new QAction("Add new group", this);
    localChannels->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(localChannels, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(onCustomContextMenu(QPoint)));
    connect(addToFavouritesAction, SIGNAL(triggered(bool)), this,
            SLOT(onAddToFavourites()));
    connect(removeFromFavouritesAction, SIGNAL(triggered(bool)), this,
            SLOT(onRemoveFromFavourites()));
    connect(addNewChannelAction, SIGNAL(triggered(bool)), this,
            SLOT(onAddNewChannel()));
    connect(removeChannelGroupAction, SIGNAL(triggered(bool)), this,
            SLOT(onRemoveChannelGroup()));
    connect(addNewChannelGroupAction, SIGNAL(triggered(bool)), this,
            SLOT(onAddNewChannelGroup()));

    xstreamChannelsContextMenu = new QMenu(xstreamChannelsTabWidget);
    refreshXstreamItemsAction = new QAction("Refresh", this);
    xstreamChannels->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(xstreamChannels, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(onCustomXStreamContextMenu(QPoint)));
    connect(refreshXstreamItemsAction, SIGNAL(triggered(bool)), this,
            SLOT(refreshXStreamItems()));

    connect(localChannelsModel, SIGNAL(updateImportedChannelIndex(qint64)),
            this, SIGNAL(updateImportedChannelIndex(qint64)));
    connect(localChannelsModel, SIGNAL(channelsImported()), this,
            SIGNAL(channelsImported()));

    connect(this, SIGNAL(cancelImportChannels()), localChannelsModel,
            SIGNAL(cancelImportChannels()));
}
M3UList ChannelsWidget::GetM3UList() const
{
    return localChannelsModel->GetM3UList();
}
void ChannelsWidget::searchTextChanged(const QString& text)
{
    localChannelsProxyModel->setFilterWildcard(text);
}
void ChannelsWidget::xstreamSearchTextChanged(const QString& text)
{
    xstreamChannelsProxyModel->setFilterWildcard(text);
}
void ChannelsWidget::ImportPlaylist(M3UList list)
{
    localChannelsModel->AddList(std::move(list));
}
void ChannelsWidget::ImportPlaylist(XStreamCollectedInfo list)
{
    localChannelsModel->AddList(std::move(list));
}
void ChannelsWidget::onDoubleClickedXStreamTreeItem(const QModelIndex& index)
{
    if (index.isValid())
    {
        auto channel = index.data(XStreamChannelsModel::ChannelRoles::DataRole);
        if (channel.isValid())
        {
            emit playChannel(channel.value<ChannelTreeItem*>());
            //  TODO: This will need to become an option
            //  Right now we're only moving forward&back within the same group
            //  we may want to (in the future) cross groups as well

            auto nextIndex = index.siblingAtRow(index.row() + 1);
            auto backIndex = index.siblingAtRow(index.row() - 1);
            emit enableSkipForward(
                nextIndex.isValid() &&
                nextIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
            emit enableSkipBack(
                backIndex.isValid() &&
                backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
        }
    }
}
void ChannelsWidget::onDoubleClickedTreeItem(const QModelIndex& index)
{
    if (index.isValid())
    {
        auto id = index.data(ChannelsModel::ChannelRoles::IdRole);
        if (id.isValid())
        {
            emit playChannel(id.toLongLong());
            // TODO: This will need to become an option
            // Right now we're only moving forward&back within the same group
            // we may want to (in the future) cross groups as well

            auto nextIndex = index.siblingAtRow(index.row() + 1);
            auto backIndex = index.siblingAtRow(index.row() - 1);
            emit enableSkipForward(
                nextIndex.isValid() &&
                nextIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
            emit enableSkipBack(
                backIndex.isValid() &&
                backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
        }
    }
}
void ChannelsWidget::itemsSelectionChanged(const QItemSelection& selected,
                                           const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    if (selected.isEmpty())
        return;
    auto firstSelectedProxyIndex = selected.indexes().front();
    if (!firstSelectedProxyIndex.isValid())
    {
        return;
    }
    auto firstSelected =
        localChannelsProxyModel->mapToSource(firstSelectedProxyIndex);
    if (!firstSelected.isValid())
    {
        return;
    }

    auto id = firstSelected.data(ChannelsModel::ChannelRoles::IdRole);
    if (id.isValid())
    {
        emit selectChannel(id.toLongLong());

        auto nextIndex = firstSelected.siblingAtRow(firstSelected.row() + 1);
        auto backIndex = firstSelected.siblingAtRow(firstSelected.row() - 1);
        emit enableSkipForward(
            nextIndex.isValid() &&
            nextIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
        emit enableSkipBack(
            backIndex.isValid() &&
            backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
    }
}
void ChannelsWidget::xstreamItemsSelectionChanged(const QItemSelection& selected,
                                                  const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    if (selected.isEmpty())
        return;
    auto firstSelectedProxyIndex = selected.indexes().front();
    if (!firstSelectedProxyIndex.isValid())
    {
        return;
    }
    auto firstSelected =
        xstreamChannelsProxyModel->mapToSource(firstSelectedProxyIndex);
    if (!firstSelected.isValid())
    {
        return;
    }

    auto data = firstSelected.data(XStreamChannelsModel::ChannelRoles::DataRole);
    if (data.isValid())
    {
        emit selectChannel(data.value<ChannelTreeItem*>());

        auto nextIndex = firstSelected.siblingAtRow(firstSelected.row() + 1);
        auto backIndex = firstSelected.siblingAtRow(firstSelected.row() - 1);
        emit enableSkipForward(
            nextIndex.isValid() &&
            nextIndex.data(XStreamChannelsModel::ChannelRoles::IdRole).isValid());
        emit enableSkipBack(
            backIndex.isValid() &&
            backIndex.data(XStreamChannelsModel::ChannelRoles::IdRole).isValid());
    }
}
void ChannelsWidget::onCustomXStreamContextMenu(const QPoint& point)
{
    xstreamChannelsContextMenu->clear();
    auto selectedIndexes = xstreamChannels->selectionModel()->selectedIndexes();
    if (selectedIndexes.count() != 1)
    {
        return;
    }
    QModelIndex proxyIndex = selectedIndexes.front();
    auto index = xstreamChannelsProxyModel->mapToSource(proxyIndex);
    if (!index.isValid())
    {
        return;
    }
    auto treeItem =
        static_cast<AbstractChannelTreeItem*>(index.internalPointer());
    if (!treeItem)
        return;
    switch (treeItem->getType())
    {
    case ChannelTreeItemType::Server:
    case ChannelTreeItemType::Group:
        refreshXstreamItemsAction->setData(QVariant::fromValue(treeItem));
        xstreamChannelsContextMenu->addAction(refreshXstreamItemsAction);
        xstreamChannelsContextMenu->exec(
            xstreamChannels->viewport()->mapToGlobal(point));
        break;
    default:
        return;
    }
}
void ChannelsWidget::onCustomContextMenu(const QPoint& point)
{
    localChannelsContextMenu->clear();
    addNewChannelAction->setData(QVariant{});
    addNewChannelGroupAction->setData(QVariant{});
    removeChannelGroupAction->setData(QVariant{});
    auto selectedIndexes = localChannels->selectionModel()->selectedIndexes();
    if (selectedIndexes.empty())
    {
        localChannelsContextMenu->addAction(addNewChannelAction);
        localChannelsContextMenu->addAction(addNewChannelGroupAction);
        localChannelsContextMenu->exec(
            localChannels->viewport()->mapToGlobal(point));
        return;
    }
    if (selectedIndexes.count() > 1)
    {
        localChannelsContextMenu->addAction(removeChannelGroupAction);
        localChannelsContextMenu->exec(
            localChannels->viewport()->mapToGlobal(point));
        return;
    }
    QModelIndex proxyIndex = selectedIndexes.front();
    auto index = localChannelsProxyModel->mapToSource(proxyIndex);
    if (!index.isValid())
    {
        localChannelsContextMenu->addAction(addNewChannelAction);
        localChannelsContextMenu->addAction(addNewChannelGroupAction);
        localChannelsContextMenu->exec(
            localChannels->viewport()->mapToGlobal(point));
        return;
    }

    auto treeItem =
        static_cast<AbstractChannelTreeItem*>(index.internalPointer());
    if (!treeItem)
        return;

    if (treeItem->getType() == ChannelTreeItemType::Channel)
    {
        auto parentItem = treeItem->getParent();
        if (parentItem && parentItem->getType() == ChannelTreeItemType::Favourite)
        {
            removeFromFavouritesAction->setData(QVariant::fromValue(treeItem));
            localChannelsContextMenu->addSeparator();
            localChannelsContextMenu->addAction(removeFromFavouritesAction);
            localChannelsContextMenu->exec(
                localChannels->viewport()->mapToGlobal(point));
        }
        else if (parentItem &&
                 (parentItem->getType() == ChannelTreeItemType::Group ||
                  parentItem->getType() == ChannelTreeItemType::Root))
        {
            TreeItemIndex parentTreeItemIndex;
            parentTreeItemIndex.index = index.parent();
            parentTreeItemIndex.treeItem = parentItem;

            TreeItemIndex treeItemIndex;
            treeItemIndex.index = index;
            treeItemIndex.treeItem = treeItem;

            addNewChannelAction->setData(QVariant::fromValue(parentTreeItemIndex));
            addNewChannelGroupAction->setData(
                QVariant::fromValue(parentTreeItemIndex));

            removeChannelGroupAction->setData(
                QVariant::fromValue(std::move(treeItemIndex)));
            addToFavouritesAction->setData(QVariant::fromValue(treeItem));

            localChannelsContextMenu->addAction(addToFavouritesAction);
            localChannelsContextMenu->addSeparator();
            localChannelsContextMenu->addAction(addNewChannelAction);
            localChannelsContextMenu->addAction(addNewChannelGroupAction);
            localChannelsContextMenu->addAction(removeChannelGroupAction);
            localChannelsContextMenu->exec(
                localChannels->viewport()->mapToGlobal(point));
        }
    }
    else if (treeItem->getType() == ChannelTreeItemType::Group)
    {
        TreeItemIndex treeItemIndex;
        treeItemIndex.index = index;
        treeItemIndex.treeItem = treeItem;

        removeChannelGroupAction->setData(QVariant::fromValue(treeItemIndex));
        addNewChannelAction->setData(QVariant::fromValue(treeItemIndex));
        addNewChannelGroupAction->setData(QVariant::fromValue(treeItemIndex));
        localChannelsContextMenu->addAction(addNewChannelAction);
        localChannelsContextMenu->addAction(addNewChannelGroupAction);
        localChannelsContextMenu->addAction(removeChannelGroupAction);
        localChannelsContextMenu->exec(
            localChannels->viewport()->mapToGlobal(point));
    }
}

void ChannelsWidget::onAddToFavourites()
{
    auto treeItem =
        addToFavouritesAction->data().value<AbstractChannelTreeItem*>();
    localChannelsModel->AddToFavourites(treeItem);
    localChannelsProxyModel->invalidate();
}
void ChannelsWidget::onRemoveFromFavourites()
{
    auto treeItem =
        removeFromFavouritesAction->data().value<AbstractChannelTreeItem*>();
    localChannelsModel->RemoveFromFavourites(treeItem);
    localChannelsProxyModel->invalidate();
}
void ChannelsWidget::onAddNewChannel()
{
    auto treeItemVariant = addNewChannelAction->data();

    QDialog* dialog = new QDialog(this);
    dialog->setModal(true);
    QLineEdit* nameLineEdit = new QLineEdit(dialog);
    QLineEdit* urlLineEdit = new QLineEdit(dialog);
    QLineEdit* iconLineEdit = new QLineEdit(dialog);
    QFormLayout* formLayout = new QFormLayout(dialog);
    formLayout->addRow(tr("&Name:"), nameLineEdit);
    formLayout->addRow(tr("&Url:"), urlLineEdit);
    formLayout->addRow(tr("&Icon:"), iconLineEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    formLayout->addWidget(buttonBox);
    dialog->setLayout(formLayout);

    TreeItemIndex treeItemIndex;
    if (treeItemVariant.isValid())
    {
        treeItemIndex = treeItemVariant.value<TreeItemIndex>();

        auto index = treeItemIndex.index;
        auto parentItem = treeItemIndex.treeItem;

        connect(dialog, &QDialog::finished, this,
                [dialog, nameLineEdit, urlLineEdit, iconLineEdit, index,
                 parentItem, this](int result)
                {
                    if (result == QDialog::DialogCode::Accepted)
                    {
                        auto channel =
                            DatabaseProvider::GetDatabase()->AddChannel(
                                nameLineEdit->text(), urlLineEdit->text(),
                                iconLineEdit->text(), parentItem->getID());
                        localChannelsModel->AddChild(channel, index);
                    }
                    dialog->deleteLater();
                });
    }
    else
    {
        // we're adding to root
        connect(
            dialog, &QDialog::finished, this,
            [dialog, nameLineEdit, urlLineEdit, iconLineEdit, this](int result)
            {
                if (result == QDialog::DialogCode::Accepted)
                {
                    auto channel = DatabaseProvider::GetDatabase()->AddChannel(
                        nameLineEdit->text(), urlLineEdit->text(),
                        iconLineEdit->text(), {});
                    localChannelsModel->AddChild(channel, QModelIndex{});
                }
                dialog->deleteLater();
            });
    }

    dialog->open();
}

void ChannelsWidget::onRemoveChannelGroup()
{
    auto treeItemIndex = removeChannelGroupAction->data().value<TreeItemIndex>();
    if (treeItemIndex.treeItem != nullptr && treeItemIndex.index.isValid())
    {
        auto selectedButton = QMessageBox::question(
            this, "Confirm",
            QString("Are you sure you want to delete channel/group %1?")
                .arg(treeItemIndex.treeItem->getName()));
        if (selectedButton == QMessageBox::StandardButton::Yes)
        {
            if (treeItemIndex.treeItem->getType() == ChannelTreeItemType::Group)
            {
                DatabaseProvider::GetDatabase()->RemoveGroup(
                    treeItemIndex.treeItem->getID());
            }
            else if (treeItemIndex.treeItem->getType() ==
                     ChannelTreeItemType::Channel)
            {
                DatabaseProvider::GetDatabase()->RemoveChannel(
                    treeItemIndex.treeItem->getID());
            }
            else
            {
                return;
            }
            localChannelsModel->RemoveChild(treeItemIndex.treeItem,
                                            treeItemIndex.index);
        }
    }
    else
    {
        auto selection = localChannels->selectionModel()->selectedIndexes();
        if (!selection.empty())
        {
            auto selectedButton = QMessageBox::question(
                this, "Confirm",
                QString("Are you sure you want to delete %1 channels/groups?")
                    .arg(selection.count()));
            if (selectedButton != QMessageBox::StandardButton::Yes)
                return;
        }
        std::vector<int64_t> groups;
        std::vector<int64_t> channels;
        for (const auto& proxyIndex : selection)
        {
            if (!proxyIndex.isValid())
                continue;
            auto index = localChannelsProxyModel->mapToSource(proxyIndex);
            if (!index.isValid())
                continue;
            auto treeItem =
                static_cast<AbstractChannelTreeItem*>(index.internalPointer());
            if (!treeItem)
                continue;
            if (treeItem->getType() == ChannelTreeItemType::Group)
            {
                groups.push_back(treeItem->getID());
            }
            else if (treeItem->getType() == ChannelTreeItemType::Channel)
            {
                channels.push_back(treeItem->getID());
            }
        }
        for (auto id : groups)
        {
            DatabaseProvider::GetDatabase()->RemoveGroup(id);
        }
        for (auto id : channels)
        {
            DatabaseProvider::GetDatabase()->RemoveChannel(id);
        }
        localChannelsModel->ReloadChannels();
    }
}
void ChannelsWidget::onAddNewChannelGroup()
{
    auto treeItemVariant = addNewChannelGroupAction->data();
    bool ok;
    QString text =
        QInputDialog::getText(this, tr("Add new group"), tr("Group name:"),
                              QLineEdit::Normal, "", &ok);
    std::optional<int64_t> parentGroupId;
    TreeItemIndex treeItemIndex;
    if (treeItemVariant.isValid())
    {
        treeItemIndex = treeItemVariant.value<TreeItemIndex>();
        parentGroupId.emplace(treeItemIndex.treeItem->getID());
    }
    if (ok && !text.isEmpty())
    {
        auto group =
            DatabaseProvider::GetDatabase()->AddGroup(text, parentGroupId);
        localChannelsModel->AddChild(group, treeItemIndex.index);
    }
}
void ChannelsWidget::refreshXStreamItems()
{
    xstreamChannelModel->refreshItemsChildren(refreshXstreamItemsAction->data());
}
void ChannelsWidget::skipForwardLocalChannels()
{
    skipLocalChannels(1);
}

void ChannelsWidget::skipBackLocalChannels()
{
    skipLocalChannels(-1);
}
void ChannelsWidget::skipLocalChannels(int amount)
{
    if (!localChannels->selectionModel()->hasSelection())
        return;
    auto currentSelectedIndex = localChannels->selectionModel()->currentIndex();
    if (!currentSelectedIndex.isValid())
        return;
    auto nextIndex =
        currentSelectedIndex.siblingAtRow(currentSelectedIndex.row() + amount);
    if (!nextIndex.isValid())
        return;
    auto id = nextIndex.data(ChannelsModel::ChannelRoles::IdRole);
    if (!id.isValid())
        return;
    localChannels->selectionModel()->clearSelection();
    localChannels->selectionModel()->setCurrentIndex(
        nextIndex, QItemSelectionModel::SelectionFlag::Select);
    emit playChannel(id.toLongLong());

    auto nextIndex2 = nextIndex.siblingAtRow(nextIndex.row() + 1);
    auto backIndex = nextIndex.siblingAtRow(nextIndex.row() - 1);
    emit enableSkipForward(
        nextIndex2.isValid() &&
        nextIndex2.data(ChannelsModel::ChannelRoles::IdRole).isValid());
    emit enableSkipBack(
        backIndex.isValid() &&
        backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
}

void ChannelsWidget::skipXStreamChannels(int amount)
{
    if (!xstreamChannels->selectionModel()->hasSelection())
        return;
    auto currentSelectedIndex = xstreamChannels->selectionModel()->currentIndex();
    if (!currentSelectedIndex.isValid())
        return;
    auto nextIndex =
        currentSelectedIndex.siblingAtRow(currentSelectedIndex.row() + amount);
    if (!nextIndex.isValid())
        return;
    auto channel = nextIndex.data(XStreamChannelsModel::ChannelRoles::DataRole);
    if (!channel.isValid())
        return;
    xstreamChannels->selectionModel()->clearSelection();
    xstreamChannels->selectionModel()->setCurrentIndex(
        nextIndex, QItemSelectionModel::SelectionFlag::Select);

    emit playChannel(channel.value<ChannelTreeItem*>());

    auto nextIndex2 = nextIndex.siblingAtRow(nextIndex.row() + 1);
    auto backIndex = nextIndex.siblingAtRow(nextIndex.row() - 1);
    emit enableSkipForward(
        nextIndex2.isValid() &&
        nextIndex2.data(XStreamChannelsModel::ChannelRoles::IdRole).isValid());
    emit enableSkipBack(
        backIndex.isValid() &&
        backIndex.data(XStreamChannelsModel::ChannelRoles::IdRole).isValid());
}

void ChannelsWidget::skipForwardXStreamChannels()
{
    skipXStreamChannels(1);
}

void ChannelsWidget::skipBackXStreamChannels()
{
    skipXStreamChannels(-1);
}

void ChannelsWidget::SkipForward()
{
    int selectedTab = currentIndex();
    switch (selectedTab)
    {
    case 0:
        skipForwardLocalChannels();
        break;
    case 1:
        skipForwardXStreamChannels();
        break;
    default:
        return;
    }
}
void ChannelsWidget::SkipBack()
{
    int selectedTab = currentIndex();
    switch (selectedTab)
    {
    case 0:
        skipBackLocalChannels();
        break;
    case 1:
        skipBackXStreamChannels();
        break;
    default:
        return;
    }
}

#include "channelswidget.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFormLayout>
#include <QDialogButtonBox>

#include "channelsfilteringmodel.h"
#include "channelsmodel.h"
#include "abstractchanneltreeitem.h"
#include "channeltreeitem.h"
#include "databaseprovider.h"
#include "database.h"
#include "grouptreeitem.h"

namespace
{
    struct TreeItemIndex
    {
        QModelIndex index;
        AbstractChannelTreeItem* treeItem = nullptr;
    };
}

ChannelsWidget::ChannelsWidget(QWidget *parent)
    : QWidget{parent}
{
    channels = new QTreeView(this);
    model = new ChannelsModel(this);
    proxyModel = new ChannelsFilteringModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(ChannelsModel::NameRole);
    channels->setSelectionBehavior(QAbstractItemView::SelectItems);
    channels->setModel(proxyModel);
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search");

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(searchField, 0);
    layout->addWidget(channels, 1);
    setLayout(layout);

    connect(searchField, SIGNAL(textChanged(QString)), this, SLOT(searchTextChanged(QString)));
    connect(channels,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(onDoubleClickedTreeItem(QModelIndex)));
    connect(channels->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this, SLOT(itemsSelectionChanged(QItemSelection,QItemSelection)));
    contextMenu = new QMenu(this);
    addToFavouritesAction = new QAction("Add to Favourites", this);
    removeFromFavouritesAction = new QAction("Remove from Favourites", this);
    addNewChannelAction = new QAction("Add new channel", this);
    removeChannelAction = new QAction("Remove channel", this);
    removeChannelGroupAction = new QAction("Remove group", this);
    addNewChannelGroupAction = new QAction("Add new group", this);
    channels->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(channels, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenu(QPoint)));
    connect(addToFavouritesAction, SIGNAL(triggered(bool)), this, SLOT(onAddToFavourites()));
    connect(removeFromFavouritesAction, SIGNAL(triggered(bool)), this, SLOT(onRemoveFromFavourites()));
    connect(addNewChannelAction, SIGNAL(triggered(bool)), this, SLOT(onAddNewChannel()));
    connect(removeChannelAction, SIGNAL(triggered(bool)), this, SLOT(onRemoveChannel()));
    connect(removeChannelGroupAction, SIGNAL(triggered(bool)), this, SLOT(onRemoveChannelGroup()));
    connect(addNewChannelGroupAction, SIGNAL(triggered(bool)), this, SLOT(onAddNewChannelGroup()));

    connect(model, SIGNAL(updateImportedChannelIndex(qint64)),this, SIGNAL(updateImportedChannelIndex(qint64)));
    connect(model, SIGNAL(channelsImported()),this, SIGNAL(channelsImported()));

}
void ChannelsWidget::searchTextChanged(const QString& text)
{
    proxyModel->setFilterWildcard(text);
}
void ChannelsWidget::ImportPlaylist(M3UList list)
{
    model->AddList(std::move(list));
}
void ChannelsWidget::onDoubleClickedTreeItem(const QModelIndex &index)
{
    if(index.isValid())
    {
        auto id = index.data(ChannelsModel::ChannelRoles::IdRole);
        if(id.isValid())
        {
            emit playChannel(id.toLongLong());
            // TODO: This will need to become an option
            // Right now we're only moving forward&back within the same group
            // we may want to (in the future) cross groups as well

            auto nextIndex = index.siblingAtRow(index.row()+1);
            auto backIndex = index.siblingAtRow(index.row()-1);
            emit enableSkipForward(nextIndex.isValid() && nextIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
            emit enableSkipBack(backIndex.isValid() && backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
        }
    }
}
void ChannelsWidget::itemsSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if(selected.isEmpty()) return;
    auto firstSelectedProxyIndex = selected.indexes().front();
    if(!firstSelectedProxyIndex.isValid())
    {
        return;
    }
    auto firstSelected = proxyModel->mapToSource(firstSelectedProxyIndex);
    if(!firstSelected.isValid())
    {
        return;
    }

    auto id = firstSelected.data(ChannelsModel::ChannelRoles::IdRole);
    if(id.isValid())
    {
        emit selectChannel(id.toLongLong());

        auto nextIndex = firstSelected.siblingAtRow(firstSelected.row()+1);
        auto backIndex = firstSelected.siblingAtRow(firstSelected.row()-1);
        emit enableSkipForward(nextIndex.isValid() && nextIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
        emit enableSkipBack(backIndex.isValid() && backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
    }
}
void ChannelsWidget::onCustomContextMenu(const QPoint &point)
{
    contextMenu->clear();
    addNewChannelAction->setData(QVariant{});
    addNewChannelGroupAction->setData(QVariant{});
    QModelIndex proxyIndex = channels->indexAt(point);    
    auto index = proxyModel->mapToSource(proxyIndex);
    if(!index.isValid())
    {
        contextMenu->addAction(addNewChannelAction);
        contextMenu->addAction(addNewChannelGroupAction);
        contextMenu->exec(channels->viewport()->mapToGlobal(point));
        return;
    }

    auto treeItem = static_cast<AbstractChannelTreeItem*>(index.internalPointer());
    if(!treeItem) return;

    if(treeItem->getType() == ChannelTreeItemType::Channel)
    {
        auto parentItem = treeItem->getParent();
        if(parentItem && parentItem->getType() == ChannelTreeItemType::Favourite)
        {
            removeFromFavouritesAction->setData(QVariant::fromValue(treeItem));
            contextMenu->addSeparator();
            contextMenu->addAction(removeFromFavouritesAction);
            contextMenu->exec(channels->viewport()->mapToGlobal(point));
        }
        else if(parentItem && (parentItem->getType() == ChannelTreeItemType::Group || parentItem->getType() == ChannelTreeItemType::Root))
        {
            TreeItemIndex parentTreeItemIndex;
            parentTreeItemIndex.index = index.parent();
            parentTreeItemIndex.treeItem = parentItem;

            TreeItemIndex treeItemIndex;
            treeItemIndex.index = index;
            treeItemIndex.treeItem = treeItem;

            addNewChannelAction->setData(QVariant::fromValue(parentTreeItemIndex));
            addNewChannelGroupAction->setData(QVariant::fromValue(parentTreeItemIndex));

            removeChannelAction->setData(QVariant::fromValue(std::move(treeItemIndex)));
            addToFavouritesAction->setData(QVariant::fromValue(treeItem));

            contextMenu->addAction(addToFavouritesAction);
            contextMenu->addSeparator();
            contextMenu->addAction(addNewChannelAction);
            contextMenu->addAction(addNewChannelGroupAction);
            contextMenu->addAction(removeChannelAction);
            contextMenu->exec(channels->viewport()->mapToGlobal(point));
        }
    }
    else if(treeItem->getType() == ChannelTreeItemType::Group)
    {
        TreeItemIndex treeItemIndex;
        treeItemIndex.index = index;
        treeItemIndex.treeItem = treeItem;

        removeChannelGroupAction->setData(QVariant::fromValue(treeItemIndex));
        addNewChannelAction->setData(QVariant::fromValue(treeItemIndex));
        addNewChannelGroupAction->setData(QVariant::fromValue(treeItemIndex));
        contextMenu->addAction(addNewChannelAction);
        contextMenu->addAction(addNewChannelGroupAction);
        contextMenu->addAction(removeChannelGroupAction);
        contextMenu->exec(channels->viewport()->mapToGlobal(point));
    }
}

void ChannelsWidget::onAddToFavourites()
{
    auto treeItem = addToFavouritesAction->data().value<AbstractChannelTreeItem*>();
    model->AddToFavourites(treeItem);
    proxyModel->invalidate();
}
void ChannelsWidget::onRemoveFromFavourites()
{
    auto treeItem = removeFromFavouritesAction->data().value<AbstractChannelTreeItem*>();
    model->RemoveFromFavourites(treeItem);
    proxyModel->invalidate();
}
void ChannelsWidget::onAddNewChannel()
{
    auto treeItemVariant = addNewChannelAction->data();

    QDialog* dialog = new QDialog(this);
    dialog->setModal(true);
    QLineEdit* nameLineEdit = new QLineEdit(dialog);
    QLineEdit* urlLineEdit = new QLineEdit(dialog);
    QLineEdit* iconLineEdit = new QLineEdit(dialog);
    QFormLayout *formLayout = new QFormLayout(dialog);
    formLayout->addRow(tr("&Name:"), nameLineEdit);
    formLayout->addRow(tr("&Url:"), urlLineEdit);
    formLayout->addRow(tr("&Icon:"), iconLineEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    formLayout->addWidget(buttonBox);
    dialog->setLayout(formLayout);

    TreeItemIndex treeItemIndex;
    if(treeItemVariant.isValid())
    {
        treeItemIndex = treeItemVariant.value<TreeItemIndex>();

        auto index = treeItemIndex.index;
        auto parentItem = treeItemIndex.treeItem;

        connect(dialog, &QDialog::finished, this,
                [dialog, nameLineEdit, urlLineEdit, iconLineEdit, index, parentItem, this](int result){
            if(result == QDialog::DialogCode::Accepted)
            {
                auto channel = DatabaseProvider::GetDatabase()->AddChannel(nameLineEdit->text(), urlLineEdit->text(), iconLineEdit->text(), parentItem->getID());
                model->AddChild(channel, index);
            }
            dialog->deleteLater();
        });
    }
    else
    {
        //we're adding to root
        connect(dialog, &QDialog::finished, this,
                [dialog, nameLineEdit, urlLineEdit, iconLineEdit, this](int result){
            if(result == QDialog::DialogCode::Accepted)
            {
                auto channel = DatabaseProvider::GetDatabase()->AddChannel(nameLineEdit->text(), urlLineEdit->text(), iconLineEdit->text(), {});
                model->AddChild(channel, QModelIndex{});
            }
            dialog->deleteLater();
        });
    }

    dialog->open();
}
void ChannelsWidget::onRemoveChannel()
{
    auto treeItemIndex = removeChannelAction->data().value<TreeItemIndex>();
    if(treeItemIndex.treeItem != nullptr && treeItemIndex.index.isValid())
    {
        auto selectedButton = QMessageBox::question(this, "Confirm",QString("Are you sure you want to delete channel %1?").arg(treeItemIndex.treeItem->getName()));
        if(selectedButton == QMessageBox::StandardButton::Yes)
        {
            DatabaseProvider::GetDatabase()->RemoveChannel(treeItemIndex.treeItem->getID());
            model->RemoveChild(treeItemIndex.treeItem, treeItemIndex.index);
        }
    }
}
void ChannelsWidget::onRemoveChannelGroup()
{
    auto treeItemIndex = removeChannelGroupAction->data().value<TreeItemIndex>();
    if(treeItemIndex.treeItem != nullptr && treeItemIndex.index.isValid())
    {
        auto selectedButton = QMessageBox::question(this, "Confirm",QString("Are you sure you want to delete channel group %1 and all its channels?").arg(treeItemIndex.treeItem->getName()));
        if(selectedButton == QMessageBox::StandardButton::Yes)
        {
            DatabaseProvider::GetDatabase()->RemoveGroup(treeItemIndex.treeItem->getID());
            model->RemoveChild(treeItemIndex.treeItem, treeItemIndex.index);
        }
    }
}
void ChannelsWidget::onAddNewChannelGroup()
{
    auto treeItemVariant = addNewChannelGroupAction->data();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add new group"),
                                         tr("Group name:"), QLineEdit::Normal,
                                         "", &ok);
    std::optional<int64_t> parentGroupId;
    TreeItemIndex treeItemIndex;
    if(treeItemVariant.isValid())
    {
        treeItemIndex = treeItemVariant.value<TreeItemIndex>();
        parentGroupId.emplace(treeItemIndex.treeItem->getID());
    }
    if(ok && !text.isEmpty())
    {
        auto group = DatabaseProvider::GetDatabase()->AddGroup(text, parentGroupId);
        model->AddChild(group, treeItemIndex.index);
    }
}

void ChannelsWidget::CancelImportChannels()
{
    model->CancelImportChannels();
}

void ChannelsWidget::SkipForward()
{
    if(!channels->selectionModel()->hasSelection()) return;
    auto currentSelectedIndex = channels->selectionModel()->currentIndex();
    if(!currentSelectedIndex.isValid()) return;
    auto nextIndex = currentSelectedIndex.siblingAtRow(currentSelectedIndex.row()+1);
    if(!nextIndex.isValid()) return;
    auto id = nextIndex.data(ChannelsModel::ChannelRoles::IdRole);
    if(!id.isValid()) return;
    channels->selectionModel()->clearSelection();
    channels->selectionModel()->setCurrentIndex(nextIndex,QItemSelectionModel::SelectionFlag::Select);
    emit playChannel(id.toLongLong());

    auto nextIndex2 = nextIndex.siblingAtRow(nextIndex.row()+1);
    auto backIndex = nextIndex.siblingAtRow(nextIndex.row()-1);
    emit enableSkipForward(nextIndex2.isValid() && nextIndex2.data(ChannelsModel::ChannelRoles::IdRole).isValid());
    emit enableSkipBack(backIndex.isValid() && backIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
}
void ChannelsWidget::SkipBack()
{
    if(!channels->selectionModel()->hasSelection()) return;
    auto currentSelectedIndex = channels->selectionModel()->currentIndex();
    if(!currentSelectedIndex.isValid()) return;
    auto backIndex = currentSelectedIndex.siblingAtRow(currentSelectedIndex.row()-1);
    if(!backIndex.isValid()) return;
    auto id = backIndex.data(ChannelsModel::ChannelRoles::IdRole);
    if(!id.isValid()) return;
    channels->selectionModel()->clearSelection();
    channels->selectionModel()->setCurrentIndex(backIndex,QItemSelectionModel::SelectionFlag::Select);
    emit playChannel(id.toLongLong());

    auto nextIndex = backIndex.siblingAtRow(backIndex.row()+1);
    auto backIndex2 = backIndex.siblingAtRow(backIndex.row()-1);
    emit enableSkipForward(nextIndex.isValid() && nextIndex.data(ChannelsModel::ChannelRoles::IdRole).isValid());
    emit enableSkipBack(backIndex2.isValid() && backIndex2.data(ChannelsModel::ChannelRoles::IdRole).isValid());

}


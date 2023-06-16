#include "database.h"
#include "roottreeitem.h"
#include "channeltreeitem.h"
#include "grouptreeitem.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

Database::Database(ConstructorKey, const std::filesystem::path& dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath.string().c_str());
    if(!db.open())
    {
        throw DatabaseException(QString{("Cannot open database "+dbPath.string()).c_str()});
    }

    executeStatement("PRAGMA foreign_keys = ON", "Cannot enable foreign_keys support");
    executeStatement("CREATE TABLE IF NOT EXISTS SCHEMA_VERSION(VERSION INT)", "Cannot create table SCHEMA_VERSION");
    executeStatement("CREATE TABLE IF NOT EXISTS CHANNEL_GROUPS(GROUP_ID INTEGER NOT NULL PRIMARY KEY, "
                     "PARENT_GROUP_ID INTEGER, NAME TEXT, FOREIGN KEY (PARENT_GROUP_ID) REFERENCES CHANNEL_GROUPS(GROUP_ID))", "Cannot create table CHANNEL_GROUPS");
    executeStatement("CREATE TABLE IF NOT EXISTS CHANNELS(CHANNEL_ID INTEGER NOT NULL PRIMARY KEY, "
                     "GROUP_ID INTEGER, NAME TEXT, URI TEXT, LOGO_URI TEXT, LOGO BLOB, FAVOURITE BOOLEAN DEFAULT 0 NOT NULL CHECK (FAVOURITE IN (0, 1)),"
                     "FOREIGN KEY (GROUP_ID) REFERENCES CHANNEL_GROUPS(GROUP_ID))", "Cannot create table CHANNELS");

    /*int version = getSchemaVersion();
    if(version < 1)
    {
        incrementSchemaVersion(version);
        rc = sqlite3_exec(db.get(),"ALTER TABLE USER ADD COLUMN LAST_LOGGEDIN INT",nullptr,nullptr,nullptr);
        DB_ERR_CHECK(rc, "Cannot alter table USER");
    }*/
}
void Database::executeStatement(const char* sql, const char* errMsg) const
{
    QSqlQuery query{sql};
    if(!query.exec())
    {
        throw DatabaseException(errMsg + query.lastError().text());
    }
}

std::unique_ptr<ChannelTreeItem> Database::GetChannel(int64_t id) const
{
    QSqlQuery query;
    query.prepare("SELECT NAME,URI,LOGO_URI,LOGO,FAVOURITE FROM CHANNELS WHERE CHANNEL_ID = ?");
    query.bindValue(0, static_cast<qlonglong>(id));

    if(!query.exec())
    {
        throw DatabaseException("Cannot select from CHANNELS " + query.lastError().text());
    }

    if(query.next())
    {
        QString name = query.value(0).toString();
        QString uri = query.value(1).toString();
        QString logoUri = query.value(2).toString();
        QByteArray logo = query.value(3).toByteArray();

        return std::make_unique<ChannelTreeItem>(name,uri,logoUri,logo,nullptr,nullptr);
    }
    return std::unique_ptr<ChannelTreeItem>{};
}

void Database::LoadChannelsAndGroups(RootTreeItem* rootItem) const
{
    QSqlQuery query;

    if(!query.exec("SELECT CHANNEL_ID,GROUP_ID,NAME,URI,LOGO_URI,LOGO,FAVOURITE FROM CHANNELS ORDER BY CHANNEL_ID"))
    {
        throw DatabaseException("Cannot select from CHANNELS " + query.lastError().text());
    }
    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        auto groupVariant = query.value(1);
        std::optional<int64_t> groupId;
        if(!groupVariant.isNull())
        {
            groupId = groupVariant.toLongLong();
        }

        QString name = query.value(2).toString();
        QString uri = query.value(3).toString();
        QString logoUri = query.value(4).toString();
        QByteArray logo = query.value(5).toByteArray();
        bool favourite = query.value(6).toBool();
        std::unique_ptr<ChannelTreeItem> channel;
        ChannelTreeItem* channelPtr = nullptr;
        if(groupId)
        {
            auto group = rootItem->getGroup(groupId.value());
            if(group == nullptr)
            {
                group = loadGroup(groupId.value(), rootItem);
            }
            channel = std::make_unique<ChannelTreeItem>(std::move(name), std::move(uri), std::move(logoUri), std::move(logo), rootItem->getNetworkManager(), group);
            channel->setID(id);
            channelPtr = channel.get();
            group->addChannel(std::move(channel));
        }
        else
        {
            channel = std::make_unique<ChannelTreeItem>(std::move(name), std::move(uri), std::move(logoUri), std::move(logo), rootItem->getNetworkManager(), rootItem);
            channel->setID(id);
            channelPtr = channel.get();
            rootItem->addChannel(std::move(channel));
        }
        if(favourite)
        {
            rootItem->addToFavourites(channelPtr);
        }
    }
}

GroupTreeItem* Database::loadGroup(int64_t id, RootTreeItem* rootItem) const
{
    QSqlQuery query;
    query.prepare("SELECT PARENT_GROUP_ID,NAME FROM CHANNEL_GROUPS WHERE GROUP_ID=?");
    query.bindValue(0, static_cast<qlonglong>(id));
    if(!query.exec())
    {
        throw DatabaseException("Cannot select from CHANNEL_GROUPS " + query.lastError().text());
    }

    if(query.next())
    {
        GroupTreeItem* groupPtr = nullptr;
        auto groupVariant = query.value(0);
        std::optional<int64_t> parentGroupId;
        if(!groupVariant.isNull())
        {
            parentGroupId = groupVariant.toLongLong();
        }

        QString name = query.value(1).toString();

        if(parentGroupId)
        {
            auto parentGroup = loadGroup(parentGroupId.value(), rootItem);
            auto group = std::make_unique<GroupTreeItem>(std::move(name), rootItem->getNetworkManager(), parentGroup);
            group->setID(id);
            groupPtr = group.get();
            parentGroup->addGroup(std::move(group));
        }
        else
        {
            auto group = std::make_unique<GroupTreeItem>(std::move(name), rootItem->getNetworkManager(), rootItem);
            group->setID(id);
            groupPtr = group.get();
            rootItem->addGroup(std::move(group));
        }
        return groupPtr;
    }
    return nullptr;
}

void Database::SetChannelLogo(ChannelTreeItem* channel) const
{
    QSqlQuery query;
    query.prepare("UPDATE CHANNELS SET LOGO =? WHERE CHANNEL_ID =?");

    query.bindValue(0, channel->getLogo());
    query.bindValue(1, static_cast<qlonglong>(channel->getID()));

    if(!query.exec())
    {
        throw DatabaseException("Cannot update CHANNELS " + query.lastError().text());
    }
}

void Database::AddChannelAndGroup(ChannelTreeItem* channel) const
{
    auto parent = channel->getParent();
    if(parent && parent->getType() == ChannelTreeItemType::Group)
    {
        auto group = dynamic_cast<GroupTreeItem*>(parent);
        assert(((void)"AbstractChannelTreeItem of type Group is not GroupTreeItem", group != nullptr));
        if(group->getID() < 0)
        {
            addGroupTree(group);
        }
        addChannel(channel, group->getID());
    }
    else
    {
        addChannel(channel, {});
    }
}

void Database::addChannel(ChannelTreeItem* channel, std::optional<int64_t> groupId) const
{
    QSqlQuery query;
    query.prepare("INSERT INTO CHANNELS(NAME, URI, LOGO_URI, LOGO, FAVOURITE, GROUP_ID) VALUES (?,?,?,?,0,?) RETURNING CHANNEL_ID");

    query.bindValue(0, channel->getName());
    query.bindValue(1, channel->getUri());
    query.bindValue(2, channel->getLogoUri());
    query.bindValue(3, channel->getLogo());

    if(groupId)
    {
        query.bindValue(4, static_cast<qlonglong>(groupId.value()));
    }
    else
    {
        query.bindValue(4, QVariant{});
    }

    if(!query.exec())
    {
        throw DatabaseException("Cannot insert into CHANNELS " + query.lastError().text());
    }

    if(query.next())
    {
        channel->setID(query.value(0).toLongLong());
    }
}

void Database::addGroup(GroupTreeItem* group, std::optional<int64_t> parentGroupId) const
{
    QSqlQuery query;
    query.prepare("INSERT INTO CHANNEL_GROUPS(NAME, PARENT_GROUP_ID) VALUES (?,?) RETURNING GROUP_ID");

    query.bindValue(0, group->getName());

    if(parentGroupId)
    {
        query.bindValue(1, static_cast<qlonglong>(parentGroupId.value()));
    }
    else
    {
        query.bindValue(1, QVariant{});
    }

    if(!query.exec())
    {
        throw DatabaseException("Cannot insert into groups " + query.lastError().text());
    }

    if(query.next())
    {
        group->setID(query.value(0).toLongLong());
    }
}

void Database::SetFavourite(int64_t id, bool flag) const
{
    QSqlQuery query;
    query.prepare("UPDATE CHANNELS SET FAVOURITE=? WHERE CHANNEL_ID=?");

    query.bindValue(0, flag);
    query.bindValue(1, static_cast<qlonglong>(id));

    if(!query.exec())
    {
        throw DatabaseException("Cannot update CHANNELS " + query.lastError().text());
    }
}

void Database::addGroupTree(GroupTreeItem* group) const
{
    auto parent = group->getParent();
    if(parent && parent->getType() == ChannelTreeItemType::Group)
    {
        auto parentGroup = dynamic_cast<GroupTreeItem*>(parent);
        assert(((void)"AbstractChannelTreeItem of type Group is not GroupTreeItem", parentGroup != nullptr));
        addGroupTree(parentGroup);
        addGroup(group, parentGroup->getID());
    }
    else
    {
        addGroup(group, {});
    }
}

void Database::WithTransaction(std::function<void()> callback) const
{
    auto db = QSqlDatabase::database();
    try
    {
        db.transaction();
        callback();
        db.commit();
    }
    catch(...)
    {
        db.rollback();
    }
}

int Database::getSchemaVersion() const
{
    QSqlQuery query;
    query.prepare("SELECT VERSION FROM SCHEMA_VERSION");
    if(!query.exec())
    {
        throw DatabaseException("Cannot find SCHEMA_VERSION " + query.lastError().text());
    }

    if(query.next())
    {
        return query.value(0).toInt();
    }
    return 0;
}
void Database::incrementSchemaVersion(int version) const
{
    if(version == 0)
    {
        executeStatement("INSERT INTO SCHEMA_VERSION (VERSION) VALUES(1)", "Cannot add version");
    }
    else
    {
        executeStatement(("UPDATE SCHEMA_VERSION SET VERSION="+std::to_string(version+1)).c_str(), "Cannot update version");
    }
}

#include "database.h"
#include "roottreeitem.h"
#include "channeltreeitem.h"
#include "grouptreeitem.h"
#include "favouritestreeitem.h"

#include <cassert>

namespace
{
#define DB_ERR_CHECK(return_code__, msg) \
    if (return_code__ != SQLITE_OK && return_code__ < SQLITE_ROW) \
    {\
        std::string err = sqlite3_errmsg(db.get());\
        int code = sqlite3_errcode(db.get()); \
        throw DatabaseException(std::string(msg)+":"+err+"("+std::to_string(code)+")");\
    }

    struct StatementFinalizer
    {
        void operator()(sqlite3_stmt* stm)
        {
            if(stm)
            {
                sqlite3_finalize(stm);
            }
        }
    };
}
Database::Database(ConstructorKey, const std::filesystem::path& dbPath) : db{nullptr, Database::DBConnectionDeleter{}}
{
    sqlite3* db_ptr;
    int rc = sqlite3_open(dbPath.string().c_str(),&db_ptr);
    DB_ERR_CHECK(rc, "Cannot open database");
    db.reset(db_ptr);
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
    int rc = sqlite3_exec(db.get(),sql,nullptr,nullptr,nullptr);
    DB_ERR_CHECK(rc, errMsg);
}

void Database::LoadChannelsAndGroups(RootTreeItem* rootItem) const
{
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"SELECT CHANNEL_ID,GROUP_ID,NAME,URI,LOGO_URI,LOGO,FAVOURITE FROM CHANNELS ORDER BY CHANNEL_ID",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot select from CHANNELS");
    while(sqlite3_step(stmt.get()) == SQLITE_ROW)
    {
        int64_t id = sqlite3_column_int64(stmt.get(), 0);
        auto groupType = sqlite3_column_type(stmt.get(), 1);
        std::optional<int64_t> groupId;
        if(groupType == SQLITE_INTEGER)
        {
            groupId = sqlite3_column_int64(stmt.get(), 1);
        }
        QByteArray nameArray{reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(),2)), sqlite3_column_bytes(stmt.get(),2)};
        QString name{nameArray};
        QByteArray uriArray{reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(),3)), sqlite3_column_bytes(stmt.get(),3)};
        QString uri{uriArray};
        QByteArray logoUriArray{reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(),4)), sqlite3_column_bytes(stmt.get(),4)};
        QString logoUri{logoUriArray};
        QByteArray logo{reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(),5)), sqlite3_column_bytes(stmt.get(),5)};
        bool favourite = sqlite3_column_int64(stmt.get(), 6) ? true : false;
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
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"SELECT PARENT_GROUP_ID,NAME FROM CHANNEL_GROUPS WHERE GROUP_ID=?",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot select from CHANNEL_GROUPS");
    rc = sqlite3_bind_int64(stmt.get(), 1,id);
    DB_ERR_CHECK(rc, "Cannot bind values to id");

    if(sqlite3_step(stmt.get()) == SQLITE_ROW)
    {
        GroupTreeItem* groupPtr = nullptr;
        auto parentGroupType = sqlite3_column_type(stmt.get(), 0);
        std::optional<int64_t> parentGroupId;
        if(parentGroupType == SQLITE_INTEGER)
        {
            parentGroupId = sqlite3_column_int64(stmt.get(), 0);
        }
        QByteArray nameArray{reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(),1)), sqlite3_column_bytes(stmt.get(),1)};
        QString name{nameArray};

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
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"UPDATE CHANNELS SET LOGO =? WHERE CHANNEL_ID =?",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot update CHANNELS");
    auto const& logo = channel->getLogo();
    if(logo.isEmpty())
    {
        rc = sqlite3_bind_null(stmt.get(), 1);
    }
    else
    {
        rc = sqlite3_bind_blob(stmt.get(), 1, logo.data(), logo.size(), nullptr);
    }
    DB_ERR_CHECK(rc, "Cannot bind values to channel logo");
    rc = sqlite3_bind_int64(stmt.get(), 2, channel->getID());
    DB_ERR_CHECK(rc, "Cannot bind values to channel ID");
    rc = sqlite3_step(stmt.get());
    DB_ERR_CHECK(rc, "Cannot update channel");
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
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"INSERT INTO CHANNELS(NAME, URI, LOGO_URI, LOGO, FAVOURITE, GROUP_ID) VALUES (?,?,?,?,0,?) RETURNING CHANNEL_ID",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot insert CHANNELS");
    auto const nameArray = channel->getName().toLocal8Bit();
    rc = sqlite3_bind_text(stmt.get(), 1,nameArray.data(),nameArray.size(),nullptr);
    DB_ERR_CHECK(rc, "Cannot bind values to "+channel->getName().toStdString());
    auto const uriArray = channel->getUri().toLocal8Bit();
    rc = sqlite3_bind_text(stmt.get(), 2,uriArray.data(),uriArray.size(),nullptr);
    DB_ERR_CHECK(rc, "Cannot bind values to "+channel->getUri().toStdString());
    auto const logoUriArray = channel->getLogoUri().toLocal8Bit();
    rc = sqlite3_bind_text(stmt.get(), 3,logoUriArray.data(),logoUriArray.size(),nullptr);
    DB_ERR_CHECK(rc, "Cannot bind values to "+channel->getLogoUri().toStdString());
    auto const& logo = channel->getLogo();
    if(logo.isEmpty())
    {
        rc = sqlite3_bind_null(stmt.get(), 4);
    }
    else
    {
        rc = sqlite3_bind_blob(stmt.get(), 4, logo.data(), logo.size(), nullptr);
    }
    DB_ERR_CHECK(rc, "Cannot bind values to channel logo");

    if(groupId)
    {
        rc = sqlite3_bind_int64(stmt.get(), 5, groupId.value());
    }
    else
    {
        rc = sqlite3_bind_null(stmt.get(), 5);
    }
    DB_ERR_CHECK(rc, "Cannot bind values to "+channel->getName().toStdString());
    if(sqlite3_step(stmt.get()) == SQLITE_ROW)
    {
        channel->setID(sqlite3_column_int64(stmt.get(),0));
    }
}

void Database::addGroup(GroupTreeItem* group, std::optional<int64_t> parentGroupId) const
{
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"INSERT INTO CHANNEL_GROUPS(NAME, PARENT_GROUP_ID) VALUES (?,?) RETURNING GROUP_ID",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot insert CHANNEL_GROUPS");
    auto const nameArray = group->getName().toLocal8Bit();
    rc = sqlite3_bind_text(stmt.get(), 1,nameArray.data(),nameArray.size(),nullptr);
    DB_ERR_CHECK(rc, "Cannot bind values to "+group->getName().toStdString());
    if(parentGroupId)
    {
        rc = sqlite3_bind_int64(stmt.get(), 2, parentGroupId.value());
    }
    else
    {
        rc = sqlite3_bind_null(stmt.get(), 2);
    }
    DB_ERR_CHECK(rc, "Cannot bind values to "+group->getName().toStdString());
    if(sqlite3_step(stmt.get()) == SQLITE_ROW)
    {
        group->setID(sqlite3_column_int64(stmt.get(),0));
    }
}

void Database::SetFavourite(int64_t id, bool flag) const
{
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"UPDATE CHANNELS SET FAVOURITE=? WHERE CHANNEL_ID=?",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot update CHANNELS");
    rc = sqlite3_bind_int(stmt.get(), 1, flag?1:0);
    DB_ERR_CHECK(rc, "Cannot bind values to favourite");
    rc = sqlite3_bind_int64(stmt.get(), 2, id);
    DB_ERR_CHECK(rc, "Cannot bind values to id");
    rc = sqlite3_step(stmt.get());
    DB_ERR_CHECK(rc, "Cannot update channel");
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
    executeStatement("BEGIN TRANSACTION", "Cannot begin transaction");
    try
    {
        callback();
        executeStatement("COMMIT TRANSACTION", "Cannot commit transaction");
    }
    catch(...)
    {
        executeStatement("ROLLBACK TRANSACTION", "Cannot rollback transaction");
    }
}

int Database::getSchemaVersion() const
{
    std::unique_ptr<sqlite3_stmt,StatementFinalizer> stmt;
    sqlite3_stmt *stmt_ptr;
    int rc = sqlite3_prepare_v2(db.get(),"SELECT VERSION FROM SCHEMA_VERSION",-1,&stmt_ptr, nullptr);
    stmt.reset(stmt_ptr);
    DB_ERR_CHECK(rc, "Cannot find SCHEMA_VERSION");
    if(sqlite3_step(stmt.get()) == SQLITE_ROW)
    {
        return sqlite3_column_int(stmt.get(),0);
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

#include "database.h"
#include "roottreeitem.h"
#include "channeltreeitem.h"
#include "grouptreeitem.h"


#include <QSqlQuery>
#include <QSqlError>
#include <QThread>

Database::Database(ConstructorKey, const std::filesystem::path& dbPath)
{
    auto name = QString("DB%1").arg(qintptr(QThread::currentThreadId()));
    if(QSqlDatabase::contains(name))
    {
        db = QSqlDatabase::database(name);
        if(!db.isValid())
        {
            db = QSqlDatabase::addDatabase("QSQLITE", name);
            db.setConnectOptions("SQLITE_CONFIG_SERIALIZED");
            db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=30000");
            db.setDatabaseName(dbPath.string().c_str());
        }
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", name);
        db.setConnectOptions("SQLITE_CONFIG_SERIALIZED");
        db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=30000");
        db.setDatabaseName(dbPath.string().c_str());
    }
    if(!db.isValid() || !db.open())
    {
        throw DatabaseException(QString{("Cannot open database "+dbPath.string()).c_str()});
    }

    executeStatement("PRAGMA journal_mode=WAL", "Cannot enable WAL journal mode");
    executeStatement("PRAGMA foreign_keys = ON", "Cannot enable foreign_keys support");
}
void Database::SetupSchema()
{
    executeStatement("CREATE TABLE IF NOT EXISTS SCHEMA_VERSION(VERSION INT)", "Cannot create table SCHEMA_VERSION");
    executeStatement("CREATE TABLE IF NOT EXISTS CHANNEL_GROUPS(GROUP_ID INTEGER NOT NULL PRIMARY KEY, "
                     "PARENT_GROUP_ID INTEGER, NAME TEXT, FOREIGN KEY (PARENT_GROUP_ID) REFERENCES CHANNEL_GROUPS(GROUP_ID))", "Cannot create table CHANNEL_GROUPS");
    executeStatement("CREATE TABLE IF NOT EXISTS CHANNELS(CHANNEL_ID INTEGER NOT NULL PRIMARY KEY, "
                     "GROUP_ID INTEGER, NAME TEXT, URI TEXT, LOGO_URI TEXT, LOGO BLOB, FAVOURITE BOOLEAN DEFAULT 0 NOT NULL CHECK (FAVOURITE IN (0, 1)),"
                     "FOREIGN KEY (GROUP_ID) REFERENCES CHANNEL_GROUPS(GROUP_ID))", "Cannot create table CHANNELS");

    int version = getSchemaVersion();
    if(version < 1)
    {
        executeStatement("ALTER TABLE CHANNELS ADD COLUMN EPG_CHANNEL_URI TEXT", "Cannot alter table CHANNELS");
        executeStatement("ALTER TABLE CHANNELS ADD COLUMN EPG_CHANNEL_ID TEXT", "Cannot alter table CHANNELS");
        incrementSchemaVersion(version);
    }
}
void Database::executeStatement(const char* sql, const char* errMsg) const
{
    QSqlQuery query{db};
    if(!query.exec(sql))
    {
        throw DatabaseException(errMsg + query.lastError().text());
    }
}
int Database::GetGroupsCount()
{
    QSqlQuery query{db};
    if(!query.exec("SELECT COUNT(*) FROM CHANNEL_GROUPS WHERE PARENT_GROUP_ID IS NULL"))
    {
        throw DatabaseException("Cannot select from CHANNEL_GROUPS " + query.lastError().text());
    }
    if(query.next())
    {
        return query.value(0).toInt();
    }
    return 0;
}
GroupTreeItem* Database::AddGroup(const QString& text, std::optional<int64_t> parentGroupId) const
{
    auto group = new GroupTreeItem(text, (RootTreeItem*)nullptr);
    addGroup(group,parentGroupId);
    return group;
}
std::vector<ChannelTreeItem*> Database::GetFavouriteChannels() const
{
    std::vector<ChannelTreeItem*> channels;
    QSqlQuery query{db};
    query.prepare("SELECT CHANNEL_ID,NAME,URI,LOGO_URI,LOGO,EPG_CHANNEL_ID,EPG_CHANNEL_URI FROM CHANNELS WHERE FAVOURITE = 1");

    if(!query.exec())
    {
        throw DatabaseException("Cannot select from CHANNELS " + query.lastError().text());
    }

    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        QString name = query.value(1).toString();
        QString uri = query.value(2).toString();
        QString logoUri = query.value(3).toString();
        QByteArray logo = query.value(4).toByteArray();
        QString epgChannelId = query.value(5).toString();
        QString epgChannelUri = query.value(6).toString();
        auto channel = new ChannelTreeItem(name,uri,logoUri,logo,nullptr);
        channel->setID(id);
        channel->setEpgChannelId(epgChannelId);
        channel->setEpgChannelUri(epgChannelUri);
        channels.push_back(channel);
    }
    return channels;
}
ChannelTreeItem* Database::GetChannel(int64_t id) const
{
    QSqlQuery query{db};
    query.prepare("SELECT NAME,URI,LOGO_URI,LOGO,EPG_CHANNEL_ID,EPG_CHANNEL_URI FROM CHANNELS WHERE CHANNEL_ID = ?");
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
        QString epgChannelId = query.value(4).toString();
        QString epgChannelUri = query.value(5).toString();

        auto channel = new ChannelTreeItem(name,uri,logoUri,logo,nullptr);
        channel->setEpgChannelId(epgChannelId);
        channel->setEpgChannelUri(epgChannelUri);

        return channel;
    }
    return nullptr;
}

void Database::LoadChannelsAndGroups(RootTreeItem* rootItem) const
{
    loadAllGroups(rootItem);
    int childCount = rootItem->childCount();
    for(int i=0;i<childCount;i++)
    {
        auto child = rootItem->child(i);
        if(child->getType() == ChannelTreeItemType::Group)
        {
            loadGroupsChannels(dynamic_cast<GroupTreeItem*>(child));
        }
    }
    loadRootChannels(rootItem);
}
std::vector<GroupTreeItem*> Database::GetAllGroups() const
{
    std::vector<GroupTreeItem*> groups;
    QSqlQuery query{db};
    if(!query.exec("SELECT GROUP_ID,NAME FROM CHANNEL_GROUPS WHERE PARENT_GROUP_ID IS NULL ORDER BY GROUP_ID"))
    {
        throw DatabaseException("Cannot select from CHANNEL_GROUPS " + query.lastError().text());
    }
    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        QString name = query.value(1).toString();
        auto group = new GroupTreeItem(name);
        group->setID(id);
        loadChildGroups(group);
        groups.push_back(group);
    }
    return groups;
}
void Database::LoadChannels(GroupTreeItem* parentItem) const
{
    loadGroupsChannels(parentItem);
}
void Database::loadRootChannels(RootTreeItem* rootItem) const
{
    QSqlQuery query{db};
    if(!query.exec("SELECT CHANNEL_ID,NAME,URI,LOGO_URI,LOGO,FAVOURITE,EPG_CHANNEL_ID,EPG_CHANNEL_URI FROM CHANNELS WHERE GROUP_ID IS NULL ORDER BY CHANNEL_ID"))
    {
        throw DatabaseException("Cannot select from CHANNELS " + query.lastError().text());
    }
    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        QString name = query.value(1).toString();
        QString uri = query.value(2).toString();
        QString logoUri = query.value(3).toString();
        QByteArray logo = query.value(4).toByteArray();
        bool favourite = query.value(5).toBool();
        QString epgChannelId = query.value(6).toString();
        QString epgChannelUri = query.value(7).toString();

        auto channel = new ChannelTreeItem(std::move(name), std::move(uri), std::move(logoUri), std::move(logo), rootItem);
        channel->setID(id);
        channel->setEpgChannelId(epgChannelId);
        channel->setEpgChannelUri(epgChannelUri);

        rootItem->addChannel(channel);
        if(favourite)
        {
            rootItem->addToFavourites(channel);
        }
    }
}
void Database::loadGroupsChannels(GroupTreeItem* parent) const
{
    QSqlQuery query{db};
    query.prepare("SELECT CHANNEL_ID,NAME,URI,LOGO_URI,LOGO,FAVOURITE,EPG_CHANNEL_ID,EPG_CHANNEL_URI FROM CHANNELS WHERE GROUP_ID = ? ORDER BY CHANNEL_ID");
    query.bindValue(0, static_cast<qlonglong>(parent->getID()));

    if(!query.exec())
    {
        throw DatabaseException("Cannot select from CHANNELS " + query.lastError().text());
    }
    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        QString name = query.value(1).toString();
        QString uri = query.value(2).toString();
        QString logoUri = query.value(3).toString();
        QByteArray logo = query.value(4).toByteArray();
        bool favourite = query.value(5).toBool();
        QString epgChannelId = query.value(6).toString();
        QString epgChannelUri = query.value(7).toString();

        auto channel = new ChannelTreeItem(std::move(name), std::move(uri), std::move(logoUri), std::move(logo), parent);
        channel->setFavourite(favourite);
        channel->setID(id);
        channel->setEpgChannelId(epgChannelId);
        channel->setEpgChannelUri(epgChannelUri);

        parent->addChannel(channel);
    }
}
void Database::loadAllGroups(RootTreeItem* rootItem) const
{
    QSqlQuery query{db};
    if(!query.exec("SELECT GROUP_ID,NAME FROM CHANNEL_GROUPS WHERE PARENT_GROUP_ID IS NULL ORDER BY GROUP_ID"))
    {
        throw DatabaseException("Cannot select from CHANNEL_GROUPS " + query.lastError().text());
    }
    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        QString name = query.value(1).toString();
        auto group = new GroupTreeItem(std::move(name), rootItem);
        group->setID(id);
        loadChildGroups(group);
        rootItem->addGroup(group);
    }
}
void Database::loadChildGroups(GroupTreeItem* parent) const
{
    QSqlQuery query{db};
    query.prepare("SELECT GROUP_ID,NAME FROM CHANNEL_GROUPS WHERE PARENT_GROUP_ID = ? ORDER BY GROUP_ID");
    query.bindValue(0, static_cast<qlonglong>(parent->getID()));
    if(!query.exec())
    {
        throw DatabaseException("Cannot select from CHANNEL_GROUPS " + query.lastError().text());
    }
    while(query.next())
    {
        int64_t id = query.value(0).toLongLong();
        QString name = query.value(1).toString();
        auto group = new GroupTreeItem(std::move(name), parent);
        group->setID(id);
        loadChildGroups(group);
        parent->addGroup(group);
    }
}

void Database::SetChannelLogo(ChannelTreeItem* channel) const
{
    QSqlQuery query{db};
    query.prepare("UPDATE CHANNELS SET LOGO =? WHERE CHANNEL_ID =?");

    query.bindValue(0, channel->getLogo());
    query.bindValue(1, static_cast<qlonglong>(channel->getID()));

    if(!query.exec())
    {
        throw DatabaseException("Cannot update CHANNELS " + query.lastError().text());
    }
}
ChannelTreeItem* Database::AddChannel(const QString& name,const QString& url,const QString& icon, std::optional<int64_t> parentGroupId) const
{
    auto channel = new ChannelTreeItem(name, url, icon, QByteArray{}, nullptr);
    addChannel(channel, parentGroupId);
    return channel;
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
    QSqlQuery query{db};
    query.prepare("INSERT INTO CHANNELS(NAME, URI, LOGO_URI, LOGO, FAVOURITE, GROUP_ID,EPG_CHANNEL_ID,EPG_CHANNEL_URI) VALUES (?,?,?,?,0,?,?,?) RETURNING CHANNEL_ID");

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
    query.bindValue(5, channel->getEpgChannelId());
    query.bindValue(6, channel->getEpgChannelUri());

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
    QSqlQuery query{db};
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
    QSqlQuery query{db};
    query.prepare("UPDATE CHANNELS SET FAVOURITE=? WHERE CHANNEL_ID=?");

    query.bindValue(0, flag);
    query.bindValue(1, static_cast<qlonglong>(id));

    if(!query.exec())
    {
        throw DatabaseException("Cannot update CHANNELS " + query.lastError().text());
    }
}
void Database::AddGroup(GroupTreeItem* group)
{
    WithTransaction([group, this](){
        addGroup(group, {});
        for(int i=0;i<group->childCount();i++)
        {
            auto child = group->child(i);
            if(child->getType() == ChannelTreeItemType::Group)
            {
                addGroup(static_cast<GroupTreeItem*>(child), group->getID());
            }
            else if (child->getType() == ChannelTreeItemType::Channel)
            {
                addChannel(static_cast<ChannelTreeItem*>(child), group->getID());
            }
        }
    });
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

void Database::WithTransaction(std::function<void()> callback)
{
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
    QSqlQuery query{db};
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
void Database::RemoveGroup(int64_t id) const
{
    {
        QSqlQuery query{db};
        query.prepare("DELETE FROM CHANNELS WHERE GROUP_ID=?");
        query.bindValue(0, static_cast<qlonglong>(id));
        if(!query.exec())
        {
            throw DatabaseException("Cannot delete from CHANNELS " + query.lastError().text());
        }
    }

    {
        QSqlQuery query{db};
        query.prepare("SELECT GROUP_ID FROM CHANNEL_GROUPS WHERE PARENT_GROUP_ID = ?");
        query.bindValue(0, static_cast<qlonglong>(id));
        if(!query.exec())
        {
            throw DatabaseException("Cannot select from CHANNEL_GROUPS " + query.lastError().text());
        }
        while(query.next())
        {
            int64_t id = query.value(0).toLongLong();
            RemoveGroup(id);
        }
    }

    {
        QSqlQuery query{db};
        query.prepare("DELETE FROM CHANNEL_GROUPS WHERE GROUP_ID = ?");
        query.bindValue(0, static_cast<qlonglong>(id));
        if(!query.exec())
        {
            throw DatabaseException("Cannot delete from CHANNEL_GROUPS " + query.lastError().text());
        }
    }
}
void Database::RemoveChannel(int64_t id) const
{
    QSqlQuery query{db};
    query.prepare("DELETE FROM CHANNELS WHERE CHANNEL_ID=?");
    query.bindValue(0, static_cast<qlonglong>(id));
    if(!query.exec())
    {
        throw DatabaseException("Cannot delete from CHANNELS " + query.lastError().text());
    }
}

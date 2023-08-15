#ifndef DATABASE_H
#define DATABASE_H

#include <filesystem>
#include <stdexcept>
#include <functional>
#include <optional>
#include <QString>
#include <QSqlDatabase>
#include "xstreaminfo.h"

class DatabaseProvider;
class ChannelTreeItem;
class GroupTreeItem;
class RootTreeItem;

class DatabaseException : public std::runtime_error
{
public:
    DatabaseException(const QString& message): std::runtime_error(message.toLocal8Bit().data())
    {
    }
};


class Database
{
private:
friend class DatabaseProvider;
struct ConstructorKey{};

public:
    Database(ConstructorKey, const std::filesystem::path& dbPath);
    void WithTransaction(std::function<void()> callback);
    void AddChannelAndGroup(ChannelTreeItem* channel) const;
    ChannelTreeItem* AddChannel(const QString& name,const QString& url,const QString& icon, std::optional<int64_t> parentGroupId) const;
    GroupTreeItem* AddGroup(const QString& text, std::optional<int64_t> parentGroupId) const;
    void AddGroup(GroupTreeItem* group);
    void SetChannelLogo(ChannelTreeItem* channel) const;
    void LoadChannelsAndGroups(RootTreeItem* rootItem) const;
    void SetFavourite(int64_t id, bool flag) const;
    ChannelTreeItem* GetChannel(int64_t id) const;
    void RemoveGroup(int64_t id) const;
    void RemoveChannel(int64_t id) const;

    std::vector<GroupTreeItem*> GetAllGroups() const;
    void LoadChannels(GroupTreeItem* parentItem) const;
    int GetGroupsCount();
    std::vector<ChannelTreeItem*> GetFavouriteChannels() const;
    void SetupSchema();
    int64_t AddRetrieveXStreamServer(const AuthenticationInfo& xstreamServerInfo) const;
    QString GetXStreamServerTimezone(int64_t id) const;
private:
    void addGroupTree(GroupTreeItem* group) const;
    void addGroup(GroupTreeItem* group, std::optional<int64_t> parentGroupId) const;
    void addChannel(ChannelTreeItem* channel, std::optional<int64_t> groupId) const;

    void loadChildGroups(GroupTreeItem* parentItem) const;
    void loadAllGroups(RootTreeItem* rootItem) const;
    void loadGroupsChannels(GroupTreeItem* parentItem) const;
    void loadRootChannels(RootTreeItem* rootItem) const;

    int getSchemaVersion() const;
    void incrementSchemaVersion(int& version) const;
    void executeStatement(const char* sql, const char* errMsg) const;
private:
    QSqlDatabase db;
};

#endif // DATABASE_H

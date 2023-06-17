#ifndef DATABASE_H
#define DATABASE_H

#include <filesystem>
#include <stdexcept>
#include <functional>
#include <optional>
#include <QString>

class DatabaseProvider;
class ChannelTreeItem;
class GroupTreeItem;
class RootTreeItem;
class QNetworkAccessManager;

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
    void WithTransaction(std::function<void()> callback) const;
    void AddChannelAndGroup(ChannelTreeItem* channel) const;
    std::unique_ptr<ChannelTreeItem> AddChannel(const QString& name,const QString& url,const QString& icon, std::optional<int64_t> parentGroupId, QNetworkAccessManager*) const;
    std::unique_ptr<GroupTreeItem> AddGroup(const QString& text, std::optional<int64_t> parentGroupId) const;
    void SetChannelLogo(ChannelTreeItem* channel) const;
    void LoadChannelsAndGroups(RootTreeItem* rootItem) const;
    void SetFavourite(int64_t id, bool flag) const;
    std::unique_ptr<ChannelTreeItem> GetChannel(int64_t id) const;
    void RemoveGroup(int64_t id) const;
    void RemoveChannel(int64_t id) const;
private:
    void addGroupTree(GroupTreeItem* group) const;
    void addGroup(GroupTreeItem* group, std::optional<int64_t> parentGroupId) const;
    void addChannel(ChannelTreeItem* channel, std::optional<int64_t> groupId) const;

    void loadChildGroups(GroupTreeItem* parentItem, QNetworkAccessManager*) const;
    void loadAllGroups(RootTreeItem* rootItem) const;
    void loadGroupsChannels(GroupTreeItem* parentItem, RootTreeItem* rootItem) const;
    void loadRootChannels(RootTreeItem* rootItem) const;

    int getSchemaVersion() const;
    void incrementSchemaVersion(int version) const;
    void executeStatement(const char* sql, const char* errMsg) const;
};

#endif // DATABASE_H

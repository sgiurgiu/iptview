#ifndef DATABASE_H
#define DATABASE_H

#include <filesystem>
#include <stdexcept>
#include <functional>
#include <optional>

#include <sqlite3.h>

class DatabaseProvider;
class ChannelTreeItem;
class GroupTreeItem;
class RootTreeItem;

class DatabaseException : public std::runtime_error
{
   using std::runtime_error::runtime_error;
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
    void SetChannelLogo(ChannelTreeItem* channel) const;
    void LoadChannelsAndGroups(RootTreeItem* rootItem) const;
    void SetFavourite(int64_t id, bool flag) const;
private:
    void addGroupTree(GroupTreeItem* group) const;
    void addGroup(GroupTreeItem* group, std::optional<int64_t> parentGroupId) const;
    void addChannel(ChannelTreeItem* channel, std::optional<int64_t> groupId) const;
    GroupTreeItem* loadGroup(int64_t id, RootTreeItem* rootItem) const;

    int getSchemaVersion() const;
    void incrementSchemaVersion(int version) const;
    void executeStatement(const char* sql, const char* errMsg) const;
private:
    struct DBConnectionDeleter
    {
        void operator()(sqlite3* db)
        {
            if(db)
            {
                sqlite3_close(db);
            }
        }
    };

    std::unique_ptr<sqlite3, DBConnectionDeleter> db;
};

#endif // DATABASE_H

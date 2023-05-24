#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include <memory>

class Database;

class DatabaseProvider
{
public:
    static Database const* GetDatabase();
private:
    static std::unique_ptr<Database> database;
};

#endif // DATABASEPROVIDER_H

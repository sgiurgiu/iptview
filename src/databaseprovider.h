#ifndef DATABASEPROVIDER_H
#define DATABASEPROVIDER_H

#include <memory>

class Database;

class DatabaseProvider
{
public:
    static std::unique_ptr<Database> GetDatabase();
};

#endif // DATABASEPROVIDER_H

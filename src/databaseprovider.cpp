#include "databaseprovider.h"
#include "database.h"

#ifdef _WIN32
#include <Shlobj.h>
#include <Shlobj_core.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

std::unique_ptr<Database> DatabaseProvider::database = nullptr;

Database const* DatabaseProvider::GetDatabase()
{
    if(!database)
    {
        std::filesystem::path homePath;
        std::string relativeConfigFolder;
    #ifdef _WIN32
        char homeDirStr[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, homeDirStr)))
        {
            homePath = homeDirStr;
        }
        relativeConfigFolder = "iptview";
    #else
        char* homeChar = getenv("HOME");
        if(homeChar == nullptr)
        {
            auto pwd = getpwuid(getuid());
            if (pwd)
            {
                homeChar = pwd->pw_dir;
            }
        }
        homePath  = homeChar;
        relativeConfigFolder = ".config/iptview";
    #endif

        if(homePath.empty())
        {
            homePath = "./";
        }

        std::filesystem::path  configFolder = homePath / relativeConfigFolder;
        std::filesystem::create_directories(configFolder);
        std::filesystem::path path = configFolder / "iptview.db";
        database = std::make_unique<Database>(Database::ConstructorKey{}, path);
    }
    return database.get();
}

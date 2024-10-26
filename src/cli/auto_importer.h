#ifndef _ES_RGMF_CLI_AUTO_IMPORTER_H
#define _ES_RGMF_CLI_AUTO_IMPORTER_H 1

#include <filesystem>

#include <sqlite3.h>

#include "../core/api.h"

namespace fitgalgo
{

class AutoImporter
{
private:
    Connection connection;
    std::filesystem::path path;
    std::filesystem::path database_path;

    inline void start() const;

public:
    explicit AutoImporter(const std::filesystem::path& path);
};

struct ImportedFile
{
    std::string absolutePath{};
    bool accepted{};
    std::string error{};
};

class ImportedFileDao
{
private:
    sqlite3* db;

public:
    ImportedFileDao(const std::string& db_path);
    ~ImportedFileDao();

    bool is_ok() const;
    bool create_table_if_not_exists() const;
    bool save(const ImportedFile& r) const;
    bool exists_absolute_path(const std::string& p) const;
};

}

#endif // _ES_RGMF_CLI_AUTO_IMPORTER_H
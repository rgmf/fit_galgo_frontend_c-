#include "auto_importer.h"
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace  fitgalgo
{

inline bool is_valid_device_fit_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: " + filename + " could not be opened" << std::endl;
        return false;
    }

    // Read 12 bytes header from FIT file
    std::vector<char> header(12);
    file.read(header.data(), header.size());

    // Validate header file that must start with: ".FIT"
    if (header[8] != '.' || header[9] != 'F' || header[10] != 'I' || header[11] != 'T')
    {
        std::cerr << "Error: " + filename + " has a bada header (it's not '.FIT')"
                    << std::endl;
        return false;
    }

    /*// Leer mensajes y buscar el campo "file" con el valor "device"
    while (file)
    {
        // Leer el siguiente byte del archivo (record header)
        char recordHeader;
        file.read(&recordHeader, 1);

        // Si el mensaje es del tipo "file_id" y el tipo de archivo es "device"
        if (true) // lógica para leer y verificar mensajes y campos específicos
        {
            // Ejemplo:
            // Si encuentras el campo 'file' con el valor esperado y el nombre 'device'
            return true;
        }
        }*/

    return true;
}

AutoImporter::AutoImporter(
    const std::filesystem::path& path, const std::string& vendorId, const std::string& productId)
    : connection{}, path{path}, database_path{}, vendorId{vendorId}, productId{productId}
{
    const char *username = std::getenv("FIT_USERNAME");
    const char *password = std::getenv("FIT_PASSWD");

    if (username == nullptr || password == nullptr)
    {
        std::cerr << "Error: username and password for FIT API needed." << std::endl
                  << "Create FIT_USERNAME and FIT_PASSWD variables environment with the values."
                  << std::endl;
        return;
    }

    Result<LoginData> login_result = this->connection.login(
        std::string{username}, std::string{password});
    if (!login_result.is_valid())
    {
        std::cerr << "Error: login error: " << login_result.get_error().error_to_string()
                  << std::endl;
        return;
    }

    // Creating $HOME/.local/share/fitgalgo folder if needed (where database must be)
    const char *xdg_data_home = std::getenv("XDG_DATA_HOME");
    std::filesystem::path local_share_path;
    if (xdg_data_home == nullptr)
    {
        const char *home = std::getenv("HOME");
        if (home == nullptr)
        {
            std::cerr << "Error: not found neither XDG_DATA_HOME nor HOME environment variables."
                      << std::endl;
            return;
        }

        local_share_path = std::filesystem::path(home) / ".local/share";
    }
    else
    {
        local_share_path = std::filesystem::path(xdg_data_home);
    }

    if (!std::filesystem::is_directory(local_share_path))
    {
        std::cerr << "Error: " << local_share_path << " is not a directory"
                  << std::endl;
        return;
    }

    std::filesystem::path fitgalgo_path = local_share_path / "fitgalgo";
    if (!std::filesystem::exists(fitgalgo_path))
    {
        std::error_code ec;
        std::filesystem::create_directories(fitgalgo_path, ec);

        if (ec)
        {
            std::cerr << "Error creating directory: " << ec.message() << std::endl;
            return;
        }
    }

    this->database_path = fitgalgo_path / "fitgalgo.db";
    this->start();
}

inline void AutoImporter::start() const
{
    std::string device_fit_file_path{};

    // Looking for device.fit file
    for (const auto& entry : std::filesystem::recursive_directory_iterator(this->path))
    {
        std::string path_lower = entry.path().string();
        std::transform(path_lower.begin(), path_lower.end(), path_lower.begin(),
            [](unsigned char c) { return std::tolower(c); }
        );
        if (path_lower.ends_with("device.fit"))
        {
            device_fit_file_path = entry.path().string();
            break;
        }
    }

    // If device.fit file was found then check it is a valid FIT file
    if (device_fit_file_path.empty())
    {
        std::cout << "Auto importer: device.fit file not found in "
                  << this->path << " directory" << std::endl;
        return;
    }

    if (is_valid_device_fit_file(device_fit_file_path))
    {
        std::cout << "Auto importer: " << device_fit_file_path
                  << " file is a valid one" << std::endl;
    }
    else
    {
        std::cout << "Auto importer: " << device_fit_file_path
                  << " file is not a valid one" << std::endl;
        return;
    }

    // Open database
    auto db = ImportedFileDao(this->database_path);
    if (!db.is_ok())
    {
        std::cerr << "Error: database cannot be opened" << std::endl;
        return;
    }
    db.create_table_if_not_exists();

    // If device.fit is a valid FIT file then import all files from its parent
    const auto& root_path = std::filesystem::path(device_fit_file_path).parent_path();
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root_path))
    {
        if (entry.path().string().ends_with(".fit") ||
            entry.path().string().ends_with(".FIT"))
        {
            std::cout << "POST file: " << entry.path() << std::endl;

            const auto& relative_path = std::filesystem::relative(entry.path(), this->path);
            if (!db.exists_imported_file(relative_path, this->vendorId, this->productId))
            {
                ImportedFile imported_file{};
                imported_file.relativePath = relative_path;
                imported_file.vendorId = this->vendorId;
                imported_file.productId = this->productId;

                const auto& results = this->connection.post_file(entry.path());
                if (results.size() > 0)
                {
                    const auto& r = results.at(0);
                    if (r.is_valid() && !r.get_data().uploaded_files.empty())
                    {
                        if (r.get_data().uploaded_files.at(0).errors.empty())
                        {
                            imported_file.accepted = true;
                            std::cout << "File uploaded" << std::endl;
                        }
                        else
                        {
                            imported_file.accepted = true;
                            imported_file.error = "Errors: ";
                            std::cerr << "Errors: " << std::endl;
                            for (auto& error : r.get_data().uploaded_files.at(0).errors)
                            {
                                imported_file.error += error + ". ";
                                std::cerr << "    - " << error << std::endl;
                            }
                        }
                    }
                    else
                    {
                        imported_file.accepted = false;
                        imported_file.error = r.get_error().error_to_string();
                        std::cerr << "Error: " << r.get_error().error_to_string() << std::endl;
                    }
                }

                if (db.save(imported_file))
                {
                    std::cout << "Saved into database" << std::endl;
                }
                else
                {
                    std::cerr << "Error saving file information into database" << std::endl;
                }

                std::cout << std::endl;
            }
        }
    }
}

ImportedFileDao::ImportedFileDao(const std::string& db_path)
{
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
    else
    {
        if (!create_table_if_not_exists())
        {
            sqlite3_close(db);
            db = nullptr;
        }
    }
}

ImportedFileDao::~ImportedFileDao()
{
    if (db)
    {
        sqlite3_close(db);
    }
}

bool ImportedFileDao::is_ok() const
{
    return db != nullptr;
}

bool ImportedFileDao::create_table_if_not_exists() const
{
    const char* sql = "CREATE TABLE IF NOT EXISTS imported_files ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "relative_path VARCHAR(1024) NOT NULL, "
                      "vendor_id VARCHAR(100) NOT NULL, "
                      "product_id VARCHAR(100) NOT NULL, "
                      "accepted INTEGER NOT NULL, "
                      "error TEXT, "
                      "inserted DATETIME DEFAULT CURRENT_TIMESTAMP, "
                      "UNIQUE (relative_path, vendor_id, product_id));";
    char* error_msg;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error_msg) != SQLITE_OK)
    {
        std::cerr << "Error creating the table imported_files: " << error_msg << std::endl;
        sqlite3_free(error_msg);
        return false;
    }
    return true;
}

bool ImportedFileDao::save(const ImportedFile& r) const
{
    const char *sql = "INSERT INTO imported_files "
                      "(relative_path, vendor_id, product_id, accepted, error) "
                      "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Error preparing the sentence: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, r.relativePath.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, r.vendorId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, r.productId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, r.accepted ? 1 : 0);
    sqlite3_bind_text(stmt, 5, r.error.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    if (!success)
    {
        std::cerr << "Error saving imported file: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool ImportedFileDao::exists_imported_file(
    const std::string& p, const std::string& vi, const std::string& pi) const
{
    const char* sql = "SELECT 1 FROM imported_files "
                      "WHERE relative_path=? AND vendor_id=? AND product_id=?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Error preparing sentence: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, p.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, vi.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, pi.c_str(), -1, SQLITE_STATIC);

    bool found = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);

    return found;
}

}
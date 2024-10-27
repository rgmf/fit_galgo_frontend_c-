#include <filesystem>
#include <optional>

#include "ui/shell.h"
#include "cli/auto_importer.h"

std::optional<std::filesystem::path> get_path(const char *pathString)
{
    std::filesystem::path filepath = std::string(pathString);
    return std::filesystem::is_directory(filepath) ? std::optional<std::filesystem::path>{filepath} : std::nullopt;
}

int main(int argc, char **argv)
{
    std::optional<std::filesystem::path> filepath;
    if (argc > 6 && strcmp(argv[1], "-d") == 0 && (filepath = get_path(argv[2])).has_value() &&
        strcmp(argv[3], "-v") == 0 && strcmp(argv[5], "-p") == 0)
    {
        fitgalgo::AutoImporter auto_importer{*filepath, std::string{argv[4]}, std::string{argv[6]}};
    }
    else
    {
        fitgalgo::Shell shell{};
        shell.loop();
    }

    return 0;
}

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
    if (argc > 2 && strcmp(argv[1], "-d") == 0 && (filepath = get_path(argv[2])).has_value())
    {
        fitgalgo::AutoImporter auto_importer{*filepath};
    }
    else
    {
        fitgalgo::Shell shell{};
        shell.loop();
    }

    return 0;
}

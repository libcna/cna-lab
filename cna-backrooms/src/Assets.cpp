#include "Assets.hpp"

#include <array>
#include <system_error>

namespace Backrooms {
std::filesystem::path FindAssetDirectory() {
    // Linux desktop is the supported target. Prefer the assets packaged beside
    // the executable; source-tree paths also support development launches.
    std::error_code error;
    const auto executable=std::filesystem::read_symlink("/proc/self/exe",error);
    const std::array<std::filesystem::path,3> candidates{{
        error ? std::filesystem::path{} : executable.parent_path()/"assets",
        "assets","../assets"
    }};
    for (const auto& directory:candidates) {
        if (directory.empty()) continue;
        error.clear();
        if (std::filesystem::is_directory(directory,error)) return directory;
    }
    return "assets";
}
}

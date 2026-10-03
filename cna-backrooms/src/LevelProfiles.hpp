#pragma once

#include "World.hpp"

#include <filesystem>
#include <string>

namespace Backrooms {
inline constexpr int kLevelProfileVersion=1;
LevelCatalog ParseLevelCatalog(const std::string& json);
LevelCatalog LoadLevelCatalog(const std::filesystem::path& path);
}

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace Backrooms {

constexpr int kFormatVersion = 2;
constexpr int kChunkCells = 8;
constexpr int kRegionCells = 6;
constexpr double kCellSize = 5.0;
constexpr double kChunkSize = kChunkCells * kCellSize;

struct ChunkCoord {
    int x = 0;
    int z = 0;
    bool operator==(const ChunkCoord&) const = default;
    bool operator<(const ChunkCoord& other) const {
        return x < other.x || (x == other.x && z < other.z);
    }
};

enum class Edge : std::uint8_t { Open, Wide, Door, Solid };

enum class RegionKind : std::uint8_t {
    OpenOffice, Columns, Rooms, Halls, Irregular, Storage, Tunnels
};

struct Wall {
    double minX, minZ, maxX, maxZ;
};

struct WorldConfig {
    std::uint64_t seed = 0xBACC0005ULL;
    int level = 0;
};

struct LevelDefinition {
    const char* name;
    float ceilingHeight;
    float fogStart;
    float fogEnd;
};

const LevelDefinition& LevelInfo(int level);

int CellOf(double position);
int ChunkOfCell(int cell);
ChunkCoord ChunkAt(double x, double z);
std::uint32_t CellHash(const WorldConfig& config, int x, int z, int salt);
RegionKind RegionAt(const WorldConfig& config, int cellX, int cellZ);
Edge VerticalEdge(const WorldConfig& config, int boundaryX, int z);
Edge HorizontalEdge(const WorldConfig& config, int x, int boundaryZ);
std::optional<Wall> CellObstacle(const WorldConfig& config, int cellX, int cellZ);
std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z);
bool Collides(const WorldConfig& config, double x, double z, double radius);
void MoveWithCollision(const WorldConfig& config, double& x, double& z,
                       double dx, double dz, double radius);

} // namespace Backrooms

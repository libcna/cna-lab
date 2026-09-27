#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace Backrooms {

constexpr int kFormatVersion = 5;
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

struct CellObstacleSet {
    std::array<Wall,5> walls{};
    int count = 0;
};

enum class PropKind : std::uint8_t {
    None, Chair, Table, EmbeddedChair, LowPartition
};

struct CellProp {
    PropKind kind = PropKind::None;
    double x = 0;
    double z = 0;
    int quarterTurn = 0;
    float sink = 0;
};

struct WorldConfig {
    std::uint64_t seed = 0xBACC0005ULL;
    int level = 0;
};

struct PortalDefinition {
    int level, cellX, cellZ, target;
    bool alongX;
    bool operator==(const PortalDefinition&) const = default;
};

inline constexpr std::array<PortalDefinition,4> kPortals{{
    {0,3,0,1,true}, {1,0,3,0,false},
    {1,3,0,2,true}, {2,0,3,1,false}
}};

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
std::optional<PortalDefinition> PortalAt(const WorldConfig& config,
                                         int cellX, int cellZ);
RegionKind RegionAt(const WorldConfig& config, int cellX, int cellZ);
Edge VerticalEdge(const WorldConfig& config, int boundaryX, int z);
Edge HorizontalEdge(const WorldConfig& config, int x, int boundaryZ);
CellProp PropAt(const WorldConfig& config, int cellX, int cellZ);
CellObstacleSet CellObstacles(const WorldConfig& config, int cellX, int cellZ);
std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z);
bool Collides(const WorldConfig& config, double x, double z, double radius);
void MoveWithCollision(const WorldConfig& config, double& x, double& z,
                       double dx, double dz, double radius);

} // namespace Backrooms

#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace Backrooms {

constexpr int kFormatVersion = 45;
constexpr int kChunkCells = 8;
constexpr int kRegionCells = 6;
constexpr double kCellSize = 5.0;
constexpr float kWallHalfThickness = 0.10f;
constexpr double kPortalEntryDepth = 0.43;
constexpr double kPortalBackDepth = 1.92;
constexpr double kPortalHalfWidth = 0.65;
constexpr double kPortalWallThickness = 0.18;
constexpr double kChunkSize = kChunkCells * kCellSize;
constexpr float kChairHalfWidth = 0.285f;
constexpr float kChairHalfDepth = 0.29f;
constexpr float kTableHalfWidth = 0.75f;
constexpr float kTableHalfDepth = 0.40f;

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

enum class OfficePartitionStyle : std::uint8_t {
    Straight, LShaped, TShaped, Staggered, ShortWall, DeadSpace
};

enum class OfficeColumnStyle : std::uint8_t {
    Rectangular, Spine, LShaped, DiagonalPair, OffsetRectangle
};

enum class HallLightingStyle : std::uint8_t { PairedRows, Staggered, Bands };

struct Wall {
    double minX, minZ, maxX, maxZ;
};

struct OpeningSpan {
    double start, end;
    bool operator==(const OpeningSpan&) const = default;
};

struct CellObstacleSet {
    std::array<Wall,5> walls{};
    int count = 0;
};

enum class PropKind : std::uint8_t {
    None, Chair, Table, EmbeddedChair, LowPartition, TallPartition
};

struct CellProp {
    PropKind kind = PropKind::None;
    double x = 0;
    double z = 0;
    int quarterTurn = 0;
    float sink = 0;
};

Wall PropBounds(const CellProp& prop);

struct LampInfo {
    bool fixture=false;
    bool lit=false;
    float x=2.5f,z=2.5f,y=0;
    bool longAxisX=true;
    bool operator==(const LampInfo&) const = default;
};

struct EntitySpawn {
    double x=0,z=0;
    float phase=0;
    bool operator==(const EntitySpawn&) const = default;
};

struct OfficeAlcove {
    bool vertical=true;
    int inward=1;
    double boundary=0,start=0,end=0,depth=0;
    bool falseDoor=false;
    bool operator==(const OfficeAlcove&) const = default;
};

class RoomLayoutCache;
struct LevelCatalog;

struct WorldConfig {
    std::uint64_t seed = 0xBACC0005ULL;
    int level = 0;
    // Optional game-owned acceleration; it never changes generated results.
    RoomLayoutCache* roomLayouts = nullptr;
    const LevelCatalog* levels = nullptr;
};

struct RoomLayout {
    std::array<Edge,kRegionCells*kRegionCells> east{},south{};
};

class RoomLayoutCache {
public:
    static constexpr std::size_t kCapacity=256;
    const RoomLayout& Get(const WorldConfig& world,int regionX,int regionZ,
                          RegionKind kind);
    std::size_t Size() const { return layouts_.size(); }
private:
    using Key=std::tuple<std::uint64_t,int,int,int,RegionKind>;
    std::map<Key,RoomLayout> layouts_;
    std::deque<Key> order_;
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
    using Rgb=std::array<std::uint8_t,3>;
    struct RoomWeight {
        RegionKind kind=RegionKind::OpenOffice;
        int weight=0;
        bool operator==(const RoomWeight&) const = default;
    };
    std::string name;
    float ceilingHeight=3,doorwayHeight=2.62f;
    float fogStart=18,fogEnd=85;
    float ambient=0.54f,lightStrength=0.54f;
    float wallBounce=0.32f,ceilingBounce=0.52f;
    unsigned entityRarity=850;
    Rgb wall{},pillar{},trim{},floor{},ceiling{},structure{},fluorescent{},fog{};
    std::array<RoomWeight,7> regions{};
    bool operator==(const LevelDefinition&) const = default;
};

struct LevelCatalog {
    std::array<LevelDefinition,3> levels;
    std::uint64_t sourceHash=0; // zero identifies built-in defaults
};

const LevelCatalog& DefaultLevelCatalog();
const LevelDefinition& LevelInfo(int level);
const LevelDefinition& LevelInfo(const WorldConfig& world);

int CellOf(double position);
int ChunkOfCell(int cell);
ChunkCoord ChunkAt(double x, double z);
std::uint32_t CellHash(const WorldConfig& config, int x, int z, int salt);
std::optional<PortalDefinition> PortalAt(const WorldConfig& config,
                                         int cellX, int cellZ);
CellObstacleSet PortalWalls(const PortalDefinition& portal);
std::optional<int> PortalTarget(const WorldConfig& config, double x, double z);
RegionKind RegionAt(const WorldConfig& config, int cellX, int cellZ);
LampInfo LampAt(const WorldConfig& config, int cellX, int cellZ);
bool IsEmptyHall(const WorldConfig& config, int cellX, int cellZ);
std::optional<HallLightingStyle> HallLightingStyleAt(
    const WorldConfig& config,int cellX,int cellZ);
bool IsServiceChamber(const WorldConfig& config, int cellX, int cellZ);
Edge VerticalEdge(const WorldConfig& config, int boundaryX, int z);
Edge HorizontalEdge(const WorldConfig& config, int x, int boundaryZ);
OpeningSpan OpeningForEdge(const WorldConfig& config, Edge edge,
                           bool vertical, int edgeX, int edgeZ);
CellProp PropAt(const WorldConfig& config, int cellX, int cellZ);
std::optional<OfficePartitionStyle> OfficePartitionStyleAt(
    const WorldConfig& config,int regionX,int regionZ);
std::optional<OfficeColumnStyle> OfficeColumnStyleAt(
    const WorldConfig& config,int regionX,int regionZ);
CellObstacleSet InteriorPartitionsAt(const WorldConfig& config,
                                     int cellX, int cellZ);
std::optional<OfficeAlcove> OfficeAlcoveAt(const WorldConfig& config,
                                        int cellX,int cellZ);
CellObstacleSet OfficeAlcoveWalls(const OfficeAlcove& alcove);
CellObstacleSet FullHeightObstaclesAt(const WorldConfig& config, int cellX, int cellZ);
std::optional<EntitySpawn> EntityAt(const WorldConfig& config,int cellX,int cellZ);
bool UtilityAlongZAt(const WorldConfig& config, int cellX, int cellZ);
CellObstacleSet CellObstacles(const WorldConfig& config, int cellX, int cellZ);
std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z);
std::vector<Wall> NearbyFullHeightWalls(const WorldConfig& config, double x, double z);
bool Collides(const WorldConfig& config, double x, double z, double radius);
void MoveWithCollision(const WorldConfig& config, double& x, double& z,
                       double dx, double dz, double radius);

} // namespace Backrooms

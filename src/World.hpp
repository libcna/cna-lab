#pragma once

#include <cstdint>
#include <vector>

namespace Backrooms {

constexpr int kFormatVersion = 1;
constexpr int kChunkCells = 8;
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

enum class Edge : std::uint8_t { Open, Door, Solid };

struct Wall {
    double minX, minZ, maxX, maxZ;
};

struct WorldConfig {
    std::uint64_t seed = 0xBACC0005ULL;
    int level = 0;
};

int CellOf(double position);
int ChunkOfCell(int cell);
ChunkCoord ChunkAt(double x, double z);
std::uint32_t CellHash(const WorldConfig& config, int x, int z, int salt);
Edge VerticalEdge(const WorldConfig& config, int boundaryX, int z);
Edge HorizontalEdge(const WorldConfig& config, int x, int boundaryZ);
std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z);
bool Collides(const WorldConfig& config, double x, double z, double radius);
void MoveWithCollision(const WorldConfig& config, double& x, double& z,
                       double dx, double dz, double radius);

} // namespace Backrooms

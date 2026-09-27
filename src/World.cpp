#include "World.hpp"

#include <algorithm>
#include <cmath>

namespace Backrooms {
namespace {
std::uint64_t Mix(std::uint64_t v) {
    v ^= v >> 30; v *= 0xbf58476d1ce4e5b9ULL;
    v ^= v >> 27; v *= 0x94d049bb133111ebULL;
    return v ^ (v >> 31);
}

void AppendEdge(std::vector<Wall>& walls, Edge edge, bool vertical,
                double boundary, double along) {
    if (edge == Edge::Open) return;
    const double t = 0.10;
    auto add = [&](double a, double b) {
        if (vertical) walls.push_back({boundary-t, a, boundary+t, b});
        else walls.push_back({a, boundary-t, b, boundary+t});
    };
    if (edge == Edge::Solid) add(along, along+kCellSize);
    else {
        const double side = (kCellSize-1.9)*0.5;
        add(along, along+side);
        add(along+kCellSize-side, along+kCellSize);
    }
}
}

int CellOf(double position) { return static_cast<int>(std::floor(position/kCellSize)); }
int ChunkOfCell(int cell) {
    const int q = cell/kChunkCells;
    return cell < 0 && cell%kChunkCells ? q-1 : q;
}
ChunkCoord ChunkAt(double x, double z) { return {ChunkOfCell(CellOf(x)), ChunkOfCell(CellOf(z))}; }

std::uint32_t CellHash(const WorldConfig& config, int x, int z, int salt) {
    std::uint64_t v = config.seed ^ (static_cast<std::uint64_t>(kFormatVersion) << 48);
    v ^= Mix(static_cast<std::uint32_t>(x) + 0x99213d67ULL);
    v ^= Mix(static_cast<std::uint32_t>(z) + 0x5c833b45ULL);
    v ^= Mix(static_cast<std::uint32_t>(salt) + 0x37ac891eULL);
    v ^= Mix(static_cast<std::uint32_t>(config.level) + 0x9133a7c5ULL);
    return static_cast<std::uint32_t>(Mix(v));
}

Edge VerticalEdge(const WorldConfig& config, int boundaryX, int z) {
    // Every fourth row is a guaranteed east-west route across chunk borders.
    if (z % 4 == 0) return Edge::Open;
    const auto h = CellHash(config, boundaryX, z, 11) % 100;
    return h < 35 ? Edge::Open : h < 88 ? Edge::Door : Edge::Solid;
}
Edge HorizontalEdge(const WorldConfig& config, int x, int boundaryZ) {
    // Every fourth column is a guaranteed north-south route.
    if (x % 4 == 0) return Edge::Open;
    const auto h = CellHash(config, x, boundaryZ, 23) % 100;
    return h < 35 ? Edge::Open : h < 88 ? Edge::Door : Edge::Solid;
}

std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z) {
    std::vector<Wall> walls;
    const int cx = CellOf(x), cz = CellOf(z);
    walls.reserve(80);
    for (int ix=cx-1; ix<=cx+1; ++ix) {
        for (int iz=cz-1; iz<=cz+1; ++iz) {
            AppendEdge(walls, VerticalEdge(config, ix, iz), true,
                       ix*kCellSize, iz*kCellSize);
            AppendEdge(walls, HorizontalEdge(config, ix, iz), false,
                       iz*kCellSize, ix*kCellSize);
        }
    }
    return walls;
}

bool Collides(const WorldConfig& config, double x, double z, double radius) {
    for (const auto& wall : NearbyWalls(config, x, z)) {
        const double closestX = std::clamp(x, wall.minX, wall.maxX);
        const double closestZ = std::clamp(z, wall.minZ, wall.maxZ);
        const double a=x-closestX, b=z-closestZ;
        if (a*a+b*b < radius*radius) return true;
    }
    return false;
}

void MoveWithCollision(const WorldConfig& config, double& x, double& z,
                       double dx, double dz, double radius) {
    const int steps = std::max(1, static_cast<int>(std::ceil(std::hypot(dx,dz)/0.18)));
    for (int i=0; i<steps; ++i) {
        const double sx=dx/steps, sz=dz/steps;
        if (!Collides(config, x+sx, z, radius)) x += sx;
        if (!Collides(config, x, z+sz, radius)) z += sz;
    }
}

} // namespace Backrooms

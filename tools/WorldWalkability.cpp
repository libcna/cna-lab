#include "World.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace Backrooms;

namespace {
constexpr int kSide=90; // three complete regions, sampled every metre
constexpr double kPlayerRadius=0.31;

struct Audit {
    struct Pocket { int points; double x,z; };
    int freePoints=0;
    int largest=0;
    int largeComponents=0;
    std::vector<Pocket> isolated;
};

Audit Scan(const WorldConfig& world, int regionX, int regionZ) {
    const double originX=regionX*kRegionCells*kCellSize;
    const double originZ=regionZ*kRegionCells*kCellSize;
    constexpr int count=kSide*kSide;
    std::vector<std::uint8_t> free(count),east(count),south(count);
    Audit audit;
    for (int z=0;z<kSide;++z) for (int x=0;x<kSide;++x) {
        const int index=z*kSide+x;
        free[index]=!Collides(world,originX+x+0.5,originZ+z+0.5,
                              kPlayerRadius);
        audit.freePoints+=free[index];
    }
    for (int z=0;z<kSide;++z) for (int x=0;x<kSide;++x) {
        const int index=z*kSide+x;
        if (!free[index]) continue;
        const double startX=originX+x+0.5,startZ=originZ+z+0.5;
        if (x+1<kSide && free[index+1]) {
            double px=startX,pz=startZ;
            MoveWithCollision(world,px,pz,1,0,kPlayerRadius);
            east[index]=px>startX+0.99;
        }
        if (z+1<kSide && free[index+kSide]) {
            double px=startX,pz=startZ;
            MoveWithCollision(world,px,pz,0,1,kPlayerRadius);
            south[index]=pz>startZ+0.99;
        }
    }
    std::vector<int> labels(count,-1),sizes,starts,pending;
    pending.reserve(count);
    for (int start=0;start<count;++start) {
        if (!free[start] || labels[start]>=0) continue;
        const int component=static_cast<int>(sizes.size());
        pending.clear();pending.push_back(start);labels[start]=component;
        for (std::size_t cursor=0;cursor<pending.size();++cursor) {
            const int index=pending[cursor];
            const int x=index%kSide,z=index/kSide;
            const auto visit=[&](int next,bool connected) {
                if (connected && labels[next]<0) {
                    labels[next]=component;
                    pending.push_back(next);
                }
            };
            if (x+1<kSide) visit(index+1,east[index]);
            if (x>0) visit(index-1,east[index-1]);
            if (z+1<kSide) visit(index+kSide,south[index]);
            if (z>0) visit(index-kSide,south[index-kSide]);
        }
        const int size=static_cast<int>(pending.size());
        sizes.push_back(size);starts.push_back(start);
        audit.largest=std::max(audit.largest,size);
    }
    const auto largestIndex=static_cast<std::size_t>(
        std::max_element(sizes.begin(),sizes.end())-sizes.begin());
    for (std::size_t i=0;i<sizes.size();++i) {
        // Tiny corners behind maintenance frames are not useful room space.
        if (sizes[i]<10) continue;
        ++audit.largeComponents;
        if (i!=largestIndex)
            audit.isolated.push_back({sizes[i],
                originX+starts[i]%kSide+0.5,
                originZ+starts[i]/kSide+0.5});
    }
    return audit;
}
}

int main() {
    int failed=0;
    std::cout << "format " << kFormatVersion
              << ", collision walkability on 90 m squares at 1 m spacing\n";
    for (std::uint64_t seed: {0ULL,1ULL,2ULL,3ULL,12345ULL,
                              31337ULL,0xBACC0005ULL})
        for (int level=0;level<3;++level)
            for (const auto origin: {std::array<int,2>{-1,-1},
                    std::array<int,2>{-7,-4},std::array<int,2>{5,8}}) {
                const Audit audit=Scan({seed,level},origin[0],origin[1]);
                std::cout << "seed " << seed << " level " << level
                          << " | region origin " << origin[0] << ',' << origin[1]
                          << " | free points " << audit.freePoints
                          << " | largest component " << audit.largest
                          << " | components of 10+ points "
                          << audit.largeComponents << '\n';
                for (const auto& pocket:audit.isolated)
                    std::cout << "  isolated " << pocket.points << " points at "
                              << pocket.x << ',' << pocket.z << '\n';
                failed+=audit.largeComponents!=1;
            }
    if (failed) std::cerr << failed << " walkability samples need inspection\n";
    return failed ? 1 : 0;
}

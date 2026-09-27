#include "World.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

using namespace Backrooms;

namespace {
constexpr int kMin=-64,kMax=63;

struct Counts {
    int cells=0,deadEnds=0,openCells=0,blocked=0;
    int openEdges=0,wideEdges=0,doorEdges=0,solidEdges=0;
    int longestSightline=0,portals=0;
    std::array<int,7> regions{};
};

void CountEdge(Counts& counts, Edge edge) {
    switch (edge) {
    case Edge::Open: ++counts.openEdges; break;
    case Edge::Wide: ++counts.wideEdges; break;
    case Edge::Door: ++counts.doorEdges; break;
    case Edge::Solid: ++counts.solidEdges; break;
    }
}

Counts Sample(const WorldConfig& world) {
    Counts counts;
    for (int x=kMin;x<=kMax;++x) for (int z=kMin;z<=kMax;++z) {
        const Edge east=VerticalEdge(world,x+1,z);
        const Edge west=VerticalEdge(world,x,z);
        const Edge north=HorizontalEdge(world,x,z);
        const Edge south=HorizontalEdge(world,x,z+1);
        const int degree=static_cast<int>(east!=Edge::Solid)+
                         static_cast<int>(west!=Edge::Solid)+
                         static_cast<int>(north!=Edge::Solid)+
                         static_cast<int>(south!=Edge::Solid);
        ++counts.cells;
        counts.deadEnds+=degree==1;
        counts.openCells+=degree==4;
        counts.blocked+=degree==0;
        CountEdge(counts,east);
        CountEdge(counts,south);
        counts.portals+=PortalAt(world,x,z).has_value();
        ++counts.regions[static_cast<int>(RegionAt(world,x,z))];
    }
    for (int z=kMin;z<=kMax;++z) {
        int run=0;
        for (int x=kMin+1;x<=kMax;++x) {
            run=VerticalEdge(world,x,z)==Edge::Solid ? 0 : run+1;
            counts.longestSightline=std::max(counts.longestSightline,run);
        }
    }
    for (int x=kMin;x<=kMax;++x) {
        int run=0;
        for (int z=kMin+1;z<=kMax;++z) {
            run=HorizontalEdge(world,x,z)==Edge::Solid ? 0 : run+1;
            counts.longestSightline=std::max(counts.longestSightline,run);
        }
    }
    return counts;
}
}

int main() {
    std::cout << "format " << kFormatVersion << ", sampled "
              << (kMax-kMin+1)*(kMax-kMin+1)
              << " cells per seed and level\n";
    for (std::uint64_t seed: {0ULL,1ULL,2ULL,3ULL,12345ULL,
                              31337ULL,0xBACC0005ULL}) {
        for (int level=0;level<3;++level) {
            const Counts c=Sample({seed,level});
            const double edges=c.openEdges+c.wideEdges+
                               c.doorEdges+c.solidEdges;
            std::cout << "seed " << seed << " level " << level
                      << " | edge open/wide/door/solid "
                      << std::fixed << std::setprecision(1)
                      << c.openEdges*100/edges << '/' << c.wideEdges*100/edges
                      << '/' << c.doorEdges*100/edges << '/'
                      << c.solidEdges*100/edges
                      << "% | dead ends " << c.deadEnds
                      << " | degree 4 " << c.openCells
                      << " | blocked " << c.blocked
                      << " | longest sightline " << c.longestSightline
                      << " cells | entrances " << c.portals
                      << " | region cells";
            for (int count:c.regions) std::cout << ' ' << count;
            std::cout << '\n';
        }
    }
}

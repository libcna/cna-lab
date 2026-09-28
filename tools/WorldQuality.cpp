#include "World.hpp"
#include "Assets.hpp"
#include "LevelProfiles.hpp"

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
    int longestSightline=0,portals=0,emptyHallCells=0,chamberCells=0,alcoves=0;
    std::array<int,7> regions{};
    std::array<int,6> partitionPlans{};
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
        counts.emptyHallCells+=IsEmptyHall(world,x,z);
        counts.chamberCells+=IsServiceChamber(world,x,z);
        counts.alcoves+=OfficeAlcoveAt(world,x,z).has_value();
        ++counts.regions[static_cast<int>(RegionAt(world,x,z))];
        if (x%kRegionCells==0 && z%kRegionCells==0)
            if (const auto style=OfficePartitionStyleAt(world,x/kRegionCells,z/kRegionCells))
                ++counts.partitionPlans[static_cast<int>(*style)];
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

int Run(bool showAlcoves,bool showPartitions) {
    const auto levels=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    RoomLayoutCache cache;
    std::cout << "format " << kFormatVersion << ", sampled "
              << (kMax-kMin+1)*(kMax-kMin+1)
              << " cells per seed and level\n";
    for (std::uint64_t seed: {0ULL,1ULL,2ULL,3ULL,12345ULL,
                              31337ULL,0xBACC0005ULL}) {
        for (int level=0;level<3;++level) {
            const Counts c=Sample({seed,level,&cache,&levels});
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
                      << " | empty hall cells " << c.emptyHallCells
                      << " | chamber cells " << c.chamberCells
                      << " | alcoves " << c.alcoves
                      << " | region cells";
            for (int count:c.regions) std::cout << ' ' << count;
            std::cout << " | partition plans";
            for (int count:c.partitionPlans) std::cout << ' ' << count;
            std::cout << '\n';
            if (showPartitions && level==0) {
                const WorldConfig world{seed,level,&cache,&levels};
                std::array<bool,12> sampled{};
                for (int rx=-10;rx<=10;++rx) for (int rz=-10;rz<=10;++rz) {
                    if (RegionAt(world,rx*kRegionCells,rz*kRegionCells)!=RegionKind::OpenOffice)
                        continue; // inspect the plan without enclosed-room occlusion
                    const auto style=OfficePartitionStyleAt(world,rx,rz);
                    if (!style) continue;
                    const bool alongX=(CellHash(world,rx,rz,3119)&1U)!=0;
                    const int index=static_cast<int>(*style)*2+alongX;
                    const double x=rx*kRegionCells*kCellSize+4.5;
                    const double z=rz*kRegionCells*kCellSize+4.5;
                    if (sampled[index] || Collides(world,x,z,0.55)) continue;
                    sampled[index]=true;
                    std::cout << "partition seed " << seed << " region " << rx << ',' << rz
                              << " style " << static_cast<int>(*style) << " along_x " << alongX
                              << " position " << x << ',' << z << '\n';
                }
            }
            if (showAlcoves && level==0) {
                const WorldConfig world{seed,level,&cache,&levels};
                std::array<bool,8> sampled{};
                for (int x=kMin;x<=kMax;++x) for (int z=kMin;z<=kMax;++z) {
                    const auto alcove=OfficeAlcoveAt(world,x,z);
                    if (!alcove) continue;
                    const int style=(alcove->vertical ? 0 : 2)+
                                     (alcove->inward<0)+(alcove->falseDoor ? 4 : 0);
                    if (sampled[style]) continue;
                    sampled[style]=true;
                    // Approach from the room center, looking toward the back.
                    std::cout << std::setprecision(2) << "alcove seed " << seed << " cell " << x << ',' << z
                              << " vertical " << alcove->vertical << " inward " << alcove->inward
                              << " door " << alcove->falseDoor << " boundary " << alcove->boundary
                              << " span " << alcove->start << ',' << alcove->end
                              << " depth " << alcove->depth << std::setprecision(1) << '\n';
                }
            }
        }
    }
    return 0;
}

int main(int argc,char** argv) {
    try {
        if (argc>2 || (argc==2 && std::string(argv[1])!="--alcoves" &&
                      std::string(argv[1])!="--partitions"))
            throw std::invalid_argument("usage: world_quality [--alcoves|--partitions]");
        return Run(argc==2 && std::string(argv[1])=="--alcoves",
                   argc==2 && std::string(argv[1])=="--partitions");
    }
    catch (const std::exception& e) {
        std::cerr << "world audit: " << e.what() << '\n';
        return 1;
    }
}

#include "World.hpp"
#include "Assets.hpp"
#include "LevelProfiles.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace Backrooms;

namespace {
constexpr int kMin=-64,kMax=63;

struct Counts {
    int cells=0,deadEnds=0,openCells=0,blocked=0;
    int openEdges=0,wideEdges=0,doorEdges=0,solidEdges=0;
    int longestSightline=0,portals=0,emptyHallCells=0,chamberCells=0,alcoves=0,entities=0;
    std::array<int,7> regions{};
    std::array<int,6> partitionPlans{};
    std::array<int,5> columnPlans{};
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
        counts.entities+=EntityAt(world,x,z).has_value();
        ++counts.regions[static_cast<int>(RegionAt(world,x,z))];
        if (x%kRegionCells==0 && z%kRegionCells==0)
            if (const auto style=OfficePartitionStyleAt(world,x/kRegionCells,z/kRegionCells))
                ++counts.partitionPlans[static_cast<int>(*style)];
        if (x%kRegionCells==0 && z%kRegionCells==0)
            if (const auto style=OfficeColumnStyleAt(world,x/kRegionCells,z/kRegionCells))
                ++counts.columnPlans[static_cast<int>(*style)];
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

int ViewSamples() {
    const auto levels=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    RoomLayoutCache cache;
    constexpr std::array<std::uint64_t,6> seeds{{42,6502,177013,94081,88801,293781}};
    for (int level=0;level<3;++level) {
        const int count=level==0 ? 6 : 3;
        for (int index=0;index<count;++index) {
            const WorldConfig world{seeds[index],level,&cache,&levels};
            bool selected=false;
            for (int attempt=0;attempt<100 && !selected;++attempt) {
                const auto a=CellHash(world,index,attempt,4441);
                const auto b=CellHash(world,index,attempt,4447);
                const int cx=static_cast<int>(a%1600)-800;
                const int cz=static_cast<int>(b%1600)-800;
                const double x=cx*kCellSize+0.8+((a>>12)%341)*0.01;
                const double z=cz*kCellSize+0.8+((b>>12)%341)*0.01;
                if (Collides(world,x,z,0.55) || PortalAt(world,cx,cz)) continue;
                const double heading=(CellHash(world,cx,cz,4451)%6284)*0.001;
                std::cout << std::fixed << std::setprecision(6)
                          << "view seed " << world.seed << " level " << level
                          << " sample " << index << " region "
                          << static_cast<int>(RegionAt(world,cx,cz))
                          << " position " << x << ',' << z << " heading " << heading << '\n';
                selected=true;
            }
            if (!selected) throw std::runtime_error("could not select a free visual sample");
        }
    }
    return 0;
}

int OfficeLightingViews() {
    const auto levels=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    RoomLayoutCache cache;
    constexpr std::array<std::uint64_t,3> seeds{{8723,55291,402717}};
    constexpr std::array<const char*,4> cases{{"empty_hall","broad","weak_circuit","enclosed"}};
    for (int category=0;category<4;++category) for (int index=0;index<3;++index) {
        const WorldConfig world{seeds[index],0,&cache,&levels};
        bool selected=false;
        for (int attempt=0;attempt<10000 && !selected;++attempt) {
            const auto a=CellHash(world,category*3+index,attempt,4481);
            const auto b=CellHash(world,category*3+index,attempt,4487);
            const int cx=static_cast<int>(a%1600)-800;
            const int cz=static_cast<int>(b%1600)-800;
            const auto kind=RegionAt(world,cx,cz);
            const bool empty=IsEmptyHall(world,cx,cz);
            const int rx=static_cast<int>(std::floor(static_cast<double>(cx)/kRegionCells));
            const int rz=static_cast<int>(std::floor(static_cast<double>(cz)/kRegionCells));
            const bool weak=CellHash(world,rx,rz,1803)%13==0;
            const bool broad=kind==RegionKind::OpenOffice || kind==RegionKind::Columns;
            if ((category==0 && !empty) ||
                (category==1 && (empty || weak || !broad)) ||
                (category==2 && (empty || !weak)) ||
                (category==3 && (empty || weak || broad))) continue;
            const double x=cx*kCellSize+0.8+((a>>12)%341)*0.01;
            const double z=cz*kCellSize+0.8+((b>>12)%341)*0.01;
            if (Collides(world,x,z,0.55) || PortalAt(world,cx,cz)) continue;
            int fixtures=0,lit=0;
            for (int lx=0;lx<kRegionCells;++lx) for (int lz=0;lz<kRegionCells;++lz) {
                const auto lamp=LampAt(world,rx*kRegionCells+lx,rz*kRegionCells+lz);
                fixtures+=lamp.fixture;lit+=lamp.lit;
            }
            const double heading=(CellHash(world,cx,cz,4491)%6284)*0.001;
            std::cout << std::fixed << std::setprecision(6)
                      << "view seed " << world.seed << " level 0 sample " << category*3+index
                      << " region " << static_cast<int>(kind)
                      << " position " << x << ',' << z << " heading " << heading
                      << " case " << cases[category] << " fixtures " << fixtures
                      << " lit " << lit << '\n';
            selected=true;
        }
        if (!selected) throw std::runtime_error("could not select an office lighting sample");
    }
    return 0;
}

int OfficeColumnViews() {
    const auto levels=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    RoomLayoutCache cache;
    constexpr std::array<std::uint64_t,3> seeds{{8723,55291,402717}};
    constexpr std::array<const char*,5> cases{{"rectangle","spine","L_plan",
                                             "diagonal_pair","offset_rectangle"}};
    for (int style=0;style<5;++style) for (int index=0;index<3;++index) {
        const WorldConfig world{seeds[index],0,&cache,&levels};
        bool selected=false;
        for (int attempt=0;attempt<10000 && !selected;++attempt) {
            const auto a=CellHash(world,style*3+index,attempt,4501);
            const auto b=CellHash(world,style*3+index,attempt,4507);
            const int rx=static_cast<int>(a%266)-133;
            const int rz=static_cast<int>(b%266)-133;
            const auto plan=OfficeColumnStyleAt(world,rx,rz);
            if (!plan || static_cast<int>(*plan)!=style) continue;
            // The central eight-metre square also avoids the legacy four
            // supports. This permits an exact camera match against format 43.
            const double x=rx*kRegionCells*kCellSize+11+((a>>12)%801)*0.01;
            const double z=rz*kRegionCells*kCellSize+11+((b>>12)%801)*0.01;
            if (Collides(world,x,z,0.55) || PortalAt(world,CellOf(x),CellOf(z))) continue;
            const double heading=(CellHash(world,rx,rz,4513)%6284)*0.001;
            std::cout << std::fixed << std::setprecision(6)
                      << "view seed " << world.seed << " level 0 sample " << style*3+index
                      << " region " << static_cast<int>(RegionKind::Columns)
                      << " position " << x << ',' << z << " heading " << heading
                      << " case " << cases[style] << '\n';
            selected=true;
        }
        if (!selected) throw std::runtime_error("could not select an office support plan");
    }
    return 0;
}

int WallEndViews() {
    const auto levels=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    RoomLayoutCache cache;
    for (int level=0;level<3;++level) {
        const WorldConfig world{12345,level,&cache,&levels};
        std::array<bool,6> selected{};
        for (int cx=kMin;cx<=kMax;++cx) for (int cz=kMin;cz<=kMax;++cz) {
            const bool north=VerticalEdge(world,cx,cz-1)!=Edge::Open;
            const bool south=VerticalEdge(world,cx,cz)!=Edge::Open;
            const bool west=HorizontalEdge(world,cx-1,cz)!=Edge::Open;
            const bool east=HorizontalEdge(world,cx,cz)!=Edge::Open;
            const int incident=north+south+west+east;
            const bool corner=incident==2 && north!=south && west!=east;
            // Enclosed industrial bays have no isolated cell-wall ends.
            // Inspect their straight joins instead of requiring a nonexistent case.
            const bool straight=level==1 && incident==2 &&
                                ((north && south) || (west && east));
            if (incident!=1 && !corner && !straight) continue;
            const int index=corner ? 2+south*2+east : north || south ? 0 : 1;
            if (selected[index]) continue;
            const double wx=cx*kCellSize,wz=cz*kCellSize;
            const double x=wx+(corner ? (west ? 0.85 : -0.85) :
                              straight ? (north ? 0.9 : 0) : west ? 0.9 : east ? -0.9 : 0);
            const double z=wz+(corner ? (north ? 0.85 : -0.85) :
                              straight ? (west ? 0.9 : 0) : north ? 0.9 : south ? -0.9 : 0);
            if (Collides(world,x,z,0.55) || PortalAt(world,CellOf(x),CellOf(z))) continue;
            selected[index]=true;
            std::cout << std::fixed << std::setprecision(6)
                      << "view seed 12345 level " << level << " sample " << index
                      << " region " << static_cast<int>(RegionAt(world,cx,cz))
                      << " position " << x << ',' << z
                      << " heading " << std::atan2(wx-x,wz-z)
                      << " case " << (corner ? "outer_corner" : straight ? "continuous_joint" : "exposed_end") << '\n';
        }
        if (!std::all_of(selected.begin(),selected.end(),[](bool found){return found;}))
            throw std::runtime_error("could not select all wall end orientations");
    }
    return 0;
}

int EntityApproaches() {
    const auto levels=LoadLevelCatalog(FindAssetDirectory()/"levels.json");
    RoomLayoutCache cache;
    for (int level=0;level<3;++level) {
        const WorldConfig world{12345,level,&cache,&levels};
        bool selected=false;
        for (int x=kMin;x<=kMax && !selected;++x)
            for (int z=kMin;z<=kMax && !selected;++z) {
                const auto entity=EntityAt(world,x,z);
                if (!entity) continue;
                for (int direction=0;direction<8 && !selected;++direction) {
                    const double angle=direction*0.785398163397;
                    const double px=entity->x+6*std::cos(angle);
                    const double pz=entity->z+6*std::sin(angle);
                    bool clear=true;
                    for (double step=0;step<=6;step+=0.25) {
                        const double t=step/6;
                        const double sx=px+(entity->x-px)*t;
                        const double sz=pz+(entity->z-pz)*t;
                        if (Collides(world,sx,sz,0.55) ||
                            PortalAt(world,CellOf(sx),CellOf(sz))) {
                            clear=false;break;
                        }
                    }
                    if (!clear) continue;
                    selected=true;
                    std::cout << std::fixed << std::setprecision(6)
                              << "approach seed 12345 level " << level
                              << " center " << entity->x << ',' << entity->z
                              << " phase " << entity->phase
                              << " position " << px << ',' << pz
                              << " heading " << std::atan2(entity->x-px,entity->z-pz) << '\n';
                }
            }
        if (!selected) throw std::runtime_error("no clear six-metre entity approach");
    }
    return 0;
}

int Run(bool showAlcoves,bool showPartitions,bool showEntities) {
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
                      << " | alcoves " << c.alcoves << " | entities " << c.entities
                      << " | region cells";
            for (int count:c.regions) std::cout << ' ' << count;
            std::cout << " | partition plans";
            for (int count:c.partitionPlans) std::cout << ' ' << count;
            std::cout << " | column plans";
            for (int count:c.columnPlans) std::cout << ' ' << count;
            std::cout << '\n';
            if (showEntities) {
                const WorldConfig world{seed,level,&cache,&levels};
                std::array<bool,3> sampled{};
                constexpr std::array<double,3> distances{{4.5,9,18}};
                for (int x=kMin;x<=kMax;++x) for (int z=kMin;z<=kMax;++z) {
                    const auto entity=EntityAt(world,x,z);
                    if (!entity) continue;
                    for (int band=0;band<3;++band) {
                        if (sampled[band]) continue;
                        for (int direction=0;direction<8;++direction) {
                            const double angle=direction*0.785398163397;
                            const double px=entity->x+distances[band]*std::cos(angle);
                            const double pz=entity->z+distances[band]*std::sin(angle);
                            if (Collides(world,px,pz,0.55) ||
                                PortalAt(world,CellOf(px),CellOf(pz))) continue;
                            bool clear=true;
                            for (double step=0;step<distances[band];step+=0.25) {
                                const double t=step/distances[band];
                                if (Collides(world,px+(entity->x-px)*t,
                                            pz+(entity->z-pz)*t,0.06)) {
                                    clear=false;break;
                                }
                            }
                            if (!clear) continue;
                            sampled[band]=true;
                            std::cout << std::setprecision(6) << "entity seed " << seed
                                      << " level " << level << " band " << band
                                      << " cell " << x << ',' << z
                                      << " position " << px << ',' << pz
                                      << " heading " << std::atan2(entity->x-px,entity->z-pz)
                                      << std::setprecision(1) << '\n';
                            break;
                        }
                    }
                }
                if (!sampled[0] || !sampled[1] || !sampled[2])
                    throw std::runtime_error("entity view sample missing a distance band");
            }
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
                      std::string(argv[1])!="--partitions" && std::string(argv[1])!="--entities" &&
                      std::string(argv[1])!="--views" && std::string(argv[1])!="--office-lighting" &&
                      std::string(argv[1])!="--wall-ends" && std::string(argv[1])!="--office-columns" &&
                      std::string(argv[1])!="--entity-approaches"))
            throw std::invalid_argument("usage: world_quality [--alcoves|--partitions|--entities|--views|--office-lighting|--office-columns|--wall-ends|--entity-approaches]");
        if (argc==2 && std::string(argv[1])=="--views") return ViewSamples();
        if (argc==2 && std::string(argv[1])=="--office-lighting") return OfficeLightingViews();
        if (argc==2 && std::string(argv[1])=="--office-columns") return OfficeColumnViews();
        if (argc==2 && std::string(argv[1])=="--wall-ends") return WallEndViews();
        if (argc==2 && std::string(argv[1])=="--entity-approaches") return EntityApproaches();
        return Run(argc==2 && std::string(argv[1])=="--alcoves",
                   argc==2 && std::string(argv[1])=="--partitions",
                   argc==2 && std::string(argv[1])=="--entities");
    }
    catch (const std::exception& e) {
        std::cerr << "world audit: " << e.what() << '\n';
        return 1;
    }
}

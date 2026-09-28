#include "World.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <set>

using namespace Backrooms;

namespace {
void Check(bool passed, const char* expression, int line) {
    if (passed) return;
    std::cerr << "world test failed at line " << line << ": "
              << expression << '\n';
    std::exit(EXIT_FAILURE);
}
}

#define CHECK(expression) Check((expression), #expression, __LINE__)

int main() {
    RoomLayoutCache cache;
    const WorldConfig world{12345,0,&cache};
    CHECK(CellOf(-0.01)==-1);
    CHECK(ChunkOfCell(-1)==-1);
    CHECK(ChunkOfCell(-8)==-1);
    CHECK(ChunkOfCell(-9)==-2);
    CHECK(ChunkAt(-0.01,-40.01)==(ChunkCoord{-1,-2}));
    // Golden values make changes to the versioned procedural world explicit.
    CHECK(kFormatVersion==24);
    CHECK(CellHash(world,12,-8,41)==511389911U);
    CHECK(VerticalEdge(world,8,3)==Edge::Open);
    CHECK(HorizontalEdge(world,-4,-5)==Edge::Open);
    CHECK(CellHash(world,12,-8,41)!=CellHash(world,13,-8,41));
    // Every complete multi-region rectangle must be reachable, including negative cells.
    for (std::uint64_t seed: {0ULL,12345ULL,31337ULL,0xffffffffffffffffULL})
    for (int level=0;level<3;++level) {
        WorldConfig selected{seed,level,&cache};
        std::queue<std::pair<int,int>> pending;
        std::set<std::pair<int,int>> visited;
        pending.push({0,0}); visited.insert({0,0});
        while (!pending.empty()) {
            const auto [cx,cz]=pending.front(); pending.pop();
            const auto visit=[&](int nx,int nz,Edge edge) {
                if (nx< -12 || nx>=12 || nz< -12 || nz>=12 || edge==Edge::Solid)
                    return;
                if (visited.insert({nx,nz}).second) pending.push({nx,nz});
            };
            visit(cx+1,cz,VerticalEdge(selected,cx+1,cz));
            visit(cx-1,cz,VerticalEdge(selected,cx,cz));
            visit(cx,cz+1,HorizontalEdge(selected,cx,cz+1));
            visit(cx,cz-1,HorizontalEdge(selected,cx,cz));
        }
        CHECK(visited.size()==24*24);
    }
    // Caching is bounded and leaves pure generation unchanged after eviction.
    const WorldConfig uncached{12345,0};
    for (int cx=-18;cx<18;++cx) for (int cz=-18;cz<18;++cz) {
        CHECK(VerticalEdge(world,cx,cz)==VerticalEdge(uncached,cx,cz));
        CHECK(HorizontalEdge(world,cx,cz)==HorizontalEdge(uncached,cx,cz));
    }
    const auto original=cache.Get(world,-3,2,RegionKind::Rooms);
    for (std::size_t i=0;i<RoomLayoutCache::kCapacity+40;++i) {
        cache.Get(world,static_cast<int>(i),-31,RegionKind::Rooms);
        CHECK(cache.Size()<=RoomLayoutCache::kCapacity);
    }
    const auto restored=cache.Get(world,-3,2,RegionKind::Rooms);
    CHECK(original.east==restored.east && original.south==restored.south);
    double x=2.5,z=2.5;
    CHECK(!Collides(world,x,z,0.31));
    // A solid partition blocks normal movement; this checks every deterministic edge type.
    bool found=false;
    for (int boundary=-30;boundary<30 && !found;++boundary) {
        if (VerticalEdge(world,boundary,1)!=Edge::Solid) continue;
        x=boundary*kCellSize-0.7;
        z=1*kCellSize+2.5;
        if (Collides(world,x,z,0.31)) continue;
        MoveWithCollision(world,x,z,2.0,0.0,0.31);
        CHECK(x<boundary*kCellSize);
        found=true;
    }
    CHECK(found);
    found=false;
    for (int boundary=-30;boundary<30 && !found;++boundary) {
        if (VerticalEdge(world,boundary,1)!=Edge::Door) continue;
        x=boundary*kCellSize-0.7;
        z=1*kCellSize+2.5;
        MoveWithCollision(world,x,z,2.0,0.0,0.31);
        CHECK(x>boundary*kCellSize);
        found=true;
    }
    CHECK(found);
    const WorldConfig tunnels{12345,2,&cache};
    CHECK(CellObstacles(tunnels,0,0).count==0 ||
          CellObstacles(tunnels,0,0).count==2 ||
          CellObstacles(tunnels,0,0).count==4);
    int pairedCabinets=0,sparsePressureBanks=0;
    for (int cx=-60;cx<60;++cx) for (int cz=-60;cz<60;++cz) {
        if (RegionAt(tunnels,cx,cz)!=RegionKind::Tunnels ||
            IsServiceChamber(tunnels,cx,cz) || PortalAt(tunnels,cx,cz) ||
            PropAt(tunnels,cx,cz).kind!=PropKind::None) continue;
        const auto equipment=CellObstacles(tunnels,cx,cz);
        pairedCabinets+=equipment.count==4;
        sparsePressureBanks+=equipment.count==2;
        CHECK(equipment.count==0 || equipment.count==2 || equipment.count==4);
        CHECK(!Collides(tunnels,(cx+0.5)*kCellSize,(cz+0.5)*kCellSize,0.55));
        for (int i=0;i<equipment.count;++i) {
            const auto& bounds=equipment.walls[i];
            CHECK(Collides(tunnels,(bounds.minX+bounds.maxX)*0.5,
                           (bounds.minZ+bounds.maxZ)*0.5,0.31));
        }
    }
    CHECK(pairedCabinets>100 && sparsePressureBanks>100);
    CHECK(!Collides(tunnels,2.5,2.5,0.31));
    bool sparseBay=false,denseBay=false;
    const WorldConfig storage{12345,1,&cache};
    for (int rx=-8;rx<=8;++rx) for (int rz=-8;rz<=8;++rz) {
        if (RegionAt(storage,rx*6,rz*6)!=RegionKind::Storage) continue;
        int racks=0,pillars=0,openEdges=0,totalEdges=0;
        for (int lx=0;lx<6;++lx) for (int lz=0;lz<6;++lz) {
            const int cx=rx*6+lx,cz=rz*6+lz;
            pillars+=FullHeightObstaclesAt(storage,cx,cz).count;
            if (lx>0) { ++totalEdges;openEdges+=VerticalEdge(storage,cx,cz)==Edge::Open; }
            if (lz>0) { ++totalEdges;openEdges+=HorizontalEdge(storage,cx,cz)==Edge::Open; }
            if (PropAt(storage,cx,cz).kind!=PropKind::None ||
                CellObstacles(storage,cx,cz).count!=1 ||
                FullHeightObstaclesAt(storage,cx,cz).count!=0) continue;
            ++racks;
            CHECK(Collides(storage,(cx+0.5)*kCellSize,
                           (cz+0.5)*kCellSize,0.31));
        }
        CHECK(pillars>=2 && pillars<=4);
        CHECK(openEdges>totalEdges*0.75);
        sparseBay|=racks==2;
        denseBay|=racks==4;
    }
    CHECK(sparseBay && denseBay);
    // Industrial columns occupy real floor space, while larger room zones
    // leave the majority of their internal edges open.
    int industrialColumns=0,industrialOpenEdges=0,industrialEdges=0;
    for (int rx=-8;rx<=8;++rx) for (int rz=-8;rz<=8;++rz) {
        if (RegionAt(storage,rx*6,rz*6)!=RegionKind::Columns) continue;
        for (int lx=0;lx<6;++lx) for (int lz=0;lz<6;++lz) {
            const int cx=rx*6+lx,cz=rz*6+lz;
            const auto pillars=FullHeightObstaclesAt(storage,cx,cz);
            industrialColumns+=pillars.count;
            for (int i=0;i<pillars.count;++i) {
                const auto& bounds=pillars.walls[i];
                CHECK(Collides(storage,(bounds.minX+bounds.maxX)*0.5,
                               (bounds.minZ+bounds.maxZ)*0.5,0.31));
                CHECK(PropAt(storage,cx,cz).kind==PropKind::None);
            }
            if (lx>0) {
                ++industrialEdges;
                industrialOpenEdges+=VerticalEdge(storage,cx,cz)==Edge::Open;
            }
            if (lz>0) {
                ++industrialEdges;
                industrialOpenEdges+=HorizontalEdge(storage,cx,cz)==Edge::Open;
            }
            const auto fixture=LampAt(storage,cx,cz);
            if (fixture.fixture) {
                const double hx=fixture.longAxisX ? 1.20 : 0.23;
                const double hz=fixture.longAxisX ? 0.23 : 1.20;
                const double wx=cx*kCellSize+fixture.x;
                const double wz=cz*kCellSize+fixture.z;
                CHECK(fixture.x-hx>0 && fixture.x+hx<kCellSize);
                CHECK(fixture.z-hz>0 && fixture.z+hz<kCellSize);
                for (int i=0;i<pillars.count;++i) {
                    const auto& bounds=pillars.walls[i];
                    CHECK(!(wx+hx>bounds.minX && wx-hx<bounds.maxX &&
                            wz+hz>bounds.minZ && wz-hz<bounds.maxZ));
                }
            }
        }
    }
    CHECK(industrialColumns>100);
    CHECK(industrialOpenEdges>industrialEdges*0.90);
    const CellProp firstChair=PropAt(world,2,1);
    CHECK(firstChair.kind==PropKind::Chair);
    CHECK(PropAt(world,2,1).x==firstChair.x);
    CHECK(Collides(world,firstChair.x,firstChair.z,0.31));
    CHECK(PropAt(world,0,0).kind==PropKind::None);
    // Shared prop bounds include the complete rotated frame and drive collision.
    for (const auto kind: {PropKind::Chair,PropKind::Table,PropKind::LowPartition,
                          PropKind::TallPartition}) {
        CellProp prop{kind,12.0,-9.0,0,0};
        const auto straight=PropBounds(prop);
        prop.quarterTurn=1;
        const auto turned=PropBounds(prop);
        CHECK(std::abs((straight.maxX-straight.minX)-
                       (turned.maxZ-turned.minZ))<1e-8);
        CHECK(std::abs((straight.maxZ-straight.minZ)-
                       (turned.maxX-turned.minX))<1e-8);
    }
    bool foundEmbedded=false;
    for (int ix=-80;ix<=80 && !foundEmbedded;++ix) {
        for (int iz=-80;iz<=80 && !foundEmbedded;++iz) {
            const auto prop=PropAt(world,ix,iz);
            if (prop.kind!=PropKind::EmbeddedChair) continue;
            CHECK(prop.sink>0);
            CHECK(Collides(world,prop.x,prop.z,0.31));
            foundEmbedded=true;
        }
    }
    bool openingLeft=false,openingRight=false;
    int checkedOpenings=0;
    for (int bx=-20;bx<=20;++bx) for (int bz=-20;bz<=20;++bz) {
        const Edge edge=VerticalEdge(world,bx,bz);
        if (edge!=Edge::Door && edge!=Edge::Wide) continue;
        const auto span=OpeningForEdge(world,edge,true,bx,bz);
        CHECK(span==OpeningForEdge(world,edge,true,bx,bz));
        CHECK(span.start>=0.4 && span.end<=4.6);
        CHECK(std::abs((span.end-span.start)-
                       (edge==Edge::Wide ? 3.5 : 1.9))<0.00001);
        const double displacement=(span.start+span.end)*0.5-2.5;
        openingLeft |= displacement < -0.12;
        openingRight |= displacement > 0.12;
        const double z=bz*kCellSize+(span.start+span.end)*0.5;
        if (std::abs(bx)>4 || std::abs(bz)>4) {
            CHECK(!Collides(world,bx*kCellSize,z,0.31));
            ++checkedOpenings;
        }
    }
    CHECK(openingLeft && openingRight && checkedOpenings>100);
    // Interior room partitions must not turn a generated door into a dead end.
    for (std::uint64_t seed: {0ULL,1ULL,12345ULL,31337ULL}) {
        const WorldConfig selected{seed,0,&cache};
        for (int bx=-60;bx<=60;++bx) for (int bz=-60;bz<=60;++bz)
            for (bool vertical: {false,true}) {
                const Edge edge=vertical ? VerticalEdge(selected,bx,bz) :
                                           HorizontalEdge(selected,bx,bz);
                if (edge!=Edge::Door && edge!=Edge::Wide) continue;
                const auto span=OpeningForEdge(selected,edge,vertical,bx,bz);
                const double middle=(span.start+span.end)*0.5;
                const double px=(bx+(vertical ? 0.0 : middle/kCellSize))*kCellSize;
                const double pz=(bz+(vertical ? middle/kCellSize : 0.0))*kCellSize;
                CHECK(!Collides(selected,px,pz,0.31));
            }
    }
    CHECK(foundEmbedded);
    bool foundLowPartition=false;
    for (int ix=-30;ix<=30 && !foundLowPartition;++ix)
        for (int iz=-30;iz<=30 && !foundLowPartition;++iz) {
            const auto prop=PropAt(world,ix,iz);
            if (prop.kind!=PropKind::LowPartition) continue;
            CHECK(Collides(world,prop.x,prop.z,0.31));
            foundLowPartition=true;
        }
    CHECK(foundLowPartition);
    bool foundTallPartition=false;
    for (int ix=-50;ix<=50 && !foundTallPartition;++ix)
        for (int iz=-50;iz<=50 && !foundTallPartition;++iz) {
            const auto prop=PropAt(world,ix,iz);
            if (prop.kind!=PropKind::TallPartition) continue;
            CHECK(Collides(world,prop.x,prop.z,0.31));
            foundTallPartition=true;
        }
    CHECK(foundTallPartition);
    bool foundInterior=false;
    for (int ix=-60;ix<=60 && !foundInterior;++ix)
        for (int iz=-60;iz<=60 && !foundInterior;++iz) {
            const auto pieces=InteriorPartitionsAt(world,ix,iz);
            CHECK(pieces.count<=2);
            if (!pieces.count) continue;
            const auto repeat=InteriorPartitionsAt(world,ix,iz);
            CHECK(repeat.count==pieces.count);
            const auto& wall=pieces.walls[0];
            const double px=(wall.minX+wall.maxX)*0.5;
            const double pz=(wall.minZ+wall.maxZ)*0.5;
            CHECK(Collides(world,px,pz,0.31));
            foundInterior=true;
        }
    CHECK(foundInterior);
    bool foundEmptyHall=false;
    for (int ix=-120;ix<=120 && !foundEmptyHall;ix+=12)
        for (int iz=-120;iz<=120 && !foundEmptyHall;iz+=12) {
            if (!IsEmptyHall(world,ix,iz)) continue;
            for (int dx=0;dx<12;++dx) for (int dz=0;dz<12;++dz) {
                CHECK(IsEmptyHall(world,ix+dx,iz+dz));
                CHECK(PropAt(world,ix+dx,iz+dz).kind==PropKind::None);
                CHECK(CellObstacles(world,ix+dx,iz+dz).count==0);
                if (dx>0) CHECK(VerticalEdge(world,ix+dx,iz+dz)==Edge::Open);
                if (dz>0) CHECK(HorizontalEdge(world,ix+dx,iz+dz)==Edge::Open);
            }
            foundEmptyHall=true;
        }
    CHECK(foundEmptyHall);
    bool foundChamber=false;
    for (int ix=-60;ix<=60 && !foundChamber;ix+=6)
        for (int iz=-60;iz<=60 && !foundChamber;iz+=6) {
            if (!IsServiceChamber(tunnels,ix,iz)) continue;
            for (int dx=0;dx<6;++dx) for (int dz=0;dz<6;++dz) {
                CHECK(IsServiceChamber(tunnels,ix+dx,iz+dz));
                CHECK(PropAt(tunnels,ix+dx,iz+dz).kind==PropKind::None);
                if (dx>0)
                    CHECK(VerticalEdge(tunnels,ix+dx,iz+dz)==Edge::Open);
                if (dz>0)
                    CHECK(HorizontalEdge(tunnels,ix+dx,iz+dz)==Edge::Open);
                if ((dx==1 || dx==4) && (dz==1 || dz==4) &&
                    !PortalAt(tunnels,ix+dx,iz+dz))
                    CHECK(CellObstacles(tunnels,ix+dx,iz+dz).count==1);
            }
            foundChamber=true;
        }
    CHECK(foundChamber);
    // Check that the movement collider agrees with generated openings, including
    // region and chunk boundaries on both sides of the origin.
    for (int level=0;level<3;++level) {
        const WorldConfig selected{12345,level,&cache};
        for (int bx=-7;bx<=7;++bx) for (int bz=-7;bz<=7;++bz) {
            const double borderX=bx*kCellSize;
            const double midZ=(bz+0.5)*kCellSize;
            double px=borderX-0.75,pz=midZ;
            if (!Collides(selected,px,pz,0.31) &&
                !Collides(selected,borderX+0.75,pz,0.31)) {
                MoveWithCollision(selected,px,pz,1.5,0,0.31);
                if (VerticalEdge(selected,bx,bz)==Edge::Solid)
                    CHECK(px<borderX-0.25);
                else CHECK(px>borderX+0.25);
            }
            const double midX=(bx+0.5)*kCellSize;
            const double borderZ=bz*kCellSize;
            px=midX; pz=borderZ-0.75;
            if (!Collides(selected,px,pz,0.31) &&
                !Collides(selected,px,borderZ+0.75,0.31)) {
                MoveWithCollision(selected,px,pz,0,1.5,0.31);
                if (HorizontalEdge(selected,bx,bz)==Edge::Solid)
                    CHECK(pz<borderZ-0.25);
                else CHECK(pz>borderZ+0.25);
            }
        }
    }
    for (const auto& portal:kPortals) {
        const WorldConfig selected{12345,portal.level,&cache};
        const double cx=(portal.cellX+0.5)*kCellSize;
        const double cz=(portal.cellZ+0.5)*kCellSize;
        if (portal.alongX) {
            CHECK(!Collides(selected,cx+1.10,cz,0.31));
            CHECK(Collides(selected,cx+1.10,cz+kPortalHalfWidth+0.08,0.31));
            CHECK(Collides(selected,cx+1.92,cz,0.31));
            double px=cx,pz=cz;
            MoveWithCollision(selected,px,pz,1.25,0,0.31);
            CHECK(px>cx+1.0);
        } else {
            CHECK(!Collides(selected,cx,cz+1.10,0.31));
            CHECK(Collides(selected,cx+kPortalHalfWidth+0.08,cz+1.10,0.31));
            CHECK(Collides(selected,cx,cz+1.92,0.31));
            double px=cx,pz=cz;
            MoveWithCollision(selected,px,pz,0,1.25,0.31);
            CHECK(pz>cz+1.0);
        }
    }
    for (const auto& portal:kPortals) {
        const WorldConfig selected{12345,portal.level,&cache};
        const double x=(portal.cellX+0.5)*kCellSize;
        const double z=(portal.cellZ+0.5)*kCellSize;
        const double tx=x+(portal.alongX ? 1.0 : 0.0);
        const double tz=z+(portal.alongX ? 0.0 : 1.0);
        CHECK(PortalTarget(selected,tx,tz)==portal.target);
        CHECK(!PortalTarget(selected,x,z));
        const double side=kPortalHalfWidth+kPortalWallThickness+0.36;
        const double outsideX=tx+(portal.alongX ? 0.0 : side);
        const double outsideZ=tz+(portal.alongX ? side : 0.0);
        CHECK(!Collides(selected,outsideX,outsideZ,0.31));
        CHECK(!PortalTarget(selected,outsideX,outsideZ));
        CHECK(PortalWalls(portal).count==3);
    }
    // Rare entrances share the rendered frame, collider, and trigger geometry.
    for (int level=0;level<3;++level) {
        const WorldConfig selected{12345,level,&cache};
        int cellX=0,cellZ=0;
        bool foundPortal=false;
        for (int ix=-96;ix<=96 && !foundPortal;ix+=32)
            for (int iz=-96;iz<=96 && !foundPortal;iz+=32)
                if (PortalAt(selected,ix+16,iz+16)) {
                    cellX=ix+16;cellZ=iz+16;foundPortal=true;
                }
        CHECK(foundPortal);
        const auto portal=PortalAt(selected,cellX,cellZ);
        CHECK(portal.has_value());
        CHECK(portal->target==(level+1)%3);
        CHECK(portal->alongX);
        CHECK(PortalAt(selected,cellX,cellZ)==portal);
        CHECK(PropAt(selected,cellX,cellZ).kind==PropKind::None);
        CHECK(CellObstacles(selected,cellX,cellZ).count==0);
        const double cx=(cellX+0.5)*kCellSize;
        const double cz=(cellZ+0.5)*kCellSize;
        CHECK(!Collides(selected,cx+1.10,cz,0.31));
        CHECK(Collides(selected,cx+1.10,cz+kPortalHalfWidth+0.08,0.31));
        CHECK(Collides(selected,cx+1.92,cz,0.31));
    }
    // Troffers replace two by one ceiling tiles, including negative cells.
    // Keep them off structural columns and room-spanning partitions.
    int fixtures=0,deadFixtures=0;
    for (const auto seed:{0ULL,1ULL,12345ULL,31337ULL}) {
        const WorldConfig office{seed,0,&cache};
        for (int x=-64;x<64;++x) for (int z=-64;z<64;++z) {
            const auto lamp=LampAt(office,x,z);
            CHECK(lamp==LampAt(office,x,z));
            CHECK(!lamp.lit || lamp.fixture);
            if (!lamp.fixture) continue;
            ++fixtures;if (!lamp.lit) ++deadFixtures;
            const double hx=lamp.longAxisX ? 0.625 : 0.3125;
            const double hz=lamp.longAxisX ? 0.3125 : 0.625;
            for (const double edge:{lamp.x-hx,lamp.x+hx,lamp.z-hz,lamp.z+hz})
                CHECK(std::abs(edge/0.625-std::round(edge/0.625))<1e-5);
            CHECK(lamp.x-hx>=0.12 && lamp.x+hx<=4.88);
            CHECK(lamp.z-hz>=0.12 && lamp.z+hz<=4.88);
            const double wx=x*kCellSize+lamp.x,wz=z*kCellSize+lamp.z;
            auto structure=FullHeightObstaclesAt(office,x,z);
            if (const auto portal=PortalAt(office,x,z)) {
                const auto frames=PortalWalls(*portal);
                for (int n=0;n<2;++n)
                    structure.walls[structure.count++]=frames.walls[n];
            }
            for (int n=0;n<structure.count;++n) {
                const auto& wall=structure.walls[n];
                CHECK(!(wx+hx>wall.minX-0.03 && wx-hx<wall.maxX+0.03 &&
                        wz+hz>wall.minZ-0.03 && wz-hz<wall.maxZ+0.03));
            }
        }
    }
    CHECK(fixtures>1000 && deadFixtures>100);
    std::cout << "world tests passed\n";
}

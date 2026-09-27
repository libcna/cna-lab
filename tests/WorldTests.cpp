#include "World.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <queue>
#include <set>

using namespace Backrooms;

int main() {
    const WorldConfig world{12345,0};
    assert(CellOf(-0.01)==-1);
    assert(ChunkOfCell(-1)==-1);
    assert(ChunkOfCell(-8)==-1);
    assert(ChunkOfCell(-9)==-2);
    assert(ChunkAt(-0.01,-40.01)==(ChunkCoord{-1,-2}));
    assert(CellHash(world,12,-8,41)==CellHash(world,12,-8,41));
    assert(CellHash(world,12,-8,41)!=CellHash(world,13,-8,41));
    // Every complete multi-region rectangle must be reachable, including negative cells.
    for (std::uint64_t seed: {0ULL,12345ULL,31337ULL,0xffffffffffffffffULL})
    for (int level=0;level<3;++level) {
        WorldConfig selected{seed,level};
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
        assert(visited.size()==24*24);
    }
    double x=2.5,z=2.5;
    assert(!Collides(world,x,z,0.31));
    // A solid partition blocks normal movement; this checks every deterministic edge type.
    bool found=false;
    for (int boundary=-30;boundary<30 && !found;++boundary) {
        if (VerticalEdge(world,boundary,1)!=Edge::Solid) continue;
        x=boundary*kCellSize-0.7;
        z=1*kCellSize+2.5;
        if (Collides(world,x,z,0.31)) continue;
        MoveWithCollision(world,x,z,2.0,0.0,0.31);
        assert(x<boundary*kCellSize);
        found=true;
    }
    assert(found);
    found=false;
    for (int boundary=-30;boundary<30 && !found;++boundary) {
        if (VerticalEdge(world,boundary,1)!=Edge::Door) continue;
        x=boundary*kCellSize-0.7;
        z=1*kCellSize+2.5;
        MoveWithCollision(world,x,z,2.0,0.0,0.31);
        assert(x>boundary*kCellSize);
        found=true;
    }
    assert(found);
    const WorldConfig tunnels{12345,2};
    assert(CellObstacles(tunnels,0,0).count==4);
    assert(Collides(tunnels,0.5,0.5,0.31));
    assert(!Collides(tunnels,2.5,2.5,0.31));
    const CellProp firstChair=PropAt(world,2,1);
    assert(firstChair.kind==PropKind::Chair);
    assert(PropAt(world,2,1).x==firstChair.x);
    assert(Collides(world,firstChair.x,firstChair.z,0.31));
    assert(PropAt(world,0,0).kind==PropKind::None);
    bool foundEmbedded=false;
    for (int ix=-80;ix<=80 && !foundEmbedded;++ix) {
        for (int iz=-80;iz<=80 && !foundEmbedded;++iz) {
            const auto prop=PropAt(world,ix,iz);
            if (prop.kind!=PropKind::EmbeddedChair) continue;
            assert(prop.sink>0);
            assert(Collides(world,prop.x,prop.z,0.31));
            foundEmbedded=true;
        }
    }
    assert(foundEmbedded);
    bool foundLowPartition=false;
    for (int ix=-30;ix<=30 && !foundLowPartition;++ix)
        for (int iz=-30;iz<=30 && !foundLowPartition;++iz) {
            const auto prop=PropAt(world,ix,iz);
            if (prop.kind!=PropKind::LowPartition) continue;
            assert(Collides(world,prop.x,prop.z,0.31));
            foundLowPartition=true;
        }
    assert(foundLowPartition);
    // Check that the movement collider agrees with generated openings, including
    // region and chunk boundaries on both sides of the origin.
    for (int level=0;level<3;++level) {
        const WorldConfig selected{12345,level};
        for (int bx=-7;bx<=7;++bx) for (int bz=-7;bz<=7;++bz) {
            const double borderX=bx*kCellSize;
            const double midZ=(bz+0.5)*kCellSize;
            double px=borderX-0.75,pz=midZ;
            if (!Collides(selected,px,pz,0.31) &&
                !Collides(selected,borderX+0.75,pz,0.31)) {
                MoveWithCollision(selected,px,pz,1.5,0,0.31);
                if (VerticalEdge(selected,bx,bz)==Edge::Solid)
                    assert(px<borderX-0.25);
                else assert(px>borderX+0.25);
            }
            const double midX=(bx+0.5)*kCellSize;
            const double borderZ=bz*kCellSize;
            px=midX; pz=borderZ-0.75;
            if (!Collides(selected,px,pz,0.31) &&
                !Collides(selected,px,borderZ+0.75,0.31)) {
                MoveWithCollision(selected,px,pz,0,1.5,0.31);
                if (HorizontalEdge(selected,bx,bz)==Edge::Solid)
                    assert(pz<borderZ-0.25);
                else assert(pz>borderZ+0.25);
            }
        }
    }
    std::cout << "world tests passed\n";
}

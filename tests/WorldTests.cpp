#include "World.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

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
    assert(VerticalEdge(world,100,0)==Edge::Open);
    assert(HorizontalEdge(world,0,-100)==Edge::Open);
    // The center of every guaranteed corridor can be crossed in either direction.
    double x=2.5,z=2.5;
    MoveWithCollision(world,x,z,20.0,0.0,0.31);
    assert(std::abs(x-22.5)<0.001);
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
    std::cout << "world tests passed\n";
}

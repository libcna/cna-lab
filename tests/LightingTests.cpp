#include "Lighting.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace Backrooms;

namespace {
void Check(bool passed,const char* expression,int line) {
    if (passed) return;
    std::cerr << "lighting test failed at line " << line << ": " << expression << '\n';
    std::exit(EXIT_FAILURE);
}
}
#define CHECK(expression) Check((expression),#expression,__LINE__)

int main() {
    RoomLayoutCache cache;
    // Independent chunk bakes must produce identical shared-boundary values,
    // including after layout eviction and at negative coordinates.
    for (int level=0;level<3;++level) {
        WorldConfig world{12345,level,&cache};
        BakedLighting first(world);
        const WorldConfig pure{12345,level};
        BakedLighting second(pure);
        int litSamples=0;
        for (int cx=-2;cx<=2;++cx) for (int cz=-2;cz<=2;++cz) {
            const double x=cx*kChunkSize,z=cz*kChunkSize;
            const float floor=first.Sample(x,z);
            CHECK(floor==second.Sample(x,z));
            CHECK(std::isfinite(floor) && floor>=LevelInfo(world).ambient && floor<=1);
            // The interpolated contact shade agrees exactly with floor vertices.
            CHECK(std::abs(first.FloorSample(x,z)-floor)<1e-6f);
            const float low=first.WallSample(x+0.14,z+2.5,1,0,0);
            const float high=first.WallSample(x+0.14,z+2.5,1,0,
                                               LevelInfo(world).ceilingHeight);
            CHECK(low==second.WallSample(x+0.14,z+2.5,1,0,0));
            CHECK(high==second.WallSample(x+0.14,z+2.5,1,0,
                                         LevelInfo(world).ceilingHeight));
            CHECK(std::isfinite(low) && low>0 && low<=1.03f);
            CHECK(std::isfinite(high) && high>=low && high<=1.03f);
            litSamples+=high>low+0.005f;
        }
        CHECK(litSamples>0);
        const float before=first.Sample(-40,0);
        for (int i=0;i<static_cast<int>(RoomLayoutCache::kCapacity)+16;++i)
            cache.Get(world,i,-31,RegionKind::Rooms);
        BakedLighting afterEviction(world);
        CHECK(afterEviction.Sample(-40,0)==before);
    }
    std::cout << "lighting tests passed\n";
}

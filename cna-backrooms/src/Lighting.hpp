#pragma once

#include "World.hpp"

namespace Backrooms {

// Keep contact shading on the same triangles as the visible floor bake.
constexpr int FloorGridDivisions(int level) { return level==0 ? 4 : 2; }

// A small reflected contribution softens the office's isolated light pools.
// Keep service floors and the ceiling/wall response unchanged.
constexpr float FloorLightResponse(int level,float illumination) {
    return level==0 ? 0.16f+0.84f*illumination : illumination;
}

// A chunk-local CPU bake. Caches die after its vertices are uploaded; none
// of these values becomes permanently active state in the streamed world.
class BakedLighting {
public:
    explicit BakedLighting(const WorldConfig& world):world_(world) {}
    // Without a height, use the broad horizontal floor/ceiling response.
    float Sample(double x,double z,int normalX=0,int normalZ=0,double height=-1);
    float WallSample(double x,double z,int normalX,int normalZ,double height=-1);
    float FloorSample(double x,double z);
private:
    const WorldConfig& world_;
    std::map<std::pair<int,int>,std::vector<Wall>> walls_;
    std::map<std::pair<int,int>,LampInfo> lamps_;
    std::map<std::tuple<double,double,int,int,double>,float> samples_;
};

} // namespace Backrooms

#include "Lighting.hpp"

#include <algorithm>
#include <cmath>

namespace Backrooms {
namespace {
bool LightBlocked(const std::vector<Wall>& walls, double x, double z,
                  double lightX, double lightZ) {
    const double dx=lightX-x,dz=lightZ-z;
    for (const auto& wall:walls) {
        double enter=0,leave=1;
        const auto clip=[&](double start,double direction,double low,double high) {
            if (std::abs(direction)<1e-8) return start>=low && start<=high;
            double a=(low-start)/direction,b=(high-start)/direction;
            if (a>b) std::swap(a,b);
            enter=std::max(enter,a);leave=std::min(leave,b);
            return enter<=leave;
        };
        if (clip(x,dx,wall.minX,wall.maxX) &&
            clip(z,dz,wall.minZ,wall.maxZ) && enter>1e-5 && enter<0.98)
            return true;
    }
    return false;
}
}

float BakedLighting::Sample(double wx,double wz,int normalX,int normalZ,double height) {
    const auto key=std::tuple{wx,wz,normalX,normalZ,height};
    if (const auto it=samples_.find(key);it!=samples_.end()) return it->second;
    const auto& definition=LevelInfo(world_);
    const float strength=definition.lightStrength;
    float value=definition.ambient;
    const int cx=CellOf(wx),cz=CellOf(wz);
    const std::vector<Wall>* walls=nullptr;
    auto [wallIt,insertedWalls]=walls_.try_emplace({cx,cz});
    if (insertedWalls) wallIt->second=NearbyFullHeightWalls(world_,wx,wz);
    walls=&wallIt->second;
    for (int dx=-1;dx<=1;++dx) for (int dz=-1;dz<=1;++dz) {
        const int gx=cx+dx,gz=cz+dz;
        auto [it,inserted]=lamps_.try_emplace({gx,gz});
        if (inserted) it->second=LampAt(world_,gx,gz);
        const auto& lamp=it->second;
        if (!lamp.lit) continue;
        const double lightX=gx*kCellSize+lamp.x;
        const double lightZ=gz*kCellSize+lamp.z;
        const double lx=lightX-wx,lz=lightZ-wz;
        const double dy=height<0 ? 0 : std::max(0.12,
                            definition.ceilingHeight-0.04-height);
        const float attenuation=std::max(0.0f,1.0f-
            static_cast<float>(std::sqrt(lx*lx+lz*lz+dy*dy))/6.5f);
        if (attenuation<=0) continue;
        float incidence=1;
        if (normalX || normalZ)
            incidence=static_cast<float>(std::max(0.0,
                (lx*normalX+lz*normalZ)/std::sqrt(lx*lx+lz*lz+
                                                   (height<0 ? 2.25 : dy*dy))));
        // Keep ambient/bounced illumination in enclosed spaces. These
        // values become vertex colors; there are no runtime shadow maps.
        const float visibility=walls && LightBlocked(*walls,wx,wz,lightX,lightZ) ? 0.12f : 1.0f;
        // An area fixture has a readable local pool on the office wall.
        // The floor/ceiling samples retain their broader response.
        const float response=height<0 ? attenuation : 2*attenuation*attenuation;
        value+=strength*response*visibility*incidence;
    }
    value=std::min(1.0f,value);
    samples_.emplace(key,value);
    return value;
}

float BakedLighting::WallSample(double wx,double wz,int normalX,int normalZ,double height) {
    const double span=kRegionCells*kCellSize;
    const int rx=static_cast<int>(std::floor(wx/span));
    const int rz=static_cast<int>(std::floor(wz/span));
    const float u=static_cast<float>(wx/span-rx);
    const float v=static_cast<float>(wz/span-rz);
    const auto finish=[&](int x,int z) {
        return 0.98f+static_cast<float>(CellHash(world_,x,z,4001)%401)*0.0001f;
    };
    const float a=finish(rx,rz)*(1-u)+finish(rx+1,rz)*u;
    const float b=finish(rx,rz+1)*(1-u)+finish(rx+1,rz+1)*u;
    const float bounce=LevelInfo(world_).wallBounce;
    return (bounce+(1-bounce)*Sample(wx,wz,normalX,normalZ,height))*(a*(1-v)+b*v);
}

float BakedLighting::FloorSample(double wx,double wz) {
    // Match the existing floor triangles rather than evaluating a brighter
    // lamp sample at the prop center. Contact shading must only darken it.
    const double x=std::floor(wx/2.5)*2.5,z=std::floor(wz/2.5)*2.5;
    const float u=static_cast<float>((wx-x)/2.5);
    const float v=static_cast<float>((wz-z)/2.5);
    const float a=Sample(x,z),b=Sample(x+2.5,z);
    const float c=Sample(x+2.5,z+2.5),d=Sample(x,z+2.5);
    return u>=v ? a*(1-u)+b*(u-v)+c*v :
                  a*(1-v)+c*u+d*(v-u);
}

} // namespace Backrooms

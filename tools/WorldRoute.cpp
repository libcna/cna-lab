#include "World.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Backrooms;

namespace {
bool NearEntrance(const WorldConfig& world,double x,double z) {
    const auto portal=PortalAt(world,CellOf(x),CellOf(z));
    if (!portal) return false;
    const double cx=(portal->cellX+0.5)*kCellSize;
    const double cz=(portal->cellZ+0.5)*kCellSize;
    const double depth=portal->alongX ? x-cx : z-cz;
    const double side=portal->alongX ? z-cz : x-cx;
    return depth>-0.2 && depth<2.3 && std::abs(side)<kPortalHalfWidth+0.7;
}
}

// This is a QA route planner, not entity pathfinding or game-side navigation.
int main(int argc,char** argv) {
    try {
        if (argc!=5) throw std::invalid_argument("usage: world_route level seed target-x target-z");
        const WorldConfig world{std::stoull(argv[2],nullptr,0),std::stoi(argv[1])};
        const double tx=std::stod(argv[3]),tz=std::stod(argv[4]);
        if (world.level<0 || world.level>2 || !std::isfinite(tx) ||
            !std::isfinite(tz) || std::abs(tx)>1000 || std::abs(tz)>1000)
            throw std::invalid_argument("invalid route arguments");
        constexpr double startX=2.5,startZ=2.5,radius=0.55,margin=45;
        const double ox=std::floor(std::min(startX,tx)-margin)+0.5;
        const double oz=std::floor(std::min(startZ,tz)-margin)+0.5;
        const int width=static_cast<int>(std::ceil(std::max(startX,tx)+margin-ox));
        const int height=static_cast<int>(std::ceil(std::max(startZ,tz)+margin-oz));
        const int count=width*height;
        const auto x=[&](int index) { return ox+index%width; };
        const auto z=[&](int index) { return oz+index/width; };
        std::vector<std::int8_t> free(count,-1);
        const auto isFree=[&](int index) {
            if (free[index]<0)
                free[index]=!NearEntrance(world,x(index),z(index)) &&
                            !Collides(world,x(index),z(index),radius);
            return free[index]!=0;
        };
        const int start=static_cast<int>(startZ-oz)*width+static_cast<int>(startX-ox);
        if (!isFree(start)) throw std::runtime_error("spawn lacks route clearance");
        std::vector<int> parent(count,-1),queue;
        queue.reserve(count);queue.push_back(start);parent[start]=start;
        int best=start;
        double bestDistance=std::numeric_limits<double>::infinity();
        for (std::size_t cursor=0;cursor<queue.size();++cursor) {
            const int current=queue[cursor];
            const double distance=std::hypot(x(current)-tx,z(current)-tz);
            if (distance<bestDistance) { best=current;bestDistance=distance; }
            if (distance<0.1) break;
            const auto visit=[&](int next) {
                if (parent[next]>=0 || !isFree(next)) return;
                double px=x(current),pz=z(current);
                MoveWithCollision(world,px,pz,x(next)-px,z(next)-pz,radius);
                if (std::hypot(px-x(next),pz-z(next))>0.01) return;
                parent[next]=current;queue.push_back(next);
            };
            if (current%width>0) visit(current-1);
            if (current%width+1<width) visit(current+1);
            if (current/width>0) visit(current-width);
            if (current/width+1<height) visit(current+width);
        }
        if (bestDistance>2.0) throw std::runtime_error("target not reachable within route bounds");
        std::vector<int> path;
        for (int index=best;index!=start;index=parent[index]) path.push_back(index);
        path.push_back(start);std::reverse(path.begin(),path.end());
        if (path.size()<2) throw std::runtime_error("route must extend beyond spawn");
        std::vector<int> corners{path.front()};
        int direction=path[1]-path[0];
        for (std::size_t i=2;i<path.size();++i) {
            const int nextDirection=path[i]-path[i-1];
            if (nextDirection!=direction) corners.push_back(path[i-1]);
            direction=nextDirection;
        }
        corners.push_back(path.back());
        std::cout << "{\n  \"format\": " << kFormatVersion
                  << ",\n  \"level\": " << world.level
                  << ",\n  \"seed\": \"" << world.seed
                  << "\",\n  \"clearance\": " << radius
                  << ",\n  \"one_way_metres\": " << path.size()-1
                  << ",\n  \"waypoints\": [\n";
        for (std::size_t i=0;i<corners.size();++i)
            std::cout << "    [" << x(corners[i]) << ", " << z(corners[i])
                      << "]" << (i+1<corners.size() ? "," : "") << '\n';
        std::cout << "  ]\n}\n";
        std::cerr << "planned " << path.size()-1 << " m and " << corners.size()
                  << " corners through " << queue.size() << " reachable points\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';return 1;
    }
}

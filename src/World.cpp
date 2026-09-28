#include "World.hpp"

#include <algorithm>
#include <cmath>

namespace Backrooms {
namespace {
// Geometry revisions can preserve the random recipe so landmarks and visual
// QA locations do not reshuffle when room composition changes.
constexpr std::uint64_t kHashRecipeVersion=11;

int DivFloor(int value, int divisor) {
    const int quotient=value/divisor;
    return value<0 && value%divisor ? quotient-1 : quotient;
}
int ModFloor(int value, int divisor) { return value-DivFloor(value,divisor)*divisor; }

std::uint64_t Mix(std::uint64_t v) {
    v ^= v >> 30; v *= 0xbf58476d1ce4e5b9ULL;
    v ^= v >> 27; v *= 0x94d049bb133111ebULL;
    return v ^ (v >> 31);
}

void AppendEdge(std::vector<Wall>& walls, const WorldConfig& config,
                Edge edge, bool vertical, int edgeX, int edgeZ,
                double boundary, double along) {
    if (edge == Edge::Open) return;
    const double t = 0.10;
    auto add = [&](double a, double b) {
        if (vertical) walls.push_back({boundary-t, a, boundary+t, b});
        else walls.push_back({a, boundary-t, b, boundary+t});
    };
    if (edge == Edge::Solid) add(along, along+kCellSize);
    else {
        const auto span=OpeningForEdge(config,edge,vertical,edgeX,edgeZ);
        add(along, along+span.start);
        add(along+span.end, along+kCellSize);
    }
}

Edge OptionalEdge(RegionKind kind, std::uint32_t hash) {
    const auto roll=hash%100;
    switch (kind) {
    case RegionKind::OpenOffice:
        return roll<72 ? Edge::Open : roll<92 ? Edge::Wide : Edge::Door;
    case RegionKind::Columns:
        return roll<78 ? Edge::Open : roll<94 ? Edge::Wide : Edge::Door;
    case RegionKind::Rooms:
        return roll<10 ? Edge::Open : roll<24 ? Edge::Wide :
               roll<65 ? Edge::Door : Edge::Solid;
    case RegionKind::Halls:
        return roll<16 ? Edge::Open : roll<37 ? Edge::Wide :
               roll<68 ? Edge::Door : Edge::Solid;
    case RegionKind::Irregular:
        return roll<10 ? Edge::Open : roll<23 ? Edge::Wide :
               roll<60 ? Edge::Door : Edge::Solid;
    case RegionKind::Storage:
        return roll<50 ? Edge::Open : roll<82 ? Edge::Wide : Edge::Door;
    case RegionKind::Tunnels:
        return roll<18 ? Edge::Open : roll<31 ? Edge::Wide :
               roll<47 ? Edge::Door : Edge::Solid;
    }
    return Edge::Solid;
}

enum class Parent { Here, West, East, North, South };

Parent ParentTowardRegionRoot(const WorldConfig& config, int x, int z) {
    const int rx=DivFloor(x,kRegionCells),rz=DivFloor(z,kRegionCells);
    const auto root=CellHash(config,rx,rz,601);
    const int rootX=static_cast<int>((root>>8)%kRegionCells);
    const int rootZ=static_cast<int>((root>>16)%kRegionCells);
    const int localX=ModFloor(x,kRegionCells),localZ=ModFloor(z,kRegionCells);
    const int dx=rootX-localX,dz=rootZ-localZ;
    if (dx==0 && dz==0) return Parent::Here;
    const bool alongX=dz==0 || (dx!=0 && (CellHash(config,x,z,603)&1U)==0);
    if (alongX) return dx<0 ? Parent::West : Parent::East;
    return dz<0 ? Parent::North : Parent::South;
}

Edge TreeEdge(RegionKind kind, std::uint32_t hash) {
    if (kind==RegionKind::OpenOffice || kind==RegionKind::Columns ||
        kind==RegionKind::Storage) return hash%3==0 ? Edge::Wide : Edge::Open;
    return hash%4==0 ? Edge::Wide : Edge::Door;
}

int RoomZoneAt(const WorldConfig& config, int cellX, int cellZ,
               RegionKind kind) {
    const int rx=DivFloor(cellX,kRegionCells);
    const int rz=DivFloor(cellZ,kRegionCells);
    const int lx=ModFloor(cellX,kRegionCells);
    const int lz=ModFloor(cellZ,kRegionCells);
    constexpr std::array<std::array<int,2>,5> anchors{{
        {{1,1}},{{4,1}},{{1,4}},{{4,4}},{{3,3}}
    }};
    const int count=kind==RegionKind::Halls ? 3 :
                    kind==RegionKind::Irregular ? 5 : 4;
    int bestZone=0,bestScore=1000;
    for (int zone=0;zone<count;++zone) {
        const auto h=CellHash(config,rx,rz,1301+zone);
        const int sx=std::clamp(anchors[zone][0]+
                    static_cast<int>((h>>5)%3)-1,0,kRegionCells-1);
        const int sz=std::clamp(anchors[zone][1]+
                    static_cast<int>((h>>11)%3)-1,0,kRegionCells-1);
        const int dx=lx-sx,dz=lz-sz;
        const int score=dx*dx+dz*dz;
        if (score<bestScore) { bestScore=score;bestZone=zone; }
    }
    return bestZone;
}

bool StorageRackAt(const WorldConfig& config, int cellX, int cellZ) {
    if (RegionAt(config,cellX,cellZ)!=RegionKind::Storage) return false;
    const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
    const int lx=ModFloor(cellX,kRegionCells),lz=ModFloor(cellZ,kRegionCells);
    const auto layout=CellHash(config,rx,rz,2961)%4;
    if (layout==0)
        return (lx==1 || lx==4) && (lz==1 || lz==4);
    if (layout==1)
        return lz==2 && (lx==1 || lx==3 || lx==5);
    if (layout==2)
        return (lz==1 && (lx==1 || lx==3)) ||
               (lz==4 && (lx==2 || lx==4));
    return (lx==2 && lz==1) || (lx==4 && lz==4);
}

Edge ComposedEdge(const WorldConfig& config, RegionKind kind,
                  int firstX, int firstZ, int secondX, int secondZ,
                  bool tree, std::uint32_t hash) {
    if (config.level==2 && tree)
        return hash%10<7 ? Edge::Open :
               hash%10<9 ? Edge::Wide : Edge::Door;
    if (config.level==0 && (kind==RegionKind::OpenOffice ||
                            kind==RegionKind::Columns)) {
        if (tree)
            return hash%(kind==RegionKind::Columns ? 14 : 9)==0 ?
                   Edge::Wide : Edge::Open;
        const unsigned roll=hash%100;
        if (kind==RegionKind::Columns)
            return roll<95 ? Edge::Open : Edge::Wide;
        return roll<88 ? Edge::Open : roll<97 ? Edge::Wide : Edge::Door;
    }
    if (config.level!=0 || (kind!=RegionKind::Rooms &&
        kind!=RegionKind::Halls && kind!=RegionKind::Irregular))
        return tree ? TreeEdge(kind,hash) : OptionalEdge(kind,hash);
    if (RoomZoneAt(config,firstX,firstZ,kind)==
        RoomZoneAt(config,secondX,secondZ,kind))
        return hash%19==0 ? Edge::Wide : Edge::Open;
    if (tree) return hash%5==0 ? Edge::Wide : Edge::Door;
    const auto roll=hash%100;
    return roll<7 ? Edge::Wide : roll<20 ? Edge::Door : Edge::Solid;
}
}

int CellOf(double position) { return static_cast<int>(std::floor(position/kCellSize)); }
int ChunkOfCell(int cell) { return DivFloor(cell,kChunkCells); }
ChunkCoord ChunkAt(double x, double z) { return {ChunkOfCell(CellOf(x)), ChunkOfCell(CellOf(z))}; }

const LevelDefinition& LevelInfo(int level) {
    static const LevelDefinition definitions[] = {
        {"The Yellow Rooms",3.0f,18.0f,85.0f},
        {"Service Storage",4.1f,20.0f,95.0f},
        {"Maintenance Tunnels",2.55f,12.0f,65.0f}
    };
    return definitions[std::clamp(level,0,2)];
}

std::uint32_t CellHash(const WorldConfig& config, int x, int z, int salt) {
    std::uint64_t v = config.seed ^ (kHashRecipeVersion << 48);
    v ^= Mix(static_cast<std::uint32_t>(x) + 0x99213d67ULL);
    v ^= Mix(static_cast<std::uint32_t>(z) + 0x5c833b45ULL);
    v ^= Mix(static_cast<std::uint32_t>(salt) + 0x37ac891eULL);
    v ^= Mix(static_cast<std::uint32_t>(config.level) + 0x9133a7c5ULL);
    return static_cast<std::uint32_t>(Mix(v));
}

OpeningSpan OpeningForEdge(const WorldConfig& config, Edge edge,
                           bool vertical, int edgeX, int edgeZ) {
    if (edge==Edge::Open) return {0.0,kCellSize};
    if (edge==Edge::Solid) return {0.0,0.0};
    const double width=edge==Edge::Wide ? 3.5 : 1.9;
    const double centered=(kCellSize-width)*0.5;
    if (config.level!=0) return {centered,centered+width};
    const auto hash=CellHash(config,edgeX,edgeZ,
                             vertical ? 1703 : 1709);
    const double step=static_cast<int>(hash%7)-3;
    const double shift=step*(edge==Edge::Wide ? 0.32 : 0.54)/3.0;
    return {centered+shift,centered+shift+width};
}

std::optional<PortalDefinition> PortalAt(const WorldConfig& config,
                                         int cellX, int cellZ) {
    for (const auto& portal:kPortals)
        if (portal.level==config.level && portal.cellX==cellX &&
            portal.cellZ==cellZ) return portal;
    // One candidate every four chunks in each direction, with most omitted.
    // The cell center belongs to the connected room graph, so a found
    // entrance can always be approached without a separate route generator.
    if (ModFloor(cellX,4*kChunkCells)!=2*kChunkCells ||
        ModFloor(cellZ,4*kChunkCells)!=2*kChunkCells ||
        CellHash(config,cellX,cellZ,1751)%3!=0) return std::nullopt;
    return PortalDefinition{config.level,cellX,cellZ,
                            (config.level+1)%3,true};
}

RegionKind RegionAt(const WorldConfig& config, int cellX, int cellZ) {
    const int rx=DivFloor(cellX,kRegionCells), rz=DivFloor(cellZ,kRegionCells);
    if (config.level==1 && rx==0 && rz==0) return RegionKind::Storage;
    if (config.level==2 && rx==0 && rz==0) return RegionKind::Tunnels;
    const auto roll=CellHash(config,rx,rz,501)%100;
    if (config.level==0)
        return roll<18 ? RegionKind::OpenOffice :
               roll<34 ? RegionKind::Columns :
               roll<65 ? RegionKind::Rooms :
               roll<86 ? RegionKind::Halls : RegionKind::Irregular;
    if (config.level==1)
        return roll<35 ? RegionKind::Storage :
               roll<58 ? RegionKind::OpenOffice :
               roll<83 ? RegionKind::Halls : RegionKind::Rooms;
    return roll<65 ? RegionKind::Tunnels :
           roll<88 ? RegionKind::Irregular : RegionKind::Halls;
}

bool IsEmptyHall(const WorldConfig& config, int cellX, int cellZ) {
    if (config.level!=0) return false;
    constexpr int extent=2*kRegionCells;
    const int sx=DivFloor(cellX,extent),sz=DivFloor(cellZ,extent);
    if (sx==0 && sz==0) return false; // preserve the first transition route
    return CellHash(config,sx,sz,2511)%17==0;
}

bool IsServiceChamber(const WorldConfig& config, int cellX, int cellZ) {
    if (config.level!=2) return false;
    const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
    if (rx==0 && rz==0) return false; // keep the first maintenance route
    return CellHash(config,rx,rz,2801)%11==0;
}

Edge VerticalEdge(const WorldConfig& config, int boundaryX, int z) {
    if (config.level<=1 && z==0 && boundaryX>=1 && boundaryX<=3)
        return Edge::Open; // readable route to the first maintenance entrance
    if (DivFloor(boundaryX-1,2*kRegionCells)==
        DivFloor(boundaryX,2*kRegionCells) &&
        IsEmptyHall(config,boundaryX-1,z)) return Edge::Open;
    if (DivFloor(boundaryX-1,kRegionCells)==
        DivFloor(boundaryX,kRegionCells) &&
        IsServiceChamber(config,boundaryX-1,z)) return Edge::Open;
    const int rx=DivFloor(boundaryX-1,kRegionCells);
    const int rz=DivFloor(z,kRegionCells);
    const int localZ=ModFloor(z,kRegionCells);
    if (ModFloor(boundaryX,kRegionCells)==0) {
        const auto hash=CellHash(config,rx,rz,701);
        const int first=static_cast<int>(hash%kRegionCells);
        const int second=(first+2+static_cast<int>((hash>>8)%3))%kRegionCells;
        if (localZ==first) return Edge::Door;
        if (config.level!=2 && hash%3==0 && localZ==second) return Edge::Wide;
        return Edge::Solid;
    }
    const RegionKind kind=RegionAt(config,boundaryX-1,z);
    const auto edgeHash=CellHash(config,boundaryX,z,11);
    const bool tree=ParentTowardRegionRoot(config,boundaryX-1,z)==Parent::East ||
                    ParentTowardRegionRoot(config,boundaryX,z)==Parent::West;
    return ComposedEdge(config,kind,boundaryX-1,z,boundaryX,z,tree,edgeHash);
}

Edge HorizontalEdge(const WorldConfig& config, int x, int boundaryZ) {
    if ((config.level==1 || config.level==2) && x==0 &&
        boundaryZ>=1 && boundaryZ<=3) return Edge::Open;
    if (DivFloor(boundaryZ-1,2*kRegionCells)==
        DivFloor(boundaryZ,2*kRegionCells) &&
        IsEmptyHall(config,x,boundaryZ-1)) return Edge::Open;
    if (DivFloor(boundaryZ-1,kRegionCells)==
        DivFloor(boundaryZ,kRegionCells) &&
        IsServiceChamber(config,x,boundaryZ-1)) return Edge::Open;
    const int rx=DivFloor(x,kRegionCells);
    const int rz=DivFloor(boundaryZ-1,kRegionCells);
    const int localX=ModFloor(x,kRegionCells);
    if (ModFloor(boundaryZ,kRegionCells)==0) {
        const auto hash=CellHash(config,rx,rz,797);
        const int first=static_cast<int>(hash%kRegionCells);
        const int second=(first+2+static_cast<int>((hash>>8)%3))%kRegionCells;
        if (localX==first) return Edge::Door;
        if (config.level!=2 && hash%3==0 && localX==second) return Edge::Wide;
        return Edge::Solid;
    }
    const RegionKind kind=RegionAt(config,x,boundaryZ-1);
    const auto edgeHash=CellHash(config,x,boundaryZ,23);
    const bool tree=ParentTowardRegionRoot(config,x,boundaryZ-1)==Parent::South ||
                    ParentTowardRegionRoot(config,x,boundaryZ)==Parent::North;
    return ComposedEdge(config,kind,x,boundaryZ-1,x,boundaryZ,tree,edgeHash);
}

CellProp PropAt(const WorldConfig& config, int cellX, int cellZ) {
    CellProp result;
    if (IsEmptyHall(config,cellX,cellZ)) return result;
    if (IsServiceChamber(config,cellX,cellZ)) return result;
    if (PortalAt(config,cellX,cellZ)) return result;
    if ((cellZ==0 && cellX>=0 && cellX<=3) ||
        (cellX==0 && cellZ>=0 && cellZ<=3)) return result;
    const RegionKind region=RegionAt(config,cellX,cellZ);
    const int lx=ModFloor(cellX,kRegionCells), lz=ModFloor(cellZ,kRegionCells);
    if ((region==RegionKind::Columns &&
         (lx==1 || lx==4) && (lz==1 || lz==4)) ||
        StorageRackAt(config,cellX,cellZ)) return result;
    const std::uint32_t hash=CellHash(config,cellX,cellZ,1081);
    const unsigned roll=hash%1000;
    if (config.level==0) {
        if (cellX==2 && cellZ==1) result.kind=PropKind::Chair;
        else if (cellX==2 && cellZ==2) result.kind=PropKind::LowPartition;
        else if (roll<4) result.kind=PropKind::Chair;
        else if (roll<6) result.kind=PropKind::Table;
        else if (roll<9) result.kind=PropKind::EmbeddedChair;
        else if (roll<14 && (region==RegionKind::OpenOffice ||
                             region==RegionKind::Irregular))
            result.kind=PropKind::LowPartition;
        else if (roll<22 && (region==RegionKind::OpenOffice ||
                            region==RegionKind::Irregular ||
                            region==RegionKind::Rooms))
            result.kind=PropKind::TallPartition;
    } else if (config.level==1) {
        if (roll<10) result.kind=PropKind::Table;
        else if (roll<12) result.kind=PropKind::Chair;
    } else if (roll<2) result.kind=PropKind::EmbeddedChair;
    if (result.kind==PropKind::None) return result;
    result.x=(cellX+0.5)*kCellSize+
        (static_cast<int>((hash>>8)%5)-2)*0.24;
    result.z=(cellZ+0.5)*kCellSize+
        (static_cast<int>((hash>>11)%5)-2)*0.24;
    if (result.kind==PropKind::TallPartition) {
        result.x=(cellX+0.5)*kCellSize+
            (static_cast<int>((hash>>8)%5)-2)*0.15;
        result.z=(cellZ+0.5)*kCellSize+
            (static_cast<int>((hash>>11)%5)-2)*0.15;
    }
    result.quarterTurn=static_cast<int>((hash>>17)&3U);
    if (result.kind==PropKind::EmbeddedChair) {
        const bool left=VerticalEdge(config,cellX,cellZ)==Edge::Solid;
        const bool right=VerticalEdge(config,cellX+1,cellZ)==Edge::Solid;
        if (!left && !right) {
            result.kind=PropKind::Chair;
        } else {
            result.x=cellX*kCellSize+(left ? 0.28 : 4.72);
            result.z=(cellZ+0.5)*kCellSize;
            result.quarterTurn=left ? 3 : 1;
            result.sink=0.28f;
        }
    }
    return result;
}

CellObstacleSet InteriorPartitionsAt(const WorldConfig& config,
                                     int cellX, int cellZ) {
    CellObstacleSet result;
    if (config.level!=0 || IsEmptyHall(config,cellX,cellZ)) return result;
    const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
    if (rx==0 && rz==0) return result; // keep the first transition route clear
    const RegionKind kind=RegionAt(config,cellX,cellZ);
    if (kind!=RegionKind::OpenOffice && kind!=RegionKind::Irregular &&
        kind!=RegionKind::Columns && kind!=RegionKind::Rooms) return result;
    const auto layout=CellHash(config,rx,rz,3119);
    if (kind==RegionKind::Columns && layout%3!=0) return result;
    if (kind==RegionKind::Rooms && layout%2!=0) return result;

    const double originX=rx*kRegionCells*kCellSize;
    const double originZ=rz*kRegionCells*kCellSize;
    const auto clearBoundary=[](double position) {
        const double cell=std::fmod(position,kCellSize);
        if (cell<1.2) return position+(1.2-cell);
        if (cell>kCellSize-1.2)
            return position-(cell-(kCellSize-1.2));
        return position;
    };
    const double cross=clearBoundary(8.4+((layout>>5)%17)*0.43);
    const double start=2.2+((layout>>12)%5)*0.48;
    const double end=25.8-((layout>>16)%6)*0.52;
    const double cellMinX=cellX*kCellSize,cellMaxX=cellMinX+kCellSize;
    const double cellMinZ=cellZ*kCellSize,cellMaxZ=cellMinZ+kCellSize;
    const auto horizontal=[&](double x0,double x1,double z) {
        const double near=std::max(z-0.11,cellMinZ);
        const double far=std::min(z+0.11,cellMaxZ);
        double a=std::max(x0,cellMinX),b=std::min(x1,cellMaxX);
        if (x0<cellMinX+1.25 &&
            VerticalEdge(config,cellX,cellZ)!=Edge::Open)
            a=std::max(a,cellMinX+1.25);
        if (x1>cellMaxX-1.25 &&
            VerticalEdge(config,cellX+1,cellZ)!=Edge::Open)
            b=std::min(b,cellMaxX-1.25);
        if (b-a>0.001 && far-near>0.001)
            result.walls[result.count++]={a,near,b,far};
    };
    const auto vertical=[&](double z0,double z1,double x) {
        const double near=std::max(x-0.11,cellMinX);
        const double far=std::min(x+0.11,cellMaxX);
        double a=std::max(z0,cellMinZ),b=std::min(z1,cellMaxZ);
        if (z0<cellMinZ+1.25 &&
            HorizontalEdge(config,cellX,cellZ)!=Edge::Open)
            a=std::max(a,cellMinZ+1.25);
        if (z1>cellMaxZ-1.25 &&
            HorizontalEdge(config,cellX,cellZ+1)!=Edge::Open)
            b=std::min(b,cellMaxZ-1.25);
        if (b-a>0.001 && far-near>0.001)
            result.walls[result.count++]={near,a,far,b};
    };
    const bool alongX=(layout&1U)!=0;
    if (alongX) horizontal(originX+start,originX+end,originZ+cross);
    else vertical(originZ+start,originZ+end,originX+cross);
    if (layout%3==0) {
        // A shorter T-shaped return makes an alcove without sealing the room.
        const double branch=clearBoundary((start+end)*0.59);
        const double low=std::max(1.5,cross-7.2);
        const double high=std::min(27.8,cross+5.4);
        if (alongX) vertical(originZ+low,originZ+high,originX+branch);
        else horizontal(originX+low,originX+high,originZ+branch);
    }
    return result;
}

CellObstacleSet CellObstacles(const WorldConfig& config, int cellX, int cellZ) {
    CellObstacleSet result;
    if (IsEmptyHall(config,cellX,cellZ)) return result;
    if (PortalAt(config,cellX,cellZ)) return result;
    if (IsServiceChamber(config,cellX,cellZ)) {
        const int lx=ModFloor(cellX,kRegionCells);
        const int lz=ModFloor(cellZ,kRegionCells);
        if ((lx==1 || lx==4) && (lz==1 || lz==4)) {
            const double cx=(cellX+0.5)*kCellSize;
            const double cz=(cellZ+0.5)*kCellSize;
            result.walls[result.count++]={cx-0.55,cz-0.55,cx+0.55,cz+0.55};
        }
        return result;
    }
    const RegionKind kind=RegionAt(config,cellX,cellZ);
    const int lx=ModFloor(cellX,kRegionCells), lz=ModFloor(cellZ,kRegionCells);
    const auto interior=InteriorPartitionsAt(config,cellX,cellZ);
    for (int i=0;i<interior.count;++i)
        result.walls[result.count++]=interior.walls[i];
    if (kind==RegionKind::Columns && (lx==1 || lx==4) &&
        (lz==1 || lz==4)) {
        const double cx=(cellX+0.5)*kCellSize,cz=(cellZ+0.5)*kCellSize;
        result.walls[result.count++]={cx-0.43,cz-0.43,cx+0.43,cz+0.43};
    }
    if (StorageRackAt(config,cellX,cellZ)) {
        const double cx=(cellX+0.5)*kCellSize,cz=(cellZ+0.5)*kCellSize;
        const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
        const bool alongX=(CellHash(config,rx,rz,2961)&0x10U)!=0;
        const double halfX=alongX ? 1.0 : 0.8;
        const double halfZ=alongX ? 0.8 : 1.0;
        result.walls[result.count++]={cx-halfX,cz-halfZ,
                                      cx+halfX,cz+halfZ};
    }
    if (kind==RegionKind::Tunnels && (lx+lz)%2==0) {
        const double x=cellX*kCellSize,z=cellZ*kCellSize;
        result.walls[result.count++]={x+0.15,z+0.15,x+0.8,z+1.5};
        result.walls[result.count++]={x+0.15,z+3.5,x+0.8,z+4.85};
        result.walls[result.count++]={x+4.2,z+0.15,x+4.85,z+1.5};
        result.walls[result.count++]={x+4.2,z+3.5,x+4.85,z+4.85};
    }
    const CellProp prop=PropAt(config,cellX,cellZ);
    if (prop.kind!=PropKind::None) {
        const double halfX=prop.kind==PropKind::Table ?
            (prop.quarterTurn%2 ? 0.62 : 0.92) :
            prop.kind==PropKind::LowPartition ?
            (prop.quarterTurn%2 ? 0.14 : 1.66) :
            prop.kind==PropKind::TallPartition ?
            (prop.quarterTurn%2 ? 0.12 : 1.25) : 0.38;
        const double halfZ=prop.kind==PropKind::Table ?
            (prop.quarterTurn%2 ? 0.92 : 0.62) :
            prop.kind==PropKind::LowPartition ?
            (prop.quarterTurn%2 ? 1.66 : 0.14) :
            prop.kind==PropKind::TallPartition ?
            (prop.quarterTurn%2 ? 1.25 : 0.12) : 0.38;
        result.walls[result.count++]={prop.x-halfX,prop.z-halfZ,
                                      prop.x+halfX,prop.z+halfZ};
    }
    return result;
}

std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z) {
    std::vector<Wall> walls;
    const int cx = CellOf(x), cz = CellOf(z);
    walls.reserve(88);
    for (int ix=cx-1; ix<=cx+1; ++ix) {
        for (int iz=cz-1; iz<=cz+1; ++iz) {
            AppendEdge(walls,config,VerticalEdge(config, ix, iz),true,
                       ix,iz,ix*kCellSize,iz*kCellSize);
            AppendEdge(walls,config,HorizontalEdge(config, ix, iz),false,
                       ix,iz,iz*kCellSize,ix*kCellSize);
            const auto obstacles=CellObstacles(config,ix,iz);
            for (int i=0;i<obstacles.count;++i)
                walls.push_back(obstacles.walls[i]);
        }
    }
    for (int ix=cx-1;ix<=cx+1;++ix) for (int iz=cz-1;iz<=cz+1;++iz) {
        const auto portal=PortalAt(config,ix,iz);
        if (!portal) continue;
        const double px=(ix+0.5)*kCellSize;
        const double pz=(iz+0.5)*kCellSize;
        if (portal->alongX) {
            walls.push_back({px+0.43,pz-1.28,px+1.92,pz-1.10});
            walls.push_back({px+0.43,pz+1.10,px+1.92,pz+1.28});
            walls.push_back({px+1.82,pz-1.10,px+2.02,pz+1.10});
        } else {
            walls.push_back({px-1.28,pz+0.43,px-1.10,pz+1.92});
            walls.push_back({px+1.10,pz+0.43,px+1.28,pz+1.92});
            walls.push_back({px-1.10,pz+1.82,px+1.10,pz+2.02});
        }
    }
    return walls;
}

bool Collides(const WorldConfig& config, double x, double z, double radius) {
    for (const auto& wall : NearbyWalls(config, x, z)) {
        const double closestX = std::clamp(x, wall.minX, wall.maxX);
        const double closestZ = std::clamp(z, wall.minZ, wall.maxZ);
        const double a=x-closestX, b=z-closestZ;
        if (a*a+b*b < radius*radius) return true;
    }
    return false;
}

void MoveWithCollision(const WorldConfig& config, double& x, double& z,
                       double dx, double dz, double radius) {
    const int steps = std::max(1, static_cast<int>(std::ceil(std::hypot(dx,dz)/0.18)));
    for (int i=0; i<steps; ++i) {
        const double sx=dx/steps, sz=dz/steps;
        if (!Collides(config, x+sx, z, radius)) x += sx;
        if (!Collides(config, x, z+sz, radius)) z += sz;
    }
}

} // namespace Backrooms

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
    const double t = kWallHalfThickness;
    auto add = [&](double a, double b) {
        if (vertical) walls.push_back({boundary-t, a, boundary+t, b});
        else walls.push_back({a, boundary-t, b, boundary+t});
    };
    // Full-thickness junction cores close the small diagonal wedges at cell
    // corners. Keep openings unchanged; only the outer endpoints extend.
    if (edge == Edge::Solid) add(along-t, along+kCellSize+t);
    else {
        const auto span=OpeningForEdge(config,edge,vertical,edgeX,edgeZ);
        add(along-t, along+span.start);
        add(along+span.end, along+kCellSize+t);
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

RoomLayout BuildRoomLayout(const WorldConfig& config,int rx,int rz,RegionKind kind) {
    constexpr int side=kRegionCells,count=side*side;
    const int ox=rx*side,oz=rz*side;
    std::array<int,count> zones{},component{},pending{};
    component.fill(-1);
    for (int i=0;i<count;++i)
        zones[i]=RoomZoneAt(config,ox+i%side,oz+i/side,kind);
    int components=0;
    // A discrete Voronoi zone can have disconnected corners. Treat each
    // connected piece as a room before selecting its shared entrances.
    for (int start=0;start<count;++start) {
        if (component[start]>=0) continue;
        int end=1;pending[0]=start;component[start]=components;
        for (int cursor=0;cursor<end;++cursor) {
            const int i=pending[cursor],x=i%side,z=i/side;
            const auto visit=[&](int next) {
                if (component[next]<0 && zones[next]==zones[start]) {
                    component[next]=components;pending[end++]=next;
                }
            };
            if (x>0) visit(i-1);
            if (x+1<side) visit(i+1);
            if (z>0) visit(i-side);
            if (z+1<side) visit(i+side);
        }
        ++components;
    }
    struct Boundary {
        int index=0;
        bool east=false;
        std::uint32_t hash=0,priority=0;
    };
    std::array<Boundary,2*side*(side-1)> boundaries{};
    std::array<int,count*count> bestTree{},bestOther{};
    bestTree.fill(-1);bestOther.fill(-1);
    RoomLayout result;
    result.east.fill(Edge::Solid);result.south.fill(Edge::Solid);
    int boundaryCount=0;
    for (int i=0;i<count;++i) for (bool east:{false,true}) {
        const int x=i%side,z=i/side;
        if (east ? x+1==side : z+1==side) continue;
        const int next=i+(east ? 1 : side);
        if (component[i]==component[next]) {
            (east ? result.east : result.south)[i]=Edge::Open;
            continue;
        }
        const int gx=ox+x,gz=oz+z;
        const int a=std::min(component[i],component[next]);
        const int b=std::max(component[i],component[next]);
        const bool tree=east ?
            ParentTowardRegionRoot(config,gx,gz)==Parent::East ||
            ParentTowardRegionRoot(config,gx+1,gz)==Parent::West :
            ParentTowardRegionRoot(config,gx,gz)==Parent::South ||
            ParentTowardRegionRoot(config,gx,gz+1)==Parent::North;
        const auto hash=CellHash(config,gx+(east?1:0),gz+(east?0:1),east?11:23);
        const auto priority=CellHash(config,gx,gz,east?4001:4003);
        const int id=boundaryCount++;
        boundaries[id]={i,east,hash,priority};
        auto& best=(tree ? bestTree : bestOther)[a*count+b];
        if (best<0 || priority<boundaries[best].priority) best=id;
    }
    for (int a=0;a<components;++a) for (int b=a+1;b<components;++b) {
        const int pair=a*count+b;
        int selected=bestTree[pair];
        if (selected<0 && CellHash(config,rx,rz,4021+pair)%7==0)
            selected=bestOther[pair];
        if (selected<0) continue;
        const auto& boundary=boundaries[selected];
        (boundary.east ? result.east : result.south)[boundary.index]=
            boundary.hash%5==0 ? Edge::Wide : Edge::Door;
    }
    // Contracting connected rooms preserves the original cell tree. Keeping
    // one crossing per neighboring room pair preserves that connectivity,
    // without exposing every five-metre tree edge as a separate doorway.
    return result;
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

bool ColumnAt(const WorldConfig& config,int cellX,int cellZ) {
    const auto kind=RegionAt(config,cellX,cellZ);
    int lx=ModFloor(cellX,kRegionCells),lz=ModFloor(cellZ,kRegionCells);
    if (config.level==0 && kind==RegionKind::Columns) {
        const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
        const auto style=OfficeColumnStyleAt(config,rx,rz);
        if (!style) return false;
        if (CellHash(config,rx,rz,4497)&0x10000U) std::swap(lx,lz);
        switch (*style) {
        case OfficeColumnStyle::Rectangular:
            return (lx==1 || lx==4) && (lz==1 || lz==4);
        case OfficeColumnStyle::Spine:
            return lx==2 && (lz==1 || lz==3 || lz==4);
        case OfficeColumnStyle::LShaped:
            return (lx==1 && (lz==1 || lz==4)) || (lx==4 && lz==4);
        case OfficeColumnStyle::DiagonalPair:
            return (lx==1 && lz==4) || (lx==4 && lz==1);
        case OfficeColumnStyle::OffsetRectangle:
            return (lx==1 && (lz==1 || lz==4)) ||
                   (lx==4 && (lz==2 || lz==4));
        }
    }
    if (kind==RegionKind::Columns)
        return (lx==1 || lx==4) && (lz==1 || lz==4);
    return config.level==1 && kind==RegionKind::Storage &&
           (lx==2 || lx==5) && (lz==2 || lz==5) &&
           !StorageRackAt(config,cellX,cellZ);
}

Edge ComposedEdge(const WorldConfig& config, RegionKind kind,
                  int firstX, int firstZ, int secondX,
                  bool tree, std::uint32_t hash) {
    if (config.level==1 && (kind==RegionKind::Storage ||
        kind==RegionKind::Columns || kind==RegionKind::OpenOffice)) {
        // Compose an entire industrial bay with supports and shelves. Its
        // enclosure and entrances belong to the shared region border.
        return Edge::Open;
    }
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
    if (config.level>1 || (kind!=RegionKind::Rooms &&
        kind!=RegionKind::Halls && kind!=RegionKind::Irregular))
        return tree ? TreeEdge(kind,hash) : OptionalEdge(kind,hash);
    const int rx=DivFloor(firstX,kRegionCells),rz=DivFloor(firstZ,kRegionCells);
    const int index=ModFloor(firstZ,kRegionCells)*kRegionCells+
                    ModFloor(firstX,kRegionCells);
    const auto select=[&](const RoomLayout& layout) {
        return (secondX!=firstX ? layout.east : layout.south)[index];
    };
    if (config.roomLayouts) return select(config.roomLayouts->Get(config,rx,rz,kind));
    return select(BuildRoomLayout(config,rx,rz,kind));
}
}

const RoomLayout& RoomLayoutCache::Get(const WorldConfig& world,int rx,int rz,
                                      RegionKind kind) {
    const Key key{world.seed,world.level,rx,rz,kind};
    if (const auto it=layouts_.find(key);it!=layouts_.end()) return it->second;
    if (layouts_.size()==kCapacity) {
        layouts_.erase(order_.front());order_.pop_front();
    }
    auto [it,inserted]=layouts_.emplace(key,BuildRoomLayout(world,rx,rz,kind));
    order_.push_back(key);
    return it->second;
}

int CellOf(double position) { return static_cast<int>(std::floor(position/kCellSize)); }
int ChunkOfCell(int cell) { return DivFloor(cell,kChunkCells); }
ChunkCoord ChunkAt(double x, double z) { return {ChunkOfCell(CellOf(x)), ChunkOfCell(CellOf(z))}; }

const LevelCatalog& DefaultLevelCatalog() {
    static const LevelCatalog defaults=[] {
        LevelCatalog result;
        auto& office=result.levels[0];
        office.name="The Yellow Rooms";
        office.ceilingHeight=2.75f;office.doorwayHeight=2.4f;
        office.ambient=0.42f;office.lightStrength=0.55f;
        office.wallBounce=0.16f;office.ceilingBounce=0.42f;
        office.wall={255,250,214};office.pillar={231,224,184};
        office.trim={255,251,215};office.floor={246,240,207};
        office.ceiling={255,252,218};office.structure={143,139,115};
        office.fluorescent={255,251,228};office.fog={108,101,78};
        office.regions={{{RegionKind::OpenOffice,18},{RegionKind::Columns,16},
                         {RegionKind::Rooms,31},{RegionKind::Halls,21},
                         {RegionKind::Irregular,14}}};
        auto& storage=result.levels[1];
        storage.name="Service Storage";
        storage.ceilingHeight=4.1f;storage.doorwayHeight=2.8f;
        storage.fogStart=20;storage.fogEnd=95;
        storage.ambient=0.60f;storage.lightStrength=0.38f;
        storage.ceilingBounce=0.60f;storage.entityRarity=650;
        storage.wall={175,197,206};storage.pillar={188,202,200};
        storage.trim={110,130,128};storage.floor={122,137,142};
        storage.ceiling={205,220,219};storage.structure={94,114,112};
        storage.fluorescent={204,230,230};storage.fog={56,67,67};
        storage.regions={{{RegionKind::Storage,35},{RegionKind::OpenOffice,18},
                          {RegionKind::Columns,15},{RegionKind::Halls,18},
                          {RegionKind::Rooms,14}}};
        auto& tunnels=result.levels[2];
        tunnels.name="Maintenance Tunnels";
        tunnels.ceilingHeight=2.55f;tunnels.doorwayHeight=2.08f;
        tunnels.fogStart=12;tunnels.fogEnd=65;
        tunnels.ambient=0.47f;tunnels.lightStrength=0.54f;
        tunnels.ceilingBounce=0.60f;tunnels.entityRarity=750;
        tunnels.wall={239,231,211};tunnels.pillar={218,210,188};
        tunnels.trim={105,113,101};tunnels.floor={215,204,178};
        tunnels.ceiling={140,143,131};tunnels.structure={104,114,99};
        tunnels.fluorescent={255,231,181};tunnels.fog={39,41,35};
        tunnels.regions={{{RegionKind::Tunnels,65},{RegionKind::Irregular,23},
                          {RegionKind::Halls,12}}};
        return result;
    }();
    return defaults;
}

const LevelDefinition& LevelInfo(int level) {
    return DefaultLevelCatalog().levels[std::clamp(level,0,2)];
}

const LevelDefinition& LevelInfo(const WorldConfig& world) {
    return world.levels ? world.levels->levels[std::clamp(world.level,0,2)] :
                          LevelInfo(world.level);
}

std::uint32_t CellHash(const WorldConfig& config, int x, int z, int salt) {
    std::uint64_t v = config.seed ^ (kHashRecipeVersion << 48);
    v ^= Mix(static_cast<std::uint32_t>(x) + 0x99213d67ULL);
    v ^= Mix(static_cast<std::uint32_t>(z) + 0x5c833b45ULL);
    v ^= Mix(static_cast<std::uint32_t>(salt) + 0x37ac891eULL);
    v ^= Mix(static_cast<std::uint32_t>(config.level) + 0x9133a7c5ULL);
    return static_cast<std::uint32_t>(Mix(v));
}

LampInfo LampAt(const WorldConfig& world, int gx, int gz) {
    LampInfo lamp;
    lamp.y=LevelInfo(world).ceilingHeight-(world.level==1 ? 0.44f : 0.04f);
    const auto h=CellHash(world,gx,gz,41);
    if (world.level==0) {
        const auto hallStyle=HallLightingStyleAt(world,gx,gz);
        const int extent=hallStyle ? 2*kRegionCells : kRegionCells;
        const int rx=DivFloor(gx,extent),rz=DivFloor(gz,extent);
        int lx=ModFloor(gx,extent),lz=ModFloor(gz,extent);
        const auto layout=CellHash(world,rx,rz,hallStyle ? 4531 : 1803);
        const int phaseX=static_cast<int>((layout>>4)&1U);
        const int phaseZ=static_cast<int>((layout>>7)&1U);
        const auto region=RegionAt(world,gx,gz);
        if (hallStyle) {
            if (layout&0x10000U) std::swap(lx,lz);
            // A sixty-metre room has one ceiling plan and circuit, rather
            // than four unrelated plans inherited from hidden region types.
            switch (*hallStyle) {
            case HallLightingStyle::PairedRows:
                lamp.fixture=(lx+phaseX)%3!=2 && (lz+phaseZ)%2==0;
                break;
            case HallLightingStyle::Staggered:
                lamp.fixture=(lx+phaseX)%2==0 &&
                             (lz+phaseZ+lx/2)%2==0;
                break;
            case HallLightingStyle::Bands:
                lamp.fixture=(lx+phaseX)%2==0 && (lz+phaseZ)%4<2;
                break;
            }
        } else if (region==RegionKind::OpenOffice || region==RegionKind::Columns)
            lamp.fixture=(layout&0x1000U) ?
                ((lx+phaseX)%2==0 || (lz+phaseZ)%3==0) :
                ((lx+phaseX)%2==0 && (lz+phaseZ)%2==0);
        else if (region==RegionKind::Halls)
            lamp.fixture=(lz+phaseZ)%2==0 && (lx+phaseX)%3!=0;
        else
            lamp.fixture=(lx+phaseX)%2==0 && (lz+phaseZ)%2==0;
        if (h%29==0) lamp.fixture=false;
        const bool weakCircuit=layout%13==0;
        lamp.lit=lamp.fixture && h%17!=0 &&
                 (!weakCircuit || h%4==0);
        lamp.longAxisX=hallStyle ? (layout&0x10000U)==0 : (layout&1U)!=0;
        // A two-by-one tile troffer must start and end on the acoustic grid.
        const auto placement=hallStyle ? layout : h;
        lamp.x=lamp.longAxisX ? ((placement&1U) ? 2.5f : 3.125f) :
                                      ((placement&1U) ? 2.1875f : 2.8125f);
        lamp.z=lamp.longAxisX ? ((placement&2U) ? 2.1875f : 2.8125f) :
                                      ((placement&2U) ? 2.5f : 3.125f);
        if (lamp.fixture) {
            auto obstacles=FullHeightObstaclesAt(world,gx,gz);
            if (const auto portal=PortalAt(world,gx,gz)) {
                const auto frames=PortalWalls(*portal);
                for (int n=0;n<2;++n)
                    obstacles.walls[obstacles.count++]=frames.walls[n];
            }
            constexpr std::array<std::array<float,2>,9> offsets{{
                {{0,0}},{{1.25f,0}},{{-1.25f,0}},{{0,1.25f}},{{0,-1.25f}},
                {{1.25f,1.25f}},{{-1.25f,1.25f}},{{1.25f,-1.25f}},{{-1.25f,-1.25f}}
            }};
            const float halfX=lamp.longAxisX ? 0.625f : 0.3125f;
            const float halfZ=lamp.longAxisX ? 0.3125f : 0.625f;
            if (const auto alcove=OfficeAlcoveAt(world,gx,gz)) {
                const double cross=alcove->boundary+alcove->inward*alcove->depth;
                // The lower acoustic lid must not hide a live ceiling panel.
                obstacles.walls[obstacles.count++]=alcove->vertical ?
                    Wall{std::min(alcove->boundary,cross),alcove->start,
                         std::max(alcove->boundary,cross),alcove->end} :
                    Wall{alcove->start,std::min(alcove->boundary,cross),
                         alcove->end,std::max(alcove->boundary,cross)};
            }
            bool placed=false;
            for (const auto& offset:offsets) {
                const float x=lamp.x+offset[0],z=lamp.z+offset[1];
                if (x-halfX<0.12f || x+halfX>4.88f ||
                    z-halfZ<0.12f || z+halfZ>4.88f) continue;
                const double wx=gx*kCellSize+x,wz=gz*kCellSize+z;
                bool blocked=false;
                for (int i=0;i<obstacles.count;++i) {
                    const auto& wall=obstacles.walls[i];
                    if (wx+halfX>wall.minX-0.03 && wx-halfX<wall.maxX+0.03 &&
                        wz+halfZ>wall.minZ-0.03 && wz-halfZ<wall.maxZ+0.03)
                        blocked=true;
                }
                if (blocked) continue;
                lamp.x=x;lamp.z=z;placed=true;break;
            }
            if (!placed) { lamp.fixture=false;lamp.lit=false; }
        }
    } else {
        lamp.fixture=IsServiceChamber(world,gx,gz) ?
                     (gx%2==0 && gz%2==0) :
                     ((gx+gz)%3==0 && h%3!=0);
        if (world.level==1) {
            const int rx=DivFloor(gx,kRegionCells),rz=DivFloor(gz,kRegionCells);
            const int lx=ModFloor(gx,kRegionCells),lz=ModFloor(gz,kRegionCells);
            const auto layout=CellHash(world,rx,rz,2901);
            const int phaseX=static_cast<int>((layout>>3)%3);
            const int phaseZ=static_cast<int>((layout>>8)%2);
            lamp.fixture=(lx+phaseX)%3==0 && (lz+phaseZ)%2==0;
            lamp.longAxisX=(layout&1U)==0;
            if (FullHeightObstaclesAt(world,gx,gz).count>0) {
                lamp.x=1.15f;lamp.longAxisX=false;
            }
        }
        if (world.level==2 && gx==0 &&
            (gz==0 || gz==1 || gz==3)) lamp.fixture=true;
        lamp.lit=lamp.fixture && (world.level==1 ? h%19!=0 :
                    !IsServiceChamber(world,gx,gz) || h%13!=0);
    }
    return lamp;
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

CellObstacleSet PortalWalls(const PortalDefinition& portal) {
    const double x=(portal.cellX+0.5)*kCellSize;
    const double z=(portal.cellZ+0.5)*kCellSize;
    constexpr double near=kPortalEntryDepth,far=kPortalBackDepth;
    constexpr double half=kPortalHalfWidth,t=kPortalWallThickness;
    constexpr double end=far+kWallHalfThickness;
    CellObstacleSet result;
    result.count=3;
    if (portal.alongX) {
        result.walls[0]={x+near,z-half-t,x+end,z-half};
        result.walls[1]={x+near,z+half,x+end,z+half+t};
        result.walls[2]={x+far-kWallHalfThickness,z-half,x+end,z+half};
    } else {
        result.walls[0]={x-half-t,z+near,x-half,z+end};
        result.walls[1]={x+half,z+near,x+half+t,z+end};
        result.walls[2]={x-half,z+far-kWallHalfThickness,x+half,z+end};
    }
    return result;
}

std::optional<int> PortalTarget(const WorldConfig& world,double x,double z) {
    const auto portal=PortalAt(world,CellOf(x),CellOf(z));
    if (!portal) return std::nullopt;
    const double px=(portal->cellX+0.5)*kCellSize;
    const double pz=(portal->cellZ+0.5)*kCellSize;
    const double depth=portal->alongX ? x-px : z-pz;
    const double side=portal->alongX ? z-pz : x-px;
    if (depth>0.92 && depth<kPortalBackDepth && std::abs(side)<kPortalHalfWidth)
        return portal->target;
    return std::nullopt;
}

RegionKind RegionAt(const WorldConfig& config, int cellX, int cellZ) {
    const int rx=DivFloor(cellX,kRegionCells), rz=DivFloor(cellZ,kRegionCells);
    if (config.level==1 && rx==0 && rz==0) return RegionKind::Storage;
    if (config.level==2 && rx==0 && rz==0) return RegionKind::Tunnels;
    const auto roll=CellHash(config,rx,rz,501)%100;
    unsigned cumulative=0;
    for (const auto& entry:LevelInfo(config).regions) {
        cumulative+=static_cast<unsigned>(entry.weight);
        if (roll<cumulative) return entry.kind;
    }
    return RegionKind::OpenOffice; // profiles validate a total weight of 100
}

bool IsEmptyHall(const WorldConfig& config, int cellX, int cellZ) {
    if (config.level!=0) return false;
    constexpr int extent=2*kRegionCells;
    const int sx=DivFloor(cellX,extent),sz=DivFloor(cellZ,extent);
    if (sx==0 && sz==0) return false; // preserve the first transition route
    return CellHash(config,sx,sz,2511)%17==0;
}

std::optional<HallLightingStyle> HallLightingStyleAt(
    const WorldConfig& config,int cellX,int cellZ) {
    if (!IsEmptyHall(config,cellX,cellZ)) return std::nullopt;
    return static_cast<HallLightingStyle>(CellHash(config,
        DivFloor(cellX,2*kRegionCells),DivFloor(cellZ,2*kRegionCells),4531)%3);
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
    return ComposedEdge(config,kind,boundaryX-1,z,boundaryX,tree,edgeHash);
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
    return ComposedEdge(config,kind,x,boundaryZ-1,x,tree,edgeHash);
}

CellProp PropAt(const WorldConfig& config, int cellX, int cellZ) {
    CellProp result;
    if (IsEmptyHall(config,cellX,cellZ)) return result;
    if (IsServiceChamber(config,cellX,cellZ)) return result;
    if (PortalAt(config,cellX,cellZ)) return result;
    if ((cellZ==0 && cellX>=0 && cellX<=3) ||
        (cellX==0 && cellZ>=0 && cellZ<=3)) return result;
    const RegionKind region=RegionAt(config,cellX,cellZ);
    if (ColumnAt(config,cellX,cellZ) ||
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

Wall PropBounds(const CellProp& prop) {
    double halfX=kChairHalfWidth,halfZ=kChairHalfDepth;
    if (prop.kind==PropKind::Table) {
        halfX=kTableHalfWidth;halfZ=kTableHalfDepth;
    } else if (prop.kind==PropKind::LowPartition) {
        halfX=1.70;halfZ=0.16;
    } else if (prop.kind==PropKind::TallPartition) {
        halfX=1.28;halfZ=0.14;
    }
    if (prop.quarterTurn%2) std::swap(halfX,halfZ);
    return {prop.x-halfX,prop.z-halfZ,prop.x+halfX,prop.z+halfZ};
}

std::optional<OfficePartitionStyle> OfficePartitionStyleAt(
    const WorldConfig& config,int rx,int rz) {
    if (config.level!=0 || (rx==0 && rz==0) ||
        IsEmptyHall(config,rx*kRegionCells,rz*kRegionCells)) return std::nullopt;
    const RegionKind kind=RegionAt(config,rx*kRegionCells,rz*kRegionCells);
    if (kind!=RegionKind::OpenOffice && kind!=RegionKind::Irregular &&
        kind!=RegionKind::Columns && kind!=RegionKind::Rooms) return std::nullopt;
    const auto layout=CellHash(config,rx,rz,3119);
    if (kind==RegionKind::Columns && layout%3!=0) return std::nullopt;
    if (kind==RegionKind::Rooms && layout%2!=0) return std::nullopt;
    return static_cast<OfficePartitionStyle>((layout>>21)%6);
}

std::optional<OfficeColumnStyle> OfficeColumnStyleAt(
    const WorldConfig& config,int rx,int rz) {
    if (config.level!=0 ||
        RegionAt(config,rx*kRegionCells,rz*kRegionCells)!=RegionKind::Columns ||
        IsEmptyHall(config,rx*kRegionCells,rz*kRegionCells)) return std::nullopt;
    return static_cast<OfficeColumnStyle>(CellHash(config,rx,rz,4497)%5);
}

CellObstacleSet InteriorPartitionsAt(const WorldConfig& config,
                                     int cellX, int cellZ) {
    CellObstacleSet result;
    const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
    const auto style=OfficePartitionStyleAt(config,rx,rz);
    if (!style) return result;
    const auto layout=CellHash(config,rx,rz,3119);

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
    const auto run=[&](double a,double b,double across) {
        if (alongX) horizontal(originX+a,originX+b,originZ+across);
        else vertical(originZ+a,originZ+b,originX+across);
    };
    const auto branch=[&](double a,double b,double along) {
        if (alongX) vertical(originZ+a,originZ+b,originX+along);
        else horizontal(originX+a,originX+b,originZ+along);
    };
    // A small set of plans composes spaces at region scale. Each wall is
    // clipped to its owning cell with the same clearance around shared doors.
    switch (*style) {
    case OfficePartitionStyle::Straight:
        run(start,end,cross);
        break;
    case OfficePartitionStyle::LShaped: {
        const double corner=clearBoundary(start+11.4);
        run(start,corner,cross);
        branch(cross,std::min(27.8,cross+7.2),corner);
        break;
    }
    case OfficePartitionStyle::TShaped: {
        run(start,end,cross);
        const double junction=clearBoundary((start+end)*0.59);
        const double low=std::max(1.5,cross-7.2);
        const double high=std::min(27.8,cross+5.4);
        branch(low,high,junction);
        break;
    }
    case OfficePartitionStyle::Staggered: {
        run(start,start+9.2,cross);
        const double second=clearBoundary(std::min(25.7,cross+7.5));
        run(start+7.4,std::min(end,start+18.4),second);
        break;
    }
    case OfficePartitionStyle::ShortWall:
        run(start+6.8,std::min(end,start+15.2),cross);
        break;
    case OfficePartitionStyle::DeadSpace: {
        const double left=clearBoundary(start+1.9);
        const double right=clearBoundary(start+15.9);
        const double back=std::min(27.8,cross+5.7);
        run(left,right,cross);
        branch(cross,back,left);branch(cross,back,right);
        break;
    }
    }
    return result;
}

std::optional<OfficeAlcove> OfficeAlcoveAt(const WorldConfig& config,
                                         int cellX,int cellZ) {
    if (config.level!=0) return std::nullopt;
    const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
    const auto h=CellHash(config,rx,rz,4211);
    // At most one candidate per region; empty offices should stay mostly empty.
    if (h%4!=0 || ModFloor(cellX,kRegionCells)!=static_cast<int>((h>>6)%6) ||
        ModFloor(cellZ,kRegionCells)!=static_cast<int>((h>>12)%6) ||
        (rx==0 && rz==0)) return std::nullopt;
    const auto kind=RegionAt(config,cellX,cellZ);
    if (kind==RegionKind::OpenOffice || kind==RegionKind::Columns ||
        IsEmptyHall(config,cellX,cellZ) || PortalAt(config,cellX,cellZ) ||
        PropAt(config,cellX,cellZ).kind!=PropKind::None ||
        InteriorPartitionsAt(config,cellX,cellZ).count>0) return std::nullopt;
    for (int i=0;i<4;++i) {
        const int side=(static_cast<int>((h>>18)%4)+i)%4;
        const bool vertical=side<2;
        const bool far=side%2!=0;
        const auto edge=vertical ? VerticalEdge(config,cellX+far,cellZ) :
                                   HorizontalEdge(config,cellX,cellZ+far);
        if (edge!=Edge::Solid) continue;
        const double center=(vertical ? cellZ : cellX)*kCellSize+2.5+
                            (static_cast<int>((h>>22)%3)-1)*0.12;
        const double half=1.05+((h>>25)%3)*0.10;
        return OfficeAlcove{vertical,far ? -1 : 1,
            ((vertical ? cellX : cellZ)+far)*kCellSize,
            center-half,center+half,0.90+((h>>28)%3)*0.14,(h&0x10000U)!=0};
    }
    return std::nullopt;
}

CellObstacleSet OfficeAlcoveWalls(const OfficeAlcove& alcove) {
    CellObstacleSet result;
    result.count=2;
    const double cross=alcove.boundary+alcove.inward*alcove.depth;
    const double near=std::min(alcove.boundary,cross);
    const double far=std::max(alcove.boundary,cross);
    for (int i=0;i<2;++i) {
        const double along=i==0 ? alcove.start : alcove.end;
        result.walls[i]=alcove.vertical ?
            Wall{near,along-0.10,far,along+0.10} :
            Wall{along-0.10,near,along+0.10,far};
    }
    return result;
}

CellObstacleSet FullHeightObstaclesAt(const WorldConfig& config, int cellX, int cellZ) {
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
    const auto interior=InteriorPartitionsAt(config,cellX,cellZ);
    for (int i=0;i<interior.count;++i)
        result.walls[result.count++]=interior.walls[i];
    if (ColumnAt(config,cellX,cellZ)) {
        double cx=(cellX+0.5)*kCellSize,cz=(cellZ+0.5)*kCellSize;
        if (config.level==0) {
            // Shift the complete support plan by ceiling tiles, not unrelated
            // per-column noise. Every support remains well inside its cell.
            const auto layout=CellHash(config,DivFloor(cellX,kRegionCells),
                                       DivFloor(cellZ,kRegionCells),4497);
            cx+=(static_cast<int>((layout>>7)%3)-1)*0.625;
            cz+=(static_cast<int>((layout>>10)%3)-1)*0.625;
        }
        result.walls[result.count++]={cx-0.43,cz-0.43,cx+0.43,cz+0.43};
    }
    if (const auto alcove=OfficeAlcoveAt(config,cellX,cellZ)) {
        const auto sides=OfficeAlcoveWalls(*alcove);
        for (int i=0;i<sides.count;++i) result.walls[result.count++]=sides.walls[i];
    }
    return result;
}

std::optional<EntitySpawn> EntityAt(const WorldConfig& config,int cellX,int cellZ) {
    const auto hash=CellHash(config,cellX,cellZ,919);
    if (hash%LevelInfo(config).entityRarity!=0 ||
        (cellX>=-4 && cellX<=4 && cellZ>=-4 && cellZ<=4) ||
        CellObstacles(config,cellX,cellZ).count!=0) return std::nullopt;
    // A deterministic atmosphere candidate, with no collision or interaction state.
    return EntitySpawn{(cellX+0.5)*kCellSize,(cellZ+0.5)*kCellSize,
                       static_cast<float>((hash>>8)%628)/100.0f,
                       static_cast<EntityKind>((hash>>24)%kEntityKindCount)};
}

EntityPosition EntityPositionAt(const EntitySpawn& entity,float seconds) {
    const float pace=entity.kind==EntityKind::Watcher ? 0.45f :
                     entity.kind==EntityKind::Crawler ? 0.72f : 1.0f;
    return {entity.x+0.55*std::sin(seconds*0.28f*pace+entity.phase),
            entity.z+0.55*std::cos(seconds*0.21f*pace+entity.phase)};
}

const char* EntityName(EntityKind kind) {
    switch (kind) {
        case EntityKind::Wanderer: return "wanderer";
        case EntityKind::Watcher: return "watcher";
        case EntityKind::Crawler: return "crawler";
    }
    return "unknown";
}

bool UtilityAlongZAt(const WorldConfig& config,int cellX,int cellZ) {
    return (CellHash(config,DivFloor(cellX,kRegionCells),
                     DivFloor(cellZ,kRegionCells),3929)&1U)==0;
}

CellObstacleSet CellObstacles(const WorldConfig& config, int cellX, int cellZ) {
    auto result=FullHeightObstaclesAt(config,cellX,cellZ);
    if (IsEmptyHall(config,cellX,cellZ) || PortalAt(config,cellX,cellZ) ||
        IsServiceChamber(config,cellX,cellZ)) return result;
    const RegionKind kind=RegionAt(config,cellX,cellZ);
    const int lx=ModFloor(cellX,kRegionCells),lz=ModFloor(cellZ,kRegionCells);
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
        const int rx=DivFloor(cellX,kRegionCells),rz=DivFloor(cellZ,kRegionCells);
        const auto assembly=CellHash(config,rx,rz,3931);
        const bool pressureRack=assembly%2==0;
        // Pressure equipment is a sparse bank on one side. Cabinets can form
        // tighter paired runs; neither layout obstructs the center junction.
        if (!pressureRack || CellHash(config,cellX,cellZ,3937)%3==0) {
            const double inset=CellHash(config,rx,rz,3923)%3==0 ? 1.50 : 1.25;
            const bool rotate=!UtilityAlongZAt(config,cellX,cellZ);
            const auto box=[&](double x0,double z0,double x1,double z1) {
                result.walls[result.count++]=rotate ? Wall{x+z0,z+x0,x+z1,z+x1} :
                                                        Wall{x+x0,z+z0,x+x1,z+z1};
            };
            if (!pressureRack || (assembly&0x10U)==0) {
                box(0.15,0.15,inset,1.5);box(0.15,3.5,inset,4.85);
            }
            if (!pressureRack || (assembly&0x10U)!=0) {
                box(5-inset,0.15,4.85,1.5);box(5-inset,3.5,4.85,4.85);
            }
        }
    }
    const CellProp prop=PropAt(config,cellX,cellZ);
    if (prop.kind!=PropKind::None) {
        result.walls[result.count++]=PropBounds(prop);
    }
    return result;
}

static std::vector<Wall> CollectNearbyWalls(const WorldConfig& config, double x, double z, bool fullHeightOnly) {
    std::vector<Wall> walls;
    const int cx = CellOf(x), cz = CellOf(z);
    walls.reserve(88);
    for (int ix=cx-1; ix<=cx+1; ++ix) {
        for (int iz=cz-1; iz<=cz+1; ++iz) {
            AppendEdge(walls,config,VerticalEdge(config, ix, iz),true,
                       ix,iz,ix*kCellSize,iz*kCellSize);
            AppendEdge(walls,config,HorizontalEdge(config, ix, iz),false,
                       ix,iz,iz*kCellSize,ix*kCellSize);
            const auto obstacles=fullHeightOnly ? FullHeightObstaclesAt(config,ix,iz) :
                                  CellObstacles(config,ix,iz);
            for (int i=0;i<obstacles.count;++i)
                walls.push_back(obstacles.walls[i]);
        }
    }
    for (int ix=cx-1;ix<=cx+1;++ix) for (int iz=cz-1;iz<=cz+1;++iz) {
        const auto portal=PortalAt(config,ix,iz);
        if (!portal) continue;
        const auto frames=PortalWalls(*portal);
        // Side walls meet the main ceiling; the lower rear wall is excluded
        // from the full-height lighting approximation.
        const int count=fullHeightOnly ? 2 : frames.count;
        for (int n=0;n<count;++n) walls.push_back(frames.walls[n]);
    }
    return walls;
}

std::vector<Wall> NearbyWalls(const WorldConfig& config, double x, double z) {
    return CollectNearbyWalls(config,x,z,false);
}

std::vector<Wall> NearbyFullHeightWalls(const WorldConfig& config, double x, double z) {
    return CollectNearbyWalls(config,x,z,true);
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

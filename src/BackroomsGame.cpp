#include "BackroomsGame.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPassCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

namespace Backrooms {
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace {
using Clock = std::chrono::steady_clock;
using Mesh = std::vector<VertexPositionColorTexture>;
using Meshes = std::array<Mesh,kMaterialCount>;

void QuadColors(Meshes& meshes, Material material, Vector3 a, Vector3 b,
                Vector3 c, Vector3 d, const std::array<Color,4>& colors,
                Vector2 uvA, Vector2 uvB, Vector2 uvC, Vector2 uvD) {
    auto& mesh=meshes[static_cast<int>(material)];
    mesh.emplace_back(a,colors[0],uvA); mesh.emplace_back(b,colors[1],uvB);
    mesh.emplace_back(c,colors[2],uvC);
    mesh.emplace_back(a,colors[0],uvA); mesh.emplace_back(c,colors[2],uvC);
    mesh.emplace_back(d,colors[3],uvD);
}

void Quad(Meshes& meshes, Material material, Vector3 a, Vector3 b, Vector3 c,
          Vector3 d, Color color, Vector2 uvA, Vector2 uvB, Vector2 uvC,
          Vector2 uvD) {
    QuadColors(meshes,material,a,b,c,d,{color,color,color,color},
               uvA,uvB,uvC,uvD);
}

void Flat(Meshes& meshes, Material material, float x0, float z0, float x1,
          float z1, float y, Color color, float repeat) {
    Quad(meshes,material,{x0,y,z0},{x1,y,z0},{x1,y,z1},{x0,y,z1},color,
         {x0*repeat,z0*repeat},{x1*repeat,z0*repeat},
         {x1*repeat,z1*repeat},{x0*repeat,z1*repeat});
}

void FlatShaded(Meshes& meshes, Material material, float x0, float z0,
                float x1, float z1, float y,
                const std::array<Color,4>& colors, float repeat) {
    QuadColors(meshes,material,{x0,y,z0},{x1,y,z0},{x1,y,z1},
               {x0,y,z1},colors,
               {x0*repeat,z0*repeat},{x1*repeat,z0*repeat},
               {x1*repeat,z1*repeat},{x0*repeat,z1*repeat});
}

Color Scale(Color color, float factor) {
    const auto scaled=[&](int channel) {
        return static_cast<std::uint8_t>(std::clamp(
            static_cast<int>(std::lround(channel*factor)),0,255));
    };
    return Color(scaled(color.getRProperty()),scaled(color.getGProperty()),
                 scaled(color.getBProperty()));
}

void WallFace(Meshes& meshes, Material material, bool vertical, float boundary,
              float a, float b, float y0, float y1, Color color) {
    const float u0=a*0.6f,u1=b*0.6f;
    const float v0=1.0f-y0/3.0f,v1=1.0f-y1/3.0f;
    const auto shade=[&](float height) {
        if (material==Material::Wallpaper)
            return 0.83f+0.17f*std::clamp(height/3.0f,0.0f,1.0f);
        if (material==Material::ConcreteWall)
            return 0.90f+0.10f*std::clamp(height/4.1f,0.0f,1.0f);
        return 1.0f;
    };
    const Color bottom=Scale(color,shade(y0));
    const Color top=Scale(color,shade(y1));
    if (vertical)
        QuadColors(meshes,material,{boundary,y0,a},{boundary,y0,b},
                   {boundary,y1,b},{boundary,y1,a},
                   {bottom,bottom,top,top},
                   {u0,v0},{u1,v0},{u1,v1},{u0,v1});
    else
        QuadColors(meshes,material,{a,y0,boundary},{b,y0,boundary},
                   {b,y1,boundary},{a,y1,boundary},
                   {bottom,bottom,top,top},
                   {u0,v0},{u1,v0},{u1,v1},{u0,v1});
}

void Partition(Meshes& meshes, Material material, Edge edge, bool vertical,
               float boundary, float along, float height, float doorHeight,
               Color wall, Color trim) {
    if (edge == Edge::Open) return;
    const auto section = [&](float a, float b, float y0, float y1) {
        WallFace(meshes,material,vertical,boundary,a,b,y0,y1,wall);
    };
    const auto full = [&](float a, float b) {
        WallFace(meshes,material,vertical,boundary,a,b,0.0f,0.19f,trim);
        section(a,b,0.19f,height-0.10f);
        WallFace(meshes,material,vertical,boundary,a,b,
                 height-0.10f,height,trim);
    };
    if (edge == Edge::Solid) full(along, along+5.0f);
    else {
        const float side=edge==Edge::Wide ? 0.75f : 1.55f;
        full(along,along+side);
        full(along+5.0f-side,along+5.0f);
        const auto cap=[&](float end) {
            WallFace(meshes,material,!vertical,end,
                     boundary-0.10f,boundary+0.10f,0.19f,height-0.10f,
                     Scale(wall,0.82f));
            WallFace(meshes,material,!vertical,end,
                     boundary-0.10f,boundary+0.10f,0,0.19f,trim);
            WallFace(meshes,material,!vertical,end,
                     boundary-0.10f,boundary+0.10f,height-0.10f,height,trim);
        };
        cap(along+side);
        cap(along+5.0f-side);
        if (edge==Edge::Door) {
            WallFace(meshes,material,vertical,boundary,along+side,
                     along+5.0f-side,doorHeight,height,wall);
            if (vertical)
                Flat(meshes,material,boundary-0.10f,along+side,
                     boundary+0.10f,along+5.0f-side,
                     doorHeight,Scale(wall,0.83f),0.6f);
            else
                Flat(meshes,material,along+side,boundary-0.10f,
                     along+5.0f-side,boundary+0.10f,
                     doorHeight,Scale(wall,0.83f),0.6f);
        }
    }
}

void BoxRange(Meshes& meshes, Material material, float x0, float z0,
              float x1, float z1, float y0, float y1, Color color) {
    WallFace(meshes,material,true,x0,z0,z1,y0,y1,color);
    WallFace(meshes,material,true,x1,z0,z1,y0,y1,color);
    WallFace(meshes,material,false,z0,x0,x1,y0,y1,color);
    WallFace(meshes,material,false,z1,x0,x1,y0,y1,color);
    Flat(meshes,material,x0,z0,x1,z1,y1,color,0.6f);
}

void PropBox(Meshes& meshes, Material material, float cx, float cz,
             int quarterTurn, float sink, float x0, float z0, float x1,
             float z1, float y0, float y1, Color color) {
    const auto rotate=[&](float x,float z) {
        switch (quarterTurn&3) {
        case 1: return std::pair{z,-x};
        case 2: return std::pair{-x,-z};
        case 3: return std::pair{-z,x};
        default: return std::pair{x,z};
        }
    };
    const auto a=rotate(x0,z0),b=rotate(x1,z1);
    BoxRange(meshes,material,cx+std::min(a.first,b.first),
             cz+std::min(a.second,b.second),cx+std::max(a.first,b.first),
             cz+std::max(a.second,b.second),y0-sink,y1-sink,color);
}

void Furniture(Meshes& meshes, const CellProp& prop, double chunkX,
               double chunkZ, int level) {
    if (prop.kind==PropKind::None) return;
    const float x=static_cast<float>(prop.x-chunkX);
    const float z=static_cast<float>(prop.z-chunkZ);
    const int turn=prop.quarterTurn;
    const float sink=prop.sink;
    const Material metal=level==0 ? Material::ConcreteWall : Material::TunnelWall;
    const Color seat=level==0 ? Color(125,113,91) : Color(100,106,103);
    const Color frame=level==0 ? Color(80,78,69) : Color(67,73,72);
    const auto box=[&](Material mat,float x0,float z0,float x1,float z1,
                       float y0,float y1,Color color) {
        PropBox(meshes,mat,x,z,turn,sink,x0,z0,x1,z1,y0,y1,color);
    };
    if (prop.kind==PropKind::Chair || prop.kind==PropKind::EmbeddedChair) {
        box(metal,-0.35f,-0.32f,0.35f,0.32f,0.40f,0.49f,seat);
        box(metal,-0.35f,0.24f,0.35f,0.32f,0.46f,1.18f,seat);
        for (float legX: {-0.27f,0.27f})
            for (float legZ: {-0.24f,0.24f})
                box(metal,legX-0.035f,legZ-0.035f,
                    legX+0.035f,legZ+0.035f,0,0.41f,frame);
    } else if (prop.kind==PropKind::LowPartition) {
        box(Material::Wallpaper,-1.66f,-0.14f,1.66f,0.14f,
            0,1.34f,Color(214,205,171));
        box(Material::Wallpaper,-1.70f,-0.16f,1.70f,0.16f,
            1.34f,1.41f,Color(151,143,112));
    } else {
        const Color top=level==0 ? Color(142,126,96) : Color(109,116,112);
        box(metal,-0.91f,-0.56f,0.91f,0.56f,0.73f,0.85f,top);
        for (float legX: {-0.78f,0.78f})
            for (float legZ: {-0.43f,0.43f})
                box(metal,legX-0.05f,legZ-0.05f,
                    legX+0.05f,legZ+0.05f,0,0.74f,frame);
    }
}

void FalseDoor(Meshes& meshes, bool vertical, float boundary, float along,
               int level) {
    const float height=level==1 ? 2.58f : level==2 ? 2.15f : 2.18f;
    const Material material=level==0 ? Material::TunnelWall : Material::ConcreteWall;
    const Color panel=level==0 ? Color(119,106,79) : Color(85,96,94);
    const Color frame=level==0 ? Color(166,154,113) : Color(126,136,130);
    const float left=along+1.90f,right=along+3.10f;
    const float face=boundary+0.045f;
    WallFace(meshes,material,vertical,face,left,right,0.02f,height,panel);
    WallFace(meshes,material,vertical,face+0.004f,left-0.075f,left,
             0,height+0.08f,frame);
    WallFace(meshes,material,vertical,face+0.004f,right,right+0.075f,
             0,height+0.08f,frame);
    WallFace(meshes,material,vertical,face+0.004f,left,right,
             height,height+0.08f,frame);
    WallFace(meshes,Material::Fluorescent,vertical,face+0.008f,
             right-0.22f,right-0.15f,0.99f,1.06f,
             Color(116,104,75));
}

Color Vary(Color first, Color second, std::uint32_t hash) {
    return hash%5 == 0 ? second : first;
}

struct LampInfo {
    bool fixture=false;
    bool lit=false;
    float x=2.5f,z=2.5f;
    bool longAxisX=true;
};

int FloorDiv(int value, int size) {
    return value>=0 ? value/size : (value-size+1)/size;
}

LampInfo LampAt(const WorldConfig& world, int gx, int gz) {
    LampInfo lamp;
    const auto h=CellHash(world,gx,gz,41);
    if (world.level==0) {
        const int rx=FloorDiv(gx,kRegionCells);
        const int rz=FloorDiv(gz,kRegionCells);
        const int lx=gx-rx*kRegionCells,lz=gz-rz*kRegionCells;
        const auto layout=CellHash(world,rx,rz,1803);
        const int phaseX=static_cast<int>((layout>>4)&1U);
        const int phaseZ=static_cast<int>((layout>>7)&1U);
        const auto region=RegionAt(world,gx,gz);
        if (region==RegionKind::OpenOffice || region==RegionKind::Columns)
            lamp.fixture=(layout&0x1000U) ?
                ((lx+phaseX)%2==0 || (lz+phaseZ)%3==0) :
                ((lx+phaseX)%2==0 && (lz+phaseZ)%2==0);
        else if (region==RegionKind::Halls)
            lamp.fixture=(lz+phaseZ)%2==0 && (lx+phaseX)%3!=0;
        else
            lamp.fixture=(lx+phaseX)%2==0 && (lz+phaseZ)%2==0;
        if (h%29==0) lamp.fixture=false;
        lamp.lit=lamp.fixture && h%17!=0;
        lamp.x=(h&1U) ? 2.1875f : 2.8125f;
        lamp.z=(h&2U) ? 2.1875f : 2.8125f;
        lamp.longAxisX=(layout&1U)!=0;
    } else {
        lamp.fixture=world.level==1 ? (gx%3==0 && gz%2==0) :
                     ((gx+gz)%3==0 && h%3!=0);
        if (world.level==2 && gx==0 &&
            (gz==0 || gz==1 || gz==3)) lamp.fixture=true;
        lamp.lit=lamp.fixture;
    }
    return lamp;
}

float LightFactor(const WorldConfig& world, double wx, double wz) {
    const float base=world.level==0 ? 0.68f : world.level==1 ? 0.60f : 0.55f;
    const float strength=world.level==0 ? 0.38f : world.level==1 ? 0.38f : 0.41f;
    float value=base;
    const int cx=CellOf(wx),cz=CellOf(wz);
    for (int dx=-1;dx<=1;++dx) for (int dz=-1;dz<=1;++dz) {
        const int gx=cx+dx,gz=cz+dz;
        const auto lamp=LampAt(world,gx,gz);
        if (!lamp.lit) continue;
        const double lightX=gx*kCellSize+lamp.x;
        const double lightZ=gz*kCellSize+lamp.z;
        const float attenuation=std::max(0.0f,1.0f-
            static_cast<float>(std::hypot(wx-lightX,wz-lightZ))/6.5f);
        value+=strength*attenuation;
    }
    return std::min(1.0f,value);
}

std::optional<int> PortalTarget(const WorldConfig& world, double x, double z) {
    const auto portal=PortalAt(world,CellOf(x),CellOf(z));
    if (!portal) return std::nullopt;
    const double px=(portal->cellX+0.5)*kCellSize;
    const double pz=(portal->cellZ+0.5)*kCellSize;
    const double depth=portal->alongX ? x-px : z-pz;
    const double side=portal->alongX ? z-pz : x-px;
    if (depth>0.92 && depth<2.05 && std::abs(side)<1.30)
        return portal->target;
    return std::nullopt;
}

void PortalVisual(Meshes& meshes, int level, bool alongX, float cx,
                  float cz, float ceiling) {
    const Material frameMat=level==0 ? Material::Wallpaper : Material::TunnelWall;
    const Color frame=level==0 ? Color(112,101,68) : Color(129,111,85);
    const Color dark(49,45,40);
    const float near=0.43f, far=1.92f, half=1.1f;
    if (alongX) {
        BoxRange(meshes,frameMat,cx+near,cz-half-0.18f,
                 cx+far,cz-half,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+near,cz+half,
                 cx+far,cz+half+0.18f,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+near,cz-half,
                 cx+far,cz+half,2.25f,ceiling,frame);
        WallFace(meshes,Material::TunnelWall,true,cx+far,cz-half,cz+half,
                 0,2.25f,dark);
        Flat(meshes,Material::TunnelFloor,cx+near,cz-half,
             cx+far,cz+half,0.012f,Color(115,105,82),0.5f);
        Flat(meshes,Material::TunnelCeiling,cx+near,cz-half,
             cx+far,cz+half,2.245f,Color(107,99,79),0.6f);
        Flat(meshes,Material::Fluorescent,cx+1.08f,cz-0.25f,
             cx+1.48f,cz+0.25f,2.235f,Color(154,136,99),1.0f);
    } else {
        BoxRange(meshes,frameMat,cx-half-0.18f,cz+near,
                 cx-half,cz+far,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+half,cz+near,
                 cx+half+0.18f,cz+far,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx-half,cz+near,
                 cx+half,cz+far,2.25f,ceiling,frame);
        WallFace(meshes,Material::TunnelWall,false,cz+far,cx-half,cx+half,
                 0,2.25f,dark);
        Flat(meshes,Material::TunnelFloor,cx-half,cz+near,
             cx+half,cz+far,0.012f,Color(115,105,82),0.5f);
        Flat(meshes,Material::TunnelCeiling,cx-half,cz+near,
             cx+half,cz+far,2.245f,Color(107,99,79),0.6f);
        Flat(meshes,Material::Fluorescent,cx-0.25f,cz+1.08f,
             cx+0.25f,cz+1.48f,2.235f,Color(154,136,99),1.0f);
    }
}
}

BackroomsGame::BackroomsGame(std::uint64_t seed, bool streamTest,
                             int startLevel, double startX, double startZ,
                             double walkSpeed, double runSpeed)
    : graphics_(this), streamTest_(streamTest) {
    if (startLevel<0 || startLevel>2 || !std::isfinite(startX) ||
        !std::isfinite(startZ) || std::abs(startX)>1.0e8 ||
        std::abs(startZ)>1.0e8 || !std::isfinite(walkSpeed) ||
        !std::isfinite(runSpeed) || walkSpeed<=0 || runSpeed<=walkSpeed ||
        runSpeed>20.0)
        throw std::invalid_argument("invalid start level, position, or movement speed");
    world_.seed = seed;
    world_.level=startLevel;
    if (Collides(world_,startX,startZ,0.31))
        throw std::invalid_argument("start position intersects generated geometry");
    x_=startX;
    z_=startZ;
    walkSpeed_=walkSpeed;
    runSpeed_=runSpeed;
    yaw_=startLevel==2 ? 0.0f : 1.5707963f;
    graphics_.setPreferredBackBufferWidthProperty(1280);
    graphics_.setPreferredBackBufferHeightProperty(720);
    graphics_.setSynchronizeWithVerticalRetraceProperty(true);
    getWindowProperty().setTitleProperty("cna-backrooms");
}

const std::string& BackroomsGame::GetTypeName() const {
    static const std::string name = "Backrooms.BackroomsGame";
    return name;
}

void BackroomsGame::Initialize() {
    Game::Initialize();
}

void BackroomsGame::LoadContent() {
    effect_ = std::make_unique<BasicEffect>(getGraphicsDeviceProperty());
    materials_ = std::make_unique<Materials>(getGraphicsDeviceProperty());
    effect_->VertexColorEnabled = true;
    effect_->setTextureEnabledProperty(true);
    effect_->setLightingEnabledProperty(false);
    effect_->setFogEnabledProperty(true);
    effect_->setFogStartProperty(LevelInfo(world_.level).fogStart);
    effect_->setFogEndProperty(LevelInfo(world_.level).fogEnd);
    setIsMouseVisibleProperty(false);
    Mouse::setIsRelativeMouseModeEXTProperty(true);
    captured_ = true;
    BuildEntityMesh();
    BuildChunk(ChunkAt(x_,z_));
    try {
        namespace fs = std::filesystem;
        fs::path directory="assets";
        if (!fs::exists(directory/"hum.wav")) directory="../assets";
        if (!fs::exists(directory/"hum.wav"))
            directory=fs::read_symlink("/proc/self/exe").parent_path()/"assets";
        humSound_=std::make_unique<Audio::SoundEffect>((directory/"hum.wav").string());
        step_=std::make_unique<Audio::SoundEffect>((directory/"step.wav").string());
        transition_=std::make_unique<Audio::SoundEffect>((directory/"transition.wav").string());
        hum_=std::make_unique<Audio::SoundEffectInstance>(humSound_->CreateInstance());
        hum_->setVolumeProperty(0.34f);
        hum_->setIsLoopedProperty(true);
        hum_->Play();
        if (hum_->getStateProperty()!=Audio::SoundState::Playing)
            throw std::runtime_error("fluorescent hum did not start playing");
        std::cerr << "Audio ready: hum loop and event sounds loaded from "
                  << directory << '\n';
    } catch (const std::exception& error) {
        hum_.reset(); humSound_.reset(); step_.reset(); transition_.reset();
        std::cerr << "Audio unavailable: " << error.what() << '\n';
    }
}

void BackroomsGame::BuildEntityMesh() {
    Meshes meshes;
    const Material body=Material::TunnelWall;
    const Color coat(72,72,70),head(85,78,69);
    BoxRange(meshes,body,-0.27f,-0.17f,0.27f,0.18f,
             0.72f,1.68f,coat);
    BoxRange(meshes,body,-0.22f,-0.18f,0.22f,0.20f,
             1.71f,2.07f,head);
    BoxRange(meshes,body,-0.22f,-0.15f,-0.04f,0.12f,
             0,0.76f,coat);
    BoxRange(meshes,body,0.04f,-0.15f,0.22f,0.12f,
             0,0.76f,coat);
    BoxRange(meshes,body,-0.39f,-0.12f,-0.26f,0.12f,
             0.82f,1.57f,coat);
    BoxRange(meshes,body,0.26f,-0.12f,0.39f,0.12f,
             0.82f,1.57f,coat);
    auto& mesh=meshes[static_cast<int>(body)];
    entityTriangles_=static_cast<int>(mesh.size()/3);
    entityVertices_=std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColorTexture::getVertexDeclarationStatic(),
        static_cast<int>(mesh.size()),BufferUsage::WriteOnly);
    entityVertices_->SetData(mesh.data(),static_cast<int>(mesh.size()));
}

std::unique_ptr<VertexBuffer> BackroomsGame::AcquireBuffer(int vertexCount) {
    std::size_t best=spareVertices_.size();
    for (std::size_t i=0;i<spareVertices_.size();++i) {
        const int capacity=spareVertices_[i]->getVertexCountProperty();
        if (capacity>=vertexCount &&
            (best==spareVertices_.size() ||
             capacity<spareVertices_[best]->getVertexCountProperty()))
            best=i;
    }
    if (best!=spareVertices_.size()) {
        auto buffer=std::move(spareVertices_[best]);
        spareVertices_.erase(spareVertices_.begin()+static_cast<std::ptrdiff_t>(best));
        ++bufferReuses_;
        return buffer;
    }
    ++bufferCreations_;
    const int capacity=((vertexCount+511)/512)*512;
    return std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColorTexture::getVertexDeclarationStatic(),
        capacity,BufferUsage::WriteOnly);
}

void BackroomsGame::RetireChunk(Chunk& chunk) {
    for (auto& buffer:chunk.vertices) {
        if (buffer && spareVertices_.size()<24)
            spareVertices_.push_back(std::move(buffer));
    }
}

void BackroomsGame::BuildChunk(ChunkCoord coord) {
    const auto start = Clock::now();
    Meshes meshes;
    Chunk chunk;
    const int ox=coord.x*kChunkCells, oz=coord.z*kChunkCells;
    const int level=world_.level;
    const float height=LevelInfo(level).ceilingHeight;
    const float doorHeight=level==0 ? 2.62f : level==1 ? 2.8f : 2.08f;
    const Material wallMat=level==0 ? Material::Wallpaper :
                           level==1 ? Material::ConcreteWall : Material::TunnelWall;
    const Material floorMat=level==0 ? Material::Carpet :
                            level==1 ? Material::ConcreteFloor : Material::TunnelFloor;
    const Material ceilingMat=level==0 ? Material::CeilingTile :
                              level==1 ? Material::IndustrialCeiling : Material::TunnelCeiling;
    const Color wallA=level==0 ? Color(255,250,239) :
                      level==1 ? Color(223,229,227) : Color(187,175,149);
    const Color wallB=level==0 ? Color(231,224,204) :
                      level==1 ? Color(188,202,200) : Color(151,139,118);
    const Color trim=level==0 ? Color(190,180,139) :
                     level==1 ? Color(110,130,128) : Color(99,83,67);
    const Color floorA=level==0 ? Color(246,240,222) :
                       level==1 ? Color(218,222,217) : Color(215,204,178);
    const Color floorB=level==0 ? Color(218,213,197) :
                       level==1 ? Color(193,201,195) : Color(181,173,153);
    const Color ceiling=level==0 ? Color(255,252,235) :
                        level==1 ? Color(205,220,219) : Color(150,143,124);
    const Color grid=level==0 ? Color(143,139,115) :
                     level==1 ? Color(94,114,112) : Color(93,79,65);
    const Color lamp=level==0 ? Color(255,251,228) :
                     level==1 ? Color(204,230,230) : Color(255,198,137);

    for (int lx=0; lx<kChunkCells; ++lx) for (int lz=0; lz<kChunkCells; ++lz) {
        const int gx=ox+lx, gz=oz+lz;
        const float x=lx*5.0f, z=lz*5.0f;
        const auto h=CellHash(world_,gx,gz,41);
        const auto lampInfo=LampAt(world_,gx,gz);
        const Color floorColor=level==0 ? floorA : Vary(floorA,floorB,h);
        const double wx=gx*kCellSize,wz=gz*kCellSize;
        const std::array<float,4> light{{
            LightFactor(world_,wx,wz),LightFactor(world_,wx+5,wz),
            LightFactor(world_,wx+5,wz+5),LightFactor(world_,wx,wz+5)
        }};
        FlatShaded(meshes,floorMat,x,z,x+5,z+5,0,
            {Scale(floorColor,light[0]),Scale(floorColor,light[1]),
             Scale(floorColor,light[2]),Scale(floorColor,light[3])},
             level==0 ? 0.125f : 0.5f);
        if (level==0) {
            const auto ceilingLight=[&](float factor) {
                return Scale(ceiling,0.52f+0.48f*factor);
            };
            FlatShaded(meshes,ceilingMat,x,z,x+5,z+5,height,
                {ceilingLight(light[0]),ceilingLight(light[1]),
                 ceilingLight(light[2]),ceilingLight(light[3])},1.6f);
        } else
            Flat(meshes,ceilingMat,x,z,x+5,z+5,height,
                 Scale(ceiling,lampInfo.lit?1.0f:0.91f),0.8f);
        if (level==1) {
            BoxRange(meshes,ceilingMat,x+0.22f,z,x+0.42f,z+5,
                     height-0.32f,height-0.10f,grid);
        } else if (level==2) {
            BoxRange(meshes,Material::TunnelWall,x+0.35f,z,x+0.50f,z+5,
                     height-0.33f,height-0.17f,grid);
            BoxRange(meshes,Material::TunnelWall,x+4.50f,z,x+4.65f,z+5,
                     height-0.33f,height-0.17f,grid);
        }
        if (level==0 && lampInfo.fixture) {
            const float halfX=lampInfo.longAxisX ? 0.625f : 0.3125f;
            const float halfZ=lampInfo.longAxisX ? 0.3125f : 0.625f;
            const float cx=x+lampInfo.x,cz=z+lampInfo.z;
            Flat(meshes,ceilingMat,cx-halfX-0.025f,cz-halfZ-0.025f,
                 cx+halfX+0.025f,cz+halfZ+0.025f,
                 height-0.022f,Color(205,201,181),1.6f);
            const Material panelMat=lampInfo.lit ?
                Material::Fluorescent : Material::IndustrialCeiling;
            const Color panelColor=lampInfo.lit ?
                Color(255,251,221) : Color(110,107,91);
            Quad(meshes,panelMat,
                 {cx-halfX,height-0.035f,cz-halfZ},
                 {cx+halfX,height-0.035f,cz-halfZ},
                 {cx+halfX,height-0.035f,cz+halfZ},
                 {cx-halfX,height-0.035f,cz+halfZ},panelColor,
                 {0,0},{1,0},{1,1},{0,1});
        } else if (level!=0 && lampInfo.lit) {
            Flat(meshes,ceilingMat,x+1.30f,z+2.27f,x+3.70f,z+2.73f,
                 height-0.024f,grid,0.8f);
            Flat(meshes,Material::Fluorescent,x+1.40f,z+2.34f,
                 x+3.60f,z+2.66f,height-0.032f,lamp,1.0f);
        }
        const Color cellWall=Scale(Vary(wallA,wallB,h),
            level==0 ? 0.52f+0.48f*LightFactor(world_,wx+2.5,wz+2.5) :
            lampInfo.lit ? 1.0f : 0.80f);
        Partition(meshes,wallMat,VerticalEdge(world_,gx,gz),true,x,z,
                  height,doorHeight,cellWall,trim);
        Partition(meshes,wallMat,HorizontalEdge(world_,gx,gz),false,z,x,
                  height,doorHeight,cellWall,trim);
        if (level==2) {
            const Color conduit(147,137,116);
            if (VerticalEdge(world_,gx,gz)==Edge::Solid)
                BoxRange(meshes,Material::IndustrialCeiling,
                         x+0.11f,z,x+0.21f,z+5,1.90f,2.00f,conduit);
            if (VerticalEdge(world_,gx+1,gz)==Edge::Solid)
                BoxRange(meshes,Material::IndustrialCeiling,
                         x+4.79f,z,x+4.89f,z+5,1.90f,2.00f,conduit);
        }
        const auto prop=PropAt(world_,gx,gz);
        const auto obstacles=CellObstacles(world_,gx,gz);
        const int structuralCount=obstacles.count-
            static_cast<int>(prop.kind!=PropKind::None);
        for (int obstacleId=0;obstacleId<structuralCount;++obstacleId) {
            const auto& obstacle=obstacles.walls[obstacleId];
            const float bx0=static_cast<float>(obstacle.minX-ox*kCellSize);
            const float bz0=static_cast<float>(obstacle.minZ-oz*kCellSize);
            const float bx1=static_cast<float>(obstacle.maxX-ox*kCellSize);
            const float bz1=static_cast<float>(obstacle.maxZ-oz*kCellSize);
            if (level==0) {
                BoxRange(meshes,wallMat,bx0,bz0,bx1,bz1,0,height,wallB);
            } else if (level==1) {
                const Color steel(111,123,121),shelfColor(133,141,136);
                for (float sx: {bx0,bx1-0.10f})
                    for (float sz: {bz0,bz1-0.10f})
                        BoxRange(meshes,Material::ConcreteWall,sx,sz,
                                 sx+0.10f,sz+0.10f,0,2.45f,steel);
                for (float shelf: {0.52f,1.16f,1.80f,2.38f})
                    BoxRange(meshes,Material::IndustrialCeiling,
                             bx0,bz0,bx1,bz1,shelf,shelf+0.07f,shelfColor);
                BoxRange(meshes,Material::TunnelWall,
                         bx0+0.18f,bz0+0.20f,bx0+0.72f,bz0+0.83f,
                         0.59f,1.13f,Color(142,123,94));
            } else {
                BoxRange(meshes,Material::TunnelWall,bx0,bz0,bx1,bz1,
                         0,height-0.23f,Color(136,122,101));
            }
        }
        Furniture(meshes,prop,ox*kCellSize,oz*kCellSize,level);
        const auto doorHash=CellHash(world_,gx,gz,1501);
        if (doorHash%170==0) {
            if ((doorHash&1U)==0 && VerticalEdge(world_,gx,gz)==Edge::Solid)
                FalseDoor(meshes,true,x,z,level);
            else if (HorizontalEdge(world_,gx,gz)==Edge::Solid)
                FalseDoor(meshes,false,z,x,level);
        }
        if (const auto portal=PortalAt(world_,gx,gz))
            PortalVisual(meshes,level,portal->alongX,x+2.5f,z+2.5f,height);
        const auto entityHash=CellHash(world_,gx,gz,919);
        const unsigned rarity=level==0 ? 850U : level==1 ? 650U : 750U;
        if (entityHash%rarity==0 && obstacles.count==0 &&
            (std::abs(gx)>4 || std::abs(gz)>4)) {
            chunk.entities.push_back({(gx+0.5)*kCellSize,
                                      (gz+0.5)*kCellSize,
                                      static_cast<float>((entityHash>>8)%628)/100.0f});
        }
    }
    getGraphicsDeviceProperty().SetVertexBuffer(nullptr);
    for (int id=0;id<kMaterialCount;++id) {
        auto& mesh=meshes[id];
        if (mesh.empty()) continue;
        chunk.materialTriangles[id]=static_cast<int>(mesh.size()/3);
        chunk.triangles+=chunk.materialTriangles[id];
        chunk.vertices[id]=AcquireBuffer(static_cast<int>(mesh.size()));
        chunk.vertices[id]->SetData(mesh.data(),static_cast<int>(mesh.size()));
    }
    chunk.buildMs = std::chrono::duration<double,std::milli>(Clock::now()-start).count();
    lastBuildMs_ = chunk.buildMs;
    peakBuildMs_ = std::max(peakBuildMs_,chunk.buildMs);
    chunks_.emplace(coord,std::move(chunk));
}

void BackroomsGame::Stream() {
    const auto center=ChunkAt(x_,z_);
    for (auto it=chunks_.begin(); it!=chunks_.end();) {
        if (std::abs(it->first.x-center.x)>2 || std::abs(it->first.z-center.z)>2) {
            RetireChunk(it->second);
            it=chunks_.erase(it);
        } else ++it;
    }
    // One upload per update bounds a frame's streaming work. Nearest missing chunk first.
    int best=100;
    ChunkCoord next{};
    bool found=false;
    for (int dx=-2; dx<=2; ++dx) for (int dz=-2; dz<=2; ++dz) {
        ChunkCoord candidate{center.x+dx,center.z+dz};
        const int distance=dx*dx+dz*dz;
        if (!chunks_.contains(candidate) && distance<best) {
            next=candidate; best=distance; found=true;
        }
    }
    if (found) BuildChunk(next);
    if (chunks_.size()>25) throw std::runtime_error("streaming exceeded chunk limit");
}

void BackroomsGame::Transition(int level) {
    if (transition_ && !transition_->Play(0.45f,0,0))
        std::cerr << "Audio: transition cue could not acquire a voice\n";
    world_.level=level;
    chunks_.clear();
    spareVertices_.clear();
    x_=2.5; z_=2.5;
    yaw_=level==2 ? 0.0f : 1.5707963f;
    pitch_=0;
    BuildChunk({0,0});
}

void BackroomsGame::UpdateTitle(double elapsed) {
    statsTime_+=elapsed;
    ++frameCount_;
    if (statsTime_<1.0) return;
    int triangles=0,entities=0;
    for (const auto& [coord,chunk]:chunks_) {
        (void)coord;
        triangles+=chunk.triangles;
        entities+=static_cast<int>(chunk.entities.size());
    }
    const auto here=ChunkAt(x_,z_);
    std::ostringstream title;
    title << "cna-backrooms | Level " << world_.level << " | seed " << world_.seed
          << " | pos " << std::fixed << std::setprecision(1) << x_ << ',' << z_
          << " | chunk " << here.x << ',' << here.z
          << " | loaded " << chunks_.size() << "/25 | tris " << triangles
          << " | VBO " << bufferCreations_ << '/' << bufferReuses_
          << " pool " << spareVertices_.size()
          << " | entities " << entities
          << " | build " << std::setprecision(2) << lastBuildMs_ << " ms"
          << " | peak " << peakBuildMs_ << " ms"
          << " | " << (running_ ? "run" : "walk")
          << " | audio " << (hum_ ? "on" : "off")
          << " | " << static_cast<int>(frameCount_/statsTime_) << " FPS";
    getWindowProperty().setTitleProperty(title.str());
    if (streamTest_) std::cout << title.str() << '\n';
    statsTime_=0;
    frameCount_=0;
}

void BackroomsGame::Update(GameTime& time) {
    const double dt=std::min(0.1,time.getElapsedGameTimeProperty().getTotalSecondsProperty());
    if (streamTest_) {
        // Streaming diagnostic: north, south past origin, and back through the
        // live renderer. Collision is bypassed so walls cannot stop the sweep.
        streamTestTime_+=dt;
        const double distance=streamTestTime_*45.0;
        z_=2.5+(distance<2400.0 ? distance :
                 distance<7200.0 ? 4800.0-distance : distance-9600.0);
        Stream();
        UpdateTitle(dt);
        if (distance>=9600.0) Exit();
        Game::Update(time);
        return;
    }
    const auto keys=Keyboard::GetState();
    const bool shiftDown=keys.IsKeyDown(Keys::LeftShift) ||
                         keys.IsKeyDown(Keys::RightShift);
    const bool wasShiftDown=previousKeys_.IsKeyDown(Keys::LeftShift) ||
                            previousKeys_.IsKeyDown(Keys::RightShift);
    if (shiftDown && !wasShiftDown)
        running_=!running_;
    if (keys.IsKeyDown(Keys::Escape) && previousKeys_.IsKeyUp(Keys::Escape)) {
        if (captured_) {
            captured_=false;
            Mouse::setIsRelativeMouseModeEXTProperty(false);
            setIsMouseVisibleProperty(true);
        } else Exit();
    }
    auto mouse=Mouse::GetState();
    if (!captured_ && mouse.getLeftButtonProperty()==ButtonState::Pressed) {
        captured_=true;
        Mouse::setIsRelativeMouseModeEXTProperty(true);
        setIsMouseVisibleProperty(false);
    }
    if (captured_ && getIsActiveProperty()) {
        // CNA/SDL reports positive relative X for physical movement to the right.
        // With this camera's +Z forward convention, decreasing yaw looks right.
        yaw_ -= mouse.getXProperty()*0.0022f;
        pitch_=std::clamp(pitch_-mouse.getYProperty()*0.0022f,-1.43f,1.43f);
        double forward=(keys.IsKeyDown(Keys::W)?1.0:0.0)-(keys.IsKeyDown(Keys::S)?1.0:0.0);
        double strafe=(keys.IsKeyDown(Keys::D)?1.0:0.0)-(keys.IsKeyDown(Keys::A)?1.0:0.0);
        const double length=std::hypot(forward,strafe);
        if (length>0) { forward/=length; strafe/=length; }
        const double speed=(running_?runSpeed_:walkSpeed_)*dt;
        const double dx=(std::sin(yaw_)*forward-std::cos(yaw_)*strafe)*speed;
        const double dz=(std::cos(yaw_)*forward+std::sin(yaw_)*strafe)*speed;
        const double oldX=x_, oldZ=z_;
        MoveWithCollision(world_,x_,z_,dx,dz,0.31);
        stepDistance_ += std::hypot(x_-oldX,z_-oldZ);
        const double stepLength=running_?2.0:1.65;
        if (stepDistance_>stepLength) {
            stepDistance_-=stepLength;
            if (step_ && !step_->Play(0.70f,0,0) && !stepWarningShown_) {
                std::cerr << "Audio: footstep could not acquire a voice\n";
                stepWarningShown_=true;
            }
        }
    }
    if (keys.IsKeyDown(Keys::R) && previousKeys_.IsKeyUp(Keys::R)) {
        x_=2.5; z_=2.5;
        yaw_=world_.level==2 ? 0.0f : 1.5707963f;
        pitch_=0;
        stepDistance_=0;
    }
    const auto portal=PortalTarget(world_,x_,z_);
    if (portal && !insidePortal_) Transition(*portal);
    insidePortal_=portal.has_value();
    Stream();
    UpdateTitle(dt);
    previousKeys_=keys;
    Game::Update(time);
}

void BackroomsGame::Draw(const GameTime& time) {
    auto& device=getGraphicsDeviceProperty();
    const Color fog=world_.level==0 ? Color(108,101,78) :
                    world_.level==1 ? Color(56,67,67) : Color(41,35,29);
    device.Clear(fog);
    device.setDepthStencilStateProperty(DepthStencilState::Default);
    device.setRasterizerStateProperty(RasterizerState::CullNone);
    effect_->setFogColorProperty(fog.ToVector3());
    effect_->setFogStartProperty(LevelInfo(world_.level).fogStart);
    effect_->setFogEndProperty(LevelInfo(world_.level).fogEnd);
    const Vector3 eye(0,1.68f,0);
    const Vector3 direction(std::sin(yaw_)*std::cos(pitch_),std::sin(pitch_),
                            std::cos(yaw_)*std::cos(pitch_));
    effect_->setViewProperty(Matrix::CreateLookAt(eye,eye+direction,Vector3::Up));
    effect_->setProjectionProperty(Matrix::CreatePerspectiveFieldOfView(
        1.20f,device.getViewportProperty().getAspectRatioProperty(),0.08f,105.0f));
    for (const auto& [coord,chunk]:chunks_) {
        effect_->setWorldProperty(Matrix::CreateTranslation(
            static_cast<float>(coord.x*kChunkSize-x_),0,
            static_cast<float>(coord.z*kChunkSize-z_)));
        for (int id=0;id<kMaterialCount;++id) {
            if (!chunk.vertices[id]) continue;
            effect_->setTextureProperty(materials_->Get(static_cast<Material>(id)));
            device.SetVertexBuffer(chunk.vertices[id].get());
            auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
            for (int i=0; i<passes.getCountProperty(); ++i) {
                passes[i]->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList,0,
                                      chunk.materialTriangles[id]);
            }
        }
    }
    effect_->setTextureProperty(materials_->Get(Material::TunnelWall));
    device.SetVertexBuffer(entityVertices_.get());
    const float seconds=static_cast<float>(
        time.getTotalGameTimeProperty().getTotalSecondsProperty());
    for (const auto& [coord,chunk]:chunks_) {
        (void)coord;
        for (const auto& entity:chunk.entities) {
            const double ex=entity.x+0.55*std::sin(seconds*0.28f+entity.phase);
            const double ez=entity.z+0.55*std::cos(seconds*0.21f+entity.phase);
            if (std::hypot(ex-x_,ez-z_)>82.0) continue;
            const float facing=static_cast<float>(std::atan2(x_-ex,z_-ez));
            effect_->setWorldProperty(Matrix::CreateRotationY(facing)*
                Matrix::CreateTranslation(static_cast<float>(ex-x_),0,
                                          static_cast<float>(ez-z_)));
            auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
            for (int i=0;i<passes.getCountProperty();++i) {
                passes[i]->Apply();
                device.DrawPrimitives(PrimitiveType::TriangleList,0,entityTriangles_);
            }
        }
    }
    Game::Draw(time);
}

} // namespace Backrooms

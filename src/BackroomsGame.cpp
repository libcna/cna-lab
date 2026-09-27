#include "BackroomsGame.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
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

void Quad(Meshes& meshes, Material material, Vector3 a, Vector3 b, Vector3 c,
          Vector3 d, Color color, Vector2 uvA, Vector2 uvB, Vector2 uvC,
          Vector2 uvD) {
    auto& mesh=meshes[static_cast<int>(material)];
    mesh.emplace_back(a,color,uvA); mesh.emplace_back(b,color,uvB);
    mesh.emplace_back(c,color,uvC);
    mesh.emplace_back(a,color,uvA); mesh.emplace_back(c,color,uvC);
    mesh.emplace_back(d,color,uvD);
}

void Flat(Meshes& meshes, Material material, float x0, float z0, float x1,
          float z1, float y, Color color, float repeat) {
    Quad(meshes,material,{x0,y,z0},{x1,y,z0},{x1,y,z1},{x0,y,z1},color,
         {x0*repeat,z0*repeat},{x1*repeat,z0*repeat},
         {x1*repeat,z1*repeat},{x0*repeat,z1*repeat});
}

void WallFace(Meshes& meshes, Material material, bool vertical, float boundary,
              float a, float b, float y0, float y1, Color color) {
    const float u0=a*0.6f,u1=b*0.6f;
    const float v0=1.0f-y0/3.0f,v1=1.0f-y1/3.0f;
    if (vertical)
        Quad(meshes,material,{boundary,y0,a},{boundary,y0,b},
             {boundary,y1,b},{boundary,y1,a},color,
             {u0,v0},{u1,v0},{u1,v1},{u0,v1});
    else
        Quad(meshes,material,{a,y0,boundary},{b,y0,boundary},
             {b,y1,boundary},{a,y1,boundary},color,
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
        section(a,b,0.19f,height-0.32f);
        WallFace(meshes,material,vertical,boundary,a,b,
                 height-0.32f,height,trim);
    };
    if (edge == Edge::Solid) full(along, along+5.0f);
    else {
        const float side=edge==Edge::Wide ? 0.75f : 1.55f;
        full(along,along+side);
        full(along+5.0f-side,along+5.0f);
        if (edge==Edge::Door)
            WallFace(meshes,material,vertical,boundary,along+side,
                     along+5.0f-side,doorHeight,height,trim);
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

Color Vary(Color first, Color second, std::uint32_t hash) {
    return hash%5 == 0 ? second : first;
}

struct Portal {
    int level, cellX, cellZ, target;
    bool alongX;
};

constexpr std::array<Portal,4> kPortals{{
    {0,3,0,1,true}, {1,0,3,0,false},
    {1,3,0,2,true}, {2,0,3,1,false}
}};

std::optional<int> PortalTarget(int level, double x, double z) {
    for (const auto& portal:kPortals) {
        if (portal.level!=level) continue;
        const double px=(portal.cellX+0.5)*kCellSize;
        const double pz=(portal.cellZ+0.5)*kCellSize;
        if (std::hypot(x-px,z-pz)<0.95) return portal.target;
    }
    return std::nullopt;
}

void PortalVisual(Meshes& meshes, int level, bool alongX, float cx,
                  float cz, float ceiling) {
    const Material frameMat=level==0 ? Material::Wallpaper : Material::TunnelWall;
    const Color frame=level==0 ? Color(112,101,68) : Color(129,111,85);
    const Color dark(49,45,40);
    const float near=0.62f, far=1.2f, half=1.1f;
    if (alongX) {
        BoxRange(meshes,frameMat,cx+near,cz-half-0.18f,
                 cx+far,cz-half,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+near,cz+half,
                 cx+far,cz+half+0.18f,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+near,cz-half,
                 cx+far,cz+half,2.25f,ceiling,frame);
        WallFace(meshes,Material::TunnelWall,true,cx+far,cz-half,cz+half,
                 0,2.25f,dark);
        Flat(meshes,Material::Fluorescent,cx+0.36f,cz-0.45f,
             cx+0.62f,cz+0.45f,ceiling-0.06f,Color(184,169,125),1.0f);
    } else {
        BoxRange(meshes,frameMat,cx-half-0.18f,cz+near,
                 cx-half,cz+far,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx+half,cz+near,
                 cx+half+0.18f,cz+far,0,ceiling,frame);
        BoxRange(meshes,frameMat,cx-half,cz+near,
                 cx+half,cz+far,2.25f,ceiling,frame);
        WallFace(meshes,Material::TunnelWall,false,cz+far,cx-half,cx+half,
                 0,2.25f,dark);
        Flat(meshes,Material::Fluorescent,cx-0.45f,cz+0.36f,
             cx+0.45f,cz+0.62f,ceiling-0.06f,Color(184,169,125),1.0f);
    }
}
}

BackroomsGame::BackroomsGame(std::uint64_t seed) : graphics_(this) {
    world_.seed = seed;
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
    effect_->setFogStartProperty(LevelInfo(0).fogStart);
    effect_->setFogEndProperty(LevelInfo(0).fogEnd);
    setIsMouseVisibleProperty(false);
    Mouse::setIsRelativeMouseModeEXTProperty(true);
    captured_ = true;
    BuildChunk({0,0});
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
        hum_->setVolumeProperty(0.17f);
        hum_->setIsLoopedProperty(true);
        hum_->Play();
    } catch (const std::exception& error) {
        hum_.reset(); humSound_.reset(); step_.reset(); transition_.reset();
        std::cerr << "Audio unavailable: " << error.what() << '\n';
    }
}

void BackroomsGame::BuildChunk(ChunkCoord coord) {
    const auto start = Clock::now();
    Meshes meshes;
    const int ox=coord.x*kChunkCells, oz=coord.z*kChunkCells;
    const int level=world_.level;
    const float height=LevelInfo(level).ceilingHeight;
    const float doorHeight=level==0 ? 2.25f : level==1 ? 2.8f : 2.08f;
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
    const Color trim=level==0 ? Color(166,156,121) :
                     level==1 ? Color(110,130,128) : Color(99,83,67);
    const Color floorA=level==0 ? Color(246,240,222) :
                       level==1 ? Color(218,222,217) : Color(155,149,128);
    const Color floorB=level==0 ? Color(218,213,197) :
                       level==1 ? Color(193,201,195) : Color(125,120,103);
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
        Flat(meshes,floorMat,x,z,x+5,z+5,0,Vary(floorA,floorB,h),0.5f);
        Flat(meshes,ceilingMat,x,z,x+5,z+5,height,ceiling,0.8f);
        if (level==0) {
            Flat(meshes,ceilingMat,x,z,x+5,z+0.04f,height-0.013f,grid,0.8f);
            Flat(meshes,ceilingMat,x,z,x+0.04f,z+5,height-0.013f,grid,0.8f);
        } else if (level==1) {
            BoxRange(meshes,ceilingMat,x+0.22f,z,x+0.42f,z+5,
                     height-0.32f,height-0.10f,grid);
        } else {
            BoxRange(meshes,Material::TunnelWall,x+0.35f,z,x+0.50f,z+5,
                     height-0.33f,height-0.17f,grid);
            BoxRange(meshes,Material::TunnelWall,x+4.50f,z,x+4.65f,z+5,
                     height-0.33f,height-0.17f,grid);
        }
        const bool hasLamp=level==0 ? ((gx+gz)%2==0 && h%7!=0) :
                           level==1 ? (gx%3==0 && gz%2==0) :
                                      ((gx+gz)%3==0 && h%3!=0);
        if (hasLamp) {
            Flat(meshes,ceilingMat,x+1.30f,z+2.27f,x+3.70f,z+2.73f,
                 height-0.024f,grid,0.8f);
            Flat(meshes,Material::Fluorescent,x+1.40f,z+2.34f,
                 x+3.60f,z+2.66f,height-0.032f,lamp,1.0f);
        }
        const Color cellWall=Vary(wallA,wallB,h);
        Partition(meshes,wallMat,VerticalEdge(world_,gx,gz),true,x,z,
                  height,doorHeight,cellWall,trim);
        Partition(meshes,wallMat,HorizontalEdge(world_,gx,gz),false,z,x,
                  height,doorHeight,cellWall,trim);
        if (const auto obstacle=CellObstacle(world_,gx,gz)) {
            const float bx0=static_cast<float>(obstacle->minX-ox*kCellSize);
            const float bz0=static_cast<float>(obstacle->minZ-oz*kCellSize);
            const float bx1=static_cast<float>(obstacle->maxX-ox*kCellSize);
            const float bz1=static_cast<float>(obstacle->maxZ-oz*kCellSize);
            if (level==0)
                BoxRange(meshes,wallMat,bx0,bz0,bx1,bz1,0,height,wallB);
            else {
                BoxRange(meshes,Material::ConcreteWall,bx0,bz0,bx1,bz1,
                         0,2.5f,Color(141,150,148));
                for (float shelf: {0.58f,1.15f,1.72f})
                    Flat(meshes,Material::IndustrialCeiling,bx0,bz0,bx1,bz1,
                         shelf,Color(103,113,111),0.8f);
            }
        }
        for (const auto& portal:kPortals) {
            if (portal.level==level && portal.cellX==gx && portal.cellZ==gz)
                PortalVisual(meshes,level,portal.alongX,x+2.5f,z+2.5f,height);
        }
    }
    Chunk chunk;
    for (int id=0;id<kMaterialCount;++id) {
        auto& mesh=meshes[id];
        if (mesh.empty()) continue;
        chunk.materialTriangles[id]=static_cast<int>(mesh.size()/3);
        chunk.triangles+=chunk.materialTriangles[id];
        chunk.vertices[id]=std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
            VertexPositionColorTexture::getVertexDeclarationStatic(),
            static_cast<int>(mesh.size()),BufferUsage::WriteOnly);
        chunk.vertices[id]->SetData(mesh.data(),static_cast<int>(mesh.size()));
    }
    chunk.buildMs = std::chrono::duration<double,std::milli>(Clock::now()-start).count();
    lastBuildMs_ = chunk.buildMs;
    chunks_.emplace(coord,std::move(chunk));
}

void BackroomsGame::Stream() {
    const auto center=ChunkAt(x_,z_);
    for (auto it=chunks_.begin(); it!=chunks_.end();) {
        if (std::abs(it->first.x-center.x)>2 || std::abs(it->first.z-center.z)>2)
            it=chunks_.erase(it);
        else ++it;
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
}

void BackroomsGame::Transition(int level) {
    if (transition_) transition_->Play(0.35f,0,0);
    world_.level=level;
    chunks_.clear();
    x_=2.5; z_=2.5;
    yaw_=level==2 ? 0.0f : 1.5707963f;
    pitch_=0;
    BuildChunk({0,0});
}

void BackroomsGame::UpdateTitle(double elapsed) {
    statsTime_+=elapsed;
    ++frameCount_;
    if (statsTime_<1.0) return;
    int triangles=0;
    for (const auto& [coord,chunk]:chunks_) { (void)coord; triangles+=chunk.triangles; }
    const auto here=ChunkAt(x_,z_);
    std::ostringstream title;
    title << "cna-backrooms | Level " << world_.level << " | seed " << world_.seed
          << " | pos " << std::fixed << std::setprecision(1) << x_ << ',' << z_
          << " | chunk " << here.x << ',' << here.z
          << " | loaded " << chunks_.size() << "/25 | tris " << triangles
          << " | gen " << std::setprecision(2) << lastBuildMs_ << " ms"
          << " | " << static_cast<int>(frameCount_/statsTime_) << " FPS";
    getWindowProperty().setTitleProperty(title.str());
    statsTime_=0;
    frameCount_=0;
}

void BackroomsGame::Update(GameTime& time) {
    const double dt=std::min(0.1,time.getElapsedGameTimeProperty().getTotalSecondsProperty());
    const auto keys=Keyboard::GetState();
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
        const double speed=(keys.IsKeyDown(Keys::LeftShift)?6.5:3.8)*dt;
        const double dx=(std::sin(yaw_)*forward-std::cos(yaw_)*strafe)*speed;
        const double dz=(std::cos(yaw_)*forward+std::sin(yaw_)*strafe)*speed;
        const double oldX=x_, oldZ=z_;
        MoveWithCollision(world_,x_,z_,dx,dz,0.31);
        stepDistance_ += std::hypot(x_-oldX,z_-oldZ);
        if (stepDistance_>0.75) {
            stepDistance_=0;
            if (step_) step_->Play(0.12f,0,0);
        }
    }
    if (keys.IsKeyDown(Keys::R) && previousKeys_.IsKeyUp(Keys::R)) {
        x_=2.5; z_=2.5; yaw_=1.5707963f;
    }
    const auto portal=PortalTarget(world_.level,x_,z_);
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
    Game::Draw(time);
}

} // namespace Backrooms

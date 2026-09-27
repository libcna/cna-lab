#include "BackroomsGame.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPass.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectPassCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/EffectTechnique.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

namespace Backrooms {
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;

namespace {
using Clock = std::chrono::steady_clock;
using Mesh = std::vector<VertexPositionColor>;

void Quad(Mesh& mesh, Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color color) {
    mesh.emplace_back(a,color); mesh.emplace_back(b,color); mesh.emplace_back(c,color);
    mesh.emplace_back(a,color); mesh.emplace_back(c,color); mesh.emplace_back(d,color);
}

void Flat(Mesh& mesh, float x0, float z0, float x1, float z1, float y, Color color) {
    Quad(mesh, {x0,y,z0}, {x1,y,z0}, {x1,y,z1}, {x0,y,z1}, color);
}

void WallFace(Mesh& mesh, bool vertical, float boundary, float a, float b,
              float y0, float y1, Color color) {
    if (vertical)
        Quad(mesh, {boundary,y0,a}, {boundary,y0,b},
             {boundary,y1,b}, {boundary,y1,a}, color);
    else
        Quad(mesh, {a,y0,boundary}, {b,y0,boundary},
             {b,y1,boundary}, {a,y1,boundary}, color);
}

void Partition(Mesh& mesh, Edge edge, bool vertical, float boundary, float along,
               Color wall, Color trim) {
    if (edge == Edge::Open) return;
    const auto section = [&](float a, float b, float y0, float y1) {
        WallFace(mesh, vertical, boundary, a, b, y0, y1, wall);
    };
    const auto full = [&](float a, float b) {
        WallFace(mesh, vertical, boundary, a, b, 0.0f, 0.19f, trim);
        section(a, b, 0.19f, 2.68f);
        WallFace(mesh, vertical, boundary, a, b, 2.68f, 3.0f, trim);
    };
    if (edge == Edge::Solid) full(along, along+5.0f);
    else {
        full(along, along+1.55f);
        full(along+3.45f, along+5.0f);
        WallFace(mesh, vertical, boundary, along+1.55f, along+3.45f,
                 2.25f, 3.0f, trim);
    }
}

Color Vary(Color first, Color second, std::uint32_t hash) {
    return hash%5 == 0 ? second : first;
}

bool PortalAt(int level, double x, double z) {
    const double px = level == 0 ? 17.5 : 2.5;
    const double pz = level == 0 ? 2.5 : 17.5;
    return std::hypot(x-px,z-pz) < 1.05;
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
    effect_->VertexColorEnabled = true;
    effect_->setLightingEnabledProperty(false);
    effect_->setFogEnabledProperty(true);
    effect_->setFogStartProperty(25.0f);
    effect_->setFogEndProperty(93.0f);
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
    Mesh mesh;
    mesh.reserve(16000);
    const int ox=coord.x*kChunkCells, oz=coord.z*kChunkCells;
    const bool yellow = world_.level == 0;
    const Color wallA = yellow ? Color(192,179,113) : Color(95,108,112);
    const Color wallB = yellow ? Color(182,169,103) : Color(86,100,105);
    const Color trim = yellow ? Color(128,119,76) : Color(57,69,73);
    const Color floorA = yellow ? Color(86,79,59) : Color(67,72,70);
    const Color floorB = yellow ? Color(80,74,54) : Color(62,68,67);
    const Color ceiling = yellow ? Color(184,179,149) : Color(116,123,120);
    const Color grid = yellow ? Color(133,131,109) : Color(75,84,84);
    const Color lamp = yellow ? Color(246,239,185) : Color(201,220,220);

    for (int lx=0; lx<kChunkCells; ++lx) for (int lz=0; lz<kChunkCells; ++lz) {
        const int gx=ox+lx, gz=oz+lz;
        const float x=lx*5.0f, z=lz*5.0f;
        const auto h=CellHash(world_,gx,gz,41);
        Flat(mesh,x,z,x+5,z+5,0,Vary(floorA,floorB,h));
        Flat(mesh,x,z,x+5,z+5,3,ceiling);
        // Thin ceiling strips make the overhead grid readable without textures.
        Flat(mesh,x,z,x+5,z+0.035f,2.989f,grid);
        Flat(mesh,x,z,x+0.035f,z+5,2.989f,grid);
        if ((gx+gz)%2 == 0 && h%7 != 0) {
            Flat(mesh,x+1.42f,z+2.35f,x+3.58f,z+2.65f,2.977f,lamp);
            Flat(mesh,x+1.37f,z+2.30f,x+3.63f,z+2.70f,2.985f,grid);
            Flat(mesh,x+1.42f,z+2.35f,x+3.58f,z+2.65f,2.974f,lamp);
        }
        const Color cellWall=Vary(wallA,wallB,h);
        Partition(mesh,VerticalEdge(world_,gx,gz),true,x,z,cellWall,trim);
        Partition(mesh,HorizontalEdge(world_,gx,gz),false,z,x,cellWall,trim);
        // A cyan-lit floor patch is the physical level transition.
        if ((world_.level==0 && gx==3 && gz==0) ||
            (world_.level==1 && gx==0 && gz==3)) {
            Flat(mesh,x+1.2f,z+1.2f,x+3.8f,z+3.8f,0.015f,Color(27,118,128));
            Flat(mesh,x+1.5f,z+1.5f,x+3.5f,z+3.5f,0.017f,Color(65,191,194));
            Flat(mesh,x+1.2f,z+1.2f,x+3.8f,z+3.8f,2.972f,Color(88,198,202));
        }
    }
    Chunk chunk;
    chunk.triangles = static_cast<int>(mesh.size()/3);
    chunk.vertices = std::make_unique<VertexBuffer>(getGraphicsDeviceProperty(),
        VertexPositionColor::getVertexDeclarationStatic(),
        static_cast<int>(mesh.size()),BufferUsage::WriteOnly);
    chunk.vertices->SetData(mesh.data(),static_cast<int>(mesh.size()));
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
    yaw_=level==0 ? 1.5707963f : 0.0f;
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
    const bool portal=PortalAt(world_.level,x_,z_);
    if (portal && !insidePortal_) Transition(1-world_.level);
    insidePortal_=portal;
    Stream();
    UpdateTitle(dt);
    previousKeys_=keys;
    Game::Update(time);
}

void BackroomsGame::Draw(const GameTime& time) {
    auto& device=getGraphicsDeviceProperty();
    const Color fog=world_.level==0 ? Color(104,99,74) : Color(55,67,70);
    device.Clear(fog);
    device.setDepthStencilStateProperty(DepthStencilState::Default);
    device.setRasterizerStateProperty(RasterizerState::CullNone);
    effect_->setFogColorProperty(fog.ToVector3());
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
        device.SetVertexBuffer(chunk.vertices.get());
        auto& passes=effect_->getCurrentTechniqueProperty()->getPassesProperty();
        for (int i=0; i<passes.getCountProperty(); ++i) {
            passes[i]->Apply();
            device.DrawPrimitives(PrimitiveType::TriangleList,0,chunk.triangles);
        }
    }
    Game::Draw(time);
}

} // namespace Backrooms

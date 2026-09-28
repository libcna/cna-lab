#pragma once

#include "World.hpp"
#include "Materials.hpp"

#include <array>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"

namespace Backrooms {

class BackroomsGame final : public Microsoft::Xna::Framework::Game {
public:
    static constexpr double kDefaultWalkSpeed=2.4;
    static constexpr double kDefaultRunSpeed=4.8;
    static constexpr float kDefaultVerticalFov=60.0f;

    explicit BackroomsGame(std::uint64_t seed, bool streamTest = false,
                           int startLevel = 0, double startX = 2.5,
                           double startZ = 2.5, double walkSpeed = kDefaultWalkSpeed,
                           double runSpeed = kDefaultRunSpeed,
                           double streamTestMetres = 9600.0,
                           float verticalFovDegrees = kDefaultVerticalFov,
                           int multiSampleCount = 4);
    ~BackroomsGame() override;
    const std::string& GetTypeName() const override;
    void Initialize() override;
    void LoadContent() override;
    void Update(Microsoft::Xna::Framework::GameTime& time) override;
    void Draw(const Microsoft::Xna::Framework::GameTime& time) override;

private:
    struct Chunk {
        std::array<std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer>,
                   kMaterialCount> vertices;
        std::array<int, kMaterialCount> materialTriangles{};
        std::vector<EntitySpawn> entities;
        Microsoft::Xna::Framework::BoundingBox bounds;
        int triangles = 0;
        double buildMs = 0;
    };

    Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::BasicEffect> effect_;
    std::unique_ptr<Materials> materials_;
    struct EntityMesh {
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> vertices;
        int triangles=0;
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> shadowVertices;
        int shadowTriangles=0;
    };
    std::array<EntityMesh,kEntityKindCount> entityMeshes_;
    Microsoft::Xna::Framework::Graphics::BlendState entityDepthOnlyBlend_;
    double nearestEntityDistance_ = 0;
    float nearestEntityOpacity_ = 0;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> humSound_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffectInstance> hum_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> step_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> carpetStep_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> transition_;
    std::array<std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect>,
               kEntityKindCount> entitySounds_;
    double entitySoundDelay_=5;
    unsigned entitySoundCount_=0;
    bool entitySoundWarningShown_=false;
    std::map<ChunkCoord, Chunk> chunks_;
    std::vector<std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer>> spareVertices_;
    LevelCatalog levelCatalog_;
    RoomLayoutCache roomLayouts_;
    WorldConfig world_;
    Microsoft::Xna::Framework::Input::KeyboardState previousKeys_;
    double x_ = 2.5, z_ = 2.5;
    float yaw_ = 1.5707963f, pitch_ = 0;
    float verticalFovDegrees_ = kDefaultVerticalFov;
    bool captured_ = false;
    bool running_ = false;
    double walkSpeed_ = kDefaultWalkSpeed;
    double runSpeed_ = kDefaultRunSpeed;
    bool insidePortal_ = false;
    int updateCount_ = 0;
    std::array<double,120> drawIntervals_{};
    std::size_t drawIntervalCount_ = 0, nextDrawInterval_ = 0;
    std::chrono::steady_clock::time_point lastDraw_{};
    double statsTime_ = 0;
    int drawnChunks_ = 0, drawnTriangles_ = 0;
    double drawWorkMs_ = 0;
    double lastBuildMs_ = 0;
    double peakBuildMs_ = 0;
    double stepDistance_ = 0;
    unsigned stepCount_ = 0;
    bool stepWarningShown_ = false;
    bool streamTest_ = false;
    double streamTestMetres_ = 9600.0;
    double streamTestTime_ = 0;
    int bufferCreations_ = 0;
    int bufferReuses_ = 0;

    void BuildChunk(ChunkCoord coord);
    void BuildEntityMesh();
    void UpdateEntityAudio(float seconds,double elapsed);
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer>
        AcquireBuffer(int vertexCount);
    void RetireChunk(Chunk& chunk);
    void Stream();
    void Transition(int level);
    void UpdateTitle(double elapsed);
    void RecordDrawInterval();
};

} // namespace Backrooms

#pragma once

#include "World.hpp"

#include <map>
#include <memory>
#include <vector>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"

namespace Backrooms {

class BackroomsGame final : public Microsoft::Xna::Framework::Game {
public:
    explicit BackroomsGame(std::uint64_t seed);
    const std::string& GetTypeName() const override;
    void Initialize() override;
    void LoadContent() override;
    void Update(Microsoft::Xna::Framework::GameTime& time) override;
    void Draw(const Microsoft::Xna::Framework::GameTime& time) override;

private:
    struct Chunk {
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> vertices;
        int triangles = 0;
        double buildMs = 0;
    };

    Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::BasicEffect> effect_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> humSound_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffectInstance> hum_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> step_;
    std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> transition_;
    std::map<ChunkCoord, Chunk> chunks_;
    WorldConfig world_;
    Microsoft::Xna::Framework::Input::KeyboardState previousKeys_;
    double x_ = 2.5, z_ = 2.5;
    float yaw_ = 1.5707963f, pitch_ = 0;
    bool captured_ = false;
    bool insidePortal_ = false;
    int frameCount_ = 0;
    double statsTime_ = 0;
    double lastBuildMs_ = 0;
    double stepDistance_ = 0;

    void BuildChunk(ChunkCoord coord);
    void Stream();
    void Transition(int level);
    void UpdateTitle(double elapsed);
};

} // namespace Backrooms

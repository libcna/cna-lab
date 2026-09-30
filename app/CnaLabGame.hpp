#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/Model.hpp"
#include "Microsoft/Xna/Framework/Graphics/PbrEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Media/Video/Video.hpp"
#include "Microsoft/Xna/Framework/Media/Video/VideoPlayer.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"

class CnaLabGame final : public Microsoft::Xna::Framework::Game
{
public:
    explicit CnaLabGame(bool smokeMedia);
    ~CnaLabGame() override;

    [[nodiscard]] bool SmokeSucceeded() const;

    GetTypeNameHPP()

protected:
    void LoadContent() override;
    void Update(Microsoft::Xna::Framework::GameTime& gameTime) override;
    void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) override;

private:
    struct AssetStatus
    {
        std::string label;
        bool loaded = false;
        std::string detail;
    };

    struct PbrGpuVertex
    {
        float px, py, pz;
        float nx, ny, nz;
        float tx, ty, tz, tw;
        float u, v;
    };
    static_assert(sizeof(PbrGpuVertex) == 48, "PBR vertex layout must remain 48 bytes.");

    void RecordLoad(const std::string& label, const std::function<void()>& loader);
    void UpdateWindowTitle();
    void DrawSpriteField(float seconds);
    void DrawClassicCube(float seconds);
    void DrawModernPbrPanel(float seconds);
    void DrawSelectedModel(float seconds);
    void DrawModel(Microsoft::Xna::Framework::Graphics::Model& model,
                   const Microsoft::Xna::Framework::Matrix& world,
                   const Microsoft::Xna::Framework::Matrix& view,
                   const Microsoft::Xna::Framework::Matrix& projection);
    void ToggleVideoPlayback();
    void PlaySound();
    [[nodiscard]] bool WasPressed(Microsoft::Xna::Framework::Input::Keys key) const;
    [[nodiscard]] bool AllAssetsLoaded() const;

    Microsoft::Xna::Framework::GraphicsDeviceManager graphics_;
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> spriteBatch_;
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::BasicEffect> classicEffect_;
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::PbrEffect> pbrEffect_;
    std::unique_ptr<Microsoft::Xna::Framework::Graphics::VertexBuffer> pbrVertices_;
    Microsoft::Xna::Framework::Graphics::Texture2D solid_;
    Microsoft::Xna::Framework::Graphics::Texture2D gradient_;
    std::optional<Microsoft::Xna::Framework::Graphics::Texture2D> xnbTexture_;
    std::array<std::optional<Microsoft::Xna::Framework::Graphics::Model>, 3> models_;
    std::optional<Microsoft::Xna::Framework::Audio::SoundEffect> sound_;
    std::optional<Microsoft::Xna::Framework::Media::Video> video_;
    Microsoft::Xna::Framework::Media::VideoPlayer videoPlayer_;
    Microsoft::Xna::Framework::Input::KeyboardState currentKeyboard_;
    Microsoft::Xna::Framework::Input::KeyboardState previousKeyboard_;
    std::vector<AssetStatus> assetStatuses_;
    std::string rendererName_;
    std::string mediaStatus_;
    bool supportsThreeD_ = false;
    bool smokeMedia_ = false;
    bool soundStarted_ = false;
    bool videoFrameDecoded_ = false;
    unsigned int drawnFrames_ = 0;
    std::size_t selectedModel_ = 0;
};

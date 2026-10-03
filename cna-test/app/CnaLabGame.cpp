#include "CnaLabGame.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "CNA/GraphicsCapability.hpp"
#include "CNA/Internal/Xnb/XnbBuiltInReaders.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/IEffectMatrices.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelBone.hpp"
#include "Microsoft/Xna/Framework/Graphics/ModelMesh.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Media/MediaState.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Audio;
using namespace Microsoft::Xna::Framework::Content;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Input;
using namespace Microsoft::Xna::Framework::Media;

namespace
{
    [[nodiscard]] std::array<VertexPositionTexture, 36> CreateCubeVertices()
    {
        std::array<VertexPositionTexture, 36> vertices{};
        std::size_t next = 0;
        const auto addFace = [&vertices, &next](const Vector3& topLeft,
                                                const Vector3& topRight,
                                                const Vector3& bottomRight,
                                                const Vector3& bottomLeft)
        {
            vertices[next++] = {topLeft, Vector2(0.0f, 0.0f)};
            vertices[next++] = {topRight, Vector2(1.0f, 0.0f)};
            vertices[next++] = {bottomRight, Vector2(1.0f, 1.0f)};
            vertices[next++] = {topLeft, Vector2(0.0f, 0.0f)};
            vertices[next++] = {bottomRight, Vector2(1.0f, 1.0f)};
            vertices[next++] = {bottomLeft, Vector2(0.0f, 1.0f)};
        };

        addFace({-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f},
                {1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f});
        addFace({1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
                {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f});
        addFace({1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f});
        addFace({-1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, 1.0f},
                {-1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f});
        addFace({-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},
                {1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f});
        addFace({-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},
                {1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f});
        return vertices;
    }

    [[nodiscard]] const char* VideoStateName(const MediaState state)
    {
        switch (state)
        {
            case MediaState::Stopped: return "stopped";
            case MediaState::Playing: return "playing";
            case MediaState::Paused: return "paused";
        }
        return "unknown";
    }

    constexpr std::array<const char*, 3> kModelNames = {
        "direct glTF", "generated CNJ", "compiled CNB",
    };
}

CnaLabGame::CnaLabGame(const bool smokeMedia)
    : graphics_(this), smokeMedia_(smokeMedia)
{
    getContentProperty().setRootDirectoryProperty("Content");
    getWindowProperty().setTitleProperty("CNA Lab - preparing content pipelines");
}

CnaLabGame::~CnaLabGame() = default;

bool CnaLabGame::SmokeSucceeded() const
{
    return AllAssetsLoaded() && soundStarted_ && videoFrameDecoded_;
}

void CnaLabGame::RecordLoad(const std::string& label, const std::function<void()>& loader)
{
    AssetStatus status{label, false, {}};
    try
    {
        loader();
        status.loaded = true;
        status.detail = "loaded";
    }
    catch (const std::exception& exception)
    {
        status.detail = exception.what();
        std::cerr << "cna-lab: " << label << " failed: " << status.detail << '\n';
    }
    assetStatuses_.push_back(std::move(status));
}

void CnaLabGame::LoadContent()
{
    auto& device = getGraphicsDeviceProperty();
    rendererName_ = device.GetGraphicsRendererName();
    supportsThreeD_ = device.SupportsCapability(CNA::GraphicsCapability::ThreeD);

    spriteBatch_ = std::make_unique<SpriteBatch>(device);
    solid_ = Texture2D(device, 1, 1);
    const Color white = Color::White;
    solid_.SetData(&white, 1);

    constexpr int textureSize = 96;
    std::vector<Color> pixels(static_cast<std::size_t>(textureSize * textureSize));
    for (int y = 0; y < textureSize; ++y)
    {
        for (int x = 0; x < textureSize; ++x)
        {
            const float u = static_cast<float>(x) / static_cast<float>(textureSize - 1);
            const float v = static_cast<float>(y) / static_cast<float>(textureSize - 1);
            pixels[static_cast<std::size_t>(y * textureSize + x)] = Color(
                static_cast<int>(55.0f + 190.0f * u),
                static_cast<int>(70.0f + 130.0f * (1.0f - v)),
                static_cast<int>(235.0f - 155.0f * u), 255);
        }
    }
    gradient_ = Texture2D(device, textureSize, textureSize);
    gradient_.SetData(pixels.data(), static_cast<int>(pixels.size()));

    if (supportsThreeD_)
    {
        classicEffect_ = std::make_unique<BasicEffect>(device);
        classicEffect_->setTextureEnabledProperty(true);
        classicEffect_->setTextureProperty(&gradient_);
        classicEffect_->setLightingEnabledProperty(false);

        // PbrEffect is a CNAEXT effect.  The 48-byte POD layout is intentional:
        // the public tangent vertex type is polymorphic and must not be uploaded
        // with sizeof() as if it were a packed GPU stream.
        constexpr std::array<PbrGpuVertex, 6> panel = {{
            {-0.90f, 0.90f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
            {-0.90f, -0.90f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
            {0.90f, -0.90f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
            {-0.90f, 0.90f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
            {0.90f, -0.90f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
            {0.90f, 0.90f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        }};
        pbrVertices_ = std::make_unique<VertexBuffer>(device, static_cast<int>(panel.size()));
        pbrVertices_->SetDataRaw(panel.data(), static_cast<int>(panel.size()),
                                 static_cast<int>(sizeof(PbrGpuVertex)));
        pbrEffect_ = std::make_unique<PbrEffect>(device);
        pbrEffect_->setTextureProperty(&gradient_);
        pbrEffect_->setDiffuseColorProperty(Vector3(0.75f, 0.95f, 1.0f));
        pbrEffect_->setMetallicFactorProperty(0.82f);
        pbrEffect_->setRoughnessFactorProperty(0.24f);
        pbrEffect_->EnableDefaultLighting();
    }

    // XNB support is deliberately opt-in in CNA; one explicit registration is
    // the required application-level operation before loading legacy content.
    CNA::Internal::Xnb::RegisterAllBuiltInXnbReaders();
    RecordLoad("XNB Texture2D", [&]
    {
        xnbTexture_.emplace(getContentProperty().Load<Texture2D>("Legacy/white-1"));
    });
    RecordLoad("direct glTF Model", [&]
    {
        models_[0].emplace(getContentProperty().Load<Model>("Models/triangle.glb"));
    });
    RecordLoad("generated CNJ Model", [&]
    {
        models_[1].emplace(getContentProperty().Load<Model>("Pipeline/triangle.cnj"));
    });
    RecordLoad("compiled CNB Model", [&]
    {
        // The extension is spelled intentionally: it proves the compiled asset
        // path rather than allowing resolver precedence to choose another form.
        models_[2].emplace(getContentProperty().Load<Model>("Pipeline/triangle.cnb"));
    });
    RecordLoad("compiled CNB SoundEffect", [&]
    {
        sound_.emplace(getContentProperty().Load<SoundEffect>("Media/lab-beep.cnb"));
    });
    RecordLoad("compiled CNB Video", [&]
    {
        video_.emplace(getContentProperty().Load<Video>("Media/lab-video.cnb"));
    });

    if (smokeMedia_)
    {
        PlaySound();
        ToggleVideoPlayback();
    }
    UpdateWindowTitle();

    std::cout << "cna-lab: renderer=" << rendererName_
              << ", 3D=" << (supportsThreeD_ ? "yes" : "no") << '\n';
    for (const AssetStatus& status : assetStatuses_)
    {
        std::cout << "  " << status.label << ": " << status.detail << '\n';
    }
}

void CnaLabGame::Update(Microsoft::Xna::Framework::GameTime& gameTime)
{
    previousKeyboard_ = currentKeyboard_;
    currentKeyboard_ = Keyboard::GetState();

    if (WasPressed(Keys::Escape))
    {
        Exit();
        return;
    }
    if (WasPressed(Keys::Space))
    {
        selectedModel_ = (selectedModel_ + 1) % models_.size();
        UpdateWindowTitle();
    }
    if (WasPressed(Keys::S))
    {
        PlaySound();
    }
    if (WasPressed(Keys::V))
    {
        ToggleVideoPlayback();
    }
    Game::Update(gameTime);
}

void CnaLabGame::Draw(const Microsoft::Xna::Framework::GameTime& gameTime)
{
    auto& device = getGraphicsDeviceProperty();
    device.Clear(Color(9, 15, 31, 255), 1.0f);
    const float seconds = static_cast<float>(gameTime.getTotalGameTimeProperty().getTotalSecondsProperty());

    if (supportsThreeD_)
    {
        DrawClassicCube(seconds);
        DrawModernPbrPanel(seconds);
        DrawSelectedModel(seconds);
    }
    DrawSpriteField(seconds);

    ++drawnFrames_;
    if (smokeMedia_ && drawnFrames_ >= 8)
    {
        Exit();
    }
    Game::Draw(gameTime);
}

void CnaLabGame::DrawSpriteField(const float seconds)
{
    if (!spriteBatch_)
    {
        return;
    }

    Texture2D* frame = nullptr;
    if (video_.has_value() && videoPlayer_.getStateProperty() != MediaState::Stopped)
    {
        try
        {
            frame = videoPlayer_.GetTexture();
            videoFrameDecoded_ = videoFrameDecoded_ || frame != nullptr;
        }
        catch (const std::exception& exception)
        {
            mediaStatus_ = std::string("video frame error: ") + exception.what();
            UpdateWindowTitle();
        }
    }

    spriteBatch_->Begin();
    spriteBatch_->Draw(gradient_, Rectangle(18, 18, 92, 92), Color::White);
    if (xnbTexture_.has_value())
    {
        spriteBatch_->Draw(*xnbTexture_, Rectangle(118, 18, 32, 32), Color(120, 220, 255, 255));
    }

    for (int index = 0; index < 36; ++index)
    {
        const float phase = seconds * 1.8f + static_cast<float>(index) * 0.52f;
        const int x = 20 + (index % 9) * 18;
        const int y = 126 + (index / 9) * 18 + static_cast<int>(std::sin(phase) * 9.0f);
        const int alpha = 55 + (index % 4) * 45;
        spriteBatch_->Draw(solid_, Rectangle(x, y, 10, 10),
                           Color(60 + (index * 29) % 180, 130, 255, alpha));
    }

    if (frame != nullptr)
    {
        spriteBatch_->Draw(*frame, Rectangle(18, 210, 200, 150), Color::White);
    }
    spriteBatch_->End();
}

void CnaLabGame::DrawClassicCube(const float seconds)
{
    if (!classicEffect_)
    {
        return;
    }

    static const std::array<VertexPositionTexture, 36> vertices = CreateCubeVertices();
    auto& device = getGraphicsDeviceProperty();
    const auto& viewport = device.getViewportProperty();
    const float aspect = viewport.getAspectRatioProperty();
    const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 8.0f), Vector3::Zero, Vector3::Up);
    const Matrix projection = Matrix::CreatePerspectiveFieldOfView(0.80f, aspect, 0.1f, 100.0f);

    classicEffect_->setWorldProperty(Matrix::CreateScale(0.70f) *
                                      Matrix::CreateRotationX(seconds * 0.55f) *
                                      Matrix::CreateRotationY(seconds * 0.82f) *
                                      Matrix::CreateTranslation(2.35f, 0.0f, 0.0f));
    classicEffect_->setViewProperty(view);
    classicEffect_->setProjectionProperty(projection);
    device.setBlendStateProperty(BlendState::Opaque);
    device.setDepthStencilStateProperty(DepthStencilState::Default);
    device.setRasterizerStateProperty(RasterizerState::CullNone);
    for (EffectPass& pass : classicEffect_->getCurrentTechniqueProperty()->getPassesProperty())
    {
        pass.Apply();
        device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices.data(), 0,
                                  static_cast<int>(vertices.size()) / 3);
    }
}

void CnaLabGame::DrawModernPbrPanel(const float seconds)
{
    if (!pbrEffect_ || !pbrVertices_)
    {
        return;
    }

    auto& device = getGraphicsDeviceProperty();
    const auto& viewport = device.getViewportProperty();
    const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 8.0f), Vector3::Zero, Vector3::Up);
    const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
        0.80f, viewport.getAspectRatioProperty(), 0.1f, 100.0f);

    pbrEffect_->setWorldProperty(Matrix::CreateRotationY(-seconds * 0.46f) *
                                  Matrix::CreateTranslation(-2.35f, 0.0f, 0.0f));
    pbrEffect_->setViewProperty(view);
    pbrEffect_->setProjectionProperty(projection);
    pbrEffect_->setMetallicFactorProperty(0.50f + 0.45f * std::sin(seconds * 0.70f));
    pbrEffect_->setRoughnessFactorProperty(0.12f + 0.50f * (0.5f + 0.5f * std::sin(seconds * 0.41f)));
    device.SetVertexBuffer(pbrVertices_.get());
    pbrEffect_->Apply();
    device.DrawPrimitives(PrimitiveType::TriangleList, 0, 2);
    device.SetVertexBuffer(nullptr);
}

void CnaLabGame::DrawSelectedModel(const float seconds)
{
    if (!models_[selectedModel_].has_value())
    {
        return;
    }

    auto& device = getGraphicsDeviceProperty();
    const auto& viewport = device.getViewportProperty();
    const Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 8.0f), Vector3::Zero, Vector3::Up);
    const Matrix projection = Matrix::CreatePerspectiveFieldOfView(
        0.80f, viewport.getAspectRatioProperty(), 0.1f, 100.0f);
    device.setDepthStencilStateProperty(DepthStencilState::Default);
    device.setBlendStateProperty(BlendState::Opaque);
    device.setRasterizerStateProperty(RasterizerState::CullNone);

    const Matrix world = Matrix::CreateScale(1.6f) * Matrix::CreateRotationY(seconds * 0.9f);
    DrawModel(*models_[selectedModel_], world, view, projection);
}

void CnaLabGame::DrawModel(Model& model, const Matrix& world,
                           const Matrix& view, const Matrix& projection)
{
    std::vector<Matrix> absoluteBones(
        static_cast<std::size_t>(model.getBonesProperty().getCountProperty()));
    if (!absoluteBones.empty())
    {
        model.CopyAbsoluteBoneTransformsTo(absoluteBones);
    }

    for (ModelMesh* mesh : model.getMeshesProperty())
    {
        if (mesh == nullptr)
        {
            continue;
        }
        Matrix meshWorld = world;
        if (mesh->getParentBoneProperty() != nullptr && !absoluteBones.empty())
        {
            const int index = mesh->getParentBoneProperty()->getIndexProperty();
            meshWorld = absoluteBones.at(static_cast<std::size_t>(index)) * world;
        }
        for (Effect* effect : mesh->getEffectsProperty())
        {
            if (auto* matrices = dynamic_cast<IEffectMatrices*>(effect))
            {
                matrices->setWorldProperty(meshWorld);
                matrices->setViewProperty(view);
                matrices->setProjectionProperty(projection);
            }
            // The compact fixture deliberately declares no glTF lights.  Give
            // all PBR-loaded representations the same visible baseline while
            // keeping the source material's texture/metallic/roughness data.
            if (auto* pbr = dynamic_cast<PbrEffect*>(effect))
            {
                pbr->EnableDefaultLighting();
            }
        }
        mesh->Draw();
    }
}

void CnaLabGame::ToggleVideoPlayback()
{
    if (!video_.has_value())
    {
        mediaStatus_ = "video asset unavailable";
        UpdateWindowTitle();
        return;
    }
    try
    {
        switch (videoPlayer_.getStateProperty())
        {
            case MediaState::Stopped: videoPlayer_.Play(&*video_); break;
            case MediaState::Playing: videoPlayer_.Pause(); break;
            case MediaState::Paused: videoPlayer_.Resume(); break;
        }
        mediaStatus_ = std::string("video ") + VideoStateName(videoPlayer_.getStateProperty());
    }
    catch (const std::exception& exception)
    {
        mediaStatus_ = std::string("video error: ") + exception.what();
        std::cerr << "cna-lab: " << mediaStatus_ << '\n';
    }
    UpdateWindowTitle();
}

void CnaLabGame::PlaySound()
{
    if (!sound_.has_value())
    {
        mediaStatus_ = "sound asset unavailable";
        UpdateWindowTitle();
        return;
    }
    soundStarted_ = sound_->Play();
    mediaStatus_ = soundStarted_ ? "sound started" : "sound instance limit reached";
    UpdateWindowTitle();
}

bool CnaLabGame::WasPressed(const Keys key) const
{
    return currentKeyboard_.IsKeyDown(key) && previousKeyboard_.IsKeyUp(key);
}

bool CnaLabGame::AllAssetsLoaded() const
{
    return !assetStatuses_.empty() && std::all_of(assetStatuses_.begin(), assetStatuses_.end(),
        [](const AssetStatus& status) { return status.loaded; });
}

void CnaLabGame::UpdateWindowTitle()
{
    const char* source = kModelNames[selectedModel_];
    const std::string videoState = video_.has_value()
        ? VideoStateName(videoPlayer_.getStateProperty()) : "unavailable";
    getWindowProperty().setTitleProperty(
        "CNA Lab | " + rendererName_ + " | " + source +
        " | S sound | V video=" + videoState + " | " + mediaStatus_);
}

GetTypeNameCPP(CnaLabGame, "CnaLabGame")

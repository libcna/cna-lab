// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/RenderContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/RenderContext.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"

namespace Myra::Graphics2D
{
    namespace
    {
        [[nodiscard]] int CheckedAdd(const int left, const int right)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) + right;
            if (result < std::numeric_limits<int>::min()
                || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error("RenderContext scissor translation overflows int.");
            }
            return static_cast<int>(result);
        }
    }

    RenderContext::RenderContext()
        : RenderContext(MyraEnvironment::getGraphicsDeviceProperty())
    {
    }

    RenderContext::RenderContext(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device)
        : RenderContext(
            std::make_unique<Microsoft::Xna::Framework::Graphics::SpriteBatch>(device),
            &device)
    {
    }

    RenderContext::RenderContext(
        std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> renderer,
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* const device)
        : renderer_(std::move(renderer)),
          device_(device),
          samplerState_(Microsoft::Xna::Framework::Graphics::SamplerState::PointClamp)
    {
        if (renderer_ == nullptr)
        {
            throw std::invalid_argument("RenderContext requires a non-null SpriteBatch.");
        }
        if (device_ == nullptr)
        {
            device_ = renderer_->getGraphicsDeviceProperty();
        }
        if (device_ != nullptr && device_->getIsDisposedProperty())
        {
            throw std::invalid_argument("RenderContext requires a live GraphicsDevice.");
        }
        uiRasterizerState_.setScissorTestEnableProperty(true);
    }

    RenderContext::~RenderContext()
    {
        Dispose();
    }

    Microsoft::Xna::Framework::Rectangle RenderContext::getDeviceScissorProperty() const
    {
        return RequireDevice().getScissorRectangleProperty();
    }

    void RenderContext::setDeviceScissorProperty(
        const Microsoft::Xna::Framework::Rectangle value)
    {
        RequireDevice().setScissorRectangleProperty(value);
    }

    Microsoft::Xna::Framework::Rectangle RenderContext::getScissorProperty() const noexcept
    {
        return scissor_;
    }

    void RenderContext::setScissorProperty(Microsoft::Xna::Framework::Rectangle value)
    {
        scissor_ = value;
        if (MyraEnvironment::getDisableClippingProperty())
        {
            return;
        }

        Flush();
        const auto& viewport = RequireDevice().getViewportProperty();
        value.X = CheckedAdd(value.X, viewport.getXProperty());
        value.Y = CheckedAdd(value.Y, viewport.getYProperty());
        setDeviceScissorProperty(value);
    }

    float RenderContext::getOpacityProperty() const noexcept
    {
        return opacity_;
    }

    void RenderContext::setOpacityProperty(const float value) noexcept
    {
        opacity_ = value;
    }

    void RenderContext::AddOpacity(const float opacity) noexcept
    {
        opacity_ *= opacity;
    }

    const Transform& RenderContext::getTransformProperty() const noexcept
    {
        return transform_;
    }

    void RenderContext::setTransformProperty(const Transform& value)
    {
        transform_ = value;
    }

    void RenderContext::SetAnisotropicFilteringMode(const bool enabled)
    {
        anisotropicFiltering_ = enabled;
    }

    TextureFiltering RenderContext::getTextureFilteringProperty() const noexcept
    {
        return textureFiltering_;
    }

    bool RenderContext::getIsRenderingProperty() const noexcept
    {
        return beginCalled_;
    }

    bool RenderContext::getIsDisposedProperty() const noexcept
    {
        return disposed_;
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Rectangle destination,
        const std::optional<Microsoft::Xna::Framework::Rectangle> source,
        const Microsoft::Xna::Framework::Color color,
        const float rotation,
        const float depth)
    {
        const int sourceWidth = source ? source->Width : texture.getWidthProperty();
        const int sourceHeight = source ? source->Height : texture.getHeightProperty();
        if (sourceWidth <= 0 || sourceHeight <= 0)
        {
            throw std::invalid_argument("RenderContext source rectangles must have positive dimensions.");
        }

        Draw(
            texture,
            {static_cast<float>(destination.X), static_cast<float>(destination.Y)},
            source,
            color,
            rotation,
            {static_cast<float>(destination.Width) / static_cast<float>(sourceWidth),
                static_cast<float>(destination.Height) / static_cast<float>(sourceHeight)},
            depth);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Rectangle destination,
        const std::optional<Microsoft::Xna::Framework::Rectangle> source,
        const Microsoft::Xna::Framework::Color color,
        const float rotation)
    {
        Draw(texture, destination, source, color, rotation, 0.0F);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Rectangle destination,
        const std::optional<Microsoft::Xna::Framework::Rectangle> source,
        const Microsoft::Xna::Framework::Color color)
    {
        Draw(texture, destination, source, color, 0.0F, 0.0F);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Rectangle destination,
        const Microsoft::Xna::Framework::Color color)
    {
        Draw(texture, destination, std::nullopt, color, 0.0F, 0.0F);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        Microsoft::Xna::Framework::Vector2 position,
        const std::optional<Microsoft::Xna::Framework::Rectangle> source,
        Microsoft::Xna::Framework::Color color,
        float rotation,
        Microsoft::Xna::Framework::Vector2 scale,
        const float depth)
    {
        SetTextureFiltering(
            anisotropicFiltering_ ? TextureFiltering::Anisotropic : TextureFiltering::Nearest);
        color = Utility::CrossEngineStuff::MultiplyColor(color, opacity_);
        scale = scale * transform_.getScaleProperty();
        rotation += transform_.getRotationProperty();
        position = transform_.Apply(position);

        EnsureUsable();
        renderer_->Draw(
            texture,
            position,
            source,
            color,
            rotation,
            Microsoft::Xna::Framework::Vector2::Zero,
            scale,
            Microsoft::Xna::Framework::Graphics::SpriteEffects::None,
            depth);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Vector2 position,
        const Microsoft::Xna::Framework::Color color,
        const Microsoft::Xna::Framework::Vector2 scale,
        const float rotation)
    {
        Draw(texture, position, std::nullopt, color, rotation, scale, 0.0F);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Vector2 position,
        const std::optional<Microsoft::Xna::Framework::Rectangle> source,
        const Microsoft::Xna::Framework::Color color,
        const float rotation)
    {
        Draw(texture, position, source, color, rotation,
            Microsoft::Xna::Framework::Vector2::One, 0.0F);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Vector2 position,
        const std::optional<Microsoft::Xna::Framework::Rectangle> source,
        const Microsoft::Xna::Framework::Color color)
    {
        Draw(texture, position, source, color, 0.0F,
            Microsoft::Xna::Framework::Vector2::One, 0.0F);
    }

    void RenderContext::Draw(
        const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Vector2 position,
        const Microsoft::Xna::Framework::Color color)
    {
        Draw(texture, position, std::nullopt, color, 0.0F,
            Microsoft::Xna::Framework::Vector2::One, 0.0F);
    }

    void RenderContext::Begin()
    {
        EnsureUsable();
        if (beginCalled_)
        {
            throw std::logic_error("RenderContext::Begin was called before End.");
        }
        if (device_ != nullptr)
        {
            RequireDevice().setRasterizerStateProperty(uiRasterizerState_);
        }
        renderer_->Begin(
            Microsoft::Xna::Framework::Graphics::SpriteSortMode::Deferred,
            Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend,
            &SelectedSamplerState(),
            nullptr,
            &uiRasterizerState_,
            nullptr);
        beginCalled_ = true;
    }

    void RenderContext::End()
    {
        EnsureUsable();
        if (!beginCalled_)
        {
            throw std::logic_error("RenderContext::End was called before Begin.");
        }
        renderer_->End();
        beginCalled_ = false;
    }

    void RenderContext::Flush()
    {
        EnsureUsable();
        if (!beginCalled_)
        {
            return;
        }
        End();
        Begin();
    }

    void RenderContext::Dispose()
    {
        if (disposed_)
        {
            return;
        }
        beginCalled_ = false;
        renderer_.reset();
        disposed_ = true;
    }

    Microsoft::Xna::Framework::Graphics::SamplerState& RenderContext::SelectedSamplerState()
    {
        return samplerState_;
    }

    void RenderContext::SetTextureFiltering(const TextureFiltering value)
    {
        EnsureUsable();
        if (textureFiltering_ == value)
        {
            return;
        }

        textureFiltering_ = value;
        switch (value)
        {
        case TextureFiltering::Nearest:
            samplerState_ = Microsoft::Xna::Framework::Graphics::SamplerState::PointClamp;
            break;
        case TextureFiltering::Linear:
            samplerState_ = Microsoft::Xna::Framework::Graphics::SamplerState::LinearClamp;
            break;
        case TextureFiltering::Anisotropic:
            samplerState_ = Microsoft::Xna::Framework::Graphics::SamplerState::AnisotropicClamp;
            break;
        }
        Flush();
    }

    void RenderContext::EnsureUsable() const
    {
        if (disposed_ || renderer_ == nullptr)
        {
            throw std::logic_error("RenderContext has been disposed.");
        }
    }

    Microsoft::Xna::Framework::Graphics::GraphicsDevice& RenderContext::RequireDevice() const
    {
        EnsureUsable();
        if (device_ == nullptr)
        {
            throw std::logic_error("This RenderContext has no GraphicsDevice for scissor operations.");
        }
        if (device_->getIsDisposedProperty())
        {
            throw std::logic_error("RenderContext's GraphicsDevice has been disposed.");
        }
        return *device_;
    }
}

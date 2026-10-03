// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra and MonoGame.Extended, MIT Licenses,
// Copyright (c) 2017-2020 The Myra Team and Copyright (c) 2015 Dylan Wilson.
// Ported from: src/Myra/Graphics2D/RenderContext.cs and
// src/Myra/Graphics2D/RenderContext.Shapes.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md, THIRD_PARTY_NOTICES.md, and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>
#include <optional>
#include <span>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Myra/Graphics2D/Transform.hpp"
#include "System/IDisposable.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
    class SpriteBatch;
    class Texture2D;
}

namespace Myra::Graphics2D
{
    struct RenderContextTestAccess;

    /** @brief Texture sampling mode selected for the current sprite batch. */
    enum class TextureFiltering
    {
        Nearest,
        Linear,
        Anisotropic
    };

    /**
     * @brief Graphics-only Myra rendering context over CNA SpriteBatch.
     *
     * The context owns its SpriteBatch and observes, but does not own, the
     * GraphicsDevice. It must therefore be destroyed before that device.
     * Font and rich-text overloads remain gated on the Phase 3 font decision.
     */
    class RenderContext : public System::IDisposable
    {
    public:
        /** @brief Creates a context for MyraEnvironment's configured device. */
        RenderContext();

        /** @brief Creates a context for an explicit caller-owned live device. */
        explicit RenderContext(Microsoft::Xna::Framework::Graphics::GraphicsDevice& device);

        /**
         * @brief Creates a context around an owned SpriteBatch, primarily for deterministic tests.
         *
         * Device-dependent scissor operations are unavailable when @p device is null.
         */
        explicit RenderContext(
            std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> renderer,
            Microsoft::Xna::Framework::Graphics::GraphicsDevice* device = nullptr);

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator=(const RenderContext&) = delete;
        RenderContext(RenderContext&&) = delete;
        RenderContext& operator=(RenderContext&&) = delete;
        ~RenderContext() override;

        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getDeviceScissorProperty() const;
        void setDeviceScissorProperty(Microsoft::Xna::Framework::Rectangle value);

        [[nodiscard]] Microsoft::Xna::Framework::Rectangle getScissorProperty() const noexcept;
        void setScissorProperty(Microsoft::Xna::Framework::Rectangle value);

        [[nodiscard]] float getOpacityProperty() const noexcept;
        void setOpacityProperty(float value) noexcept;
        void AddOpacity(float opacity) noexcept;

        [[nodiscard]] const Transform& getTransformProperty() const noexcept;
        void setTransformProperty(const Transform& value);

        /** @brief Selects anisotropic rather than nearest filtering for texture draws. */
        void SetAnisotropicFilteringMode(bool enabled);

        [[nodiscard]] TextureFiltering getTextureFilteringProperty() const noexcept;
        [[nodiscard]] bool getIsRenderingProperty() const noexcept;
        [[nodiscard]] bool getIsDisposedProperty() const noexcept;

        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Rectangle destination,
            std::optional<Microsoft::Xna::Framework::Rectangle> source,
            Microsoft::Xna::Framework::Color color,
            float rotation,
            float depth);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Rectangle destination,
            std::optional<Microsoft::Xna::Framework::Rectangle> source,
            Microsoft::Xna::Framework::Color color,
            float rotation);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Rectangle destination,
            std::optional<Microsoft::Xna::Framework::Rectangle> source,
            Microsoft::Xna::Framework::Color color);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Rectangle destination,
            Microsoft::Xna::Framework::Color color);

        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Vector2 position,
            std::optional<Microsoft::Xna::Framework::Rectangle> source,
            Microsoft::Xna::Framework::Color color,
            float rotation,
            Microsoft::Xna::Framework::Vector2 scale,
            float depth);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Vector2 position,
            Microsoft::Xna::Framework::Color color,
            Microsoft::Xna::Framework::Vector2 scale,
            float rotation = 0.0F);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Vector2 position,
            std::optional<Microsoft::Xna::Framework::Rectangle> source,
            Microsoft::Xna::Framework::Color color,
            float rotation);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Vector2 position,
            std::optional<Microsoft::Xna::Framework::Rectangle> source,
            Microsoft::Xna::Framework::Color color);
        void Draw(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            Microsoft::Xna::Framework::Vector2 position,
            Microsoft::Xna::Framework::Color color);

        void FillRectangle(
            Microsoft::Xna::Framework::Rectangle rectangle,
            Microsoft::Xna::Framework::Color color);
        void FillRectangle(
            Microsoft::Xna::Framework::Vector2 location,
            Microsoft::Xna::Framework::Vector2 size,
            Microsoft::Xna::Framework::Color color);
        void FillRectangle(
            float x,
            float y,
            float width,
            float height,
            Microsoft::Xna::Framework::Color color);

        void DrawRectangle(
            Microsoft::Xna::Framework::Rectangle rectangle,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);
        void DrawRectangle(
            Microsoft::Xna::Framework::Vector2 location,
            Microsoft::Xna::Framework::Vector2 size,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);

        void DrawPolygon(
            Microsoft::Xna::Framework::Vector2 offset,
            std::span<const Microsoft::Xna::Framework::Vector2> points,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);

        void DrawLine(
            float x1,
            float y1,
            float x2,
            float y2,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);
        void DrawLine(
            Microsoft::Xna::Framework::Vector2 point1,
            Microsoft::Xna::Framework::Vector2 point2,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);
        void DrawLine(
            Microsoft::Xna::Framework::Vector2 point,
            float length,
            float angle,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);

        void DrawPoint(
            float x,
            float y,
            Microsoft::Xna::Framework::Color color,
            float size = 1.0F);
        void DrawPoint(
            Microsoft::Xna::Framework::Vector2 position,
            Microsoft::Xna::Framework::Color color,
            float size = 1.0F);

        void DrawCircle(
            Microsoft::Xna::Framework::Vector2 center,
            float radius,
            int sides,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);
        void DrawCircle(
            float x,
            float y,
            float radius,
            int sides,
            Microsoft::Xna::Framework::Color color,
            float thickness = 1.0F);

        void DrawArc(
            Microsoft::Xna::Framework::Vector2 center,
            float radius,
            int sides,
            Microsoft::Xna::Framework::Color color,
            float startAngle,
            float endAngle,
            float thickness = 1.0F);
        void DrawArc(
            float x,
            float y,
            float radius,
            int sides,
            Microsoft::Xna::Framework::Color color,
            float startAngle,
            float endAngle,
            float thickness = 1.0F);

        /** @brief Starts a sprite batch using the currently selected filtering mode. */
        void Begin();
        /** @brief Ends the active sprite batch. */
        void End();
        /** @brief Restarts an active batch, or does nothing before Begin. */
        void Flush();
        /** @brief Releases the owned SpriteBatch; safe to call repeatedly. */
        void Dispose() override;

    private:
        friend struct RenderContextTestAccess;

        [[nodiscard]] Microsoft::Xna::Framework::Graphics::SamplerState& SelectedSamplerState();
        void SetTextureFiltering(TextureFiltering value);
        void EnsureUsable() const;
        [[nodiscard]] Microsoft::Xna::Framework::Graphics::GraphicsDevice& RequireDevice() const;

        std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> renderer_;
        Microsoft::Xna::Framework::Graphics::GraphicsDevice* device_ = nullptr;
        Microsoft::Xna::Framework::Graphics::RasterizerState uiRasterizerState_;
        Microsoft::Xna::Framework::Graphics::SamplerState samplerState_;
        bool beginCalled_ = false;
        bool disposed_ = false;
        bool anisotropicFiltering_ = false;
        Microsoft::Xna::Framework::Rectangle scissor_;
        float opacity_ = 0.0F;
        TextureFiltering textureFiltering_ = TextureFiltering::Nearest;
        Transform transform_{{0.0F, 0.0F}, {0.0F, 0.0F}, {1.0F, 1.0F}, 0.0F};
    };
}

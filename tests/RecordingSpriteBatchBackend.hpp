// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
#pragma once

#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "CNA/Internal/Renderers/Common/IGraphicsRenderer.hpp"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace Myra::Tests
{
    class DummyTextureBackend final : public CNA::Internal::Renderers::ITextureRenderer
    {
    public:
        explicit DummyTextureBackend(const int width = 2, const int height = 2)
            : width_(width), height_(height)
        {
        }

        [[nodiscard]] int GetWidth() const override
        {
            return width_;
        }

        [[nodiscard]] int GetHeight() const override
        {
            return height_;
        }

        [[nodiscard]] SDL_Texture* GetNativeTexture() const override
        {
            return nullptr;
        }

    private:
        int width_;
        int height_;
    };

    class RecordingSpriteBatchBackend final
        : public CNA::Internal::Renderers::ISpriteBatchRenderer
    {
    public:
        struct DrawCall
        {
            Microsoft::Xna::Framework::Rectangle destination;
            Microsoft::Xna::Framework::Rectangle source;
            Microsoft::Xna::Framework::Color color = Microsoft::Xna::Framework::Color::White;
            float rotation = 0.0F;
            Microsoft::Xna::Framework::Vector2 origin = Microsoft::Xna::Framework::Vector2::Zero;
            Microsoft::Xna::Framework::Graphics::SpriteEffects effects =
                Microsoft::Xna::Framework::Graphics::SpriteEffects::None;
            float depth = 0.0F;
        };

        int beginCount = 0;
        int endCount = 0;
        std::vector<int> samplerFilters;
        std::vector<DrawCall> draws;

        void Begin() override
        {
            ++beginCount;
        }

        void End() override
        {
            ++endCount;
        }

        void SetSamplerFilter(const int value) override
        {
            samplerFilters.push_back(value);
        }

        void Draw(
            const CNA::Internal::Renderers::ITextureRenderer&,
            const float x,
            const float y) override
        {
            DrawCall call;
            call.destination = Microsoft::Xna::Framework::Rectangle(
                static_cast<int>(x), static_cast<int>(y), 0, 0);
            draws.push_back(call);
        }

        void Draw(
            const CNA::Internal::Renderers::ITextureRenderer&,
            const Microsoft::Xna::Framework::Rectangle& destination,
            const Microsoft::Xna::Framework::Rectangle& source,
            const Microsoft::Xna::Framework::Color& color) override
        {
            draws.push_back({destination, source, color});
        }

        void Draw(
            const CNA::Internal::Renderers::ITextureRenderer&,
            const Microsoft::Xna::Framework::Rectangle& destination,
            const Microsoft::Xna::Framework::Rectangle& source,
            const Microsoft::Xna::Framework::Color& color,
            const float rotation,
            const Microsoft::Xna::Framework::Vector2& origin,
            const Microsoft::Xna::Framework::Graphics::SpriteEffects effects,
            const float depth) override
        {
            draws.push_back({destination, source, color, rotation, origin, effects, depth});
        }
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/CrossEngineStuff.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <cstdint>
#include <span>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace Myra::Utility
{
    /** @brief CNA-specific helpers shared by rendering and platform integration. */
    class CrossEngineStuff final
    {
    public:
        CrossEngineStuff() = delete;

        /** @brief Multiplies every color channel by the supplied scalar. */
        [[nodiscard]] static Microsoft::Xna::Framework::Color MultiplyColor(
            const Microsoft::Xna::Framework::Color& color,
            float value)
        {
            return Microsoft::Xna::Framework::Color::Multiply(color, value);
        }

        /** @brief Returns the configured GraphicsDevice viewport dimensions. */
        [[nodiscard]] static Microsoft::Xna::Framework::Point getViewSizeProperty();

        /** @brief Creates an RGBA Texture2D on a live CNA graphics device. */
        [[nodiscard]] static Microsoft::Xna::Framework::Graphics::Texture2D CreateTexture(
            Microsoft::Xna::Framework::Graphics::GraphicsDevice& device,
            int width,
            int height);

        /**
         * @brief Uploads row-major RGBA8 bytes into a texture region.
         *
         * Extra trailing bytes are ignored, matching the element-count supplied
         * by upstream Myra. A short buffer or invalid region is rejected before
         * CNA's typed Color upload is invoked.
         */
        static void SetTextureData(
            Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            const Microsoft::Xna::Framework::Rectangle& bounds,
            std::span<const std::uint8_t> data);
    };
}

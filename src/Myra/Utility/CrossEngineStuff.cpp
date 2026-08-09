// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/CrossEngineStuff.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/CrossEngineStuff.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace Myra::Utility
{
    namespace
    {
        [[nodiscard]] std::size_t ValidateTextureRegion(
            const Microsoft::Xna::Framework::Graphics::Texture2D& texture,
            const Microsoft::Xna::Framework::Rectangle& bounds)
        {
            const int x = bounds.X;
            const int y = bounds.Y;
            const int width = bounds.Width;
            const int height = bounds.Height;
            if (x < 0 || y < 0 || width <= 0 || height <= 0)
            {
                throw std::invalid_argument(
                    "A texture upload region must have a non-negative origin and positive dimensions.");
            }

            const std::int64_t right = static_cast<std::int64_t>(x) + width;
            const std::int64_t bottom = static_cast<std::int64_t>(y) + height;
            if (right > texture.getWidthProperty() || bottom > texture.getHeightProperty())
            {
                throw std::out_of_range("The texture upload region exceeds the texture bounds.");
            }

            const std::size_t pixelCount =
                static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
            if (pixelCount > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            {
                throw std::overflow_error("The texture upload region contains too many pixels.");
            }
            return pixelCount;
        }
    }

    Microsoft::Xna::Framework::Point CrossEngineStuff::getViewSizeProperty()
    {
        const Microsoft::Xna::Framework::Graphics::Viewport& viewport =
            MyraEnvironment::getGraphicsDeviceProperty().getViewportProperty();
        return {viewport.getWidthProperty(), viewport.getHeightProperty()};
    }

    Microsoft::Xna::Framework::Graphics::Texture2D CrossEngineStuff::CreateTexture(
        Microsoft::Xna::Framework::Graphics::GraphicsDevice& device,
        const int width,
        const int height)
    {
        if (device.getIsDisposedProperty())
        {
            throw std::invalid_argument("Cannot create a Myra texture on a disposed GraphicsDevice.");
        }
        if (width <= 0 || height <= 0)
        {
            throw std::invalid_argument("A Myra texture must have positive dimensions.");
        }
        return Microsoft::Xna::Framework::Graphics::Texture2D(device, width, height);
    }

    void CrossEngineStuff::SetTextureData(
        Microsoft::Xna::Framework::Graphics::Texture2D& texture,
        const Microsoft::Xna::Framework::Rectangle& bounds,
        const std::span<const std::uint8_t> data)
    {
        if (texture.getIsDisposedProperty())
        {
            throw std::invalid_argument("Cannot upload data to a disposed Myra texture.");
        }

        const std::size_t pixelCount = ValidateTextureRegion(texture, bounds);
        if (pixelCount > std::numeric_limits<std::size_t>::max() / 4U)
        {
            throw std::overflow_error("The texture upload byte count overflows size_t.");
        }
        const std::size_t requiredBytes = pixelCount * 4U;
        if (data.size() < requiredBytes)
        {
            throw std::out_of_range("The RGBA texture upload buffer is shorter than the requested region.");
        }

        std::vector<Microsoft::Xna::Framework::Color> colors;
        colors.reserve(pixelCount);
        for (std::size_t index = 0; index < pixelCount; ++index)
        {
            const std::size_t byteIndex = index * 4U;
            colors.emplace_back(
                data[byteIndex], data[byteIndex + 1U], data[byteIndex + 2U], data[byteIndex + 3U]);
        }

        texture.SetData(0, &bounds, colors.data(), 0, static_cast<int>(pixelCount));
    }
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/DefaultAssets.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/DefaultAssets.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <span>

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"

namespace Myra
{
    std::shared_ptr<Graphics2D::TextureAtlases::TextureRegion>
        DefaultAssets::whiteRegion_;

    std::shared_ptr<Graphics2D::TextureAtlases::TextureRegion>
        DefaultAssets::getWhiteRegionProperty()
    {
        if (whiteRegion_ != nullptr)
        {
            const auto texture = whiteRegion_->getTextureProperty();
            if (texture != nullptr && !texture->getIsDisposedProperty())
            {
                return whiteRegion_;
            }
            whiteRegion_.reset();
        }

        auto texture = std::make_shared<Microsoft::Xna::Framework::Graphics::Texture2D>(
            Utility::CrossEngineStuff::CreateTexture(
                MyraEnvironment::getGraphicsDeviceProperty(), 1, 1));
        constexpr std::array<std::uint8_t, 4> white{255U, 255U, 255U, 255U};
        Utility::CrossEngineStuff::SetTextureData(
            *texture,
            Microsoft::Xna::Framework::Rectangle(0, 0, 1, 1),
            std::span<const std::uint8_t>(white));
        whiteRegion_ =
            std::make_shared<Graphics2D::TextureAtlases::TextureRegion>(std::move(texture));
        return whiteRegion_;
    }

    void DefaultAssets::Dispose() noexcept
    {
        whiteRegion_.reset();
    }
}

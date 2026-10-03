// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/DefaultAssets.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

namespace Myra::Graphics2D::TextureAtlases
{
    class TextureRegion;
}

namespace Myra
{
    /** @brief Font-independent core of Myra's process-wide default assets. */
    class DefaultAssets final
    {
    public:
        DefaultAssets() = delete;

        /** @brief Returns a retained, cached 1x1 opaque-white texture region. */
        [[nodiscard]] static std::shared_ptr<Graphics2D::TextureAtlases::TextureRegion>
            getWhiteRegionProperty();

        /** @brief Releases Myra's cached handles without invalidating external owners. */
        static void Dispose() noexcept;

    private:
        static std::shared_ptr<Graphics2D::TextureAtlases::TextureRegion> whiteRegion_;
    };
}

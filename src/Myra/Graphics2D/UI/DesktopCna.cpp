// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include "Myra/Utility/CrossEngineStuff.hpp"

namespace Myra::Graphics2D::UI
{
    Microsoft::Xna::Framework::Rectangle Desktop::DefaultBoundsFetcher()
    {
        const Microsoft::Xna::Framework::Point size = Utility::CrossEngineStuff::getViewSizeProperty();
        return {0, 0, size.X, size.Y};
    }
} // namespace Myra::Graphics2D::UI

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <stdexcept>

namespace Myra::Graphics2D::UI
{
    Microsoft::Xna::Framework::Rectangle Desktop::DefaultBoundsFetcher()
    {
        throw std::logic_error("The default desktop bounds fetcher requires Myra-CNA to be linked with a CNA target.");
    }
} // namespace Myra::Graphics2D::UI

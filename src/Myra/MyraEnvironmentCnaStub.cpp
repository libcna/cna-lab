// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MyraEnvironment.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MyraEnvironment.hpp"

#include <stdexcept>

#include "Microsoft/Xna/Framework/Color.hpp"

namespace Myra
{
    const Microsoft::Xna::Framework::Color &MyraEnvironment::getDarkeningColorProperty()
    {
        throw std::logic_error("The modal darkening color requires Myra-CNA to be linked with a CNA target.");
    }

    void MyraEnvironment::setDarkeningColorProperty(Microsoft::Xna::Framework::Color)
    {
        throw std::logic_error("The modal darkening color requires Myra-CNA to be linked with a CNA target.");
    }

    void MyraEnvironment::ApplyMouseCursorType(const Graphics2D::UI::MouseCursorType value)
    {
        using Graphics2D::UI::MouseCursorType;

        switch (value)
        {
        case MouseCursorType::Arrow:
        case MouseCursorType::IBeam:
        case MouseCursorType::Wait:
        case MouseCursorType::Crosshair:
        case MouseCursorType::WaitArrow:
        case MouseCursorType::SizeNWSE:
        case MouseCursorType::SizeNESW:
        case MouseCursorType::SizeWE:
        case MouseCursorType::SizeNS:
        case MouseCursorType::SizeAll:
        case MouseCursorType::No:
        case MouseCursorType::Hand:
            return;
        }

        throw std::invalid_argument("Could not map the requested Myra mouse cursor type.");
    }
} // namespace Myra

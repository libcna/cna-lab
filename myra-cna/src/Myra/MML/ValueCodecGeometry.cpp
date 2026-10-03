// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the necessary C++ value-conversion adaptation of Myra MML semantics.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/ValueCodecRegistry.hpp"

namespace Myra::MML
{
    void ValueCodecRegistry::RegisterAuditedGeometry()
    {
        Register(std::make_unique<Vector2Serializer>());
        RegisterOptional<Microsoft::Xna::Framework::Vector2>();
        Register(std::make_unique<ThicknessSerializer>());
        RegisterOptional<Graphics2D::Thickness>();
        Register(std::make_unique<RectangleSerializer>());
        RegisterOptional<Microsoft::Xna::Framework::Rectangle>();
    }
}

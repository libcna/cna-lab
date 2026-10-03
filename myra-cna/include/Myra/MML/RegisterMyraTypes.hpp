// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Implements the explicit C++ metadata table required in place of .NET reflection.
// MyraUI/Myra is MIT, Copyright (c) 2017-2020 The Myra Team.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include "Myra/MML/TypeRegistry.hpp"

namespace Myra::MML
{
    /** @brief Registers every currently ported concrete/core Myra MML type and property. */
    void RegisterMyraTypes(TypeRegistry& registry);

    /** @brief Creates a registry populated by RegisterMyraTypes. */
    [[nodiscard]] TypeRegistry CreateMyraTypeRegistry();
}

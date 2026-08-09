// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Panel.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/Container.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief A container that places all visible children at the same origin. */
    class Panel : public Container
    {
    public:
        Panel() = default;
        ~Panel() override = default;

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        [[nodiscard]] Microsoft::Xna::Framework::Point InternalMeasure(
            Microsoft::Xna::Framework::Point availableSize) override;
        void InternalArrange() override;
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Proportion.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string>

#include "Myra/Events/MyraEventHandler.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Specifies how a grid or stack cell divides available space. */
    enum class ProportionType
    {
        Auto,
        Part,
        Fill,
        Pixels
    };

    /** @brief Defines the size rule for one grid or stack cell. */
    class Proportion
    {
    public:
        static const Proportion Auto;
        static const Proportion Fill;
        static const Proportion GridDefault;
        static const Proportion StackPanelDefault;

        Events::MyraEventHandler Changed;

        Proportion() = default;
        explicit Proportion(ProportionType type) noexcept;
        Proportion(ProportionType type, float value) noexcept;

        [[nodiscard]] ProportionType getTypeProperty() const noexcept;
        void setTypeProperty(ProportionType value);
        [[nodiscard]] float getValueProperty() const noexcept;
        void setValueProperty(float value);
        [[nodiscard]] std::string ToString() const;

    private:
        void FireChanged();

        ProportionType type_ = ProportionType::Auto;
        float value_ = 1.0F;
    };
}

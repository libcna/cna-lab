// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/IContent.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

namespace Myra::Graphics2D::UI
{
    class Widget;
}

namespace Myra::Graphics2D
{
    /** @brief Represents an object that can hold one content widget. */
    class IContent
    {
    public:
        virtual ~IContent() = default;

        [[nodiscard]] virtual std::shared_ptr<UI::Widget> getContentProperty() const = 0;
        virtual void setContentProperty(std::shared_ptr<UI::Widget> value) = 0;
    };
}

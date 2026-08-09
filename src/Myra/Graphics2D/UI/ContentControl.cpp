// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/ContentControl.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/ContentControl.hpp"

#include <stdexcept>

namespace Myra::Graphics2D::UI
{
    void ContentControl::CopyFrom(const Widget& source)
    {
        Widget::CopyFrom(source);
        const auto* const contentControl = dynamic_cast<const ContentControl*>(&source);
        if (contentControl == nullptr)
        {
            throw std::invalid_argument("ContentControl copy source must be a ContentControl.");
        }

        const std::shared_ptr<Widget> content = contentControl->getContentProperty();
        setContentProperty(content ? content->Clone() : nullptr);
    }
}

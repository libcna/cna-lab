// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <stdexcept>

namespace Myra::Graphics2D::UI
{
    void Widget::Render(Graphics2D::RenderContext&)
    {
        throw std::logic_error(
            "Widget rendering requires Myra-CNA to be linked with a CNA target.");
    }

    void Widget::InternalRender(Graphics2D::RenderContext&)
    {
        throw std::logic_error(
            "Widget rendering requires Myra-CNA to be linked with a CNA target.");
    }
}

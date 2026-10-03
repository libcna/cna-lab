// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/CheckButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/CheckButton.hpp"

namespace Myra::Graphics2D::UI
{
    CheckButton::CheckButton() : IsCheckedChanged(PressedChanged) {}

    bool CheckButton::getIsCheckedProperty() const noexcept
    {
        return getIsPressedProperty();
    }

    void CheckButton::setIsCheckedProperty(const bool value)
    {
        setIsPressedProperty(value);
    }

    std::shared_ptr<Widget> CheckButton::CreateCloneInstance() const
    {
        return std::make_shared<CheckButton>();
    }
} // namespace Myra::Graphics2D::UI

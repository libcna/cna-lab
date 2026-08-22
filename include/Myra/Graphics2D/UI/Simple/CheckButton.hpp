// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/CheckButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief A check button whose checked state is its pressed state. */
    class CheckButton final : public CheckButtonBase
    {
      public:
        CheckButton();
        ~CheckButton() override = default;

        /** @brief Alias of PressedChanged, matching upstream IsCheckedChanged. */
        Events::MyraEventHandler &IsCheckedChanged;

        [[nodiscard]] bool getIsCheckedProperty() const noexcept;
        void setIsCheckedProperty(bool value);

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };
} // namespace Myra::Graphics2D::UI

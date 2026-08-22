// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/RadioButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief A check button that keeps one direct sibling selected in its group. */
    class RadioButton final : public CheckButtonBase
    {
      public:
        RadioButton() = default;
        ~RadioButton() override = default;

        [[nodiscard]] bool getIsPressedProperty() const noexcept override;
        void setIsPressedProperty(bool value) override;
        void OnPressedChanged() override;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
    };
} // namespace Myra::Graphics2D::UI

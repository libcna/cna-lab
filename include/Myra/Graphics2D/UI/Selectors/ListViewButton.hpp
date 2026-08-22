// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/ListViewButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Myra/Graphics2D/UI/Simple/ToggleButton.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Internal toggle wrapper that keeps one ListView item pressed in its group. */
    class ListViewButton final : public ToggleButton
    {
      public:
        ListViewButton() = default;
        ~ListViewButton() override = default;

        /** @brief Optional group root; otherwise the direct parent is used. This is non-owning. */
        [[nodiscard]] Widget *getButtonsContainerProperty() const noexcept;
        void setButtonsContainerProperty(Widget *value) noexcept;

        [[nodiscard]] bool getIsPressedProperty() const noexcept override;
        void setIsPressedProperty(bool value) override;
        void OnPressedChanged() override;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;

      private:
        [[nodiscard]] Widget *GetTopParent() const noexcept;

        Widget *buttonsContainer_ = nullptr;
    };
} // namespace Myra::Graphics2D::UI

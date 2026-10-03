// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/ToggleButton.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Single-content button that toggles its pressed state. */
    class ToggleButton : public ButtonBase
    {
    public:
        ToggleButton();
        ~ToggleButton() override = default;

        /** @brief Alias of PressedChanged, matching the upstream event facade. */
        Events::MyraEventHandler& IsToggledChanged;

        [[nodiscard]] bool getIsToggledProperty() const noexcept;
        void setIsToggledProperty(bool value);

        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override;
        void setContentProperty(std::shared_ptr<Widget> value) override;

        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;

    protected:
        void InternalOnTouchUp() override;
        void InternalOnTouchDown() override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;

    private:
        SingleItemLayout<Widget> layout_;
    };
}

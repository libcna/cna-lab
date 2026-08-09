// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/Button.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"

namespace Myra::Graphics2D::UI
{
    class Slider;
    class SplitPane;

    /** @brief Clickable single-content button with touch and keyboard behavior. */
    class Button : public ButtonBase
    {
    public:
        Button();
        ~Button() override = default;

        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override;
        void setContentProperty(std::shared_ptr<Widget> value) override;

        void OnTouchLeft() override;
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;

    protected:
        void InternalOnTouchUp() override;
        void InternalOnTouchDown() override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;

    private:
        friend class Slider;
        friend class SplitPane;

        SingleItemLayout<Widget> layout_;
        bool releaseOnTouchLeft_ = true;
    };
}

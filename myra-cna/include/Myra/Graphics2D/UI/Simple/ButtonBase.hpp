// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/ButtonBase.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include "Myra/Graphics2D/UI/ContentControl.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Abstract press/click state machine shared by button-like controls. */
    class ButtonBase : public ContentControl
    {
    public:
        ~ButtonBase() override = default;

        Events::MyraEventHandler Click;

        [[nodiscard]] bool getReadOnlyProperty() const noexcept;
        void setReadOnlyProperty(bool value) noexcept;

        void DoClick();
        void OnTouchUp() override;
        void OnTouchDown() override;

    protected:
        ButtonBase() = default;

        virtual void InternalOnTouchUp() = 0;
        virtual void InternalOnTouchDown() = 0;
        void CopyFrom(const Widget& source) override;

    private:
        bool isClicked_ = false;
        bool readOnly_ = false;
    };
}

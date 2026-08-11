// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/CheckButtonBase.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Graphics2D/UI/Layouts/StackPanelLayout.hpp"
#include "Myra/Graphics2D/UI/Simple/ButtonBase.hpp"
#include "Myra/Graphics2D/UI/Simple/Image.hpp"

namespace Myra::Graphics2D
{
    class IImage;
}

namespace Myra::Graphics2D::UI
{
    /** @brief Position of a check image relative to check-button content. */
    enum class CheckPosition
    {
        Left,
        Right
    };

    /** @brief Non-publicly constructible core shared by check and radio buttons. */
    class CheckButtonBase : public ButtonBase
    {
    public:
        ~CheckButtonBase() override = default;

        [[nodiscard]] CheckPosition getCheckPositionProperty() const noexcept;
        void setCheckPositionProperty(CheckPosition value);
        [[nodiscard]] int getCheckContentSpacingProperty() const noexcept;
        void setCheckContentSpacingProperty(int value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getUncheckedImageProperty() const;
        void setUncheckedImageProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getCheckedImageProperty() const;
        void setCheckedImageProperty(std::shared_ptr<Graphics2D::IImage> value);

        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override;
        void setContentProperty(std::shared_ptr<Widget> value) override;
        [[nodiscard]] std::shared_ptr<Image> getCheckImageProperty() const noexcept;

        void OnPressedChanged() override;
        void OnKeyDown(Microsoft::Xna::Framework::Input::Keys key) override;

    protected:
        CheckButtonBase();

        void InternalOnTouchUp() override;
        void InternalOnTouchDown() override;
        void CopyFrom(const Widget& source) override;

    private:
        class CheckImageInternal final : public Image
        {
        public:
            void CopyFromImage(const Image& source);

        protected:
            [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        };

        void UpdateChildren();
        void UpdateImage();

        StackPanelLayout layout_;
        CheckPosition checkPosition_ = CheckPosition::Left;
        std::shared_ptr<CheckImageInternal> check_;
        std::shared_ptr<Widget> content_;
        std::shared_ptr<Graphics2D::IImage> checkedImage_;
        std::shared_ptr<Graphics2D::IImage> uncheckedImage_;
    };
}

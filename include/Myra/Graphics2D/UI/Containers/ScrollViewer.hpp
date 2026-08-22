// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/ScrollViewer.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <memory>

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/UI/ContentControl.hpp"
#include "Myra/Graphics2D/UI/Layouts/SingleItemLayout.hpp"

namespace Myra::Graphics2D
{
    class IImage;
    class RenderContext;
} // namespace Myra::Graphics2D

namespace Myra::Graphics2D::UI
{
    /** @brief Single-content viewport with optional image-based horizontal and vertical scrolling. */
    class ScrollViewer final : public ContentControl
    {
      public:
        ScrollViewer();
        ~ScrollViewer() override = default;

        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override;
        void setContentProperty(std::shared_ptr<Widget> value) override;

        [[nodiscard]] Microsoft::Xna::Framework::Point getScrollMaximumProperty() const;
        [[nodiscard]] Microsoft::Xna::Framework::Point getScrollPositionProperty() const;
        void setScrollPositionProperty(Microsoft::Xna::Framework::Point value);
        void ResetScroll();

        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getHorizontalScrollBackgroundProperty() const;
        void setHorizontalScrollBackgroundProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getHorizontalScrollKnobProperty() const;
        void setHorizontalScrollKnobProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getVerticalScrollBackgroundProperty() const;
        void setVerticalScrollBackgroundProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getVerticalScrollKnobProperty() const;
        void setVerticalScrollKnobProperty(std::shared_ptr<Graphics2D::IImage> value);

        [[nodiscard]] int getScrollMultiplierProperty() const noexcept;
        void setScrollMultiplierProperty(int value) noexcept;
        [[nodiscard]] bool getShowHorizontalScrollBarProperty() const noexcept;
        void setShowHorizontalScrollBarProperty(bool value);
        [[nodiscard]] bool getShowVerticalScrollBarProperty() const noexcept;
        void setShowVerticalScrollBarProperty(bool value);

        [[nodiscard]] bool getHorizontalScrollingOnProperty() const noexcept;
        [[nodiscard]] bool getVerticalScrollingOnProperty() const noexcept;

        /** @brief Scrolls vertically when invoked by a parent or future Desktop wheel dispatch. */
        void OnMouseWheel(float delta);
        void InternalRender(Graphics2D::RenderContext &context) override;

      protected:
        [[nodiscard]] Microsoft::Xna::Framework::Point
        InternalMeasure(Microsoft::Xna::Framework::Point availableSize) override;
        void InternalArrange() override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        [[nodiscard]] int getHorizontalScrollbarHeight() const;
        [[nodiscard]] int getVerticalScrollbarWidth() const;
        [[nodiscard]] int getVerticalThumbWidth() const noexcept;
        [[nodiscard]] int getHorizontalThumbHeight() const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Point getThumbPosition() const;
        void MoveThumb(int delta);

        SingleItemLayout<Widget> layout_;
        Orientation scrollbarOrientation_ = Orientation::Vertical;
        bool horizontalScrollingOn_ = false;
        bool verticalScrollingOn_ = false;
        bool showHorizontalScrollBar_ = true;
        bool showVerticalScrollBar_ = true;
        Microsoft::Xna::Framework::Rectangle horizontalScrollbarFrame_;
        Microsoft::Xna::Framework::Rectangle horizontalScrollbarThumb_;
        Microsoft::Xna::Framework::Rectangle verticalScrollbarFrame_;
        Microsoft::Xna::Framework::Rectangle verticalScrollbarThumb_;
        int thumbMaximumX_ = 1;
        int thumbMaximumY_ = 1;
        std::shared_ptr<Graphics2D::IImage> horizontalScrollBackground_;
        std::shared_ptr<Graphics2D::IImage> horizontalScrollKnob_;
        std::shared_ptr<Graphics2D::IImage> verticalScrollBackground_;
        std::shared_ptr<Graphics2D::IImage> verticalScrollKnob_;
        int scrollMultiplier_ = 10;
    };
} // namespace Myra::Graphics2D::UI

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::ScrollViewer;
    using Myra::Graphics2D::UI::Widget;

    class FixedWidget final : public Widget
    {
      public:
        explicit FixedWidget(const Point measuredSize) : measuredSize_(measuredSize) {}

      protected:
        [[nodiscard]] Point InternalMeasure(const Point) override { return measuredSize_; }

        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return std::make_shared<FixedWidget>(measuredSize_);
        }

      private:
        Point measuredSize_;
    };

    class TestImage final : public IImage
    {
      public:
        explicit TestImage(const Point size) : size_(size) {}

        [[nodiscard]] Point getSizeProperty() const override { return size_; }

        void Draw(RenderContext &, Rectangle, Color) const override {}

      private:
        Point size_;
    };

    TEST(ScrollViewerTests, ArrangesOversizedContentAndClampsScrollPositionThroughImageThumbs)
    {
        ScrollViewer viewer;
        const auto content = std::make_shared<FixedWidget>(Point(300, 200));
        viewer.setContentProperty(content);
        viewer.setHorizontalScrollKnobProperty(std::make_shared<TestImage>(Point(8, 12)));
        viewer.setVerticalScrollKnobProperty(std::make_shared<TestImage>(Point(10, 9)));

        viewer.Arrange(Rectangle(0, 0, 100, 100));
        EXPECT_TRUE(viewer.getHorizontalScrollingOnProperty());
        EXPECT_TRUE(viewer.getVerticalScrollingOnProperty());
        EXPECT_EQ(viewer.getScrollMaximumProperty(), Point(210, 112));

        viewer.setScrollPositionProperty(Point(999, 999));
        viewer.InvalidateArrange();
        viewer.UpdateArrange();
        EXPECT_EQ(viewer.getScrollPositionProperty(), Point(210, 112));
        EXPECT_EQ(content->getLeftProperty(), -210);
        EXPECT_EQ(content->getTopProperty(), -112);

        viewer.ResetScroll();
        viewer.OnMouseWheel(-1.0F);
        EXPECT_EQ(viewer.getScrollPositionProperty(), Point(0, 22));
    }

    TEST(ScrollViewerTests, PreservesContentAndStyleIndependentStateWhenCloned)
    {
        ScrollViewer source;
        const auto content = std::make_shared<FixedWidget>(Point(21, 34));
        const auto horizontalKnob = std::make_shared<TestImage>(Point(3, 4));
        source.setContentProperty(content);
        source.setHorizontalScrollKnobProperty(horizontalKnob);
        source.setShowHorizontalScrollBarProperty(false);
        source.setShowVerticalScrollBarProperty(false);
        source.setScrollMultiplierProperty(17);

        const auto clone = std::dynamic_pointer_cast<ScrollViewer>(source.Clone());
        ASSERT_NE(clone, nullptr);
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->Measure(Point(100, 100)), Point(21, 34));
        EXPECT_EQ(clone->getHorizontalScrollKnobProperty(), horizontalKnob);
        EXPECT_FALSE(clone->getShowHorizontalScrollBarProperty());
        EXPECT_FALSE(clone->getShowVerticalScrollBarProperty());
        EXPECT_EQ(clone->getScrollMultiplierProperty(), 17);
    }
} // namespace

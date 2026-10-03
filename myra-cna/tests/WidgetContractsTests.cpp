// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/IContent.hpp"
#include "Myra/Graphics2D/UI/ContentControl.hpp"
#include "Myra/Graphics2D/UI/ILayout.hpp"
#include "Myra/Graphics2D/UI/InputContext.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <vector>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    class ContentHost final : public Myra::Graphics2D::IContent
    {
    public:
        [[nodiscard]] std::shared_ptr<Myra::Graphics2D::UI::Widget> getContentProperty() const override
        {
            return content_;
        }

        void setContentProperty(std::shared_ptr<Myra::Graphics2D::UI::Widget> value) override
        {
            content_ = std::move(value);
        }

    private:
        std::shared_ptr<Myra::Graphics2D::UI::Widget> content_;
    };

    static_assert(std::is_abstract_v<Myra::Graphics2D::UI::ContentControl>);
    static_assert(std::is_base_of_v<Myra::Graphics2D::UI::Widget,
        Myra::Graphics2D::UI::ContentControl>);
    static_assert(std::is_base_of_v<Myra::Graphics2D::IContent,
        Myra::Graphics2D::UI::ContentControl>);

    class Layout final : public Myra::Graphics2D::UI::ILayout
    {
    public:
        [[nodiscard]] Point Measure(
            const std::vector<std::shared_ptr<Myra::Graphics2D::UI::Widget>>& widgets,
            Point availableSize) override
        {
            measureWidgetCount = widgets.size();
            return availableSize;
        }

        void Arrange(
            const std::vector<std::shared_ptr<Myra::Graphics2D::UI::Widget>>& widgets,
            Rectangle bounds) override
        {
            arrangeWidgetCount = widgets.size();
            arrangedBounds = bounds;
        }

        std::size_t measureWidgetCount = 0;
        std::size_t arrangeWidgetCount = 0;
        Rectangle arrangedBounds;
    };

    TEST(WidgetContractsTests, InputContextResetRestoresUpstreamDefaults)
    {
        Myra::Graphics2D::UI::InputContext context;
        context.MouseOrTouchHandled = true;
        context.ParentContainsMouse = false;
        context.ParentContainsTouch = false;

        context.Reset();

        EXPECT_FALSE(context.MouseOrTouchHandled);
        EXPECT_EQ(context.MouseWheelWidget, nullptr);
        EXPECT_TRUE(context.ParentContainsMouse);
        EXPECT_TRUE(context.ParentContainsTouch);
    }

    TEST(WidgetContractsTests, ContentAndLayoutUseSharedWidgetCollections)
    {
        ContentHost host;
        EXPECT_EQ(host.getContentProperty(), nullptr);
        host.setContentProperty(nullptr);
        EXPECT_EQ(host.getContentProperty(), nullptr);

        Layout layout;
        const std::vector<std::shared_ptr<Myra::Graphics2D::UI::Widget>> widgets;
        const Point availableSize(320, 200);
        const Point desiredSize = layout.Measure(widgets, availableSize);
        layout.Arrange(widgets, Rectangle(1, 2, 3, 4));

        EXPECT_EQ(layout.measureWidgetCount, 0U);
        EXPECT_EQ(layout.arrangeWidgetCount, 0U);
        EXPECT_EQ(desiredSize, availableSize);
        EXPECT_EQ(layout.arrangedBounds, Rectangle(1, 2, 3, 4));
    }

}

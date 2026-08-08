// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <vector>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::Widget;

    class FixedWidget final : public Widget
    {
    public:
        explicit FixedWidget(const Point desiredSize) : desiredSize_(desiredSize) {}

        Point lastAvailableSize;
        int measureCalls = 0;
        int arrangeCalls = 0;

    protected:
        [[nodiscard]] Point InternalMeasure(const Point availableSize) override
        {
            lastAvailableSize = availableSize;
            ++measureCalls;
            return desiredSize_;
        }

        void InternalArrange() override { ++arrangeCalls; }

    private:
        Point desiredSize_;
    };

    TEST(WidgetTests, MeasureAppliesMarginBorderPaddingAndSizeConstraints)
    {
        FixedWidget widget(Point(20, 30));
        widget.setMarginProperty(Thickness(1, 2, 3, 4));
        widget.setBorderThicknessProperty(Thickness(2, 1, 2, 1));
        widget.setPaddingProperty(Thickness(3, 4, 3, 4));
        widget.setMinWidthProperty(40);
        widget.setMaxHeightProperty(40);

        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(40, 40));
        EXPECT_EQ(widget.lastAvailableSize, Point(86, 24));

        widget.setWidthProperty(50);
        widget.setHeightProperty(60);
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(50, 60));
    }

    TEST(WidgetTests, ArrangeAlignsAndRaisesEventsOnlyForPropertyChanges)
    {
        FixedWidget widget(Point(30, 20));
        int locationChanges = 0;
        int sizeChanges = 0;
        int arrangeUpdates = 0;

        widget.LocationChanged += [&](void*, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::LocationChanged);
            ++locationChanges;
        };
        widget.SizeChanged += [&](void*, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SizeChanged);
            ++sizeChanges;
        };
        widget.ArrangeUpdated += [&](void*, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ArrangeUpdated);
            ++arrangeUpdates;
        };

        widget.setLeftProperty(4);
        widget.setLeftProperty(4);
        widget.setWidthProperty(30);
        widget.setHorizontalAlignmentProperty(HorizontalAlignment::Center);
        widget.setVerticalAlignmentProperty(VerticalAlignment::Bottom);
        widget.Arrange(Rectangle(10, 20, 100, 80));
        widget.Arrange(Rectangle(10, 20, 100, 80));

        EXPECT_EQ(locationChanges, 1);
        EXPECT_EQ(sizeChanges, 1);
        EXPECT_EQ(arrangeUpdates, 1);
        EXPECT_EQ(widget.getBoundsProperty(), Rectangle(45, 80, 30, 20));
        EXPECT_EQ(widget.getContainerBoundsProperty(), Rectangle(10, 20, 100, 80));
    }

    TEST(WidgetTests, ChildrenHaveOneParentAndAreStablyOrderedByZIndex)
    {
        auto firstParent = std::make_shared<Widget>();
        auto secondParent = std::make_shared<Widget>();
        auto high = std::make_shared<Widget>();
        auto low = std::make_shared<Widget>();
        high->setZIndexProperty(2);
        low->setZIndexProperty(1);

        firstParent->AddChild(high);
        firstParent->AddChild(low);
        const auto& sortedChildren = firstParent->getChildrenCopyProperty();
        ASSERT_EQ(sortedChildren.size(), 2U);
        EXPECT_EQ(sortedChildren[0], low);
        EXPECT_EQ(sortedChildren[1], high);
        EXPECT_EQ(high->getParentProperty(), firstParent.get());

        secondParent->AddChild(high);
        ASSERT_EQ(firstParent->getChildrenProperty().size(), 1U);
        EXPECT_EQ(firstParent->getChildrenProperty().front(), low);
        ASSERT_EQ(secondParent->getChildrenProperty().size(), 1U);
        EXPECT_EQ(high->getParentProperty(), secondParent.get());

        high->RemoveFromParent();
        EXPECT_TRUE(secondParent->getChildrenProperty().empty());
        EXPECT_EQ(high->getParentProperty(), nullptr);
    }

    TEST(WidgetTests, RootTransformConvertsBetweenLocalAndGlobalCoordinates)
    {
        Widget widget;
        widget.setLeftProperty(5);
        widget.setTopProperty(7);
        widget.setWidthProperty(20);
        widget.setHeightProperty(10);
        widget.Arrange(Rectangle(10, 20, 100, 80));

        const Vector2 global = widget.ToGlobal(Vector2(0.0F, 0.0F));
        EXPECT_FLOAT_EQ(global.X, 15.0F);
        EXPECT_FLOAT_EQ(global.Y, 27.0F);

        const Vector2 local = widget.ToLocal(global);
        EXPECT_FLOAT_EQ(local.X, 0.0F);
        EXPECT_FLOAT_EQ(local.Y, 0.0F);
        EXPECT_TRUE(widget.ContainsGlobalPoint(Point(15, 27)));
        EXPECT_FALSE(widget.ContainsGlobalPoint(Point(35, 37)));
    }

    TEST(WidgetTests, OpacityRejectsValuesOutsideTheUpstreamRange)
    {
        Widget widget;
        EXPECT_THROW(widget.setOpacityProperty(-0.01F), std::out_of_range);
        EXPECT_THROW(widget.setOpacityProperty(1.01F), std::out_of_range);
        widget.setOpacityProperty(0.5F);
        EXPECT_FLOAT_EQ(widget.getOpacityProperty(), 0.5F);
    }

    TEST(WidgetTests, AttachedPropertyOptionsInvalidateTheMatchingLayoutPhase)
    {
        static const Myra::MML::AttachedPropertyInfo<int>* measureProperty =
            &Myra::MML::AttachedPropertiesRegistry::Create(
                typeid(Widget), "WidgetTestMeasure", 0, Myra::MML::AttachedPropertyOption::AffectsMeasure);
        static const Myra::MML::AttachedPropertyInfo<int>* arrangeProperty =
            &Myra::MML::AttachedPropertiesRegistry::Create(
                typeid(Widget), "WidgetTestArrange", 0, Myra::MML::AttachedPropertyOption::AffectsArrange);

        FixedWidget widget(Point(10, 10));
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(10, 10));
        EXPECT_EQ(widget.measureCalls, 1);
        measureProperty->SetValue(widget, 1);
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(10, 10));
        EXPECT_EQ(widget.measureCalls, 2);

        widget.Arrange(Rectangle(0, 0, 100, 100));
        EXPECT_EQ(widget.arrangeCalls, 1);
        arrangeProperty->SetValue(widget, 1);
        widget.UpdateArrange();
        EXPECT_EQ(widget.arrangeCalls, 2);
    }
}

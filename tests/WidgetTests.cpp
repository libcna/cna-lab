// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::ILayout;
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
        bool invalidateDuringMeasure = false;

        void SetSuppressInvalidateMeasure(const bool value)
        {
            setSuppressInvalidateMeasureProperty(value);
        }

        [[nodiscard]] bool GetSuppressInvalidateMeasure() const
        {
            return getSuppressInvalidateMeasureProperty();
        }

        void SetPressedByUser(const bool value)
        {
            SetIsPressedByUser(value);
        }

        [[nodiscard]] Rectangle GetBorderBounds() const
        {
            return getBorderBoundsProperty();
        }

        [[nodiscard]] Rectangle GetBackgroundBounds() const
        {
            return getBackgroundBoundsProperty();
        }

    protected:
        [[nodiscard]] Point InternalMeasure(const Point availableSize) override
        {
            lastAvailableSize = availableSize;
            ++measureCalls;
            if (invalidateDuringMeasure)
            {
                invalidateDuringMeasure = false;
                InvalidateMeasure();
            }
            return desiredSize_;
        }

        void InternalArrange() override { ++arrangeCalls; }

    private:
        Point desiredSize_;
    };

    class TaggedWidget final : public Widget
    {
    };

    class SnapshotLayout final : public ILayout
    {
    public:
        SnapshotLayout(Widget& owner, Widget& childToRemove)
            : owner_(owner), childToRemove_(childToRemove)
        {
        }

        std::vector<Widget*> measuredWidgets;

        [[nodiscard]] Point Measure(
            const std::vector<std::shared_ptr<Widget>>& widgets, Point) override
        {
            EXPECT_TRUE(owner_.RemoveChild(&childToRemove_));
            static_cast<void>(owner_.getChildrenCopyProperty());
            for (const std::shared_ptr<Widget>& widget : widgets)
            {
                measuredWidgets.push_back(widget.get());
            }
            return {};
        }

        void Arrange(const std::vector<std::shared_ptr<Widget>>&, Rectangle) override
        {
        }

    private:
        Widget& owner_;
        Widget& childToRemove_;
    };

    class ReentrantChildrenWidget final : public Widget
    {
    public:
        std::shared_ptr<Widget> childToAddDuringRemoval;
        std::weak_ptr<Widget> childBeingAdded;
        bool addedChildWasRetainedThroughCallback = false;
        bool removeNewChildImmediately = false;

    protected:
        void OnChildAdded(Widget& child) override
        {
            Widget::OnChildAdded(child);
            if (removeNewChildImmediately)
            {
                removeNewChildImmediately = false;
                EXPECT_TRUE(RemoveChild(&child));
                addedChildWasRetainedThroughCallback = !childBeingAdded.expired();
            }
        }

        void OnChildRemoved(Widget& child) override
        {
            Widget::OnChildRemoved(child);
            if (childToAddDuringRemoval)
            {
                std::shared_ptr<Widget> addition = std::move(childToAddDuringRemoval);
                AddChild(std::move(addition));
            }
        }
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
        widget.Arrange(Rectangle(0, 0, 40, 40));
        EXPECT_EQ(widget.GetBorderBounds(), Rectangle(1, 2, 36, 34));
        EXPECT_EQ(widget.GetBackgroundBounds(), Rectangle(3, 3, 32, 32));
        EXPECT_EQ(widget.getActualBoundsProperty(), Rectangle(6, 7, 26, 24));

        widget.setWidthProperty(50);
        widget.setHeightProperty(60);
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(50, 60));
    }

    TEST(WidgetTests, MeasureCacheUsesTheCallersAvailableSize)
    {
        FixedWidget widget(Point(20, 30));
        widget.setMarginProperty(Thickness(10, 0, 10, 0));

        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(40, 30));
        EXPECT_EQ(widget.lastAvailableSize, Point(80, 100));
        EXPECT_EQ(widget.measureCalls, 1);

        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(40, 30));
        EXPECT_EQ(widget.measureCalls, 1);

        EXPECT_EQ(widget.Measure(Point(80, 100)), Point(40, 30));
        EXPECT_EQ(widget.lastAvailableSize, Point(60, 100));
        EXPECT_EQ(widget.measureCalls, 2);
    }

    TEST(WidgetTests, ReentrantInvalidationSurvivesTheCurrentLayoutPass)
    {
        FixedWidget widget(Point(20, 30));
        widget.invalidateDuringMeasure = true;

        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(20, 30));
        EXPECT_EQ(widget.measureCalls, 1);
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(20, 30));
        EXPECT_EQ(widget.measureCalls, 2);

        bool invalidateNextArrange = true;
        widget.ArrangeUpdated += [&](void*, Myra::Events::MyraEventArgs&) {
            if (invalidateNextArrange)
            {
                invalidateNextArrange = false;
                widget.InvalidateArrange();
            }
        };
        widget.Arrange(Rectangle(0, 0, 100, 100));
        EXPECT_EQ(widget.arrangeCalls, 1);
        widget.UpdateArrange();
        EXPECT_EQ(widget.arrangeCalls, 2);
        widget.UpdateArrange();
        EXPECT_EQ(widget.arrangeCalls, 2);
    }

    TEST(WidgetTests, EnabledPropagationUsesAStableChildSnapshot)
    {
        Widget parent;
        auto first = std::make_shared<Widget>();
        auto second = std::make_shared<Widget>();
        parent.AddChild(first);
        parent.AddChild(second);
        first->EnabledChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            EXPECT_TRUE(parent.RemoveChild(second.get()));
            static_cast<void>(parent.getChildrenCopyProperty());
        };

        parent.setEnabledProperty(false);

        EXPECT_FALSE(parent.getEnabledProperty());
        EXPECT_FALSE(first->getEnabledProperty());
        EXPECT_FALSE(second->getEnabledProperty());
        EXPECT_EQ(second->getParentProperty(), nullptr);
    }

    TEST(WidgetTests, DependencyFreeBehaviorPropertiesPreserveUpstreamDefaultsAndPropagation)
    {
        FixedWidget parent(Point(10, 10));
        auto child = std::make_shared<Widget>();
        parent.AddChild(child);

        EXPECT_EQ(parent.getDragDirectionProperty(), Myra::Graphics2D::UI::DragDirection::None);
        EXPECT_FALSE(parent.getStyleNameProperty().has_value());
        EXPECT_FALSE(parent.getIsDraggableProperty());
        EXPECT_FALSE(parent.getMouseCursorProperty().has_value());
        EXPECT_FALSE(parent.getTooltipProperty().has_value());
        EXPECT_FALSE(parent.getIsModalProperty());
        EXPECT_FALSE(parent.getIsPressedProperty());
        EXPECT_FALSE(parent.getClipToBoundsProperty());
        EXPECT_FALSE(parent.getAcceptsKeyboardFocusProperty());
        EXPECT_FALSE(parent.getIsKeyboardFocusedProperty());
        EXPECT_EQ(parent.getDragHandleProperty(), &parent);
        EXPECT_FALSE(parent.getTagProperty().has_value());

        parent.setStyleNameProperty(std::string("accent"));
        parent.setDragDirectionProperty(Myra::Graphics2D::UI::DragDirection::Both);
        parent.setMouseCursorProperty(Myra::Graphics2D::UI::MouseCursorType::Hand);
        parent.setTooltipProperty(std::string());
        parent.setIsModalProperty(true);
        parent.setClipToBoundsProperty(true);
        parent.setAcceptsKeyboardFocusProperty(true);
        parent.setDragHandleProperty(child.get());
        parent.setTagProperty(42);

        EXPECT_EQ(parent.getStyleNameProperty(), std::optional<std::string>("accent"));
        EXPECT_TRUE(parent.getIsDraggableProperty());
        EXPECT_EQ(child->getMouseCursorProperty(), Myra::Graphics2D::UI::MouseCursorType::Hand);
        ASSERT_TRUE(parent.getTooltipProperty().has_value());
        EXPECT_TRUE(parent.getTooltipProperty()->empty());
        EXPECT_TRUE(parent.getIsModalProperty());
        EXPECT_TRUE(parent.getClipToBoundsProperty());
        EXPECT_TRUE(parent.getAcceptsKeyboardFocusProperty());
        EXPECT_EQ(parent.getDragHandleProperty(), child.get());
        EXPECT_EQ(std::any_cast<int>(parent.getTagProperty()), 42);

        int keyboardFocusEvents = 0;
        parent.KeyboardFocusChanged += [&](void* sender, Myra::Events::MyraEventArgs& arguments) {
            EXPECT_EQ(sender, &parent);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::KeyboardFocusChanged);
            ++keyboardFocusEvents;
        };
        parent.OnGotKeyboardFocus();
        parent.OnGotKeyboardFocus();
        EXPECT_TRUE(parent.getIsKeyboardFocusedProperty());
        EXPECT_EQ(keyboardFocusEvents, 1);
        parent.OnLostKeyboardFocus();
        EXPECT_FALSE(parent.getIsKeyboardFocusedProperty());
        EXPECT_EQ(keyboardFocusEvents, 2);

        std::vector<int> pressedEvents;
        parent.PressedChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            pressedEvents.push_back(1);
        };
        child->PressedChanged += [&](void*, Myra::Events::MyraEventArgs&) {
            pressedEvents.push_back(2);
        };
        parent.setIsPressedProperty(true);
        EXPECT_TRUE(parent.getIsPressedProperty());
        EXPECT_TRUE(child->getIsPressedProperty());
        EXPECT_EQ(pressedEvents, (std::vector<int>{2, 1}));

        parent.PressedChangingByUser += [&](void*, Myra::Events::ValueChangingEventArgs<bool>& arguments) {
            EXPECT_TRUE(arguments.getOldValueProperty());
            EXPECT_FALSE(arguments.getNewValueProperty());
            arguments.Cancel = true;
        };
        parent.SetPressedByUser(false);
        EXPECT_TRUE(parent.getIsPressedProperty());
        EXPECT_EQ(pressedEvents, (std::vector<int>{2, 1}));

        parent.PressedChangingByUser.Clear();
        parent.SetPressedByUser(false);
        EXPECT_FALSE(parent.getIsPressedProperty());
        EXPECT_FALSE(child->getIsPressedProperty());
        EXPECT_EQ(pressedEvents, (std::vector<int>{2, 1, 2, 1}));
    }

    TEST(WidgetTests, LayoutAndChildCallbacksOwnStableReentrantSnapshots)
    {
        Widget layoutOwner;
        auto first = std::make_shared<Widget>();
        auto second = std::make_shared<Widget>();
        layoutOwner.AddChild(first);
        layoutOwner.AddChild(second);
        SnapshotLayout layout(layoutOwner, *second);
        layoutOwner.setChildrenLayoutProperty(&layout);

        static_cast<void>(layoutOwner.Measure(Point(100, 100)));
        EXPECT_EQ(layout.measuredWidgets, (std::vector<Widget*>{first.get(), second.get()}));
        EXPECT_EQ(second->getParentProperty(), nullptr);

        ReentrantChildrenWidget parent;
        auto originalFirst = std::make_shared<Widget>();
        auto originalSecond = std::make_shared<Widget>();
        auto replacement = std::make_shared<Widget>();
        parent.AddChild(originalFirst);
        parent.AddChild(originalSecond);
        parent.childToAddDuringRemoval = replacement;
        parent.ClearChildren();

        EXPECT_EQ(parent.getChildrenProperty(),
            (std::vector<std::shared_ptr<Widget>>{replacement}));
        EXPECT_EQ(replacement->getParentProperty(), &parent);
        EXPECT_EQ(originalFirst->getParentProperty(), nullptr);
        EXPECT_EQ(originalSecond->getParentProperty(), nullptr);

        auto transient = std::make_shared<Widget>();
        parent.childBeingAdded = transient;
        parent.removeNewChildImmediately = true;
        parent.AddChild(std::move(transient));
        EXPECT_TRUE(parent.addedChildWasRetainedThroughCallback);
        EXPECT_TRUE(parent.childBeingAdded.expired());
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
        EXPECT_EQ(widget.getBoundsProperty(), Rectangle(0, 0, 30, 20));
        EXPECT_EQ(widget.GetBorderBounds(), Rectangle(0, 0, 30, 20));
        EXPECT_EQ(widget.GetBackgroundBounds(), Rectangle(0, 0, 30, 20));
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

        high->setZIndexProperty(0);
        const auto& reorderedChildren = firstParent->getChildrenCopyProperty();
        ASSERT_EQ(reorderedChildren.size(), 2U);
        EXPECT_EQ(reorderedChildren[0], high);
        EXPECT_EQ(reorderedChildren[1], low);

        secondParent->AddChild(high);
        ASSERT_EQ(firstParent->getChildrenProperty().size(), 1U);
        EXPECT_EQ(firstParent->getChildrenProperty().front(), low);
        ASSERT_EQ(secondParent->getChildrenProperty().size(), 1U);
        EXPECT_EQ(high->getParentProperty(), secondParent.get());

        high->RemoveFromParent();
        EXPECT_TRUE(secondParent->getChildrenProperty().empty());
        EXPECT_EQ(high->getParentProperty(), nullptr);
    }

    TEST(WidgetTests, ChildQueriesUseDepthFirstZOrderAndTypedFiltering)
    {
        Widget root;
        auto later = std::make_shared<Widget>();
        auto earlier = std::make_shared<Widget>();
        auto nested = std::make_shared<TaggedWidget>();
        auto otherTagged = std::make_shared<TaggedWidget>();
        later->setZIndexProperty(5);
        earlier->setZIndexProperty(1);
        otherTagged->setZIndexProperty(3);
        nested->setIdProperty(std::string("target"));
        otherTagged->setIdProperty(std::string("other"));
        earlier->AddChild(nested);
        root.AddChild(later);
        root.AddChild(earlier);
        root.AddChild(otherTagged);

        EXPECT_EQ(root.FindChild(), earlier.get());
        EXPECT_EQ(root.FindChildById("target"), nested.get());
        EXPECT_EQ(&root.EnsureWidgetById("target"), nested.get());
        EXPECT_EQ(root.FindChildById("missing"), nullptr);
        EXPECT_THROW(static_cast<void>(root.EnsureWidgetById("missing")), std::out_of_range);
        EXPECT_EQ(root.FindChildById<TaggedWidget>("target"), nested.get());
        EXPECT_EQ(root.FindChild<TaggedWidget>(), nested.get());
        EXPECT_EQ(root.FindChild<TaggedWidget>([](TaggedWidget& widget) {
            return widget.getIdProperty() == std::optional<std::string>("other");
        }), otherTagged.get());

        EXPECT_EQ(root.GetChildren(), (std::vector<Widget*>{earlier.get(), otherTagged.get(), later.get()}));
        EXPECT_EQ(root.GetChildren(true),
            (std::vector<Widget*>{earlier.get(), nested.get(), otherTagged.get(), later.get()}));
        EXPECT_EQ(root.GetChildren(true, [](Widget& widget) {
            return dynamic_cast<TaggedWidget*>(&widget) != nullptr;
        }), (std::vector<Widget*>{nested.get(), otherTagged.get()}));
    }

    TEST(WidgetTests, ChildCountExcludesEntireInvisibleSubtreesWhenRequested)
    {
        Widget root;
        auto visible = std::make_shared<Widget>();
        auto hidden = std::make_shared<Widget>();
        auto visibleNested = std::make_shared<Widget>();
        auto hiddenNested = std::make_shared<Widget>();
        visible->AddChild(visibleNested);
        hidden->AddChild(hiddenNested);
        hidden->setVisibleProperty(false);
        root.AddChild(visible);
        root.AddChild(hidden);

        EXPECT_EQ(root.CalculateTotalChildCount(false), 4U);
        EXPECT_EQ(root.CalculateTotalChildCount(true), 2U);
    }

    TEST(WidgetTests, ChildQueriesUseSnapshotsAndOwnershipRejectsCycles)
    {
        auto root = std::make_shared<Widget>();
        auto first = std::make_shared<Widget>();
        auto second = std::make_shared<Widget>();
        root->AddChild(first);
        root->AddChild(second);

        std::vector<Widget*> visited;
        EXPECT_EQ(root->FindChild([&](Widget& widget) {
            visited.push_back(&widget);
            if (&widget == first.get())
            {
                EXPECT_TRUE(root->RemoveChild(second.get()));
                static_cast<void>(root->getChildrenCopyProperty());
            }
            return false;
        }), nullptr);
        EXPECT_EQ(visited, (std::vector<Widget*>{first.get(), second.get()}));

        first->AddChild(second);
        EXPECT_THROW(second->AddChild(root), std::invalid_argument);
        EXPECT_EQ(root->getParentProperty(), nullptr);
        EXPECT_EQ(second->getParentProperty(), first.get());
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
        EXPECT_THROW(widget.setOpacityProperty(std::numeric_limits<float>::quiet_NaN()), std::out_of_range);
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

    TEST(WidgetTests, SuppressInvalidateMeasurePreservesTheUpstreamBatchingContract)
    {
        FixedWidget widget(Point(10, 10));
        EXPECT_FALSE(widget.GetSuppressInvalidateMeasure());
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(10, 10));
        EXPECT_EQ(widget.measureCalls, 1);

        widget.SetSuppressInvalidateMeasure(true);
        widget.InvalidateMeasure();
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(10, 10));
        EXPECT_EQ(widget.measureCalls, 1);

        widget.SetSuppressInvalidateMeasure(false);
        widget.InvalidateMeasure();
        EXPECT_EQ(widget.Measure(Point(100, 100)), Point(10, 10));
        EXPECT_EQ(widget.measureCalls, 2);
    }

    TEST(WidgetTests, RejectsOverflowingBoxAndTransformGeometry)
    {
        FixedWidget boxWidget(Point(1, 1));
        boxWidget.setMarginProperty(Thickness(std::numeric_limits<int>::max(), 0, 1, 0));
        EXPECT_THROW(static_cast<void>(boxWidget.getMBPWidthProperty()), std::overflow_error);
        EXPECT_THROW(static_cast<void>(boxWidget.Measure(Point(100, 100))), std::overflow_error);

        Widget transformWidget;
        transformWidget.setLeftProperty(std::numeric_limits<int>::max());
        transformWidget.setWidthProperty(1);
        transformWidget.setHeightProperty(1);
        transformWidget.Arrange(Rectangle(1, 0, 1, 1));
        EXPECT_THROW(static_cast<void>(transformWidget.ToGlobal(Vector2(0.0F, 0.0F))),
            std::overflow_error);
    }
}

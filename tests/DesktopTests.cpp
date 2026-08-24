// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"

namespace
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Myra::Events::CancellableEventArgsT;
    using Myra::Events::GenericEventArgs;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalMenu;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::Widget;

    class ProbeWidget final : public Widget
    {
      public:
        explicit ProbeWidget(const Point desiredSize = Point(10, 10)) : desiredSize_(desiredSize) {}

        int arrangeCalls = 0;
        int placedCalls = 0;
        int gotFocusCalls = 0;
        int lostFocusCalls = 0;
        std::function<void()> onArrange;

      protected:
        [[nodiscard]] Point InternalMeasure(Point) override { return desiredSize_; }

        void InternalArrange() override
        {
            ++arrangeCalls;
            if (onArrange)
            {
                std::function<void()> callback = std::move(onArrange);
                callback();
            }
        }

        void OnPlacedChanged() override
        {
            ++placedCalls;
            Widget::OnPlacedChanged();
        }

      public:
        void OnGotKeyboardFocus() override
        {
            ++gotFocusCalls;
            Widget::OnGotKeyboardFocus();
        }

        void OnLostKeyboardFocus() override
        {
            ++lostFocusCalls;
            Widget::OnLostKeyboardFocus();
        }

      private:
        Point desiredSize_;
    };

    void UseFixedBounds(Desktop &desktop, const Rectangle bounds = Rectangle(0, 0, 320, 200))
    {
        desktop.setBoundsFetcherProperty([bounds] { return bounds; });
    }

    TEST(DesktopTests, OwnsRootsPropagatesPlacementAndTransfersAWidgetBetweenOwners)
    {
        Desktop first;
        Desktop second;
        UseFixedBounds(first);
        UseFixedBounds(second);

        auto root = std::make_shared<Panel>();
        auto child = std::make_shared<ProbeWidget>();
        root->AddWidget(child);
        EXPECT_FALSE(root->getIsPlacedProperty());
        EXPECT_FALSE(child->getIsPlacedProperty());

        first.AddWidget(root);
        EXPECT_EQ(root->getDesktopProperty(), &first);
        EXPECT_EQ(child->getDesktopProperty(), &first);
        EXPECT_EQ(child->placedCalls, 1);
        EXPECT_THROW(first.getWidgetsProperty().Add(root), std::invalid_argument);
        EXPECT_THROW(first.getWidgetsProperty().Add(nullptr), std::invalid_argument);

        second.AddWidget(root);
        EXPECT_EQ(first.getWidgetsProperty().getCountProperty(), 0);
        EXPECT_EQ(second.getRootProperty(), root);
        EXPECT_EQ(root->getDesktopProperty(), &second);
        EXPECT_EQ(child->getDesktopProperty(), &second);

        Desktop reentrantDesktop;
        UseFixedBounds(reentrantDesktop);
        auto reentrantRoot = std::make_shared<ProbeWidget>();
        auto addedDuringPlacement = std::make_shared<ProbeWidget>();
        reentrantRoot->PlacedChanged += [&](void *, Myra::Events::MyraEventArgs &)
        {
            reentrantDesktop.AddWidget(addedDuringPlacement);
            throw std::runtime_error("reentrant placed callback failed");
        };
        EXPECT_THROW(reentrantDesktop.AddWidget(reentrantRoot), std::runtime_error);
        EXPECT_EQ(reentrantDesktop.getWidgetsProperty().getCountProperty(), 2);
        EXPECT_EQ(reentrantRoot->getDesktopProperty(), &reentrantDesktop);
        EXPECT_EQ(addedDuringPlacement->getDesktopProperty(), &reentrantDesktop);
        reentrantRoot->PlacedChanged.Clear();
        EXPECT_EQ(child->placedCalls, 3);

        auto formerRoot = std::make_shared<ProbeWidget>();
        second.AddWidget(formerRoot);
        root->AddWidget(formerRoot);
        EXPECT_EQ(second.getWidgetsProperty().getCountProperty(), 1);
        EXPECT_EQ(formerRoot->getParentProperty(), root.get());
        EXPECT_EQ(formerRoot->getDesktopProperty(), &second);

        EXPECT_TRUE(root->RemoveWidget(formerRoot.get()));
        EXPECT_FALSE(formerRoot->getIsPlacedProperty());
        EXPECT_EQ(formerRoot->getParentProperty(), nullptr);
    }

    TEST(DesktopTests, RootPropertyAndDestructorDetachRetainedWidgets)
    {
        auto root = std::make_shared<ProbeWidget>();
        {
            Desktop desktop;
            UseFixedBounds(desktop);
            desktop.setRootProperty(root);
            EXPECT_EQ(desktop.getRootProperty(), root);
            EXPECT_TRUE(root->getIsPlacedProperty());

            desktop.setRootProperty(nullptr);
            EXPECT_EQ(desktop.getRootProperty(), nullptr);
            EXPECT_FALSE(root->getIsPlacedProperty());
            desktop.setRootProperty(root);
        }
        EXPECT_FALSE(root->getIsPlacedProperty());
        EXPECT_EQ(root->placedCalls, 4);
    }

    TEST(DesktopTests, PlacementExceptionsPropagateAfterOwnershipLinksBecomeConsistent)
    {
        Desktop first;
        Desktop second;
        UseFixedBounds(first);
        UseFixedBounds(second);
        auto root = std::make_shared<Panel>();
        auto child = std::make_shared<ProbeWidget>();
        root->AddWidget(child);
        child->PlacedChanged +=
            [](void *, Myra::Events::MyraEventArgs &) { throw std::runtime_error("placed callback failed"); };

        EXPECT_THROW(first.AddWidget(root), std::runtime_error);
        EXPECT_EQ(first.getRootProperty(), root);
        EXPECT_EQ(root->getDesktopProperty(), &first);
        EXPECT_EQ(child->getDesktopProperty(), &first);

        EXPECT_THROW(second.AddWidget(root), std::runtime_error);
        EXPECT_EQ(first.getRootProperty(), nullptr);
        EXPECT_EQ(second.getRootProperty(), root);
        EXPECT_EQ(root->getDesktopProperty(), &second);
        EXPECT_EQ(child->getDesktopProperty(), &second);
    }

    TEST(DesktopTests, LayoutUsesLocalBoundsAndDiscoversMenuTraversalModalAndZOrder)
    {
        Desktop desktop;
        UseFixedBounds(desktop, Rectangle(20, 30, 200, 100));

        auto high = std::make_shared<Panel>();
        high->setZIndexProperty(5);
        high->setIdProperty(std::string("high"));
        auto menu = std::make_shared<HorizontalMenu>();
        auto hidden = std::make_shared<ProbeWidget>();
        hidden->setVisibleProperty(false);
        high->AddWidget(menu);
        high->AddWidget(hidden);

        auto low = std::make_shared<ProbeWidget>(Point(30, 20));
        low->setZIndexProperty(-1);
        desktop.AddWidget(high);
        desktop.AddWidget(low);
        desktop.UpdateLayout();

        EXPECT_EQ(desktop.getInternalBoundsProperty(), Rectangle(20, 30, 200, 100));
        EXPECT_EQ(desktop.getLayoutBoundsProperty(), Rectangle(0, 0, 200, 100));
        EXPECT_EQ(desktop.GetChild(0), low);
        EXPECT_EQ(desktop.GetChild(1), high);
        EXPECT_EQ(low->getContainerBoundsProperty(), Rectangle(0, 0, 200, 100));
        EXPECT_EQ(desktop.getMenuBarProperty(), menu.get());
        EXPECT_EQ(desktop.FindChildById("high"), high.get());
        EXPECT_EQ(desktop.FindChild([&](Widget &widget) { return &widget == hidden.get(); }), hidden.get());
        EXPECT_EQ(desktop.CalculateTotalWidgets(false), 4U);
        EXPECT_EQ(desktop.CalculateTotalWidgets(true), 3U);

        std::vector<Widget *> visited;
        desktop.ProcessWidgets(
            [&visited](Widget &widget)
            {
                visited.push_back(&widget);
                return true;
            });
        ASSERT_EQ(visited.size(), 3U);
        EXPECT_EQ(visited[0], low.get());
        EXPECT_EQ(visited[1], high.get());
        EXPECT_EQ(visited[2], menu.get());

        high->setIsModalProperty(true);
        EXPECT_TRUE(desktop.getHasModalWidgetProperty());
        high->setEnabledProperty(false);
        EXPECT_FALSE(desktop.getHasModalWidgetProperty());

        low->setZIndexProperty(10);
        EXPECT_EQ(desktop.GetChild(0), high);
        EXPECT_EQ(desktop.GetChild(1), low);
    }

    TEST(DesktopTests, ReentrantTreeChangesRemainDirtyForTheNextLayoutPass)
    {
        Desktop desktop;
        UseFixedBounds(desktop);
        auto mutating = std::make_shared<ProbeWidget>();
        auto added = std::make_shared<ProbeWidget>();
        mutating->onArrange = [&desktop, added] { desktop.AddWidget(added); };
        desktop.AddWidget(mutating);

        desktop.UpdateLayout();
        EXPECT_EQ(mutating->arrangeCalls, 1);
        EXPECT_EQ(added->arrangeCalls, 0);

        desktop.UpdateLayout();
        EXPECT_EQ(added->arrangeCalls, 1);
    }

    TEST(DesktopTests, FocusEventsCanCancelNormalChangesButDetachingAlwaysClearsFocus)
    {
        Desktop desktop;
        UseFixedBounds(desktop);
        auto root = std::make_shared<Panel>();
        auto first = std::make_shared<ProbeWidget>();
        auto second = std::make_shared<ProbeWidget>();
        root->AddWidget(first);
        root->AddWidget(second);
        desktop.AddWidget(root);

        Widget *gotData = nullptr;
        InputEventType gotType = InputEventType::None;
        desktop.WidgetGotKeyboardFocus += [&](void *, GenericEventArgs<Widget *> &arguments)
        {
            gotData = arguments.getDataProperty();
            gotType = arguments.getEventTypeProperty();
        };

        first->SetKeyboardFocus();
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), first.get());
        EXPECT_TRUE(first->getIsKeyboardFocusedProperty());
        EXPECT_EQ(first->gotFocusCalls, 1);
        EXPECT_EQ(gotData, first.get());
        EXPECT_EQ(gotType, InputEventType::KeyboardFocusLosing);

        desktop.WidgetLosingKeyboardFocus +=
            [](void *, CancellableEventArgsT<Widget *> &arguments) { arguments.Cancel = true; };
        second->SetKeyboardFocus();
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), first.get());
        EXPECT_EQ(first->lostFocusCalls, 0);
        EXPECT_EQ(second->gotFocusCalls, 0);

        desktop.WidgetLosingKeyboardFocus.Clear();
        second->SetKeyboardFocus();
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), second.get());
        EXPECT_EQ(first->lostFocusCalls, 1);
        EXPECT_EQ(second->gotFocusCalls, 1);

        desktop.WidgetLosingKeyboardFocus +=
            [](void *, CancellableEventArgsT<Widget *> &arguments) { arguments.Cancel = true; };
        EXPECT_TRUE(root->RemoveWidget(second.get()));
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), nullptr);
        EXPECT_FALSE(second->getIsKeyboardFocusedProperty());
        EXPECT_EQ(second->lostFocusCalls, 1);

        ProbeWidget detached;
        EXPECT_THROW(desktop.setFocusedKeyboardWidgetProperty(&detached), std::invalid_argument);
        EXPECT_THROW(detached.SetKeyboardFocus(), std::logic_error);

        first->SetKeyboardFocus();
        desktop.WidgetLosingKeyboardFocus.Clear();
        desktop.WidgetLosingKeyboardFocus +=
            [](void *, CancellableEventArgsT<Widget *> &) { throw std::runtime_error("focus callback failed"); };
        EXPECT_THROW(root->RemoveWidget(first.get()), std::runtime_error);
        EXPECT_EQ(desktop.getFocusedKeyboardWidgetProperty(), nullptr);
        EXPECT_FALSE(first->getIsKeyboardFocusedProperty());
        EXPECT_FALSE(first->getIsPlacedProperty());
        EXPECT_EQ(first->getParentProperty(), nullptr);
    }

    TEST(DesktopTests, DesktopTransformComposesWithRootWidgetCoordinates)
    {
        Desktop desktop;
        UseFixedBounds(desktop, Rectangle(10, 20, 100, 80));
        auto root = std::make_shared<ProbeWidget>();
        root->setLeftProperty(5);
        root->setTopProperty(6);
        desktop.AddWidget(root);
        desktop.setTransformOriginProperty(Vector2(0.0F, 0.0F));
        desktop.setScaleProperty(Vector2(2.0F, 2.0F));
        desktop.UpdateLayout();

        EXPECT_EQ(desktop.ToGlobal(Point(3, 4)), Point(16, 28));
        EXPECT_EQ(desktop.ToLocal(Point(16, 28)), Point(3, 4));
        EXPECT_EQ(root->ToGlobal(Point(0, 0)), Point(20, 32));
        EXPECT_EQ(root->ToLocal(Point(20, 32)), Point(0, 0));

        desktop.setRotationProperty(90.0F);
        const Vector2 source(7.0F, 9.0F);
        const Vector2 roundTrip = desktop.ToLocal(desktop.ToGlobal(source));
        EXPECT_NEAR(roundTrip.X, source.X, 1.0e-4F);
        EXPECT_NEAR(roundTrip.Y, source.Y, 1.0e-4F);
    }
} // namespace

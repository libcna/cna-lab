// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/SplitPane.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/Graphics2D/UI/MouseInfo.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace
{
    using Myra::Graphics2D::UI::HorizontalSplitPane;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::MouseCursorType;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::VerticalSplitPane;
    using Myra::Graphics2D::UI::Widget;

    class InputProviderGuard final
    {
      public:
        InputProviderGuard()
            : mouseInfoGetter_(Myra::MyraEnvironment::getMouseInfoGetterProperty()),
              downKeysGetter_(Myra::MyraEnvironment::getDownKeysGetterProperty()),
              eventHandlingModel_(Myra::MyraEnvironment::getEventHandlingModelProperty()),
              setMouseCursorFromWidget_(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty()),
              mouseCursor_(Myra::MyraEnvironment::getMouseCursorTypeProperty()),
              defaultMouseCursor_(Myra::MyraEnvironment::getDefaultMouseCursorTypeProperty())
        {
            Myra::MyraEnvironment::setEventHandlingModelProperty(Myra::Events::EventHandlingStrategy::EventCapturing);
            Myra::MyraEnvironment::setSetMouseCursorFromWidgetProperty(true);
            Myra::MyraEnvironment::setDefaultMouseCursorTypeProperty(MouseCursorType::Arrow);
        }

        ~InputProviderGuard()
        {
            InputEventsManager::ProcessEvents();
            Myra::MyraEnvironment::setMouseInfoGetterProperty(std::move(mouseInfoGetter_));
            Myra::MyraEnvironment::setDownKeysGetterProperty(std::move(downKeysGetter_));
            Myra::MyraEnvironment::setEventHandlingModelProperty(eventHandlingModel_);
            Myra::MyraEnvironment::setDefaultMouseCursorTypeProperty(defaultMouseCursor_);
            try
            {
                Myra::MyraEnvironment::setMouseCursorTypeProperty(mouseCursor_);
            }
            catch (...)
            {
                // Linked windowless tests still restore the backing cursor state.
            }
            Myra::MyraEnvironment::setSetMouseCursorFromWidgetProperty(setMouseCursorFromWidget_);
        }

      private:
        Myra::MyraEnvironment::MouseInfoGetter mouseInfoGetter_;
        Myra::MyraEnvironment::DownKeysGetter downKeysGetter_;
        Myra::Events::EventHandlingStrategy eventHandlingModel_;
        bool setMouseCursorFromWidget_;
        MouseCursorType mouseCursor_;
        MouseCursorType defaultMouseCursor_;
    };

    void InstallInputProviders(MouseInfo &snapshot)
    {
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&snapshot] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });
    }

    void PumpInput(Myra::Graphics2D::UI::Desktop &desktop)
    {
        desktop.UpdateInput();
        desktop.ProcessWidgetInput();
        InputEventsManager::ProcessEvents();
    }

    TEST(SplitPaneTests, BuildsTheLogicalWidgetsAndInterspersedHandleLayout)
    {
        HorizontalSplitPane splitPane;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        const auto third = std::make_shared<Widget>();
        int changed = 0;
        splitPane.ProportionsChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &splitPane);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ProportionChanged);
            ++changed;
        };

        splitPane.AddWidget(first);
        splitPane.AddWidget(second);
        splitPane.InsertWidget(1, third);
        ASSERT_EQ(splitPane.getWidgetsProperty().size(), 3U);
        EXPECT_EQ(splitPane.getWidgetsProperty()[0], first);
        EXPECT_EQ(splitPane.getWidgetsProperty()[1], third);
        EXPECT_EQ(splitPane.getWidgetsProperty()[2], second);
        EXPECT_EQ(splitPane.getChildrenProperty().size(), 5U);
        EXPECT_EQ(splitPane.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(splitPane.GetProportion(0), 1.0F);
        EXPECT_EQ(splitPane.GetProportion(1), 1.0F);
        EXPECT_EQ(splitPane.GetProportion(2), 1.0F);
        EXPECT_EQ(splitPane.GetProportion(-1), 0.0F);
        EXPECT_EQ(splitPane.GetProportion(3), 0.0F);
        EXPECT_EQ(changed, 3);
    }

    TEST(SplitPaneTests, UpdatesAdjacentSplitterProportionsAndRejectsInvalidIndices)
    {
        HorizontalSplitPane splitPane;
        splitPane.AddWidget(std::make_shared<Widget>());
        splitPane.AddWidget(std::make_shared<Widget>());
        splitPane.AddWidget(std::make_shared<Widget>());

        EXPECT_FLOAT_EQ(splitPane.GetSplitterPosition(0), 1.0F / 3.0F);
        splitPane.SetSplitterPosition(0, 0.25F);
        EXPECT_FLOAT_EQ(splitPane.GetProportion(0), 0.75F);
        EXPECT_FLOAT_EQ(splitPane.GetProportion(1), 1.25F);
        EXPECT_FLOAT_EQ(splitPane.GetProportion(2), 1.0F);
        EXPECT_FLOAT_EQ(splitPane.GetSplitterPosition(0), 0.25F);

        EXPECT_THROW(static_cast<void>(splitPane.GetSplitterPosition(-1)), std::out_of_range);
        EXPECT_THROW(static_cast<void>(splitPane.GetSplitterPosition(2)), std::out_of_range);
        EXPECT_THROW(splitPane.SetSplitterPosition(2, 0.5F), std::out_of_range);
    }

    TEST(SplitPaneTests, SupportsVerticalCollectionsRemovalAndExactTypeCloning)
    {
        VerticalSplitPane source;
        const auto first = std::make_shared<Widget>();
        const auto second = std::make_shared<Widget>();
        source.AddWidget(first);
        source.AddWidget(second);
        EXPECT_EQ(source.getOrientationProperty(), Orientation::Vertical);
        EXPECT_TRUE(source.RemoveWidget(first.get()));
        EXPECT_FALSE(source.RemoveWidget(first.get()));
        EXPECT_EQ(source.getWidgetsProperty().size(), 1U);
        EXPECT_EQ(source.getChildrenProperty().size(), 1U);

        source.AddWidget(first);
        const auto clone = std::dynamic_pointer_cast<VerticalSplitPane>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getWidgetsProperty().size(), 2U);
        EXPECT_NE(clone->getWidgetsProperty()[0], second);
        EXPECT_NE(clone->getWidgetsProperty()[1], first);
        EXPECT_EQ(clone->getChildrenProperty().size(), 3U);

        source.ClearWidgets();
        EXPECT_TRUE(source.getWidgetsProperty().empty());
        EXPECT_TRUE(source.getChildrenProperty().empty());
    }

    TEST(SplitPaneTests, HorizontalHandleDragUpdatesAdjacentProportionsOnceAndReleasesGlobally)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, true, false, false, 0.0F};
        InstallInputProviders(snapshot);

        Myra::Graphics2D::UI::Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto splitPane = std::make_shared<HorizontalSplitPane>();
        splitPane->setWidthProperty(100);
        splitPane->setHeightProperty(20);
        splitPane->setMouseCursorProperty(MouseCursorType::Hand);
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        const auto handle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        ASSERT_NE(handle, nullptr);
        handle->setWidthProperty(10);
        desktop.AddWidget(splitPane);
        desktop.UpdateLayout();
        ASSERT_EQ(handle->getBoundsProperty().Width, 10);
        ASSERT_EQ(handle->ToGlobal(Microsoft::Xna::Framework::Point(0, 0)).X, 45);

        int changed = 0;
        splitPane->ProportionsChanged += [&](void *, Myra::Events::MyraEventArgs &) { ++changed; };
        PumpInput(desktop);
        EXPECT_TRUE(handle->getIsPressedProperty());
        EXPECT_FALSE(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty());
        EXPECT_EQ(Myra::MyraEnvironment::getMouseCursorTypeProperty(), MouseCursorType::SizeWE);

        snapshot.Position = {68, 10};
        PumpInput(desktop);
        EXPECT_NEAR(splitPane->GetProportion(0), 1.4F, 0.0001F);
        EXPECT_NEAR(splitPane->GetProportion(1), 0.6F, 0.0001F);
        EXPECT_EQ(changed, 1);

        snapshot.Position = {115, 10};
        PumpInput(desktop);
        EXPECT_TRUE(handle->getIsPressedProperty());
        EXPECT_NEAR(splitPane->GetProportion(0), 1.4F, 0.0001F);
        EXPECT_NEAR(splitPane->GetProportion(1), 0.6F, 0.0001F);
        EXPECT_EQ(changed, 1);

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
        EXPECT_FALSE(handle->getIsPressedProperty());
        EXPECT_TRUE(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty());
        EXPECT_EQ(Myra::MyraEnvironment::getMouseCursorTypeProperty(), MouseCursorType::Hand);
    }

    TEST(SplitPaneTests, VerticalHandleDragUsesThePointerYCoordinate)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{10, 50}, true, false, false, 0.0F};
        InstallInputProviders(snapshot);

        Myra::Graphics2D::UI::Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 40, 120); });
        auto splitPane = std::make_shared<VerticalSplitPane>();
        splitPane->setWidthProperty(20);
        splitPane->setHeightProperty(100);
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        const auto handle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        ASSERT_NE(handle, nullptr);
        handle->setHeightProperty(10);
        desktop.AddWidget(splitPane);
        desktop.UpdateLayout();
        ASSERT_EQ(handle->ToGlobal(Microsoft::Xna::Framework::Point(0, 0)).Y, 45);

        int changed = 0;
        splitPane->ProportionsChanged += [&](void *, Myra::Events::MyraEventArgs &) { ++changed; };
        PumpInput(desktop);
        EXPECT_TRUE(handle->getIsPressedProperty());
        EXPECT_EQ(Myra::MyraEnvironment::getMouseCursorTypeProperty(), MouseCursorType::SizeNS);

        snapshot.Position = {10, 68};
        PumpInput(desktop);
        EXPECT_NEAR(splitPane->GetProportion(0), 1.4F, 0.0001F);
        EXPECT_NEAR(splitPane->GetProportion(1), 0.6F, 0.0001F);
        EXPECT_EQ(changed, 1);

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
        EXPECT_FALSE(handle->getIsPressedProperty());
        EXPECT_TRUE(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty());
    }

    TEST(SplitPaneTests, LaterHandleDragSubtractsEveryEarlierGridCell)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{0, 0}, false, false, false, 0.0F};
        InstallInputProviders(snapshot);

        Myra::Graphics2D::UI::Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 180, 40); });
        auto splitPane = std::make_shared<HorizontalSplitPane>();
        splitPane->setWidthProperty(160);
        splitPane->setHeightProperty(20);
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        const auto firstHandle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        const auto secondHandle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[3]);
        ASSERT_NE(firstHandle, nullptr);
        ASSERT_NE(secondHandle, nullptr);
        firstHandle->setWidthProperty(10);
        secondHandle->setWidthProperty(10);
        desktop.AddWidget(splitPane);
        desktop.UpdateLayout();

        const int oldSecondWidth = secondHandle->ToGlobal(Microsoft::Xna::Framework::Point(0, 0)).X -
                                   firstHandle->ToGlobal(Microsoft::Xna::Framework::Point(10, 0)).X;
        const Microsoft::Xna::Framework::Point handleCenter =
            secondHandle->ToGlobal(Microsoft::Xna::Framework::Point(5, 10));
        snapshot.Position = handleCenter;
        snapshot.IsLeftButtonDown = true;
        PumpInput(desktop);
        ASSERT_TRUE(secondHandle->getIsPressedProperty());

        int changed = 0;
        splitPane->ProportionsChanged += [&](void *, Myra::Events::MyraEventArgs &) { ++changed; };
        snapshot.Position.X += 13;
        PumpInput(desktop);
        const float expectedSecond = 3.0F * static_cast<float>(oldSecondWidth + 13) / 140.0F;
        EXPECT_FLOAT_EQ(splitPane->GetProportion(0), 1.0F);
        EXPECT_NEAR(splitPane->GetProportion(1), expectedSecond, 0.0001F);
        EXPECT_NEAR(splitPane->GetProportion(2), 2.0F - expectedSecond, 0.0001F);
        EXPECT_EQ(changed, 1);

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
    }

    TEST(SplitPaneTests, DegenerateHandleExtentDoesNotMutateProportions)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, true, false, false, 0.0F};
        InstallInputProviders(snapshot);

        Myra::Graphics2D::UI::Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto splitPane = std::make_shared<HorizontalSplitPane>();
        splitPane->setWidthProperty(100);
        splitPane->setHeightProperty(20);
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        const auto handle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        ASSERT_NE(handle, nullptr);
        handle->setWidthProperty(100);
        desktop.AddWidget(splitPane);
        desktop.UpdateLayout();

        int changed = 0;
        splitPane->ProportionsChanged += [&](void *, Myra::Events::MyraEventArgs &) { ++changed; };
        PumpInput(desktop);
        snapshot.Position = {60, 10};
        PumpInput(desktop);
        EXPECT_EQ(splitPane->GetProportion(0), 1.0F);
        EXPECT_EQ(splitPane->GetProportion(1), 1.0F);
        EXPECT_EQ(changed, 0);
        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
    }

    TEST(SplitPaneTests, RetainedHandlesBecomeInertAfterResetAndOwnerDestruction)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{0, 0}, false, false, false, 0.0F};
        InstallInputProviders(snapshot);

        Myra::Graphics2D::UI::Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto splitPane = std::make_shared<HorizontalSplitPane>();
        splitPane->setWidthProperty(100);
        splitPane->setHeightProperty(20);
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        auto oldHandle = std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        ASSERT_NE(oldHandle, nullptr);
        desktop.AddWidget(splitPane);
        desktop.UpdateLayout();

        splitPane->AddWidget(std::make_shared<Widget>());
        auto currentHandle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        ASSERT_NE(currentHandle, nullptr);
        EXPECT_NE(currentHandle, oldHandle);
        Myra::MyraEnvironment::setSetMouseCursorFromWidgetProperty(true);
        oldHandle->setIsPressedProperty(true);
        EXPECT_TRUE(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty());

        const std::weak_ptr<HorizontalSplitPane> weakSplitPane = splitPane;
        EXPECT_TRUE(desktop.RemoveWidget(splitPane.get()));
        splitPane.reset();
        static_cast<void>(desktop.getChildrenCopyProperty());
        EXPECT_TRUE(weakSplitPane.expired());
        currentHandle->setIsPressedProperty(true);
        EXPECT_TRUE(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty());
    }

    TEST(SplitPaneTests, ReentrantMoveRemovalInvalidatesTheRetainedDesktopCallbackSafely)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, true, false, false, 0.0F};
        InstallInputProviders(snapshot);

        Myra::Graphics2D::UI::Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto splitPane = std::make_shared<HorizontalSplitPane>();
        splitPane->setWidthProperty(100);
        splitPane->setHeightProperty(20);
        splitPane->AddWidget(std::make_shared<Widget>());
        splitPane->AddWidget(std::make_shared<Widget>());
        const auto handle =
            std::dynamic_pointer_cast<Myra::Graphics2D::UI::Button>(splitPane->getChildrenProperty()[1]);
        ASSERT_NE(handle, nullptr);
        handle->setWidthProperty(10);
        HorizontalSplitPane *const rawSplitPane = splitPane.get();
        const std::weak_ptr<HorizontalSplitPane> weakSplitPane = splitPane;
        desktop.TouchMoved +=
            [&](void *, Myra::Events::MyraEventArgs &) { static_cast<void>(desktop.RemoveWidget(rawSplitPane)); };
        desktop.AddWidget(splitPane);
        desktop.UpdateLayout();
        PumpInput(desktop);
        splitPane.reset();

        snapshot.Position = {68, 10};
        PumpInput(desktop);
        EXPECT_EQ(desktop.getWidgetsProperty().getCountProperty(), 0);
        static_cast<void>(desktop.getChildrenCopyProperty());
        EXPECT_TRUE(weakSplitPane.expired());
        EXPECT_TRUE(Myra::MyraEnvironment::getSetMouseCursorFromWidgetProperty());
    }
} // namespace

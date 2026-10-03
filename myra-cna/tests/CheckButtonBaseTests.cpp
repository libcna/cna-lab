// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::ButtonBase;
    using Myra::Graphics2D::UI::CheckButtonBase;
    using Myra::Graphics2D::UI::CheckPosition;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::Image;
    using Myra::Graphics2D::UI::ImageResizeMode;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;

    class TestImage final : public IImage
    {
      public:
        explicit TestImage(const Point size) : size_(size) {}

        [[nodiscard]] Point getSizeProperty() const override { return size_; }

        void Draw(RenderContext &, Rectangle, Color) const override {}

      private:
        Point size_;
    };

    class TestCheckButton final : public CheckButtonBase
    {
      public:
        TestCheckButton() = default;

      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return std::make_shared<TestCheckButton>();
        }
    };

    class InputProviderGuard final
    {
      public:
        InputProviderGuard()
            : mouseInfoGetter_(Myra::MyraEnvironment::getMouseInfoGetterProperty()),
              downKeysGetter_(Myra::MyraEnvironment::getDownKeysGetterProperty()),
              eventHandlingModel_(Myra::MyraEnvironment::getEventHandlingModelProperty())
        {
            Myra::MyraEnvironment::setEventHandlingModelProperty(Myra::Events::EventHandlingStrategy::EventCapturing);
        }

        ~InputProviderGuard()
        {
            InputEventsManager::ProcessEvents();
            Myra::MyraEnvironment::setMouseInfoGetterProperty(std::move(mouseInfoGetter_));
            Myra::MyraEnvironment::setDownKeysGetterProperty(std::move(downKeysGetter_));
            Myra::MyraEnvironment::setEventHandlingModelProperty(eventHandlingModel_);
        }

      private:
        Myra::MyraEnvironment::MouseInfoGetter mouseInfoGetter_;
        Myra::MyraEnvironment::DownKeysGetter downKeysGetter_;
        Myra::Events::EventHandlingStrategy eventHandlingModel_;
    };

    void PumpInput(Desktop &desktop)
    {
        desktop.UpdateInput();
        desktop.ProcessWidgetInput();
        InputEventsManager::ProcessEvents();
    }

    static_assert(!std::is_default_constructible_v<CheckButtonBase>);
    static_assert(static_cast<int>(CheckPosition::Left) == 0);
    static_assert(static_cast<int>(CheckPosition::Right) == 1);

    TEST(CheckButtonBaseTests, DefaultsLayoutOrderingAndSpacingMatchTheSelectedSurface)
    {
        TestCheckButton button;
        EXPECT_EQ(button.getCheckPositionProperty(), CheckPosition::Left);
        EXPECT_EQ(button.getCheckContentSpacingProperty(), 0);
        EXPECT_EQ(button.getUncheckedImageProperty(), nullptr);
        EXPECT_EQ(button.getCheckedImageProperty(), nullptr);

        const std::shared_ptr<Image> check = button.getCheckImageProperty();
        ASSERT_NE(check, nullptr);
        EXPECT_EQ(check->getHorizontalAlignmentProperty(), HorizontalAlignment::Left);
        EXPECT_EQ(check->getVerticalAlignmentProperty(), VerticalAlignment::Center);
        ASSERT_EQ(button.getChildrenProperty().size(), 1U);
        EXPECT_EQ(button.getChildrenProperty().front(), check);
        EXPECT_EQ(check->getParentProperty(), &button);

        auto unchecked = std::make_shared<TestImage>(Point(9, 7));
        auto checked = std::make_shared<TestImage>(Point(13, 11));
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(20);
        content->setHeightProperty(5);
        button.setUncheckedImageProperty(unchecked);
        button.setCheckedImageProperty(checked);
        button.setContentProperty(content);
        button.setCheckContentSpacingProperty(3);

        EXPECT_EQ(check->getRenderableProperty(), unchecked);
        ASSERT_EQ(button.getChildrenProperty().size(), 2U);
        EXPECT_EQ(button.getChildrenProperty()[0], check);
        EXPECT_EQ(button.getChildrenProperty()[1], content);
        EXPECT_EQ(button.Measure(Point(100, 100)), Point(32, 7));

        button.setCheckContentSpacingProperty(7);
        EXPECT_EQ(button.Measure(Point(100, 100)), Point(36, 7));

        button.setIsPressedProperty(true);
        EXPECT_EQ(check->getRenderableProperty(), checked);
        EXPECT_TRUE(check->getIsPressedProperty());
        EXPECT_EQ(button.Measure(Point(100, 100)), Point(40, 11));

        button.setCheckPositionProperty(CheckPosition::Right);
        ASSERT_EQ(button.getChildrenProperty().size(), 2U);
        EXPECT_EQ(button.getChildrenProperty()[0], content);
        EXPECT_EQ(button.getChildrenProperty()[1], check);

        button.setContentProperty(nullptr);
        ASSERT_EQ(button.getChildrenProperty().size(), 1U);
        EXPECT_EQ(button.getChildrenProperty().front(), check);
        EXPECT_EQ(content->getParentProperty(), nullptr);
    }

    TEST(CheckButtonBaseTests, TouchTogglesPersistentStateAndTouchUpClicks)
    {
        TestCheckButton button;
        int changes = 0;
        int clicks = 0;
        button.PressedChanged += [&](void *, Myra::Events::MyraEventArgs &) { ++changes; };
        button.Click += [&](void *, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::TouchUp);
            ++clicks;
        };

        button.OnTouchDown();
        EXPECT_TRUE(button.getIsPressedProperty());
        button.OnTouchLeft();
        EXPECT_TRUE(button.getIsPressedProperty());
        button.OnTouchUp();
        EXPECT_TRUE(button.getIsPressedProperty());
        EXPECT_EQ(changes, 1);
        EXPECT_EQ(clicks, 1);

        button.DoClick();
        EXPECT_FALSE(button.getIsPressedProperty());
        EXPECT_EQ(changes, 2);
        EXPECT_EQ(clicks, 2);

        button.setReadOnlyProperty(true);
        button.DoClick();
        EXPECT_FALSE(button.getIsPressedProperty());
        EXPECT_EQ(changes, 2);
        EXPECT_EQ(clicks, 2);
    }

    TEST(CheckButtonBaseTests, CancelledUserChangesRetainTouchClickArming)
    {
        TestCheckButton button;
        int changing = 0;
        int clicks = 0;
        const auto token = button.PressedChangingByUser.Add(
            [&](void *, Myra::Events::ValueChangingEventArgs<bool> &arguments)
            {
                EXPECT_FALSE(arguments.getOldValueProperty());
                EXPECT_TRUE(arguments.getNewValueProperty());
                ++changing;
                arguments.Cancel = true;
            });
        button.Click += [&](void *, Myra::Events::MyraEventArgs &) { ++clicks; };

        button.OnTouchDown();
        EXPECT_FALSE(button.getIsPressedProperty());
        button.OnTouchUp();
        EXPECT_EQ(changing, 1);
        EXPECT_EQ(clicks, 1);

        button.OnKeyDown(Keys::Space);
        EXPECT_FALSE(button.getIsPressedProperty());
        EXPECT_EQ(changing, 2);
        EXPECT_EQ(clicks, 1);

        EXPECT_TRUE(button.PressedChangingByUser.Remove(token));
        button.OnKeyDown(Keys::Space);
        EXPECT_TRUE(button.getIsPressedProperty());
    }

    TEST(CheckButtonBaseTests, SpaceOrdersKeyEventBeforeToggleAndPreservesReadOnlyQuirk)
    {
        TestCheckButton button;
        std::vector<std::string> calls;
        button.KeyDown += [&](void *, Myra::Events::GenericEventArgs<Keys> &arguments)
        {
            EXPECT_EQ(arguments.getDataProperty(), Keys::Space);
            calls.emplace_back("key");
        };
        button.PressedChanged += [&](void *, Myra::Events::MyraEventArgs &)
        { calls.emplace_back(button.getIsPressedProperty() ? "on" : "off"); };

        button.setReadOnlyProperty(true);
        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key", "on"}));

        calls.clear();
        button.setEnabledProperty(false);
        button.OnKeyDown(Keys::Space);
        EXPECT_EQ(calls, (std::vector<std::string>{"key"}));
        EXPECT_TRUE(button.getIsPressedProperty());
    }

    TEST(CheckButtonBaseTests, CloneDeepCopiesWidgetsAndPreservesImageConfiguration)
    {
        TestCheckButton source;
        auto unchecked = std::make_shared<TestImage>(Point(8, 6));
        auto checked = std::make_shared<TestImage>(Point(12, 10));
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(17);
        source.setUncheckedImageProperty(unchecked);
        source.setCheckedImageProperty(checked);
        source.setContentProperty(content);
        source.setCheckPositionProperty(CheckPosition::Right);
        source.setCheckContentSpacingProperty(5);
        source.getCheckImageProperty()->setColorProperty(Color(1, 2, 3, 4));
        source.getCheckImageProperty()->setResizeModeProperty(ImageResizeMode::KeepAspectRatio);
        source.setIsPressedProperty(true);
        source.setReadOnlyProperty(true);

        const std::shared_ptr<TestCheckButton> clone = std::dynamic_pointer_cast<TestCheckButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getCheckPositionProperty(), CheckPosition::Right);
        EXPECT_EQ(clone->getCheckContentSpacingProperty(), 5);
        EXPECT_EQ(clone->getUncheckedImageProperty(), unchecked);
        EXPECT_EQ(clone->getCheckedImageProperty(), checked);
        EXPECT_TRUE(clone->getIsPressedProperty());
        EXPECT_TRUE(clone->getReadOnlyProperty());
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 17);
        ASSERT_NE(clone->getCheckImageProperty(), nullptr);
        EXPECT_NE(clone->getCheckImageProperty(), source.getCheckImageProperty());
        EXPECT_EQ(clone->getCheckImageProperty()->getColorProperty(), Color(1, 2, 3, 4));
        EXPECT_EQ(clone->getCheckImageProperty()->getResizeModeProperty(), ImageResizeMode::KeepAspectRatio);
        EXPECT_EQ(clone->getCheckImageProperty()->getRenderableProperty(), checked);
        ASSERT_EQ(clone->getChildrenProperty().size(), 2U);
        EXPECT_EQ(clone->getChildrenProperty()[0], clone->getContentProperty());
        EXPECT_EQ(clone->getChildrenProperty()[1], clone->getCheckImageProperty());

        clone->setIsPressedProperty(false);
        EXPECT_EQ(clone->getCheckImageProperty()->getRenderableProperty(), unchecked);
        EXPECT_TRUE(source.getIsPressedProperty());
        EXPECT_EQ(source.getCheckImageProperty()->getRenderableProperty(), checked);
    }

    TEST(CheckButtonBaseTests, InternalImageUsesParentHoverWithPressedAndDisabledPrecedence)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{30, 10}, false, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 30); });
        auto button = std::make_shared<TestCheckButton>();
        button->setWidthProperty(100);
        button->setHeightProperty(30);
        button->setUncheckedImageProperty(std::make_shared<TestImage>(Point(10, 10)));
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(40);
        content->setHeightProperty(10);
        button->setContentProperty(content);

        const std::shared_ptr<Image> check = button->getCheckImageProperty();
        const auto normal = std::make_shared<TestImage>(Point(1, 1));
        const auto over = std::make_shared<TestImage>(Point(1, 1));
        const auto pressed = std::make_shared<TestImage>(Point(1, 1));
        const auto disabled = std::make_shared<TestImage>(Point(1, 1));
        check->setBackgroundProperty(normal);
        check->setOverBackgroundProperty(over);
        check->setPressedBackgroundProperty(pressed);
        check->setDisabledBackgroundProperty(disabled);
        desktop.AddWidget(button);
        desktop.UpdateLayout();

        PumpInput(desktop);
        EXPECT_TRUE(button->getIsMouseInsideProperty());
        EXPECT_FALSE(check->getIsMouseInsideProperty());
        EXPECT_EQ(check->GetCurrentBackground(), over);

        button->setIsPressedProperty(true);
        EXPECT_TRUE(check->getIsPressedProperty());
        EXPECT_EQ(check->GetCurrentBackground(), pressed);

        button->setEnabledProperty(false);
        EXPECT_FALSE(check->getEnabledProperty());
        EXPECT_EQ(check->GetCurrentBackground(), disabled);

        button->setEnabledProperty(true);
        button->setIsPressedProperty(false);
        EXPECT_EQ(check->GetCurrentBackground(), over);
    }

    TEST(CheckButtonBaseTests, ClonedAndDetachedInternalImageKeepsTheHoverOverride)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{30, 10}, false, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        TestCheckButton source;
        source.setWidthProperty(100);
        source.setHeightProperty(30);
        source.setUncheckedImageProperty(std::make_shared<TestImage>(Point(10, 10)));
        auto content = std::make_shared<Widget>();
        content->setWidthProperty(40);
        content->setHeightProperty(10);
        source.setContentProperty(content);
        const auto normal = std::make_shared<TestImage>(Point(1, 1));
        const auto over = std::make_shared<TestImage>(Point(1, 1));
        source.getCheckImageProperty()->setBackgroundProperty(normal);
        source.getCheckImageProperty()->setOverBackgroundProperty(over);

        const std::shared_ptr<TestCheckButton> clone = std::dynamic_pointer_cast<TestCheckButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        const std::shared_ptr<Image> check = clone->getCheckImageProperty();
        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 30); });
        desktop.AddWidget(clone);
        desktop.UpdateLayout();
        PumpInput(desktop);

        EXPECT_TRUE(clone->getIsMouseInsideProperty());
        EXPECT_FALSE(check->getIsMouseInsideProperty());
        EXPECT_EQ(check->GetCurrentBackground(), over);

        ASSERT_TRUE(clone->RemoveChild(check.get()));
        ASSERT_TRUE(desktop.RemoveWidget(clone.get()));
        check->setWidthProperty(20);
        check->setHeightProperty(20);
        desktop.AddWidget(check);
        desktop.UpdateLayout();
        snapshot.Position = Point(5, 5);
        PumpInput(desktop);

        EXPECT_TRUE(check->getIsMouseInsideProperty());
        EXPECT_EQ(check->GetCurrentBackground(), over);
    }

    TEST(CheckButtonBaseTests, RegistersAbstractMetadataAndExternalImageProperties)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(CheckButtonBase));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "CheckButtonBase");
        EXPECT_FALSE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(ButtonBase)));

        const PropertyDescriptor *position = registry.FindPropertyByName(typeid(CheckButtonBase), "CheckPosition");
        const PropertyDescriptor *spacing = registry.FindPropertyByName(typeid(CheckButtonBase), "CheckContentSpacing");
        const PropertyDescriptor *unchecked = registry.FindPropertyByName(typeid(CheckButtonBase), "UncheckedImage");
        const PropertyDescriptor *checked = registry.FindPropertyByName(typeid(CheckButtonBase), "CheckedImage");
        const PropertyDescriptor *checkImage = registry.FindPropertyByName(typeid(CheckButtonBase), "CheckImage");
        const PropertyDescriptor *content = registry.FindPropertyByName(typeid(CheckButtonBase), "Content");
        ASSERT_NE(position, nullptr);
        ASSERT_NE(spacing, nullptr);
        ASSERT_NE(unchecked, nullptr);
        ASSERT_NE(checked, nullptr);
        ASSERT_NE(checkImage, nullptr);
        ASSERT_NE(content, nullptr);

        EXPECT_EQ(std::any_cast<CheckPosition>(*position->getDefaultValueProperty()), CheckPosition::Left);
        EXPECT_EQ(std::any_cast<int>(*spacing->getDefaultValueProperty()), 0);
        ASSERT_TRUE(spacing->getMetadataProperty().StylePropertyPath.has_value());
        EXPECT_EQ(*spacing->getMetadataProperty().StylePropertyPath, "ImageTextSpacing");
        EXPECT_TRUE(unchecked->getMetadataProperty().ExternalAsset);
        EXPECT_TRUE(checked->getMetadataProperty().ExternalAsset);
        EXPECT_EQ(*unchecked->getMetadataProperty().StylePropertyPath, "ImageStyle/Image");
        EXPECT_EQ(*checked->getMetadataProperty().StylePropertyPath, "ImageStyle/PressedImage");
        EXPECT_TRUE(checkImage->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(checkImage->getCanReadProperty());
        EXPECT_FALSE(checkImage->getCanWriteProperty());
        EXPECT_TRUE(content->getMetadataProperty().Content);

        TestCheckButton button;
        position->Set(static_cast<CheckButtonBase *>(&button), CheckPosition::Right);
        spacing->Set(static_cast<CheckButtonBase *>(&button), 4);
        EXPECT_EQ(button.getCheckPositionProperty(), CheckPosition::Right);
        EXPECT_EQ(button.getCheckContentSpacingProperty(), 4);
        EXPECT_EQ(std::any_cast<std::shared_ptr<Image>>(checkImage->Get(static_cast<const CheckButtonBase *>(&button))),
                  button.getCheckImageProperty());
    }
} // namespace

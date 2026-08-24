// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Range/HorizontalSlider.hpp"
#include "Myra/Graphics2D/UI/Range/VerticalSlider.hpp"

#include <gtest/gtest.h>

#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <utility>

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Events/ValueChangedEventArgs.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/Graphics2D/UI/MouseInfo.hpp"
#include "Myra/Graphics2D/UI/Range/Slider.hpp"
#include "Myra/Graphics2D/UI/Simple/Button.hpp"
#include "Myra/Graphics2D/UI/Simple/Image.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Myra::Events::ValueChangedEventArgs;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::HorizontalSlider;
    using Myra::Graphics2D::UI::Image;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::Slider;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalSlider;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

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

    TEST(SliderTests, ConcreteDefaultsAndRetainedKnobsMatchTheSelectedSurface)
    {
        HorizontalSlider horizontal;
        EXPECT_EQ(horizontal.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(horizontal.getMinimumProperty(), 0.0F);
        EXPECT_EQ(horizontal.getMaximumProperty(), 100.0F);
        EXPECT_EQ(horizontal.getValueProperty(), 0.0F);
        EXPECT_FALSE(horizontal.getWheelAdjustmentProperty());
        EXPECT_EQ(horizontal.getWheelStepProperty(), 1.0F);
        EXPECT_EQ(horizontal.getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(horizontal.getVerticalAlignmentProperty(), VerticalAlignment::Top);
        ASSERT_NE(horizontal.getImageButtonProperty(), nullptr);
        EXPECT_TRUE(std::dynamic_pointer_cast<Image>(horizontal.getImageButtonProperty()->getContentProperty()));

        VerticalSlider vertical;
        EXPECT_EQ(vertical.getOrientationProperty(), Orientation::Vertical);
        EXPECT_EQ(vertical.getHorizontalAlignmentProperty(), HorizontalAlignment::Left);
        EXPECT_EQ(vertical.getVerticalAlignmentProperty(), VerticalAlignment::Stretch);
        ASSERT_NE(vertical.getImageButtonProperty(), nullptr);
    }

    TEST(SliderTests, ValueClampsToTheCurrentRangeAndRaisesTypedEvents)
    {
        HorizontalSlider slider;
        slider.setMinimumProperty(10.0F);
        slider.setMaximumProperty(20.0F);

        int calls = 0;
        float oldValue = 0.0F;
        float newValue = 0.0F;
        void *sender = nullptr;
        const auto token = slider.ValueChanged.Add(
            [&](void *receivedSender, ValueChangedEventArgs<float> &arguments)
            {
                ++calls;
                sender = receivedSender;
                oldValue = arguments.getOldValueProperty();
                newValue = arguments.getNewValueProperty();
            });

        slider.setValueProperty(-5.0F);
        EXPECT_EQ(slider.getValueProperty(), 10.0F);
        EXPECT_EQ(calls, 1);
        EXPECT_EQ(oldValue, 0.0F);
        EXPECT_EQ(newValue, 10.0F);
        EXPECT_EQ(sender, &slider);

        slider.setValueProperty(50.0F);
        EXPECT_EQ(slider.getValueProperty(), 20.0F);
        EXPECT_EQ(calls, 2);
        EXPECT_EQ(oldValue, 10.0F);
        EXPECT_EQ(newValue, 20.0F);
        slider.setValueProperty(20.0F);
        EXPECT_EQ(calls, 2);

        slider.setMaximumProperty(5.0F);
        EXPECT_EQ(slider.getValueProperty(), 20.0F);
        slider.setValueProperty(7.0F);
        EXPECT_EQ(slider.getValueProperty(), 10.0F);
        EXPECT_EQ(calls, 3);
        EXPECT_TRUE(slider.ValueChanged.Remove(token));
    }

    TEST(SliderTests, NonFiniteKnobSynchronizationUsesTheSafeZeroHint)
    {
        HorizontalSlider slider;
        slider.getImageButtonProperty()->setLeftProperty(7);
        slider.setValueProperty(std::numeric_limits<float>::quiet_NaN());
        EXPECT_EQ(slider.getImageButtonProperty()->getLeftProperty(), 0);

        slider.getImageButtonProperty()->setLeftProperty(9);
        slider.setMinimumProperty(std::numeric_limits<float>::infinity());
        slider.setValueProperty(3.0F);
        EXPECT_EQ(slider.getImageButtonProperty()->getLeftProperty(), 0);
    }

    TEST(SliderTests, RejectsAnOverflowingKnobTravelRangeDeterministically)
    {
        HorizontalSlider slider;
        slider.getImageButtonProperty()->setWidthProperty(std::numeric_limits<int>::min());
        slider.getImageButtonProperty()->setHeightProperty(1);

        EXPECT_THROW(slider.Arrange(Microsoft::Xna::Framework::Rectangle(0, 0, std::numeric_limits<int>::max(), 20)),
                     std::overflow_error);
    }

    TEST(SliderTests, ClonePreservesConcreteStateAndDeepCopiesTheKnob)
    {
        VerticalSlider source;
        source.setMinimumProperty(-5.0F);
        source.setMaximumProperty(25.0F);
        source.setValueProperty(12.0F);
        source.setWheelAdjustmentProperty(true);
        source.setWheelStepProperty(2.5F);
        source.getImageButtonProperty()->setReadOnlyProperty(true);
        source.getImageButtonProperty()->setContentProperty(std::make_shared<Image>());

        const std::shared_ptr<VerticalSlider> clone = std::dynamic_pointer_cast<VerticalSlider>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getMinimumProperty(), -5.0F);
        EXPECT_EQ(clone->getMaximumProperty(), 25.0F);
        EXPECT_EQ(clone->getValueProperty(), 12.0F);
        EXPECT_TRUE(clone->getWheelAdjustmentProperty());
        EXPECT_EQ(clone->getWheelStepProperty(), 2.5F);
        ASSERT_NE(clone->getImageButtonProperty(), nullptr);
        EXPECT_NE(clone->getImageButtonProperty(), source.getImageButtonProperty());
        EXPECT_TRUE(clone->getImageButtonProperty()->getReadOnlyProperty());
        EXPECT_NE(clone->getImageButtonProperty()->getContentProperty(),
                  source.getImageButtonProperty()->getContentProperty());

        clone->getImageButtonProperty()->setIsPressedProperty(true);
        clone->getImageButtonProperty()->OnTouchLeft();
        EXPECT_TRUE(clone->getImageButtonProperty()->getIsPressedProperty());
    }

    TEST(SliderTests, DesktopWheelRoutingPreservesUserAndGeneralEventSemantics)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, false, false, false, -120.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto slider = std::make_shared<HorizontalSlider>();
        slider->setWidthProperty(100);
        slider->setHeightProperty(20);
        slider->setMinimumProperty(0.0F);
        slider->setMaximumProperty(10.0F);
        slider->setValueProperty(5.0F);
        slider->setWheelAdjustmentProperty(true);
        slider->setWheelStepProperty(2.0F);
        slider->getImageButtonProperty()->setWidthProperty(10);
        slider->getImageButtonProperty()->setHeightProperty(10);

        int generalCalls = 0;
        int userCalls = 0;
        slider->ValueChanged += [&](void *, ValueChangedEventArgs<float> &arguments)
        {
            ++generalCalls;
            EXPECT_EQ(arguments.getOldValueProperty(), 5.0F);
            EXPECT_EQ(arguments.getNewValueProperty(), 3.0F);
        };
        slider->ValueChangedByUser += [&](void *, ValueChangedEventArgs<float> &arguments)
        {
            ++userCalls;
            EXPECT_EQ(arguments.getOldValueProperty(), 5.0F);
            EXPECT_EQ(arguments.getNewValueProperty(), 3.0F);
        };

        desktop.AddWidget(slider);
        desktop.UpdateLayout();
        PumpInput(desktop);

        EXPECT_EQ(slider->getValueProperty(), 3.0F);
        EXPECT_EQ(generalCalls, 2);
        EXPECT_EQ(userCalls, 1);

        slider->setWheelAdjustmentProperty(false);
        snapshot.Wheel = -240.0F;
        PumpInput(desktop);
        EXPECT_EQ(slider->getValueProperty(), 3.0F);
        EXPECT_EQ(generalCalls, 2);
        EXPECT_EQ(userCalls, 1);
    }

    TEST(SliderTests, DragTracksTheDesktopOutsideTheSliderAndGlobalReleaseStopsCapture)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto slider = std::make_shared<HorizontalSlider>();
        slider->setWidthProperty(100);
        slider->setHeightProperty(20);
        slider->getImageButtonProperty()->setWidthProperty(10);
        slider->getImageButtonProperty()->setHeightProperty(10);
        int generalCalls = 0;
        int userCalls = 0;
        slider->ValueChanged += [&](void *, ValueChangedEventArgs<float> &) { ++generalCalls; };
        slider->ValueChangedByUser += [&](void *, ValueChangedEventArgs<float> &) { ++userCalls; };
        desktop.AddWidget(slider);
        desktop.UpdateLayout();

        PumpInput(desktop);
        EXPECT_FLOAT_EQ(slider->getValueProperty(), 50.0F);
        EXPECT_EQ(slider->getImageButtonProperty()->getLeftProperty(), 45);
        EXPECT_TRUE(slider->getImageButtonProperty()->getIsPressedProperty());
        EXPECT_EQ(generalCalls, 1);
        EXPECT_EQ(userCalls, 1);

        snapshot.Position = {115, 10};
        PumpInput(desktop);
        EXPECT_FLOAT_EQ(slider->getValueProperty(), 100.0F);
        EXPECT_EQ(slider->getImageButtonProperty()->getLeftProperty(), 90);
        EXPECT_TRUE(slider->getImageButtonProperty()->getIsPressedProperty());
        EXPECT_EQ(generalCalls, 2);
        EXPECT_EQ(userCalls, 2);

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
        EXPECT_FALSE(slider->getImageButtonProperty()->getIsPressedProperty());

        snapshot.IsLeftButtonDown = true;
        PumpInput(desktop);
        snapshot.Position = {10, 10};
        PumpInput(desktop);
        EXPECT_FLOAT_EQ(slider->getValueProperty(), 100.0F);
        EXPECT_EQ(generalCalls, 2);
        EXPECT_EQ(userCalls, 2);
        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
    }

    TEST(SliderTests, VerticalTouchUsesTheYCoordinateAndKnobHeight)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{10, 73}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 40, 120); });
        auto slider = std::make_shared<VerticalSlider>();
        slider->setWidthProperty(20);
        slider->setHeightProperty(100);
        slider->getImageButtonProperty()->setWidthProperty(10);
        slider->getImageButtonProperty()->setHeightProperty(10);
        desktop.AddWidget(slider);
        desktop.UpdateLayout();

        PumpInput(desktop);
        EXPECT_EQ(slider->getImageButtonProperty()->getTopProperty(), 68);
        EXPECT_FLOAT_EQ(slider->getValueProperty(), 68.0F * 100.0F / 90.0F);

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
    }

    TEST(SliderTests, ReentrantMoveRemovalInvalidatesTheRetainedSliderCallbackSafely)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto slider = std::make_shared<HorizontalSlider>();
        slider->setWidthProperty(100);
        slider->setHeightProperty(20);
        slider->getImageButtonProperty()->setWidthProperty(10);
        slider->getImageButtonProperty()->setHeightProperty(10);
        Slider *const rawSlider = slider.get();
        const std::weak_ptr<Slider> weakSlider = slider;
        desktop.TouchMoved +=
            [&](void *, Myra::Events::MyraEventArgs &) { static_cast<void>(desktop.RemoveWidget(rawSlider)); };
        desktop.AddWidget(slider);
        desktop.UpdateLayout();
        PumpInput(desktop);
        slider.reset();

        snapshot.Position = {80, 10};
        PumpInput(desktop);
        EXPECT_EQ(desktop.getWidgetsProperty().getCountProperty(), 0);
        static_cast<void>(desktop.getChildrenCopyProperty());
        EXPECT_TRUE(weakSlider.expired());
    }

    TEST(SliderTests, ReentrantReleaseRemovalInvalidatesTheRetainedKnobCallbackSafely)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{50, 10}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Microsoft::Xna::Framework::Rectangle(0, 0, 120, 40); });
        auto slider = std::make_shared<HorizontalSlider>();
        slider->setWidthProperty(100);
        slider->setHeightProperty(20);
        slider->getImageButtonProperty()->setWidthProperty(10);
        slider->getImageButtonProperty()->setHeightProperty(10);
        Slider *const rawSlider = slider.get();
        const std::weak_ptr<Slider> weakSlider = slider;
        const std::weak_ptr<Myra::Graphics2D::UI::Button> weakKnob = slider->getImageButtonProperty();
        desktop.TouchUp +=
            [&](void *, Myra::Events::MyraEventArgs &) { static_cast<void>(desktop.RemoveWidget(rawSlider)); };
        desktop.AddWidget(slider);
        desktop.UpdateLayout();
        PumpInput(desktop);
        slider.reset();

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
        EXPECT_EQ(desktop.getWidgetsProperty().getCountProperty(), 0);
        static_cast<void>(desktop.getChildrenCopyProperty());
        EXPECT_TRUE(weakSlider.expired());
        EXPECT_TRUE(weakKnob.expired());
    }

    TEST(SliderTests, RegistersAndRoundTripsTheConcreteRangeMml)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(HorizontalSlider));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_EQ(descriptor->getNameProperty(), "HorizontalSlider");
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Slider)));
        const PropertyDescriptor *orientation = registry.FindPropertyByName(typeid(HorizontalSlider), "Orientation");
        const PropertyDescriptor *imageButton = registry.FindPropertyByName(typeid(HorizontalSlider), "ImageButton");
        ASSERT_NE(orientation, nullptr);
        ASSERT_NE(imageButton, nullptr);
        EXPECT_TRUE(orientation->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(imageButton->getMetadataProperty().XmlIgnore);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<HorizontalSlider Minimum=\"5\" Maximum=\"20\" Value=\"9\" "
                         "WheelAdjustment=\"True\" WheelStep=\"2.5\" />");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(HorizontalSlider));
        const auto *slider = static_cast<const HorizontalSlider *>(loaded.Value.get());
        EXPECT_EQ(slider->getMinimumProperty(), 5.0F);
        EXPECT_EQ(slider->getMaximumProperty(), 20.0F);
        EXPECT_EQ(slider->getValueProperty(), 9.0F);
        EXPECT_TRUE(slider->getWheelAdjustmentProperty());
        EXPECT_EQ(slider->getWheelStepProperty(), 2.5F);

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(slider, typeid(HorizontalSlider));
        EXPECT_NE(xml.find("Minimum=\"5\""), std::string::npos);
        EXPECT_NE(xml.find("Maximum=\"20\""), std::string::npos);
        EXPECT_NE(xml.find("Value=\"9\""), std::string::npos);
        EXPECT_NE(xml.find("WheelAdjustment=\"True\""), std::string::npos);
        EXPECT_NE(xml.find("WheelStep=\"2.5\""), std::string::npos);
        EXPECT_EQ(xml.find("ImageButton="), std::string::npos);
        EXPECT_EQ(xml.find("Orientation="), std::string::npos);
    }
} // namespace

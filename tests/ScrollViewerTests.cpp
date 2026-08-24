// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Containers/ScrollViewer.hpp"

#include <gtest/gtest.h>

#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <utility>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::ScrollViewer;
    using Myra::Graphics2D::UI::Widget;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
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

    void PumpInput(Desktop &desktop)
    {
        desktop.UpdateInput();
        desktop.ProcessWidgetInput();
        InputEventsManager::ProcessEvents();
    }

    std::shared_ptr<ScrollViewer> CreateScrollableViewer()
    {
        auto viewer = std::make_shared<ScrollViewer>();
        viewer->setContentProperty(std::make_shared<FixedWidget>(Point(300, 200)));
        viewer->setHorizontalScrollKnobProperty(std::make_shared<TestImage>(Point(8, 12)));
        viewer->setVerticalScrollKnobProperty(std::make_shared<TestImage>(Point(10, 9)));
        return viewer;
    }

    TEST(ScrollViewerTests, ArrangesOversizedContentAndClampsScrollPositionThroughImageThumbs)
    {
        InputProviderGuard guard;
        auto viewer = std::make_shared<ScrollViewer>();
        const auto content = std::make_shared<FixedWidget>(Point(300, 200));
        viewer->setContentProperty(content);
        viewer->setHorizontalScrollKnobProperty(std::make_shared<TestImage>(Point(8, 12)));
        viewer->setVerticalScrollKnobProperty(std::make_shared<TestImage>(Point(10, 9)));

        viewer->Arrange(Rectangle(0, 0, 100, 100));
        EXPECT_TRUE(viewer->getHorizontalScrollingOnProperty());
        EXPECT_TRUE(viewer->getVerticalScrollingOnProperty());
        EXPECT_EQ(viewer->getScrollMaximumProperty(), Point(210, 112));

        viewer->setScrollPositionProperty(Point(999, 999));
        viewer->InvalidateArrange();
        viewer->UpdateArrange();
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(210, 112));
        EXPECT_EQ(content->getLeftProperty(), -210);
        EXPECT_EQ(content->getTopProperty(), -112);

        viewer->ResetScroll();
        viewer->OnMouseWheel(-1.0F);
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(0, 22));

        viewer->ResetScroll();
        Myra::MyraEnvironment::setMouseInfoGetterProperty([]
                                                          { return MouseInfo{{10, 10}, false, false, false, -1.0F}; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });
        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 100); });
        desktop.AddWidget(viewer);
        desktop.UpdateLayout();
        desktop.UpdateInput();
        desktop.ProcessWidgetInput();
        InputEventsManager::ProcessEvents();
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(0, 22));
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

    TEST(ScrollViewerTests, RegistersAndRoundTripsContentAndExternalImageMml)
    {
        const Myra::MML::TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor *descriptor = registry.FindByType(typeid(ScrollViewer));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_TRUE(descriptor->getCanCreateProperty());
        ASSERT_TRUE(descriptor->getBaseTypeProperty().has_value());
        EXPECT_EQ(*descriptor->getBaseTypeProperty(), std::type_index(typeid(Myra::Graphics2D::UI::ContentControl)));

        const PropertyDescriptor *content = registry.FindPropertyByName(typeid(ScrollViewer), "Content");
        const PropertyDescriptor *scrollMaximum = registry.FindPropertyByName(typeid(ScrollViewer), "ScrollMaximum");
        const PropertyDescriptor *scrollPosition = registry.FindPropertyByName(typeid(ScrollViewer), "ScrollPosition");
        const PropertyDescriptor *horizontalBackground =
            registry.FindPropertyByName(typeid(ScrollViewer), "HorizontalScrollBackground");
        const PropertyDescriptor *horizontalAlignment =
            registry.FindPropertyByName(typeid(ScrollViewer), "HorizontalAlignment");
        const PropertyDescriptor *verticalAlignment =
            registry.FindPropertyByName(typeid(ScrollViewer), "VerticalAlignment");
        const PropertyDescriptor *clipToBounds = registry.FindPropertyByName(typeid(ScrollViewer), "ClipToBounds");
        ASSERT_NE(content, nullptr);
        ASSERT_NE(scrollMaximum, nullptr);
        ASSERT_NE(scrollPosition, nullptr);
        ASSERT_NE(horizontalBackground, nullptr);
        ASSERT_NE(horizontalAlignment, nullptr);
        ASSERT_NE(verticalAlignment, nullptr);
        ASSERT_NE(clipToBounds, nullptr);
        EXPECT_TRUE(content->getMetadataProperty().Content);
        EXPECT_TRUE(scrollMaximum->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(scrollPosition->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(horizontalBackground->getMetadataProperty().ExternalAsset);
        EXPECT_TRUE(horizontalBackground->getCanBeNullProperty());
        EXPECT_EQ(
            std::any_cast<Myra::Graphics2D::UI::HorizontalAlignment>(*horizontalAlignment->getDefaultValueProperty()),
            Myra::Graphics2D::UI::HorizontalAlignment::Stretch);
        EXPECT_EQ(std::any_cast<Myra::Graphics2D::UI::VerticalAlignment>(*verticalAlignment->getDefaultValueProperty()),
                  Myra::Graphics2D::UI::VerticalAlignment::Stretch);
        EXPECT_TRUE(std::any_cast<bool>(*clipToBounds->getDefaultValueProperty()));

        const auto horizontalBackgroundImage = std::make_shared<TestImage>(Point(11, 12));
        const auto horizontalKnobImage = std::make_shared<TestImage>(Point(13, 14));
        const auto verticalBackgroundImage = std::make_shared<TestImage>(Point(15, 16));
        const auto verticalKnobImage = std::make_shared<TestImage>(Point(17, 18));
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        loader.LoadExternalAsset = [&](const PropertyDescriptor &, const std::string &assetName) -> std::any
        {
            if (assetName == "atlas:hbg")
            {
                return std::shared_ptr<IImage>(horizontalBackgroundImage);
            }
            if (assetName == "atlas:hknob")
            {
                return std::shared_ptr<IImage>(horizontalKnobImage);
            }
            if (assetName == "atlas:vbg")
            {
                return std::shared_ptr<IImage>(verticalBackgroundImage);
            }
            if (assetName == "atlas:vknob")
            {
                return std::shared_ptr<IImage>(verticalKnobImage);
            }
            throw std::invalid_argument("Unexpected ScrollViewer test asset.");
        };

        System::Xml::XmlDocument document;
        document.LoadXml("<ScrollViewer ScrollMultiplier=\"17\" ShowHorizontalScrollBar=\"False\" "
                         "ShowVerticalScrollBar=\"False\" HorizontalScrollBackground=\"atlas:hbg\" "
                         "HorizontalScrollKnob=\"atlas:hknob\" VerticalScrollBackground=\"atlas:vbg\" "
                         "VerticalScrollKnob=\"atlas:vknob\"><Panel Width=\"25\"/></ScrollViewer>");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(ScrollViewer));
        const auto *viewer = static_cast<const ScrollViewer *>(loaded.Value.get());
        EXPECT_EQ(viewer->getScrollMultiplierProperty(), 17);
        EXPECT_FALSE(viewer->getShowHorizontalScrollBarProperty());
        EXPECT_FALSE(viewer->getShowVerticalScrollBarProperty());
        EXPECT_EQ(viewer->getHorizontalScrollBackgroundProperty(), horizontalBackgroundImage);
        EXPECT_EQ(viewer->getHorizontalScrollKnobProperty(), horizontalKnobImage);
        EXPECT_EQ(viewer->getVerticalScrollBackgroundProperty(), verticalBackgroundImage);
        EXPECT_EQ(viewer->getVerticalScrollKnobProperty(), verticalKnobImage);
        const auto loadedPanel = std::dynamic_pointer_cast<Panel>(viewer->getContentProperty());
        ASSERT_NE(loadedPanel, nullptr);
        EXPECT_EQ(loadedPanel->getWidthProperty(), 25);

        SaveContext saver(registry, codecs);
        saver.SaveExternalAsset = [&](const PropertyDescriptor &, const std::any &value)
        {
            const std::shared_ptr<IImage> &image = std::any_cast<const std::shared_ptr<IImage> &>(value);
            if (image == horizontalBackgroundImage)
            {
                return std::string("atlas:hbg");
            }
            if (image == horizontalKnobImage)
            {
                return std::string("atlas:hknob");
            }
            if (image == verticalBackgroundImage)
            {
                return std::string("atlas:vbg");
            }
            if (image == verticalKnobImage)
            {
                return std::string("atlas:vknob");
            }
            throw std::invalid_argument("Unexpected ScrollViewer image during save.");
        };
        const std::string xml = saver.ToXml(viewer, typeid(ScrollViewer));
        EXPECT_NE(xml.find("ScrollMultiplier=\"17\""), std::string::npos);
        EXPECT_NE(xml.find("ShowHorizontalScrollBar=\"False\""), std::string::npos);
        EXPECT_NE(xml.find("ShowVerticalScrollBar=\"False\""), std::string::npos);
        EXPECT_NE(xml.find("HorizontalScrollBackground=\"atlas:hbg\""), std::string::npos);
        EXPECT_NE(xml.find("HorizontalScrollKnob=\"atlas:hknob\""), std::string::npos);
        EXPECT_NE(xml.find("VerticalScrollBackground=\"atlas:vbg\""), std::string::npos);
        EXPECT_NE(xml.find("VerticalScrollKnob=\"atlas:vknob\""), std::string::npos);
        EXPECT_NE(xml.find("<Panel Width=\"25\""), std::string::npos);
        EXPECT_EQ(xml.find("ScrollMaximum="), std::string::npos);
        EXPECT_EQ(xml.find("ScrollPosition="), std::string::npos);
        EXPECT_EQ(xml.find("HorizontalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("VerticalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("ClipToBounds="), std::string::npos);
    }

    TEST(ScrollViewerTests, DesktopThumbDragTracksOutsideBoundsAndGlobalReleaseStopsCapture)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{10, 94}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 100); });
        const std::shared_ptr<ScrollViewer> viewer = CreateScrollableViewer();
        desktop.AddWidget(viewer);
        desktop.UpdateLayout();

        PumpInput(desktop);
        snapshot.Position = {31, 94};
        PumpInput(desktop);
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(70, 0));

        snapshot.Position = {115, 94};
        PumpInput(desktop);
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(210, 0));

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
        snapshot.IsLeftButtonDown = true;
        PumpInput(desktop);
        snapshot.Position = {0, 94};
        PumpInput(desktop);
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(210, 0));
        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);

        viewer->ResetScroll();
        snapshot.Position = {50, 94};
        snapshot.IsLeftButtonDown = true;
        PumpInput(desktop);
        snapshot.Position = {60, 94};
        PumpInput(desktop);
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(0, 0));
        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);

        snapshot.Position = {95, 10};
        snapshot.IsLeftButtonDown = true;
        PumpInput(desktop);
        snapshot.Position = {95, 35};
        PumpInput(desktop);
        EXPECT_EQ(viewer->getScrollPositionProperty(), Point(0, 56));
        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
    }

    TEST(ScrollViewerTests, TransparentContentFallsThroughExceptOnScrollbarFrames)
    {
        const std::shared_ptr<ScrollViewer> viewer = CreateScrollableViewer();
        viewer->Arrange(Rectangle(0, 0, 100, 100));

        EXPECT_TRUE(viewer->InputFallsThrough(Point(50, 50)));
        EXPECT_FALSE(viewer->InputFallsThrough(Point(50, 94)));
        EXPECT_FALSE(viewer->InputFallsThrough(Point(95, 50)));

        viewer->setBackgroundProperty(std::make_shared<TestImage>(Point(1, 1)));
        EXPECT_FALSE(viewer->InputFallsThrough(Point(50, 50)));
    }

    TEST(ScrollViewerTests, ReentrantMoveRemovalInvalidatesTheRetainedCaptureSafely)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{10, 94}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 100); });
        std::shared_ptr<ScrollViewer> viewer = CreateScrollableViewer();
        ScrollViewer *const rawViewer = viewer.get();
        const std::weak_ptr<ScrollViewer> weakViewer = viewer;
        desktop.TouchMoved +=
            [&](void *, Myra::Events::MyraEventArgs &) { static_cast<void>(desktop.RemoveWidget(rawViewer)); };
        desktop.AddWidget(viewer);
        desktop.UpdateLayout();
        PumpInput(desktop);
        viewer.reset();

        snapshot.Position = {31, 94};
        PumpInput(desktop);
        EXPECT_EQ(desktop.getWidgetsProperty().getCountProperty(), 0);
        static_cast<void>(desktop.getChildrenCopyProperty());
        EXPECT_TRUE(weakViewer.expired());
    }

    TEST(ScrollViewerTests, ReentrantReleaseRemovalInvalidatesTheRetainedCaptureSafely)
    {
        InputProviderGuard guard;
        MouseInfo snapshot{{10, 94}, true, false, false, 0.0F};
        Myra::MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        Myra::MyraEnvironment::setDownKeysGetterProperty([](Myra::MyraEnvironment::DownKeys &keys)
                                                         { keys.fill(false); });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 100); });
        std::shared_ptr<ScrollViewer> viewer = CreateScrollableViewer();
        ScrollViewer *const rawViewer = viewer.get();
        const std::weak_ptr<ScrollViewer> weakViewer = viewer;
        desktop.TouchUp +=
            [&](void *, Myra::Events::MyraEventArgs &) { static_cast<void>(desktop.RemoveWidget(rawViewer)); };
        desktop.AddWidget(viewer);
        desktop.UpdateLayout();
        PumpInput(desktop);
        viewer.reset();

        snapshot.IsLeftButtonDown = false;
        PumpInput(desktop);
        EXPECT_EQ(desktop.getWidgetsProperty().getCountProperty(), 0);
        static_cast<void>(desktop.getChildrenCopyProperty());
        EXPECT_TRUE(weakViewer.expired());
    }

    TEST(ScrollViewerTests, RejectsUndefinedScrollArithmeticWithoutPartialMutation)
    {
        ScrollViewer viewer;
        const std::shared_ptr<FixedWidget> content = std::make_shared<FixedWidget>(Point(300, 200));
        viewer.setContentProperty(content);

        EXPECT_THROW(viewer.setScrollPositionProperty(Point(std::numeric_limits<int>::min(), 1)), std::overflow_error);
        EXPECT_EQ(content->getLeftProperty(), 0);
        EXPECT_EQ(content->getTopProperty(), 0);
        EXPECT_THROW(viewer.setScrollPositionProperty(Point(1, std::numeric_limits<int>::min())), std::overflow_error);
        EXPECT_EQ(content->getLeftProperty(), 0);
        EXPECT_EQ(content->getTopProperty(), 0);

        const std::shared_ptr<ScrollViewer> scrollable = CreateScrollableViewer();
        scrollable->Arrange(Rectangle(0, 0, 100, 100));
        scrollable->setScrollMultiplierProperty(std::numeric_limits<int>::max());
        EXPECT_THROW(scrollable->OnMouseWheel(-1.0F), std::overflow_error);
        EXPECT_EQ(scrollable->getScrollPositionProperty(), Point(0, 0));

        ScrollViewer overflowingExtent;
        overflowingExtent.setContentProperty(
            std::make_shared<FixedWidget>(Point(std::numeric_limits<int>::max(), 200)));
        overflowingExtent.setVerticalScrollKnobProperty(std::make_shared<TestImage>(Point(10, 9)));
        EXPECT_THROW(overflowingExtent.Arrange(Rectangle(0, 0, 1, 100)), std::overflow_error);
    }
} // namespace

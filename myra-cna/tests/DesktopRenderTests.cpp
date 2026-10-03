// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Desktop.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Desktop.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/CrossEngineStuff.hpp"
#include "RecordingSpriteBatchBackend.hpp"

namespace Myra::Graphics2D::UI
{
    struct DesktopTestAccess
    {
        static void SetRenderContext(Desktop &desktop, std::shared_ptr<Graphics2D::RenderContext> context)
        {
            desktop.renderContext_ = std::move(context);
        }
    };
} // namespace Myra::Graphics2D::UI

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using Myra::MyraEnvironment;
    using Myra::Graphics2D::IBrush;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::DesktopTestAccess;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::Widget;
    using Myra::Tests::RecordingSpriteBatchBackend;

    class RecordingBrush final : public IBrush
    {
      public:
        RecordingBrush(std::vector<std::string> &events,
                       std::optional<Rectangle> &destination,
                       std::optional<Color> &color,
                       Point &transformedOrigin,
                       float &opacity)
            : events_(events),
              destination_(destination),
              color_(color),
              transformedOrigin_(transformedOrigin),
              opacity_(opacity)
        {
        }

        void Draw(RenderContext &context, const Rectangle destination, const Color color) const override
        {
            events_.emplace_back("background");
            destination_ = destination;
            color_ = color;
            transformedOrigin_ = context.getTransformProperty().Apply(Point(0, 0));
            opacity_ = context.getOpacityProperty();
        }

      private:
        std::vector<std::string> &events_;
        std::optional<Rectangle> &destination_;
        std::optional<Color> &color_;
        Point &transformedOrigin_;
        float &opacity_;
    };

    class VisualProbeWidget final : public Widget
    {
      public:
        VisualProbeWidget(std::string name, std::vector<std::string> &events, std::vector<float> &opacities)
            : name_(std::move(name)), events_(events), opacities_(opacities)
        {
        }

      protected:
        void InternalRender(RenderContext &context) override
        {
            events_.push_back(name_);
            opacities_.push_back(context.getOpacityProperty());
        }

      private:
        std::string name_;
        std::vector<std::string> &events_;
        std::vector<float> &opacities_;
    };

    class PipelineProbeWidget final : public Widget
    {
      public:
        explicit PipelineProbeWidget(std::vector<std::string> &events) : events_(events) {}

      protected:
        void InternalArrange() override { events_.emplace_back("arrange"); }

        void InternalRender(RenderContext &) override { events_.emplace_back("render"); }

      private:
        std::vector<std::string> &events_;
    };

    class DesktopRenderTests : public testing::Test
    {
      protected:
        void SetUp() override
        {
            MyraEnvironment::ClearGame();
            oldMouseInfoGetter_ = MyraEnvironment::getMouseInfoGetterProperty();
            oldDownKeysGetter_ = MyraEnvironment::getDownKeysGetterProperty();
            oldEnableModalDarkening_ = MyraEnvironment::getEnableModalDarkeningProperty();
            oldDarkeningColor_ = MyraEnvironment::getDarkeningColorProperty();
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
        }

        void TearDown() override
        {
            InputEventsManager::ProcessEvents();
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
            MyraEnvironment::setMouseInfoGetterProperty(std::move(oldMouseInfoGetter_));
            MyraEnvironment::setDownKeysGetterProperty(std::move(oldDownKeysGetter_));
            MyraEnvironment::setEnableModalDarkeningProperty(oldEnableModalDarkening_);
            MyraEnvironment::setDarkeningColorProperty(oldDarkeningColor_);
            MyraEnvironment::ClearGame();
        }

        static std::shared_ptr<RenderContext> MakeContext(
            Game &game, RecordingSpriteBatchBackend *&recording)
        {
            auto backend = std::make_unique<RecordingSpriteBatchBackend>();
            recording = backend.get();
            auto spriteBatch = std::make_unique<SpriteBatch>(std::move(backend));
            return std::make_shared<RenderContext>(
                std::move(spriteBatch), &game.getGraphicsDeviceProperty());
        }

      private:
        MyraEnvironment::MouseInfoGetter oldMouseInfoGetter_;
        MyraEnvironment::DownKeysGetter oldDownKeysGetter_;
        bool oldEnableModalDarkening_ = false;
        Color oldDarkeningColor_{0, 0, 0, 192};
    };

    TEST_F(DesktopRenderTests, RenderVisualRestoresScissorAndDrawsBackgroundModalOverlayAndRootsInZOrder)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);
        auto &device = game.getGraphicsDeviceProperty();
        device.setViewportProperty(Viewport(7, 9, 320, 200));
        const Rectangle oldScissor(3, 4, 17, 19);
        device.setScissorRectangleProperty(oldScissor);

        RecordingSpriteBatchBackend *recording = nullptr;
        const std::shared_ptr<RenderContext> context = MakeContext(game, recording);
        Desktop desktop;
        DesktopTestAccess::SetRenderContext(desktop, context);
        desktop.setBoundsFetcherProperty([] { return Rectangle(20, 30, 100, 80); });
        desktop.setTransformOriginProperty({0.0F, 0.0F});
        desktop.setOpacityProperty(0.5F);

        std::vector<std::string> events;
        std::vector<float> opacities;
        std::optional<Rectangle> backgroundDestination;
        std::optional<Color> backgroundColor;
        Point backgroundOrigin;
        float backgroundOpacity = 0.0F;
        desktop.setBackgroundProperty(std::make_shared<RecordingBrush>(
            events, backgroundDestination, backgroundColor, backgroundOrigin, backgroundOpacity));

        auto low = std::make_shared<VisualProbeWidget>("low", events, opacities);
        low->setWidthProperty(100);
        low->setHeightProperty(80);
        low->setTransformOriginProperty({0.0F, 0.0F});
        low->setOpacityProperty(0.5F);
        auto hidden = std::make_shared<VisualProbeWidget>("hidden", events, opacities);
        hidden->setVisibleProperty(false);
        hidden->setZIndexProperty(1);
        auto modal = std::make_shared<VisualProbeWidget>("modal", events, opacities);
        modal->setWidthProperty(100);
        modal->setHeightProperty(80);
        modal->setTransformOriginProperty({0.0F, 0.0F});
        modal->setIsModalProperty(true);
        modal->setZIndexProperty(2);
        desktop.AddWidget(modal);
        desktop.AddWidget(hidden);
        desktop.AddWidget(low);
        desktop.UpdateLayout();

        const Color darkeningColor(10, 20, 30, 200);
        MyraEnvironment::setEnableModalDarkeningProperty(true);
        MyraEnvironment::setDarkeningColorProperty(darkeningColor);
        desktop.RenderVisual();

        EXPECT_EQ(events, (std::vector<std::string>{"background", "low", "modal"}));
        ASSERT_EQ(opacities.size(), 2U);
        EXPECT_FLOAT_EQ(opacities[0], 0.25F);
        EXPECT_FLOAT_EQ(opacities[1], 0.5F);
        EXPECT_EQ(backgroundDestination, Rectangle(0, 0, 100, 80));
        EXPECT_EQ(backgroundColor, Color::White);
        EXPECT_EQ(backgroundOrigin, Point(20, 30));
        EXPECT_FLOAT_EQ(backgroundOpacity, 0.5F);
        ASSERT_NE(recording, nullptr);
        ASSERT_EQ(recording->draws.size(), 1U);
        EXPECT_EQ(recording->draws.front().destination, Rectangle(20, 30, 100, 80));
        EXPECT_EQ(
            recording->draws.front().color,
            Myra::Utility::CrossEngineStuff::MultiplyColor(darkeningColor, 0.5F));
        EXPECT_EQ(device.getScissorRectangleProperty(), oldScissor);
        EXPECT_EQ(recording->beginCount, recording->endCount);
        EXPECT_FALSE(context->getIsRenderingProperty());

        low->BeforeRender = [](RenderContext &) { throw std::runtime_error("visual failed"); };
        EXPECT_THROW(desktop.RenderVisual(), std::runtime_error);
        EXPECT_EQ(device.getScissorRectangleProperty(), oldScissor);
        EXPECT_EQ(recording->beginCount, recording->endCount);
        EXPECT_FALSE(context->getIsRenderingProperty());
        low->BeforeRender = {};
        EXPECT_NO_THROW(desktop.RenderVisual());
        EXPECT_EQ(recording->beginCount, recording->endCount);

        desktop.Dispose();
        desktop.Dispose();
        EXPECT_TRUE(desktop.getIsDisposedProperty());
        EXPECT_TRUE(context->getIsDisposedProperty());
        EXPECT_THROW(desktop.RenderVisual(), std::logic_error);

        MyraEnvironment::setEnableModalDarkeningProperty(false);
        Desktop reentrantDesktop;
        reentrantDesktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 40, 30); });
        reentrantDesktop.setTransformOriginProperty({0.0F, 0.0F});
        auto reentrantRoot = std::make_shared<VisualProbeWidget>("dispose", events, opacities);
        reentrantRoot->setWidthProperty(40);
        reentrantRoot->setHeightProperty(30);
        reentrantRoot->setTransformOriginProperty({0.0F, 0.0F});
        reentrantRoot->BeforeRender = [&](RenderContext &) { reentrantDesktop.Dispose(); };
        reentrantDesktop.AddWidget(reentrantRoot);
        reentrantDesktop.UpdateLayout();
        EXPECT_THROW(reentrantDesktop.RenderVisual(), std::logic_error);
        EXPECT_TRUE(reentrantDesktop.getIsDisposedProperty());
    }

    TEST_F(DesktopRenderTests, RenderProcessesInputAndASecondLayoutBeforeRenderingVisuals)
    {
        Game game;
        MyraEnvironment::setGameProperty(game);
        game.getGraphicsDeviceProperty().setViewportProperty(Viewport(0, 0, 100, 80));
        RecordingSpriteBatchBackend *recording = nullptr;
        const std::shared_ptr<RenderContext> context = MakeContext(game, recording);

        MouseInfo snapshot{{5, 5}, false, false, false, 0.0F};
        MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        MyraEnvironment::setDownKeysGetterProperty([](MyraEnvironment::DownKeys &keys) { keys.fill(false); });

        std::vector<std::string> events;
        Desktop desktop;
        DesktopTestAccess::SetRenderContext(desktop, context);
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 80); });
        desktop.setTransformOriginProperty({0.0F, 0.0F});
        auto root = std::make_shared<PipelineProbeWidget>(events);
        root->setWidthProperty(60);
        root->setHeightProperty(40);
        root->setTransformOriginProperty({0.0F, 0.0F});
        root->MouseEntered += [&](void *, Myra::Events::MyraEventArgs &)
        {
            events.emplace_back("input");
            root->setHeightProperty(30);
        };
        desktop.AddWidget(root);

        desktop.Render();

        EXPECT_EQ(events, (std::vector<std::string>{"arrange", "input", "arrange", "render"}));
        ASSERT_NE(recording, nullptr);
        EXPECT_EQ(recording->beginCount, recording->endCount);
        EXPECT_FALSE(context->getIsRenderingProperty());
    }
} // namespace

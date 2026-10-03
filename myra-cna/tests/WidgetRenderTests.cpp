// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/Transform.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/Graphics2D/UI/InputEventsManager.hpp"
#include "Myra/MyraEnvironment.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using Myra::MyraEnvironment;
    using Myra::Graphics2D::IBrush;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::Transform;
    using Myra::Graphics2D::UI::Desktop;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::InputEventsManager;
    using Myra::Graphics2D::UI::MouseInfo;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::Widget;

    struct BrushObservation
    {
        std::string name;
        Rectangle destination;
        Color color;
        float opacity = 0.0F;
        Rectangle scissor;
        Vector2 transformedOrigin;
    };

    class RecordingBrush final : public IBrush
    {
    public:
        RecordingBrush(std::string name,
            std::vector<std::string>& events,
            std::vector<BrushObservation>& observations)
            : name_(std::move(name)), events_(events), observations_(observations)
        {
        }

        void Draw(RenderContext& context, const Rectangle destination, const Color color) const override
        {
            events_.push_back(name_);
            observations_.push_back(BrushObservation{
                name_,
                destination,
                color,
                context.getOpacityProperty(),
                context.getScissorProperty(),
                context.getTransformProperty().Apply(Vector2::Zero)});
        }

    private:
        std::string name_;
        std::vector<std::string>& events_;
        std::vector<BrushObservation>& observations_;
    };

    class RenderProbeWidget final : public Widget
    {
    public:
        bool hovered = false;
        std::vector<std::string>* events = nullptr;
        std::function<void(RenderContext&)> internalRender;

        void InternalRender(RenderContext& context) override
        {
            if (events != nullptr)
            {
                events->push_back("internal");
            }
            if (internalRender)
            {
                internalRender(context);
            }
            else
            {
                Widget::InternalRender(context);
            }
        }

    protected:
        [[nodiscard]] bool UseOverBackground() const noexcept override { return hovered; }
    };

    class WidgetRenderTests : public testing::Test
    {
    protected:
        void SetUp() override
        {
            mouseInfoGetter_ = MyraEnvironment::getMouseInfoGetterProperty();
            tooltipDelay_ = MyraEnvironment::getTooltipDelayInMsProperty();
            tooltipOffset_ = MyraEnvironment::getTooltipOffsetProperty();
            tooltipCreator_ = MyraEnvironment::getTooltipCreatorProperty();
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
        }

        void TearDown() override
        {
            InputEventsManager::ProcessEvents();
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
            MyraEnvironment::setMouseInfoGetterProperty(std::move(mouseInfoGetter_));
            MyraEnvironment::setTooltipDelayInMsProperty(tooltipDelay_);
            MyraEnvironment::setTooltipOffsetProperty(tooltipOffset_);
            MyraEnvironment::setTooltipCreatorProperty(std::move(tooltipCreator_));
            MyraEnvironment::ClearGame();
        }

        static RenderContext MakeContext(Game& game)
        {
            auto& device = game.getGraphicsDeviceProperty();
            device.setViewportProperty(Viewport(0, 0, 400, 300));
            return RenderContext(device);
        }

      private:
        MyraEnvironment::MouseInfoGetter mouseInfoGetter_;
        int tooltipDelay_ = 0;
        Microsoft::Xna::Framework::Point tooltipOffset_;
        MyraEnvironment::TooltipCreator tooltipCreator_;
    };

    TEST_F(WidgetRenderTests, SelectsRetainedVisualsInUpstreamPriorityOrder)
    {
        std::vector<std::string> events;
        std::vector<BrushObservation> observations;
        auto normal = std::make_shared<RecordingBrush>("normal", events, observations);
        auto over = std::make_shared<RecordingBrush>("over", events, observations);
        auto disabled = std::make_shared<RecordingBrush>("disabled", events, observations);
        auto focused = std::make_shared<RecordingBrush>("focused", events, observations);
        auto pressed = std::make_shared<RecordingBrush>("pressed", events, observations);

        RenderProbeWidget widget;
        widget.setBackgroundProperty(normal);
        widget.setOverBackgroundProperty(over);
        widget.setDisabledBackgroundProperty(disabled);
        widget.setFocusedBackgroundProperty(focused);
        widget.setPressedBackgroundProperty(pressed);
        widget.setBorderProperty(normal);
        widget.setOverBorderProperty(over);
        widget.setDisabledBorderProperty(disabled);
        widget.setFocusedBorderProperty(focused);
        widget.setPressedBorderProperty(pressed);

        EXPECT_EQ(widget.GetCurrentBackground(), normal);
        EXPECT_EQ(widget.GetCurrentBorder(), normal);

        widget.hovered = true;
        EXPECT_EQ(widget.GetCurrentBackground(), over);
        EXPECT_EQ(widget.GetCurrentBorder(), over);

        widget.OnGotKeyboardFocus();
        EXPECT_EQ(widget.GetCurrentBackground(), focused);
        EXPECT_EQ(widget.GetCurrentBorder(), focused);

        widget.setIsPressedProperty(true);
        EXPECT_EQ(widget.GetCurrentBackground(), pressed);
        EXPECT_EQ(widget.GetCurrentBorder(), pressed);

        widget.setIsPressedProperty(false);
        widget.OnLostKeyboardFocus();
        widget.hovered = false;
        widget.setEnabledProperty(false);
        EXPECT_EQ(widget.GetCurrentBackground(), disabled);
        EXPECT_EQ(widget.GetCurrentBorder(), disabled);

        widget.setDisabledBackgroundProperty(nullptr);
        widget.setDisabledBorderProperty(nullptr);
        EXPECT_EQ(widget.GetCurrentBackground(), over);
        EXPECT_EQ(widget.GetCurrentBorder(), over);

        std::weak_ptr<RecordingBrush> retained = normal;
        normal.reset();
        EXPECT_FALSE(retained.expired());
        widget.setBackgroundProperty(nullptr);
        widget.setBorderProperty(nullptr);
        EXPECT_TRUE(retained.expired());
    }

    TEST_F(WidgetRenderTests, CloneSharesManagedVisualsAndCopiesRenderCallbacks)
    {
        std::vector<std::string> events;
        std::vector<BrushObservation> observations;
        auto background = std::make_shared<RecordingBrush>("background", events, observations);

        Widget source;
        source.setBackgroundProperty(background);
        source.BeforeRender = [&](RenderContext&) { events.push_back("before"); };
        source.AfterRender = [&](RenderContext&) { events.push_back("after"); };

        const std::shared_ptr<Widget> clone = source.Clone();

        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getBackgroundProperty(), background);
        ASSERT_TRUE(clone->BeforeRender);
        ASSERT_TRUE(clone->AfterRender);

        Game game;
        RenderContext context = MakeContext(game);
        clone->BeforeRender(context);
        clone->AfterRender(context);
        EXPECT_EQ(events, (std::vector<std::string>{"before", "after"}));
    }

    TEST_F(WidgetRenderTests, RendersDecorationAndCallbacksWithNestedContextState)
    {
        Game game;
        RenderContext context = MakeContext(game);
        const Rectangle outerScissor(0, 0, 400, 300);
        context.setScissorProperty(outerScissor);
        context.setOpacityProperty(0.8F);
        const Transform outerTransform(
            Vector2(4.0F, 6.0F), Vector2::Zero, Vector2(2.0F, 2.0F), 0.0F);
        context.setTransformProperty(outerTransform);

        std::vector<std::string> events;
        std::vector<BrushObservation> observations;
        auto background = std::make_shared<RecordingBrush>("background", events, observations);
        auto border = std::make_shared<RecordingBrush>("border", events, observations);

        RenderProbeWidget widget;
        widget.events = &events;
        widget.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        widget.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        widget.setMarginProperty(Thickness(1, 2, 3, 4));
        widget.setBorderThicknessProperty(Thickness(2, 3, 4, 5));
        widget.setBackgroundProperty(background);
        widget.setBorderProperty(border);
        widget.setClipToBoundsProperty(true);
        widget.setOpacityProperty(0.5F);
        widget.BeforeRender = [&](RenderContext&) { events.push_back("before"); };
        widget.AfterRender = [&](RenderContext&) { events.push_back("after"); };
        widget.Arrange(Rectangle(10, 20, 100, 80));

        widget.Render(context);

        EXPECT_EQ(events, (std::vector<std::string>{
                              "background", "border", "border", "border", "border",
                              "before", "internal", "after"}));
        ASSERT_EQ(observations.size(), 5U);
        EXPECT_EQ(observations[0].destination, Rectangle(3, 5, 90, 66));
        EXPECT_EQ(observations[1].destination, Rectangle(1, 2, 2, 74));
        EXPECT_EQ(observations[2].destination, Rectangle(1, 2, 96, 3));
        EXPECT_EQ(observations[3].destination, Rectangle(93, 2, 4, 74));
        EXPECT_EQ(observations[4].destination, Rectangle(1, 71, 96, 5));
        for (const BrushObservation& observation : observations)
        {
            EXPECT_EQ(observation.color, Color::White);
            EXPECT_FLOAT_EQ(observation.opacity, 0.4F);
            EXPECT_EQ(observation.scissor, Rectangle(10, 20, 100, 80));
            EXPECT_EQ(observation.transformedOrigin, Vector2(10.0F, 20.0F));
        }

        EXPECT_EQ(context.getScissorProperty(), outerScissor);
        EXPECT_FLOAT_EQ(context.getOpacityProperty(), 0.8F);
        EXPECT_EQ(context.getTransformProperty().Apply(Vector2(3.0F, 5.0F)),
            outerTransform.Apply(Vector2(3.0F, 5.0F)));
    }

    TEST_F(WidgetRenderTests, CullsDisjointWidgetsWithoutCallbacksOrContextLeaks)
    {
        Game game;
        RenderContext context = MakeContext(game);
        const Rectangle outerScissor(0, 0, 10, 10);
        context.setScissorProperty(outerScissor);
        context.setOpacityProperty(0.7F);
        const Transform outerTransform(
            Vector2(3.0F, 7.0F), Vector2::Zero, Vector2(1.5F, 2.0F), 0.0F);
        context.setTransformProperty(outerTransform);

        std::vector<std::string> events;
        std::vector<BrushObservation> observations;
        RenderProbeWidget widget;
        widget.events = &events;
        widget.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        widget.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        widget.setBackgroundProperty(
            std::make_shared<RecordingBrush>("background", events, observations));
        widget.BeforeRender = [&](RenderContext&) { events.push_back("before"); };
        widget.AfterRender = [&](RenderContext&) { events.push_back("after"); };
        widget.Arrange(Rectangle(100, 100, 20, 20));

        widget.Render(context);

        EXPECT_TRUE(events.empty());
        EXPECT_TRUE(observations.empty());
        EXPECT_EQ(context.getScissorProperty(), outerScissor);
        EXPECT_FLOAT_EQ(context.getOpacityProperty(), 0.7F);
        EXPECT_EQ(context.getTransformProperty().Apply(Vector2(2.0F, 3.0F)),
            outerTransform.Apply(Vector2(2.0F, 3.0F)));
    }

    TEST_F(WidgetRenderTests, PreservesUpstreamUnclippedTraversalForRotatedWidgets)
    {
        Game game;
        RenderContext context = MakeContext(game);
        const Rectangle outerScissor(0, 0, 10, 10);
        context.setScissorProperty(outerScissor);
        context.setOpacityProperty(1.0F);

        std::vector<std::string> events;
        std::vector<BrushObservation> observations;
        RenderProbeWidget widget;
        widget.events = &events;
        widget.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        widget.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        widget.setBackgroundProperty(
            std::make_shared<RecordingBrush>("background", events, observations));
        widget.setClipToBoundsProperty(true);
        widget.setRotationProperty(30.0F);
        widget.Arrange(Rectangle(100, 100, 20, 20));

        widget.Render(context);

        EXPECT_EQ(events, (std::vector<std::string>{"background", "internal"}));
        ASSERT_EQ(observations.size(), 1U);
        EXPECT_EQ(observations.front().scissor, outerScissor);
        EXPECT_EQ(context.getScissorProperty(), outerScissor);
        EXPECT_FLOAT_EQ(context.getOpacityProperty(), 1.0F);
        EXPECT_EQ(context.getTransformProperty().Apply(Vector2(2.0F, 3.0F)),
            Vector2(2.0F, 3.0F));
    }

    TEST_F(WidgetRenderTests, RestoresAllContextStateWhenUserRenderingThrows)
    {
        Game game;
        RenderContext context = MakeContext(game);
        const Rectangle outerScissor(0, 0, 400, 300);
        context.setScissorProperty(outerScissor);
        context.setOpacityProperty(0.75F);
        const Transform outerTransform(
            Vector2(8.0F, 9.0F), Vector2::Zero, Vector2(2.0F, 3.0F), 0.0F);
        context.setTransformProperty(outerTransform);

        RenderProbeWidget widget;
        widget.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        widget.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        widget.setClipToBoundsProperty(true);
        widget.setOpacityProperty(0.25F);
        widget.BeforeRender = [](RenderContext&) { throw std::runtime_error("render failed"); };
        widget.Arrange(Rectangle(10, 20, 100, 80));

        EXPECT_THROW(widget.Render(context), std::runtime_error);
        EXPECT_EQ(context.getScissorProperty(), outerScissor);
        EXPECT_FLOAT_EQ(context.getOpacityProperty(), 0.75F);
        EXPECT_EQ(context.getTransformProperty().Apply(Vector2(3.0F, 4.0F)),
            outerTransform.Apply(Vector2(3.0F, 4.0F)));
    }

    TEST_F(WidgetRenderTests, RetainsTheChildSnapshotAcrossReentrantTreeMutation)
    {
        Game game;
        RenderContext context = MakeContext(game);
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        context.setOpacityProperty(1.0F);

        auto parent = std::make_shared<Widget>();
        auto first = std::make_shared<Widget>();
        auto second = std::make_shared<Widget>();
        parent->AddChild(first);
        parent->AddChild(second);
        first->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        first->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        second->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        second->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        first->Arrange(Rectangle(0, 0, 20, 20));
        second->Arrange(Rectangle(0, 0, 20, 20));
        parent->setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        parent->setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        parent->Arrange(Rectangle(10, 20, 100, 80));

        std::vector<std::string> events;
        first->BeforeRender = [&](RenderContext&) {
            events.push_back("first");
            EXPECT_TRUE(parent->RemoveChild(second.get()));
            static_cast<void>(parent->getChildrenCopyProperty());
        };
        second->BeforeRender = [&](RenderContext&) { events.push_back("second"); };

        parent->Render(context);

        EXPECT_EQ(events, (std::vector<std::string>{"first", "second"}));
        EXPECT_EQ(second->getParentProperty(), nullptr);
        EXPECT_EQ(parent->getChildrenProperty().size(), 1U);
    }

    TEST_F(WidgetRenderTests, StationaryHoverCreatesAndMouseLeaveHidesTheTooltip)
    {
        MouseInfo snapshot{{5, 5}, false, false, false, 0.0F};
        MyraEnvironment::setMouseInfoGetterProperty([&] { return snapshot; });
        MyraEnvironment::setTooltipDelayInMsProperty(-1);
        MyraEnvironment::setTooltipOffsetProperty({2, 3});

        auto tooltip = std::make_shared<RenderProbeWidget>();
        tooltip->setWidthProperty(20);
        tooltip->setHeightProperty(10);
        Widget *creatorOwner = nullptr;
        MyraEnvironment::setTooltipCreatorProperty(
            [&](Widget &owner)
            {
                creatorOwner = &owner;
                return tooltip;
            });

        Desktop desktop;
        desktop.setBoundsFetcherProperty([] { return Rectangle(0, 0, 100, 80); });
        auto owner = std::make_shared<RenderProbeWidget>();
        owner->setWidthProperty(50);
        owner->setHeightProperty(40);
        owner->setTooltipProperty(std::string("details"));
        desktop.AddWidget(owner);
        desktop.UpdateLayout();
        desktop.UpdateMouseInput();
        desktop.ProcessWidgetInput();
        InputEventsManager::ProcessEvents();

        Game game;
        RenderContext context = MakeContext(game);
        context.setScissorProperty(Rectangle(0, 0, 400, 300));
        owner->Render(context);

        EXPECT_EQ(creatorOwner, owner.get());
        EXPECT_EQ(desktop.getTooltipProperty(), tooltip);
        EXPECT_EQ(tooltip->getLeftProperty(), 7);
        EXPECT_EQ(tooltip->getTopProperty(), 8);

        snapshot.Position = {90, 70};
        desktop.UpdateMouseInput();
        desktop.ProcessWidgetInput();
        InputEventsManager::ProcessEvents();

        EXPECT_EQ(desktop.getTooltipProperty(), nullptr);
        EXPECT_FALSE(tooltip->getVisibleProperty());
        EXPECT_FALSE(tooltip->getIsPlacedProperty());
    }
}

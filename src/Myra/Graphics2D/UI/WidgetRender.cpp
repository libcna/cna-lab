// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Widget.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        [[nodiscard]] int CheckedAdd(const int left, const int right, const char* const message)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) + right;
            if (result < std::numeric_limits<int>::min()
                || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(result);
        }

        [[nodiscard]] int CheckedSubtract(const int left, const int right, const char* const message)
        {
            const std::int64_t result = static_cast<std::int64_t>(left) - right;
            if (result < std::numeric_limits<int>::min()
                || result > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(result);
        }
    }

    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;

    void Widget::Render(Graphics2D::RenderContext& context)
    {
        if (!visible_)
        {
            return;
        }

        UpdateArrange();
        const Graphics2D::Transform oldTransform = context.getTransformProperty();
        const float oldOpacity = context.getOpacityProperty();
        const Graphics2D::Transform& widgetTransform = getTransformProperty();
        context.setTransformProperty(widgetTransform);

        std::optional<Rectangle> oldScissorRectangle;
        const auto restoreContext = [&]() {
            if (oldScissorRectangle.has_value())
            {
                context.setScissorProperty(*oldScissorRectangle);
            }
            context.setTransformProperty(oldTransform);
            context.setOpacityProperty(oldOpacity);
        };

        try
        {
            if (Utility::Mathematics::IsZero(widgetTransform.getRotationProperty()))
            {
                const Rectangle absoluteBounds = widgetTransform.Apply(getBoundsProperty());
                const Rectangle scissorBounds = Rectangle::Intersect(
                    context.getScissorProperty(), absoluteBounds);
                if (scissorBounds.Width == 0 || scissorBounds.Height == 0)
                {
                    context.setTransformProperty(oldTransform);
                    return;
                }
                if (clipToBounds_)
                {
                    oldScissorRectangle = context.getScissorProperty();
                    context.setScissorProperty(scissorBounds);
                }
            }

            context.AddOpacity(opacity_);

            const std::shared_ptr<Graphics2D::IBrush> background = GetCurrentBackground();
            if (background)
            {
                background->Draw(context, getBackgroundBoundsProperty(), Color::White);
            }

            const std::shared_ptr<Graphics2D::IBrush> border = GetCurrentBorder();
            if (border)
            {
                const Rectangle borderBounds = getBorderBoundsProperty();
                if (borderThickness_.Left > 0)
                {
                    border->Draw(context,
                        Rectangle(borderBounds.X, borderBounds.Y,
                            borderThickness_.Left, borderBounds.Height),
                        Color::White);
                }
                if (borderThickness_.Top > 0)
                {
                    border->Draw(context,
                        Rectangle(borderBounds.X, borderBounds.Y,
                            borderBounds.Width, borderThickness_.Top),
                        Color::White);
                }
                if (borderThickness_.Right > 0)
                {
                    const int right = CheckedAdd(borderBounds.X, borderBounds.Width,
                        "Widget right border edge exceeds the supported integer range.");
                    border->Draw(context,
                        Rectangle(CheckedSubtract(right, borderThickness_.Right,
                                      "Widget right border position exceeds the supported integer range."),
                            borderBounds.Y, borderThickness_.Right, borderBounds.Height),
                        Color::White);
                }
                if (borderThickness_.Bottom > 0)
                {
                    const int bottom = CheckedAdd(borderBounds.Y, borderBounds.Height,
                        "Widget bottom border edge exceeds the supported integer range.");
                    border->Draw(context,
                        Rectangle(borderBounds.X,
                            CheckedSubtract(bottom, borderThickness_.Bottom,
                                "Widget bottom border position exceeds the supported integer range."),
                            borderBounds.Width, borderThickness_.Bottom),
                        Color::White);
                }
            }

            RenderCallback beforeRender = BeforeRender;
            if (beforeRender)
            {
                beforeRender(context);
            }
            InternalRender(context);
            RenderCallback afterRender = AfterRender;
            if (afterRender)
            {
                afterRender(context);
            }

            if (oldScissorRectangle.has_value())
            {
                context.setScissorProperty(*oldScissorRectangle);
                oldScissorRectangle.reset();
            }

            if (MyraEnvironment::getDrawWidgetsFramesProperty())
            {
                context.DrawRectangle(getBoundsProperty(), Color::LightGreen);
            }
            if (MyraEnvironment::getDrawKeyboardFocusedWidgetFrameProperty()
                && isKeyboardFocused_)
            {
                context.DrawRectangle(getBoundsProperty(), Color::Red);
            }
        }
        catch (...)
        {
            restoreContext();
            throw;
        }

        restoreContext();
    }

    void Widget::InternalRender(Graphics2D::RenderContext& context)
    {
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& child : snapshot)
        {
            child->Render(context);
        }
    }
}

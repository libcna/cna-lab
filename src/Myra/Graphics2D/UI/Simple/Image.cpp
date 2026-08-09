// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/Image.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/Image.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Utility/Mathematics.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    Image::Image() : color_(Color::White) {}

    Image::~Image() = default;

    std::shared_ptr<Graphics2D::IImage> Image::getRenderableProperty() const
    {
        return renderables_[WidgetVisualStateNormal];
    }

    void Image::setRenderableProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == renderables_[WidgetVisualStateNormal])
        {
            return;
        }
        renderables_[WidgetVisualStateNormal] = std::move(value);
        InvalidateMeasure();
    }

    std::shared_ptr<Graphics2D::IImage> Image::getDisabledRenderableProperty() const
    {
        return renderables_[WidgetVisualStateDisabled];
    }

    void Image::setDisabledRenderableProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == renderables_[WidgetVisualStateDisabled])
        {
            return;
        }
        renderables_[WidgetVisualStateDisabled] = std::move(value);
        InvalidateMeasure();
    }

    std::shared_ptr<Graphics2D::IImage> Image::getOverRenderableProperty() const
    {
        return renderables_[WidgetVisualStateOver];
    }

    void Image::setOverRenderableProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == renderables_[WidgetVisualStateOver])
        {
            return;
        }
        renderables_[WidgetVisualStateOver] = std::move(value);
        InvalidateMeasure();
    }

    std::shared_ptr<Graphics2D::IImage> Image::getFocusedRenderableProperty() const
    {
        return renderables_[WidgetVisualStateFocused];
    }

    void Image::setFocusedRenderableProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == renderables_[WidgetVisualStateFocused])
        {
            return;
        }
        renderables_[WidgetVisualStateFocused] = std::move(value);
        InvalidateMeasure();
    }

    std::shared_ptr<Graphics2D::IImage> Image::getPressedRenderableProperty() const
    {
        return renderables_[WidgetVisualStatePressed];
    }

    void Image::setPressedRenderableProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == renderables_[WidgetVisualStatePressed])
        {
            return;
        }
        renderables_[WidgetVisualStatePressed] = std::move(value);
        InvalidateMeasure();
    }

    Color Image::getColorProperty() const
    {
        return color_;
    }

    void Image::setColorProperty(const Color value) noexcept
    {
        color_ = value;
    }

    ImageResizeMode Image::getResizeModeProperty() const noexcept
    {
        return resizeMode_;
    }

    void Image::setResizeModeProperty(const ImageResizeMode value) noexcept
    {
        resizeMode_ = value;
    }

    Point Image::InternalMeasure(const Point availableSize)
    {
        static_cast<void>(availableSize);
        Point result(0, 0);
        for (const std::shared_ptr<Graphics2D::IImage>& renderable : renderables_)
        {
            if (renderable)
            {
                const Point size = renderable->getSizeProperty();
                result.X = std::max(result.X, size.X);
                result.Y = std::max(result.Y, size.Y);
            }
        }
        return result;
    }

    void Image::InternalRender(Graphics2D::RenderContext& context)
    {
        const std::shared_ptr<Graphics2D::IImage> image = GetCurrentVisual(renderables_);
        if (!image)
        {
            return;
        }

        Rectangle bounds = getActualBoundsProperty();
        if (resizeMode_ == ImageResizeMode::KeepAspectRatio)
        {
            const Point size = image->getSizeProperty();
            if (size.Y == 0)
            {
                throw std::invalid_argument(
                    "An aspect-preserving Image renderable must have a non-zero height.");
            }
            const float aspect = static_cast<float>(size.X) / static_cast<float>(size.Y);
            bounds.Height = Utility::Mathematics::TruncateToInt(
                static_cast<float>(bounds.Width) * aspect);
        }
        image->Draw(context, bounds, color_);
    }

    std::shared_ptr<Widget> Image::CreateCloneInstance() const
    {
        return std::make_shared<Image>();
    }

    void Image::CopyFrom(const Widget& source)
    {
        Widget::CopyFrom(source);
        const auto* const image = dynamic_cast<const Image*>(&source);
        if (image == nullptr)
        {
            throw std::invalid_argument("Image copy source must be an Image.");
        }
        color_ = image->color_;
        resizeMode_ = image->resizeMode_;
        renderables_ = image->renderables_;
    }
}

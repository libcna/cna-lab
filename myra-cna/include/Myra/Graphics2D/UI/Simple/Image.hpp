// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/Image.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <array>
#include <memory>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

namespace Myra::Graphics2D
{
    class IImage;
}

namespace Myra::Graphics2D::UI
{
    /** @brief Specifies how an image is resized into its content bounds. */
    enum class ImageResizeMode
    {
        Stretch,
        KeepAspectRatio
    };

    /** @brief Displays a retained image selected from the current widget visual state. */
    class Image : public Widget
    {
    public:
        Image();
        ~Image() override;

        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getRenderableProperty() const;
        void setRenderableProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getDisabledRenderableProperty() const;
        void setDisabledRenderableProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getOverRenderableProperty() const;
        void setOverRenderableProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getFocusedRenderableProperty() const;
        void setFocusedRenderableProperty(std::shared_ptr<Graphics2D::IImage> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IImage> getPressedRenderableProperty() const;
        void setPressedRenderableProperty(std::shared_ptr<Graphics2D::IImage> value);

        [[nodiscard]] Microsoft::Xna::Framework::Color getColorProperty() const;
        void setColorProperty(Microsoft::Xna::Framework::Color value) noexcept;
        [[nodiscard]] ImageResizeMode getResizeModeProperty() const noexcept;
        void setResizeModeProperty(ImageResizeMode value) noexcept;

        void InternalRender(Graphics2D::RenderContext& context) override;

    protected:
        [[nodiscard]] Microsoft::Xna::Framework::Point InternalMeasure(
            Microsoft::Xna::Framework::Point availableSize) override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget& source) override;

    private:
        std::array<std::shared_ptr<Graphics2D::IImage>, WidgetVisualStateTotal> renderables_;
        Microsoft::Xna::Framework::Color color_;
        ImageResizeMode resizeMode_ = ImageResizeMode::Stretch;
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/IBrush.cs and IImage.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/IImage.hpp"

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IBrush;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;

    class FixedSizeImage final : public IImage
    {
    public:
        void Draw(RenderContext&, Rectangle, Color) const override
        {
        }

        [[nodiscard]] Point getSizeProperty() const override
        {
            return {23, 17};
        }
    };

    using DrawSignature = void (IBrush::*)(RenderContext&, Rectangle, Color) const;

    static_assert(std::is_abstract_v<IBrush>);
    static_assert(std::is_abstract_v<IImage>);
    static_assert(std::has_virtual_destructor_v<IBrush>);
    static_assert(std::is_base_of_v<IBrush, IImage>);
    static_assert(std::is_same_v<
        decltype(static_cast<DrawSignature>(&IBrush::Draw)), DrawSignature>);

    TEST(GraphicsContractsTests, ImageExtendsBrushAndExposesItsPixelSize)
    {
        const FixedSizeImage image;

        EXPECT_EQ(image.getSizeProperty(), Point(23, 17));
    }
}

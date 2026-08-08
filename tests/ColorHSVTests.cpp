// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Utility/ColorHSV.hpp"

#include <gtest/gtest.h>

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Myra::Utility::ColorHSV;

    TEST(ColorHSVTests, ConvertsCanonicalRgbAndHsvColors)
    {
        const ColorHSV red = ColorHSV::FromRGB(Color(255, 0, 0));
        EXPECT_EQ(red.H, 0);
        EXPECT_EQ(red.S, 100);
        EXPECT_EQ(red.V, 100);

        const ColorHSV blue = ColorHSV::FromRGB(Color(0, 0, 255));
        EXPECT_EQ(blue.H, 240);
        EXPECT_EQ(blue.S, 100);
        EXPECT_EQ(blue.V, 100);

        const ColorHSV white = ColorHSV::FromRGB(Color(255, 255, 255));
        EXPECT_EQ(white.H, 0);
        EXPECT_EQ(white.S, 0);
        EXPECT_EQ(white.V, 100);

        const ColorHSV black = ColorHSV::FromRGB(Color(0, 0, 0));
        EXPECT_EQ(black.H, 0);
        EXPECT_EQ(black.S, 0);
        EXPECT_EQ(black.V, 0);

        const Color green = ColorHSV{120, 100, 100}.ToRGB();
        EXPECT_EQ(green.getRProperty(), 0);
        EXPECT_EQ(green.getGProperty(), 255);
        EXPECT_EQ(green.getBProperty(), 0);
        EXPECT_EQ(green.getAProperty(), 255);
    }

    TEST(ColorHSVTests, PreservesTheSelectedUpstreamEqualityBehavior)
    {
        const ColorHSV value{1, 2, 3};
        EXPECT_FALSE(value == value);
        EXPECT_TRUE((ColorHSV{1, 3, 3} == ColorHSV{1, 7, 3}));
    }
}

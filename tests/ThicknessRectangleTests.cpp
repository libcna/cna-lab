// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/Thickness.hpp"

#include <gtest/gtest.h>

namespace
{
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::Thickness;

    TEST(ThicknessRectangleTests, SubtractionShrinksAndClampsTheRectangle)
    {
        const Rectangle reduced = Rectangle(10, 20, 100, 100) - Thickness(1, 2, 3, 4);
        EXPECT_EQ(reduced.X, 11);
        EXPECT_EQ(reduced.Y, 22);
        EXPECT_EQ(reduced.Width, 96);
        EXPECT_EQ(reduced.Height, 94);

        const Rectangle clamped = Rectangle(5, 6, 3, 4) - Thickness(2, 3, 4, 5);
        EXPECT_EQ(clamped.X, 7);
        EXPECT_EQ(clamped.Y, 9);
        EXPECT_EQ(clamped.Width, 0);
        EXPECT_EQ(clamped.Height, 0);
    }
}

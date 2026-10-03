// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/LayoutUtils.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

using Microsoft::Xna::Framework::Point;
using Myra::Graphics2D::UI::HorizontalAlignment;
using Myra::Graphics2D::UI::LayoutUtils;
using Myra::Graphics2D::UI::VerticalAlignment;

TEST(LayoutUtilsTests, AlignsAndStretchesUsingUpstreamRules)
{
    const auto centered = LayoutUtils::Align(Point(100, 60), Point(20, 10),
                                             HorizontalAlignment::Center, VerticalAlignment::Center);
    EXPECT_EQ(centered.X, 40);
    EXPECT_EQ(centered.Y, 25);
    EXPECT_EQ(centered.Width, 20);
    EXPECT_EQ(centered.Height, 10);

    const auto stretched = LayoutUtils::Align(Point(100, 60), Point(20, 10),
                                              HorizontalAlignment::Stretch, VerticalAlignment::Stretch);
    EXPECT_EQ(stretched.X, 0);
    EXPECT_EQ(stretched.Y, 0);
    EXPECT_EQ(stretched.Width, 100);
    EXPECT_EQ(stretched.Height, 60);

    EXPECT_THROW(static_cast<void>(LayoutUtils::Align(
        Point(std::numeric_limits<int>::max(), 0), Point(std::numeric_limits<int>::min(), 0),
        HorizontalAlignment::Right, VerticalAlignment::Top)), std::overflow_error);
}

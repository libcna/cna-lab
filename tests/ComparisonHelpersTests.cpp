// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "ComparisonHelpers.hpp"

#include <gtest/gtest.h>

#include <limits>

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Tests::ColorEqual;
    using Myra::Tests::NumericNear;
    using Myra::Tests::RectangleEqual;

    TEST(ComparisonHelpersTests, NumericComparisonHandlesToleranceAndSpecialValues)
    {
        EXPECT_TRUE(NumericNear("expected", "actual", "tolerance", 1.0, 1.001, 0.01));
        EXPECT_FALSE(NumericNear("expected", "actual", "tolerance", 1.0, 1.1, 0.01));
        EXPECT_TRUE(NumericNear("expected", "actual", "tolerance",
            std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::infinity(), 0.0));
        EXPECT_FALSE(NumericNear("expected", "actual", "tolerance",
            std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0));
        EXPECT_FALSE(NumericNear("expected", "actual", "tolerance", 1.0, 1.0, -0.1));
    }

    TEST(ComparisonHelpersTests, RectangleAndColorComparisonsReportComponents)
    {
        EXPECT_TRUE(RectangleEqual("expected", "actual",
            Rectangle(1, 2, 3, 4), Rectangle(1, 2, 3, 4)));
        EXPECT_FALSE(RectangleEqual("expected", "actual",
            Rectangle(1, 2, 3, 4), Rectangle(1, 2, 4, 3)));

        EXPECT_TRUE(ColorEqual("expected", "actual",
            Color(10, 20, 30, 40), Color(10, 20, 30, 40)));
        EXPECT_FALSE(ColorEqual("expected", "actual",
            Color(10, 20, 30, 40), Color(10, 20, 31, 40)));
    }
}

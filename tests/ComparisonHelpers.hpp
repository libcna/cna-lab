// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#pragma once

#include <cmath>
#include <concepts>
#include <type_traits>

#include <gtest/gtest.h>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

namespace Myra::Tests
{
    template<typename Expected, typename Actual, typename Tolerance>
        requires std::is_arithmetic_v<Expected> && std::is_arithmetic_v<Actual>
            && std::is_arithmetic_v<Tolerance>
    [[nodiscard]] ::testing::AssertionResult NumericNear(
        const char* const expectedExpression,
        const char* const actualExpression,
        const char* const toleranceExpression,
        const Expected expected,
        const Actual actual,
        const Tolerance tolerance)
    {
        const long double expectedValue = static_cast<long double>(expected);
        const long double actualValue = static_cast<long double>(actual);
        const long double toleranceValue = static_cast<long double>(tolerance);

        if (toleranceValue < 0.0L || !std::isfinite(toleranceValue))
        {
            return ::testing::AssertionFailure()
                << toleranceExpression << " must be finite and non-negative, but is " << toleranceValue;
        }
        if (expectedValue == actualValue)
        {
            return ::testing::AssertionSuccess();
        }

        const long double difference = std::fabs(expectedValue - actualValue);
        if (std::isfinite(expectedValue) && std::isfinite(actualValue)
            && difference <= toleranceValue)
        {
            return ::testing::AssertionSuccess();
        }

        return ::testing::AssertionFailure()
            << actualExpression << " is " << actualValue << ", " << expectedExpression
            << " is " << expectedValue << ", and their absolute difference " << difference
            << " exceeds " << toleranceExpression << " (" << toleranceValue << ')';
    }

    [[nodiscard]] inline ::testing::AssertionResult RectangleEqual(
        const char* const expectedExpression,
        const char* const actualExpression,
        const Microsoft::Xna::Framework::Rectangle& expected,
        const Microsoft::Xna::Framework::Rectangle& actual)
    {
        if (expected == actual)
        {
            return ::testing::AssertionSuccess();
        }

        return ::testing::AssertionFailure()
            << actualExpression << " is {X:" << actual.X << " Y:" << actual.Y
            << " Width:" << actual.Width << " Height:" << actual.Height << "}, while "
            << expectedExpression << " is {X:" << expected.X << " Y:" << expected.Y
            << " Width:" << expected.Width << " Height:" << expected.Height << '}';
    }

    [[nodiscard]] inline ::testing::AssertionResult ColorEqual(
        const char* const expectedExpression,
        const char* const actualExpression,
        const Microsoft::Xna::Framework::Color& expected,
        const Microsoft::Xna::Framework::Color& actual)
    {
        if (expected == actual)
        {
            return ::testing::AssertionSuccess();
        }

        return ::testing::AssertionFailure()
            << actualExpression << " is RGBA(" << static_cast<int>(actual.getRProperty()) << ", "
            << static_cast<int>(actual.getGProperty()) << ", "
            << static_cast<int>(actual.getBProperty()) << ", "
            << static_cast<int>(actual.getAProperty()) << "), while " << expectedExpression
            << " is RGBA(" << static_cast<int>(expected.getRProperty()) << ", "
            << static_cast<int>(expected.getGProperty()) << ", "
            << static_cast<int>(expected.getBProperty()) << ", "
            << static_cast<int>(expected.getAProperty()) << ')';
    }
}

#define EXPECT_MYRA_NUMERIC_NEAR(expected, actual, tolerance) \
    EXPECT_PRED_FORMAT3(::Myra::Tests::NumericNear, expected, actual, tolerance)
#define ASSERT_MYRA_NUMERIC_NEAR(expected, actual, tolerance) \
    ASSERT_PRED_FORMAT3(::Myra::Tests::NumericNear, expected, actual, tolerance)
#define EXPECT_MYRA_RECTANGLE_EQ(expected, actual) \
    EXPECT_PRED_FORMAT2(::Myra::Tests::RectangleEqual, expected, actual)
#define ASSERT_MYRA_RECTANGLE_EQ(expected, actual) \
    ASSERT_PRED_FORMAT2(::Myra::Tests::RectangleEqual, expected, actual)
#define EXPECT_MYRA_COLOR_EQ(expected, actual) \
    EXPECT_PRED_FORMAT2(::Myra::Tests::ColorEqual, expected, actual)
#define ASSERT_MYRA_COLOR_EQ(expected, actual) \
    ASSERT_PRED_FORMAT2(::Myra::Tests::ColorEqual, expected, actual)

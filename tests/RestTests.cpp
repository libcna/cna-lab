// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Utility/Rest.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/Rest.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    TEST(RestTests, CloneArrayCopiesRowsButRetainsReferenceLikeElements)
    {
        const auto first = std::make_shared<int>(1);
        const auto second = std::make_shared<int>(2);
        const std::vector<std::vector<std::shared_ptr<int>>> source{{first}, {second}};

        auto clone = Myra::Utility::Rest::CloneArray(source);
        clone[0].push_back(second);

        ASSERT_EQ(source[0].size(), 1U);
        ASSERT_EQ(clone[0].size(), 2U);
        EXPECT_EQ(clone[0][0], first);
        EXPECT_EQ(clone[0][1], second);
    }

    TEST(RestTests, SortByColumnOrdersAscendingAndDescending)
    {
        std::vector<std::vector<std::string>> rows{
            {"third", "gamma"},
            {"first", "alpha"},
            {"second", "beta"},
        };

        Myra::Utility::Rest::SortByColumn(rows, 1, true);
        EXPECT_EQ(rows[0][0], "first");
        EXPECT_EQ(rows[1][0], "second");
        EXPECT_EQ(rows[2][0], "third");

        Myra::Utility::Rest::SortByColumn(rows, 1, false);
        EXPECT_EQ(rows[0][0], "third");
        EXPECT_EQ(rows[1][0], "second");
        EXPECT_EQ(rows[2][0], "first");
    }

    TEST(RestTests, SortByColumnRejectsInvalidJaggedColumnsBeforeMutation)
    {
        const std::vector<std::vector<int>> original{{2, 20}, {1}};
        auto rows = original;

        EXPECT_THROW(Myra::Utility::Rest::SortByColumn(rows, -1, true), std::out_of_range);
        EXPECT_EQ(rows, original);

        EXPECT_THROW(Myra::Utility::Rest::SortByColumn(rows, 1, true), std::out_of_range);
        EXPECT_EQ(rows, original);
    }
}

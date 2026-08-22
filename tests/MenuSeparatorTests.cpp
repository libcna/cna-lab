// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/MenuSeparator.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "Myra/Graphics2D/UI/Simple/HorizontalSeparator.hpp"

namespace
{
    using Myra::Graphics2D::UI::HorizontalSeparator;
    using Myra::Graphics2D::UI::MenuSeparator;

    TEST(MenuSeparatorTests, StoresMenuIndependentIdentityIndexAndSeparatorVisual)
    {
        MenuSeparator separator;
        EXPECT_EQ(separator.getMenuProperty(), nullptr);
        EXPECT_FALSE(separator.getIdProperty().has_value());
        EXPECT_FALSE(separator.getUnderscoreCharProperty().has_value());
        EXPECT_EQ(separator.getIndexProperty(), 0);
        EXPECT_EQ(separator.Separator, nullptr);

        separator.setIdProperty(std::string("divider"));
        separator.setIndexProperty(7);
        separator.Separator = std::make_shared<HorizontalSeparator>();
        ASSERT_TRUE(separator.getIdProperty().has_value());
        EXPECT_EQ(*separator.getIdProperty(), "divider");
        EXPECT_EQ(separator.getIndexProperty(), 7);
        EXPECT_NE(separator.Separator, nullptr);
    }
} // namespace

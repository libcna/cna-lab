// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/IHasColor.hpp"

#include <gtest/gtest.h>

namespace
{
    class ColorOwner final : public Myra::MML::IHasColor
    {
    public:
        [[nodiscard]] Microsoft::Xna::Framework::Color getColorProperty() const override
        {
            return Microsoft::Xna::Framework::Color(12, 34, 56, 78);
        }
    };

    TEST(IHasColorTests, ExposesTheCnaColorContract)
    {
        const ColorOwner owner;
        const auto color = owner.getColorProperty();

        EXPECT_EQ(color.getRProperty(), 12U);
        EXPECT_EQ(color.getGProperty(), 34U);
        EXPECT_EQ(color.getBProperty(), 56U);
        EXPECT_EQ(color.getAProperty(), 78U);
    }
}

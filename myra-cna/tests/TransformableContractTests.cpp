// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/ITransformable.hpp"

#include <gtest/gtest.h>

namespace
{
    using Microsoft::Xna::Framework::Vector2;

    class Transformable final : public Myra::Graphics2D::UI::ITransformable
    {
    public:
        [[nodiscard]] Vector2 ToLocal(Vector2 source) override
        {
            return source;
        }

        [[nodiscard]] Vector2 ToGlobal(Vector2 position) override
        {
            return position;
        }
    };

    TEST(TransformableContractTests, PreservesCnaVector2ByValueContract)
    {
        Transformable transformable;
        const Vector2 input(3.0F, 7.0F);

        EXPECT_EQ(transformable.ToLocal(input), input);
        EXPECT_EQ(transformable.ToGlobal(input), input);
    }
}

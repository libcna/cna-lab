// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Attributes/ContentAttribute.hpp"
#include "Myra/Attributes/DesignerFoldedAttribute.hpp"
#include "Myra/Attributes/FilePathAttribute.hpp"
#include "Myra/Attributes/RangeAttribute.hpp"
#include "Myra/Attributes/SkipLoadAttribute.hpp"
#include "Myra/Attributes/SkipSaveAttribute.hpp"
#include "Myra/Attributes/StylePropertyPathAttribute.hpp"
#include "Myra/Attributes/XmlNameAttribute.hpp"

#include <gtest/gtest.h>

#include <type_traits>

namespace
{
    TEST(AttributeMetadataTests, MarkerAttributesAreUsableAsRegistryTags)
    {
        static_assert(std::is_empty_v<Myra::Attributes::ContentAttribute>);
        static_assert(std::is_empty_v<Myra::Attributes::DesignerFoldedAttribute>);
        static_assert(std::is_empty_v<Myra::Attributes::SkipLoadAttribute>);
        static_assert(std::is_empty_v<Myra::Attributes::SkipSaveAttribute>);

        SUCCEED();
    }

    TEST(AttributeMetadataTests, FilePathStoresTheUpstreamDialogMetadata)
    {
        const Myra::Attributes::FilePathAttribute attribute(
            Myra::Graphics2D::UI::File::FileDialogMode::SaveFile, "*.mml", true);

        EXPECT_EQ(attribute.getDialogModeProperty(), Myra::Graphics2D::UI::File::FileDialogMode::SaveFile);
        EXPECT_EQ(attribute.getFilterProperty(), "*.mml");
        EXPECT_TRUE(attribute.getShowPathProperty());
    }

    TEST(AttributeMetadataTests, RangePreservesOptionalBoundsAndRejectsAnInvertedRange)
    {
        const Myra::Attributes::RangeAttribute minimumOnly(2.5F);
        ASSERT_TRUE(minimumOnly.getMinimumProperty().has_value());
        EXPECT_FLOAT_EQ(*minimumOnly.getMinimumProperty(), 2.5F);
        EXPECT_FALSE(minimumOnly.getMaximumProperty().has_value());

        const Myra::Attributes::RangeAttribute closed(1.0F, 3.0F);
        ASSERT_TRUE(closed.getMinimumProperty().has_value());
        ASSERT_TRUE(closed.getMaximumProperty().has_value());
        EXPECT_FLOAT_EQ(*closed.getMinimumProperty(), 1.0F);
        EXPECT_FLOAT_EQ(*closed.getMaximumProperty(), 3.0F);
        EXPECT_THROW(Myra::Attributes::RangeAttribute(4.0F, 3.0F), std::invalid_argument);
    }

    TEST(AttributeMetadataTests, StylePathAndXmlNamePreserveAndValidateNames)
    {
        const Myra::Attributes::StylePropertyPathAttribute stylePath("LabelStyle/TextColor");
        EXPECT_EQ(stylePath.getNameProperty(), "LabelStyle/TextColor");

        const Myra::Attributes::XmlNameAttribute xmlName("TextureRegionAtlas");
        EXPECT_EQ(xmlName.getXmlNameProperty(), "TextureRegionAtlas");
        EXPECT_THROW(Myra::Attributes::XmlNameAttribute(""), std::invalid_argument);
    }
}

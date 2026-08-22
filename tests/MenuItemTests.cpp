// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/InputEventType.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::MenuItem;

    class TestImage final : public IImage
    {
      public:
        void Draw(RenderContext &, const Rectangle, const Color) const override {}
        [[nodiscard]] Point getSizeProperty() const override { return Point(1, 1); }
    };

    TEST(MenuItemTests, ExtractsMnemonicAndDefersRichTextStylingToTheFutureMenuLayer)
    {
        MenuItem item;
        int changed = 0;
        item.Changed += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
            ++changed;
        };

        EXPECT_EQ(item.getTextProperty(), std::optional<std::string>(""));
        item.setTextProperty(std::string("E&xit"));
        item.setTextProperty(std::string("E&xit"));
        EXPECT_EQ(item.getUnderscoreCharProperty(), std::optional<char>('x'));
        EXPECT_EQ(item.getDisplayTextProperty(), std::optional<std::string>("Exit"));
        EXPECT_EQ(item.getDisabledDisplayTextProperty(), std::optional<std::string>("Exit"));
        EXPECT_EQ(changed, 1);

        item.setTextProperty(std::string("Fish && chips"));
        EXPECT_EQ(item.getUnderscoreCharProperty(), std::optional<char>('&'));
        EXPECT_EQ(item.getDisplayTextProperty(), std::optional<std::string>("Fish & chips"));

        item.setTextProperty(std::string("Trailing &"));
        EXPECT_FALSE(item.getUnderscoreCharProperty().has_value());
        EXPECT_EQ(item.getDisplayTextProperty(), std::optional<std::string>("Trailing &"));
    }

    TEST(MenuItemTests, RetainsDataCoreStateAndReportsChangedAndSelectedEvents)
    {
        MenuItem item("file", "&File", std::string("designer tag"));
        const auto image = std::make_shared<TestImage>();
        int changed = 0;
        int selected = 0;
        item.Changed += [&](void *, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
            ++changed;
        };
        item.Selected += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &item);
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::SelectionChanged);
            ++selected;
        };

        item.setImageProperty(image);
        item.setImageProperty(image);
        item.setShortcutTextProperty(std::string("Ctrl+F"));
        item.setShortcutTextProperty(std::string("Ctrl+F"));
        item.setIdProperty(std::string("file-menu"));
        item.setEnabledProperty(false);
        item.setIndexProperty(4);
        item.FireSelected();

        EXPECT_EQ(changed, 3);
        EXPECT_EQ(std::any_cast<const std::string &>(item.getTagProperty()), "designer tag");
        EXPECT_EQ(item.getImageProperty(), image);
        EXPECT_EQ(item.getShortcutTextProperty(), std::optional<std::string>("Ctrl+F"));
        EXPECT_FALSE(item.getEnabledProperty());
        EXPECT_EQ(item.getIndexProperty(), 4);
        EXPECT_EQ(item.getIdProperty(), std::optional<std::string>("file-menu"));
        EXPECT_EQ(selected, 1);
    }
} // namespace

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/HorizontalMenu.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuItem.hpp"
#include "Myra/Graphics2D/UI/Selectors/MenuSeparator.hpp"
#include "Myra/Graphics2D/UI/Selectors/VerticalMenu.hpp"

namespace
{
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::HorizontalMenu;
    using Myra::Graphics2D::UI::MenuItem;
    using Myra::Graphics2D::UI::MenuSeparator;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalMenu;

    TEST(MenuTests, RetainsItemsUpdatesOwnershipAndSearchesNestedSubmenus)
    {
        HorizontalMenu menu;
        const auto file = std::make_shared<MenuItem>("file", "&File");
        const auto separator = std::make_shared<MenuSeparator>();
        const auto edit = std::make_shared<MenuItem>("edit", "&Edit");
        const auto nested = std::make_shared<MenuItem>("new", "&New");

        menu.getItemsProperty().Add(file);
        menu.getItemsProperty().Add(separator);
        menu.getItemsProperty().Add(edit);
        file->getItemsProperty().Add(nested);

        EXPECT_EQ(menu.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(menu.getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(menu.getVerticalAlignmentProperty(), VerticalAlignment::Top);
        EXPECT_TRUE(menu.getAcceptsKeyboardFocusProperty());
        EXPECT_EQ(file->getMenuProperty(), &menu);
        EXPECT_EQ(separator->getMenuProperty(), &menu);
        EXPECT_EQ(edit->getMenuProperty(), &menu);
        EXPECT_EQ(file->getIndexProperty(), 0);
        EXPECT_EQ(separator->getIndexProperty(), 1);
        EXPECT_EQ(edit->getIndexProperty(), 2);
        EXPECT_TRUE(file->getCanOpenProperty());
        EXPECT_EQ(nested->getMenuProperty(), file->getSubMenuProperty().get());
        EXPECT_EQ(menu.FindMenuItemById("new"), nested.get());
        EXPECT_EQ(file->FindMenuItemById("new"), nested.get());
        EXPECT_EQ(menu.FindMenuItemById("missing"), nullptr);

        EXPECT_TRUE(menu.getItemsProperty().Remove(file));
        EXPECT_EQ(file->getMenuProperty(), nullptr);
        EXPECT_EQ(separator->getIndexProperty(), 0);
        EXPECT_EQ(edit->getIndexProperty(), 1);
    }

    TEST(MenuTests, NavigatesSkipsSeparatorsExecutesMnemonicsAndOpensSubmenuState)
    {
        HorizontalMenu menu;
        const auto file = std::make_shared<MenuItem>("file", "&File");
        const auto separator = std::make_shared<MenuSeparator>();
        const auto edit = std::make_shared<MenuItem>("edit", "&Edit");
        const auto nested = std::make_shared<MenuItem>("new", "&New");
        int editSelections = 0;
        int nestedSelections = 0;
        edit->Selected += [&](void *, Myra::Events::MyraEventArgs &) { ++editSelections; };
        nested->Selected += [&](void *, Myra::Events::MyraEventArgs &) { ++nestedSelections; };

        menu.getItemsProperty().Add(file);
        menu.getItemsProperty().Add(separator);
        menu.getItemsProperty().Add(edit);
        file->getItemsProperty().Add(nested);

        menu.MoveHover(1);
        EXPECT_EQ(menu.getHoverIndexProperty(), 0);
        menu.OnKeyDown(Keys::Right);
        EXPECT_EQ(menu.getHoverIndexProperty(), 2);
        menu.OnKeyDown(Keys::Left);
        EXPECT_EQ(menu.getHoverIndexProperty(), 0);

        menu.OnKeyDown(Keys::E);
        EXPECT_EQ(editSelections, 1);
        EXPECT_FALSE(menu.getHoverIndexProperty().has_value());
        EXPECT_FALSE(menu.getSelectedIndexProperty().has_value());

        menu.Click(0);
        EXPECT_TRUE(menu.getIsOpenProperty());
        EXPECT_EQ(menu.getOpenMenuItemProperty(), file.get());
        EXPECT_EQ(menu.getSelectedIndexProperty(), 0);
        menu.OnKeyDown(Keys::N);
        EXPECT_EQ(nestedSelections, 1);

        menu.Close();
        EXPECT_FALSE(menu.getIsOpenProperty());
        EXPECT_FALSE(menu.getHoverIndexProperty().has_value());
        EXPECT_FALSE(menu.getSelectedIndexProperty().has_value());
    }

    TEST(MenuTests, VerticalMenuUsesUpAndDownAndItsOwnAlignmentDefaults)
    {
        VerticalMenu menu;
        const auto first = std::make_shared<MenuItem>("first", "&First");
        const auto second = std::make_shared<MenuItem>("second", "&Second");
        menu.getItemsProperty().Add(first);
        menu.getItemsProperty().Add(second);

        EXPECT_EQ(menu.getOrientationProperty(), Orientation::Vertical);
        EXPECT_EQ(menu.getHorizontalAlignmentProperty(), HorizontalAlignment::Left);
        EXPECT_EQ(menu.getVerticalAlignmentProperty(), VerticalAlignment::Top);
        menu.OnKeyDown(Keys::Down);
        EXPECT_EQ(menu.getHoverIndexProperty(), 0);
        menu.OnKeyDown(Keys::Up);
        EXPECT_EQ(menu.getHoverIndexProperty(), 1);
    }
} // namespace

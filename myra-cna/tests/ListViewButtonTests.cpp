// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/ListViewButton.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "Myra/Graphics2D/UI/Widget.hpp"

namespace
{
    using Myra::Graphics2D::UI::ListViewButton;
    using Myra::Graphics2D::UI::Widget;

    TEST(ListViewButtonTests, RetainsOnePressedButtonAcrossAnExplicitNestedGroup)
    {
        Widget buttonsContainer;
        const auto first = std::make_shared<ListViewButton>();
        const auto wrapper = std::make_shared<Widget>();
        const auto second = std::make_shared<ListViewButton>();
        buttonsContainer.AddChild(first);
        buttonsContainer.AddChild(wrapper);
        wrapper->AddChild(second);
        first->setButtonsContainerProperty(&buttonsContainer);
        second->setButtonsContainerProperty(&buttonsContainer);

        first->setIsPressedProperty(true);
        first->setIsPressedProperty(false);
        EXPECT_TRUE(first->getIsPressedProperty());

        second->setIsPressedProperty(true);
        EXPECT_FALSE(first->getIsPressedProperty());
        EXPECT_TRUE(second->getIsPressedProperty());
        second->setIsPressedProperty(false);
        EXPECT_TRUE(second->getIsPressedProperty());

        first->setIsPressedProperty(true);
        EXPECT_TRUE(first->getIsPressedProperty());
        EXPECT_FALSE(second->getIsPressedProperty());
    }

    TEST(ListViewButtonTests, ClonePreservesTheConcreteTypeAndContent)
    {
        ListViewButton source;
        source.setIsPressedProperty(true);
        const auto content = std::make_shared<Widget>();
        content->setWidthProperty(19);
        source.setContentProperty(content);

        const auto clone = std::dynamic_pointer_cast<ListViewButton>(source.Clone());
        ASSERT_NE(clone, nullptr);
        EXPECT_TRUE(clone->getIsPressedProperty());
        ASSERT_NE(clone->getContentProperty(), nullptr);
        EXPECT_NE(clone->getContentProperty(), content);
        EXPECT_EQ(clone->getContentProperty()->getWidthProperty(), 19);
        EXPECT_EQ(clone->getButtonsContainerProperty(), nullptr);
    }
} // namespace

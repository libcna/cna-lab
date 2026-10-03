// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Selectors/Selector.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace
{
    using Myra::Graphics2D::UI::InputEventType;
    using Myra::Graphics2D::UI::ISelectorItem;
    using Myra::Graphics2D::UI::SelectionMode;
    using Myra::Graphics2D::UI::Selector;
    using Myra::Graphics2D::UI::Widget;

    class TestItem final : public ISelectorItem
    {
      public:
        [[nodiscard]] bool getIsSelectedProperty() const noexcept override { return selected_; }

        void setIsSelectedProperty(const bool value) noexcept override { selected_ = value; }

      private:
        bool selected_ = false;
    };

    class TestDisplay final : public Widget
    {
      protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return std::make_shared<TestDisplay>();
        }
    };

    class TestSelector final : public Selector<TestDisplay, TestItem>
    {
      public:
        TestSelector() : Selector<TestDisplay, TestItem>(std::make_shared<TestDisplay>()) {}

        [[nodiscard]] const std::vector<std::shared_ptr<TestItem>> &getDisplayedItems() const noexcept
        {
            return displayedItems_;
        }

        [[nodiscard]] int getResetCount() const noexcept { return resetCount_; }

      protected:
        void Reset() override
        {
            ++resetCount_;
            displayedItems_.clear();
        }

        void InsertItem(std::shared_ptr<TestItem> item, const int index) override
        {
            displayedItems_.insert(displayedItems_.begin() + index, std::move(item));
        }

        void RemoveItem(const std::shared_ptr<TestItem> &item) override
        {
            const auto iterator = std::find(displayedItems_.begin(), displayedItems_.end(), item);
            if (iterator != displayedItems_.end())
            {
                displayedItems_.erase(iterator);
            }
        }

      private:
        std::vector<std::shared_ptr<TestItem>> displayedItems_;
        int resetCount_ = 0;
    };

    TEST(SelectorTests, CollectionMutationsDriveTheDerivedDisplayAndEvent)
    {
        TestSelector selector;
        const auto first = std::make_shared<TestItem>();
        const auto second = std::make_shared<TestItem>();
        int collectionChanges = 0;
        InputEventType eventType = InputEventType::None;
        selector.ItemsCollectionChanged += [&](void *sender, Myra::Events::MyraEventArgs &arguments)
        {
            EXPECT_EQ(sender, &selector);
            ++collectionChanges;
            eventType = arguments.getEventTypeProperty();
        };

        selector.getItemsProperty().Add(first);
        selector.getItemsProperty().Insert(0, second);
        ASSERT_EQ(selector.getDisplayedItems().size(), 2U);
        EXPECT_EQ(selector.getDisplayedItems()[0], second);
        EXPECT_EQ(selector.getDisplayedItems()[1], first);
        EXPECT_EQ(collectionChanges, 2);
        EXPECT_EQ(eventType, InputEventType::ValueChanged);

        EXPECT_TRUE(selector.getItemsProperty().Remove(second));
        ASSERT_EQ(selector.getDisplayedItems().size(), 1U);
        EXPECT_EQ(selector.getDisplayedItems().front(), first);
        selector.getItemsProperty().Clear();
        EXPECT_TRUE(selector.getDisplayedItems().empty());
        EXPECT_EQ(selector.getResetCount(), 1);
        EXPECT_EQ(collectionChanges, 4);
    }

    TEST(SelectorTests, SelectionPreservesThePinnedSingleAndMultipleSemantics)
    {
        TestSelector selector;
        const auto first = std::make_shared<TestItem>();
        const auto second = std::make_shared<TestItem>();
        selector.getItemsProperty().Add(first);
        selector.getItemsProperty().Add(second);

        int changes = 0;
        selector.SelectedIndexChanged += [&](void *, Myra::Events::MyraEventArgs &arguments)
        {
            ++changes;
            EXPECT_EQ(arguments.getEventTypeProperty(), InputEventType::ValueChanged);
        };

        selector.setSelectedIndexProperty(0);
        EXPECT_EQ(selector.getSelectedItemProperty(), first);
        EXPECT_EQ(selector.getSelectedIndexProperty(), 0);
        EXPECT_TRUE(first->getIsSelectedProperty());

        selector.setSelectedItemProperty(second);
        EXPECT_FALSE(first->getIsSelectedProperty());
        EXPECT_TRUE(second->getIsSelectedProperty());
        EXPECT_EQ(selector.getSelectedIndexProperty(), 1);

        selector.setSelectionModeProperty(SelectionMode::Multiple);
        selector.setSelectedItemProperty(first);
        EXPECT_TRUE(first->getIsSelectedProperty());
        EXPECT_TRUE(second->getIsSelectedProperty());
        EXPECT_EQ(changes, 3);

        EXPECT_TRUE(selector.getItemsProperty().Remove(first));
        EXPECT_EQ(selector.getSelectedItemProperty(), first);
        EXPECT_EQ(selector.getSelectedIndexProperty(), -1);
        selector.setSelectedIndexProperty(std::nullopt);
        EXPECT_EQ(selector.getSelectedItemProperty(), nullptr);
        EXPECT_EQ(changes, 4);
    }
} // namespace

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/UI/ContentControl.hpp"
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/Panel.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"
#include "Myra/Graphics2D/UI/Widget.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace
{
    using Microsoft::Xna::Framework::Vector2;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::UI::ContentControl;
    using Myra::Graphics2D::UI::DragDirection;
    using Myra::Graphics2D::UI::Grid;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::MouseCursorType;
    using Myra::Graphics2D::UI::Panel;
    using Myra::Graphics2D::UI::Proportion;
    using Myra::Graphics2D::UI::ProportionType;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalStackPanel;
    using Myra::Graphics2D::UI::Widget;

    class CustomWidget final : public Widget
    {
    public:
        explicit CustomWidget(const int payload) : payload_(payload) {}

        [[nodiscard]] int getPayloadProperty() const noexcept { return payload_; }

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return std::make_shared<CustomWidget>(0);
        }

        void CopyFrom(const Widget& source) override
        {
            Widget::CopyFrom(source);
            const auto* const custom = dynamic_cast<const CustomWidget*>(&source);
            if (custom == nullptr)
            {
                throw std::invalid_argument("CustomWidget copy source must preserve its type.");
            }
            payload_ = custom->payload_;
        }

    private:
        int payload_;
    };

    static_assert(!std::is_default_constructible_v<CustomWidget>);

    class MissingFactoryWidget final : public Widget {};

    class NullFactoryWidget final : public Widget
    {
    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return nullptr;
        }
    };

    class TestContentControl final : public ContentControl
    {
    public:
        [[nodiscard]] std::shared_ptr<Widget> getContentProperty() const override
        {
            return content_;
        }

        void setContentProperty(std::shared_ptr<Widget> value) override
        {
            content_ = std::move(value);
        }

    protected:
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override
        {
            return std::make_shared<TestContentControl>();
        }

    private:
        std::shared_ptr<Widget> content_;
    };

    TEST(WidgetCloneTests, CopiesPortedStateAndAttachedValuesButNotTransientIdentity)
    {
        Widget source;
        source.setStyleNameProperty(std::string("accent"));
        source.setLeftProperty(2);
        source.setTopProperty(3);
        source.setMinWidthProperty(4);
        source.setMaxWidthProperty(40);
        source.setWidthProperty(20);
        source.setMinHeightProperty(5);
        source.setMaxHeightProperty(50);
        source.setHeightProperty(25);
        source.setMarginProperty(Thickness(1, 2, 3, 4));
        source.setBorderThicknessProperty(Thickness(2));
        source.setPaddingProperty(Thickness(3));
        source.setHorizontalAlignmentProperty(HorizontalAlignment::Right);
        source.setVerticalAlignmentProperty(VerticalAlignment::Bottom);
        source.setEnabledProperty(false);
        source.setVisibleProperty(false);
        source.setDragDirectionProperty(DragDirection::Both);
        source.setZIndexProperty(7);
        source.setMouseCursorProperty(MouseCursorType::Crosshair);
        source.setTooltipProperty(std::string("tip"));
        source.setScaleProperty(Vector2(2.0F, 3.0F));
        source.setTransformOriginProperty(Vector2(0.25F, 0.75F));
        source.setRotationProperty(0.5F);
        source.setIsModalProperty(true);
        source.setOpacityProperty(0.4F);
        source.setClipToBoundsProperty(true);
        source.setTagProperty(std::string("tag"));
        source.setAcceptsKeyboardFocusProperty(true);
        source.setIsPressedProperty(true);
        source.OnGotKeyboardFocus();
        source.setIdProperty(std::string("identity"));
        source.getUserDataProperty().emplace("_note", "not cloned upstream");
        source.AttachedPropertiesValues.emplace(91, 42);

        const std::shared_ptr<Widget> clone = source.Clone();

        ASSERT_NE(clone, nullptr);
        EXPECT_NE(clone.get(), &source);
        EXPECT_EQ(clone->getStyleNameProperty(), source.getStyleNameProperty());
        EXPECT_EQ(clone->getLeftProperty(), 2);
        EXPECT_EQ(clone->getTopProperty(), 3);
        EXPECT_EQ(clone->getMinWidthProperty(), 4);
        EXPECT_EQ(clone->getMaxWidthProperty(), 40);
        EXPECT_EQ(clone->getWidthProperty(), 20);
        EXPECT_EQ(clone->getMinHeightProperty(), 5);
        EXPECT_EQ(clone->getMaxHeightProperty(), 50);
        EXPECT_EQ(clone->getHeightProperty(), 25);
        EXPECT_EQ(clone->getMarginProperty(), Thickness(1, 2, 3, 4));
        EXPECT_EQ(clone->getBorderThicknessProperty(), Thickness(2));
        EXPECT_EQ(clone->getPaddingProperty(), Thickness(3));
        EXPECT_EQ(clone->getHorizontalAlignmentProperty(), HorizontalAlignment::Right);
        EXPECT_EQ(clone->getVerticalAlignmentProperty(), VerticalAlignment::Bottom);
        EXPECT_FALSE(clone->getEnabledProperty());
        EXPECT_FALSE(clone->getVisibleProperty());
        EXPECT_EQ(clone->getDragDirectionProperty(), DragDirection::Both);
        EXPECT_EQ(clone->getZIndexProperty(), 7);
        EXPECT_EQ(clone->getMouseCursorProperty(), MouseCursorType::Crosshair);
        EXPECT_EQ(clone->getTooltipProperty(), std::optional<std::string>("tip"));
        EXPECT_EQ(clone->getScaleProperty(), Vector2(2.0F, 3.0F));
        EXPECT_EQ(clone->getTransformOriginProperty(), Vector2(0.25F, 0.75F));
        EXPECT_FLOAT_EQ(clone->getRotationProperty(), 0.5F);
        EXPECT_TRUE(clone->getIsModalProperty());
        EXPECT_FLOAT_EQ(clone->getOpacityProperty(), 0.4F);
        EXPECT_TRUE(clone->getClipToBoundsProperty());
        EXPECT_EQ(std::any_cast<std::string>(clone->getTagProperty()), "tag");
        EXPECT_TRUE(clone->getAcceptsKeyboardFocusProperty());
        EXPECT_EQ(clone->getDragHandleProperty(), clone.get());
        EXPECT_FALSE(clone->getIsPressedProperty());
        EXPECT_FALSE(clone->getIsKeyboardFocusedProperty());
        EXPECT_FALSE(clone->getIdProperty().has_value());
        EXPECT_TRUE(clone->getUserDataProperty().empty());
        ASSERT_TRUE(clone->AttachedPropertiesValues.contains(91));
        EXPECT_EQ(std::any_cast<int>(clone->AttachedPropertiesValues.at(91)), 42);
    }

    TEST(WidgetCloneTests, VirtualFactoryPreservesCustomNonDefaultConstructibleType)
    {
        CustomWidget source(73);
        source.setStyleNameProperty(std::string("custom"));

        const std::shared_ptr<Widget> baseClone = source.Clone();
        const std::shared_ptr<CustomWidget> clone = std::dynamic_pointer_cast<CustomWidget>(baseClone);

        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getPayloadProperty(), 73);
        EXPECT_EQ(clone->getStyleNameProperty(), std::optional<std::string>("custom"));
    }

    TEST(WidgetCloneTests, RejectsMissingOrNullDynamicFactoriesInsteadOfSlicing)
    {
        MissingFactoryWidget missing;
        NullFactoryWidget nullFactory;

        EXPECT_THROW(static_cast<void>(missing.Clone()), std::logic_error);
        EXPECT_THROW(static_cast<void>(nullFactory.Clone()), std::logic_error);
    }

    TEST(WidgetCloneTests, ContainerCloneOwnsIndependentDynamicChildren)
    {
        Panel source;
        auto child = std::make_shared<CustomWidget>(11);
        child->AttachedPropertiesValues.emplace(17, std::string("attached"));
        source.AddWidget(child);

        const std::shared_ptr<Panel> clone = std::dynamic_pointer_cast<Panel>(source.Clone());

        ASSERT_NE(clone, nullptr);
        ASSERT_EQ(clone->getWidgetsProperty().size(), 1U);
        const std::shared_ptr<CustomWidget> clonedChild =
            std::dynamic_pointer_cast<CustomWidget>(clone->getWidgetsProperty().front());
        ASSERT_NE(clonedChild, nullptr);
        EXPECT_NE(clonedChild, child);
        EXPECT_EQ(clonedChild->getPayloadProperty(), 11);
        EXPECT_EQ(clonedChild->getParentProperty(), clone.get());
        EXPECT_EQ(child->getParentProperty(), &source);
        EXPECT_EQ(std::any_cast<std::string>(clonedChild->AttachedPropertiesValues.at(17)), "attached");
    }

    TEST(WidgetCloneTests, ContentControlDeepCopiesPresentContentAndPreservesNull)
    {
        TestContentControl empty;
        const std::shared_ptr<TestContentControl> emptyClone =
            std::dynamic_pointer_cast<TestContentControl>(empty.Clone());
        ASSERT_NE(emptyClone, nullptr);
        EXPECT_EQ(emptyClone->getContentProperty(), nullptr);

        TestContentControl source;
        auto content = std::make_shared<CustomWidget>(29);
        source.setContentProperty(content);

        const std::shared_ptr<TestContentControl> clone =
            std::dynamic_pointer_cast<TestContentControl>(source.Clone());

        ASSERT_NE(clone, nullptr);
        const std::shared_ptr<CustomWidget> clonedContent =
            std::dynamic_pointer_cast<CustomWidget>(clone->getContentProperty());
        ASSERT_NE(clonedContent, nullptr);
        EXPECT_NE(clonedContent, content);
        EXPECT_EQ(clonedContent->getPayloadProperty(), 29);
    }

    TEST(WidgetCloneTests, GridAndStackPanelFactoriesCopyTheirPortedState)
    {
        Grid grid;
        grid.setColumnSpacingProperty(3);
        grid.setRowSpacingProperty(4);
        auto defaultColumn = std::make_shared<Proportion>(ProportionType::Pixels, 21.0F);
        auto defaultRow = std::make_shared<Proportion>(ProportionType::Fill);
        auto explicitColumn = std::make_shared<Proportion>(ProportionType::Part, 2.0F);
        auto explicitRow = std::make_shared<Proportion>(ProportionType::Auto);
        grid.setDefaultColumnProportionProperty(defaultColumn);
        grid.setDefaultRowProportionProperty(defaultRow);
        grid.getColumnsProportionsProperty().Add(explicitColumn);
        grid.getRowsProportionsProperty().Add(explicitRow);
        grid.AddWidget(std::make_shared<Widget>());

        const std::shared_ptr<Grid> gridClone = std::dynamic_pointer_cast<Grid>(grid.Clone());

        ASSERT_NE(gridClone, nullptr);
        EXPECT_EQ(gridClone->getColumnSpacingProperty(), 3);
        EXPECT_EQ(gridClone->getRowSpacingProperty(), 4);
        EXPECT_EQ(gridClone->getDefaultColumnProportionProperty(), defaultColumn);
        EXPECT_EQ(gridClone->getDefaultRowProportionProperty(), defaultRow);
        ASSERT_EQ(gridClone->getColumnsProportionsProperty().getCountProperty(), 1);
        ASSERT_EQ(gridClone->getRowsProportionsProperty().getCountProperty(), 1);
        EXPECT_EQ(gridClone->getColumnsProportionsProperty()[0], explicitColumn);
        EXPECT_EQ(gridClone->getRowsProportionsProperty()[0], explicitRow);
        ASSERT_EQ(gridClone->getWidgetsProperty().size(), 1U);
        EXPECT_NE(gridClone->getWidgetsProperty().front(), grid.getWidgetsProperty().front());

        VerticalStackPanel stack;
        stack.setSpacingProperty(9);
        auto defaultProportion = std::make_shared<Proportion>(ProportionType::Pixels, 15.0F);
        stack.setDefaultProportionProperty(defaultProportion);
        stack.AddWidget(std::make_shared<Widget>());

        const std::shared_ptr<VerticalStackPanel> stackClone =
            std::dynamic_pointer_cast<VerticalStackPanel>(stack.Clone());

        ASSERT_NE(stackClone, nullptr);
        EXPECT_EQ(stackClone->getSpacingProperty(), 9);
        EXPECT_EQ(stackClone->getDefaultProportionProperty(), defaultProportion);
        ASSERT_EQ(stackClone->getWidgetsProperty().size(), 1U);
        EXPECT_NE(stackClone->getWidgetsProperty().front(), stack.getWidgetsProperty().front());
    }
}

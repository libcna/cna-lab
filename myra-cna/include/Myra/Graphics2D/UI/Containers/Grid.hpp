// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Grid.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <optional>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Myra/Events/MyraEventHandler.hpp"
#include "Myra/Graphics2D/IBrush.hpp"
#include "Myra/Graphics2D/UI/Container.hpp"
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

namespace Myra::Graphics2D::UI
{
    /** @brief Selectable unit used by Grid pointer interaction and rendering. */
    enum class GridSelectionMode
    {
        None,
        Row,
        Column,
        Cell
    };

    /** @brief Retained-mode container that places children into rows and columns. */
    class Grid : public Container
    {
      public:
        Grid();
        ~Grid() override;

        Events::MyraEventHandler SelectedIndexChanged;
        Events::MyraEventHandler HoverIndexChanged;

        [[nodiscard]] bool getShowGridLinesProperty() const noexcept;
        void setShowGridLinesProperty(bool value) noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Color getGridLinesColorProperty() const noexcept;
        void setGridLinesColorProperty(Microsoft::Xna::Framework::Color value) noexcept;
        [[nodiscard]] int getColumnSpacingProperty() const noexcept;
        void setColumnSpacingProperty(int value);
        [[nodiscard]] int getRowSpacingProperty() const noexcept;
        void setRowSpacingProperty(int value);
        [[nodiscard]] const std::shared_ptr<Proportion> &getDefaultColumnProportionProperty() const noexcept;
        void setDefaultColumnProportionProperty(std::shared_ptr<Proportion> value);
        [[nodiscard]] const std::shared_ptr<Proportion> &getDefaultRowProportionProperty() const noexcept;
        void setDefaultRowProportionProperty(std::shared_ptr<Proportion> value);
        [[nodiscard]] const ProportionCollection &getColumnsProportionsProperty() const noexcept;
        [[nodiscard]] ProportionCollection &getColumnsProportionsProperty() noexcept;
        [[nodiscard]] const ProportionCollection &getRowsProportionsProperty() const noexcept;
        [[nodiscard]] ProportionCollection &getRowsProportionsProperty() noexcept;
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getSelectionBackgroundProperty() const;
        void setSelectionBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] std::shared_ptr<Graphics2D::IBrush> getSelectionHoverBackgroundProperty() const;
        void setSelectionHoverBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value);
        [[nodiscard]] GridSelectionMode getGridSelectionModeProperty() const noexcept;
        void setGridSelectionModeProperty(GridSelectionMode value) noexcept;
        [[nodiscard]] bool getHoverIndexCanBeNullProperty() const noexcept;
        void setHoverIndexCanBeNullProperty(bool value) noexcept;
        [[nodiscard]] bool getCanSelectNothingProperty() const noexcept;
        void setCanSelectNothingProperty(bool value) noexcept;

        [[nodiscard]] const std::vector<int> &getGridLinesXProperty() const noexcept;
        [[nodiscard]] const std::vector<int> &getGridLinesYProperty() const noexcept;
        [[nodiscard]] const std::vector<int> &getColWidthsProperty() const noexcept;
        [[nodiscard]] const std::vector<int> &getRowHeightsProperty() const noexcept;
        [[nodiscard]] const std::vector<int> &getCellLocationsXProperty() const noexcept;
        [[nodiscard]] const std::vector<int> &getCellLocationsYProperty() const noexcept;

        [[nodiscard]] const std::optional<int> &getHoverRowIndexProperty() const noexcept;
        void setHoverRowIndexProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int> &getHoverColumnIndexProperty() const noexcept;
        void setHoverColumnIndexProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int> &getSelectedRowIndexProperty() const noexcept;
        void setSelectedRowIndexProperty(std::optional<int> value);
        [[nodiscard]] const std::optional<int> &getSelectedColumnIndexProperty() const noexcept;
        void setSelectedColumnIndexProperty(std::optional<int> value);

        [[nodiscard]] int GetColumnWidth(int index) const noexcept;
        [[nodiscard]] int GetRowHeight(int index) const noexcept;
        [[nodiscard]] int GetCellLocationX(int column) const noexcept;
        [[nodiscard]] int GetCellLocationY(int row) const noexcept;
        [[nodiscard]] Microsoft::Xna::Framework::Rectangle GetCellRectangle(int column, int row) const noexcept;

        [[nodiscard]] static const MML::AttachedPropertyInfo<int> &getColumnProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<int> &getRowProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<int> &getColumnSpanProperty();
        [[nodiscard]] static const MML::AttachedPropertyInfo<int> &getRowSpanProperty();
        [[nodiscard]] static int GetColumn(const Widget &widget);
        static void SetColumn(Widget &widget, int value);
        [[nodiscard]] static int GetRow(const Widget &widget);
        static void SetRow(Widget &widget, int value);
        [[nodiscard]] static int GetColumnSpan(const Widget &widget);
        static void SetColumnSpan(Widget &widget, int value);
        [[nodiscard]] static int GetRowSpan(const Widget &widget);
        static void SetRowSpan(Widget &widget, int value);

        void OnMouseLeft() override;
        void OnMouseEntered() override;
        void OnMouseMoved() override;
        void OnTouchDown() override;

      protected:
        void InternalRender(Graphics2D::RenderContext &context) override;
        void OnChildAdded(Widget &child) override;
        void OnChildRemoved(Widget &child) override;
        [[nodiscard]] std::shared_ptr<Widget> CreateCloneInstance() const override;
        void CopyFrom(const Widget &source) override;

      private:
        struct ProportionSubscription
        {
            std::shared_ptr<Proportion> proportion;
            Events::MyraEventHandler::Token token = Events::MyraEventHandler::InvalidToken;
        };

        void OnProportionsCollectionChanged();
        void RebuildProportionSubscriptions();
        void ClearProportionSubscriptions();
        void RenderSelection(Graphics2D::RenderContext &context);
        void UpdateHoverPosition(std::optional<Microsoft::Xna::Framework::Point> position);

        GridLayout layout_;
        std::vector<ProportionSubscription> proportionSubscriptions_;
        bool showGridLines_ = false;
        Microsoft::Xna::Framework::Color gridLinesColor_;
        std::shared_ptr<Graphics2D::IBrush> selectionBackground_;
        std::shared_ptr<Graphics2D::IBrush> selectionHoverBackground_;
        GridSelectionMode gridSelectionMode_ = GridSelectionMode::None;
        bool hoverIndexCanBeNull_ = true;
        bool canSelectNothing_ = false;
        std::optional<int> hoverRowIndex_;
        std::optional<int> hoverColumnIndex_;
        std::optional<int> selectedRowIndex_;
        std::optional<int> selectedColumnIndex_;
    };
} // namespace Myra::Graphics2D::UI

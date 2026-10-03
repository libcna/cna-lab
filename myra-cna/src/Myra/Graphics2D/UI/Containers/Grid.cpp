// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/Grid.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/Grid.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "Myra/Attributes/RangeAttribute.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/Desktop.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    namespace
    {
        MML::PropertyMetadata RangeMetadata(const float minimum)
        {
            MML::PropertyMetadata metadata;
            metadata.Range = Attributes::RangeAttribute(minimum);
            return metadata;
        }

        [[nodiscard]] int CheckedGridCoordinate(const std::int64_t value, const char *const message)
        {
            if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            {
                throw std::overflow_error(message);
            }
            return static_cast<int>(value);
        }

        [[nodiscard]] int CheckedGridValue(const std::vector<int> &values, const int index, const char *const message)
        {
            if (index < 0 || static_cast<std::size_t>(index) >= values.size())
            {
                throw std::out_of_range(message);
            }
            return values[static_cast<std::size_t>(index)];
        }
    } // namespace

    Grid::Grid() : gridLinesColor_(Microsoft::Xna::Framework::Color::White)
    {
        setChildrenLayoutProperty(&layout_);
        layout_.getColumnsProportionsProperty().CollectionChanged.emplace_back([this](void *, const auto &)
                                                                               { OnProportionsCollectionChanged(); });
        layout_.getRowsProportionsProperty().CollectionChanged.emplace_back([this](void *, const auto &)
                                                                            { OnProportionsCollectionChanged(); });
    }

    Grid::~Grid()
    {
        ClearProportionSubscriptions();
    }

    std::shared_ptr<Widget> Grid::CreateCloneInstance() const
    {
        return std::make_shared<Grid>();
    }

    void Grid::CopyFrom(const Widget &source)
    {
        Container::CopyFrom(source);
        const auto *const grid = dynamic_cast<const Grid *>(&source);
        if (grid == nullptr)
        {
            throw std::invalid_argument("Grid copy source must be a Grid.");
        }

        setColumnSpacingProperty(grid->getColumnSpacingProperty());
        setRowSpacingProperty(grid->getRowSpacingProperty());
        setShowGridLinesProperty(grid->getShowGridLinesProperty());
        setGridLinesColorProperty(grid->getGridLinesColorProperty());
        setDefaultColumnProportionProperty(grid->getDefaultColumnProportionProperty());
        setDefaultRowProportionProperty(grid->getDefaultRowProportionProperty());
        setSelectionBackgroundProperty(grid->getSelectionBackgroundProperty());
        setSelectionHoverBackgroundProperty(grid->getSelectionHoverBackgroundProperty());
        setGridSelectionModeProperty(grid->getGridSelectionModeProperty());
        setHoverIndexCanBeNullProperty(grid->getHoverIndexCanBeNullProperty());
        setCanSelectNothingProperty(grid->getCanSelectNothingProperty());
        for (const std::shared_ptr<Proportion> &proportion : grid->getColumnsProportionsProperty())
        {
            getColumnsProportionsProperty().Add(proportion);
        }
        for (const std::shared_ptr<Proportion> &proportion : grid->getRowsProportionsProperty())
        {
            getRowsProportionsProperty().Add(proportion);
        }
    }

    bool Grid::getShowGridLinesProperty() const noexcept
    {
        return showGridLines_;
    }

    void Grid::setShowGridLinesProperty(const bool value) noexcept
    {
        showGridLines_ = value;
    }

    Microsoft::Xna::Framework::Color Grid::getGridLinesColorProperty() const noexcept
    {
        return gridLinesColor_;
    }

    void Grid::setGridLinesColorProperty(const Microsoft::Xna::Framework::Color value) noexcept
    {
        gridLinesColor_ = value;
    }

    int Grid::getColumnSpacingProperty() const noexcept
    {
        return layout_.getColumnSpacingProperty();
    }

    void Grid::setColumnSpacingProperty(const int value)
    {
        if (value == layout_.getColumnSpacingProperty())
        {
            return;
        }
        layout_.setColumnSpacingProperty(value);
        InvalidateMeasure();
    }

    int Grid::getRowSpacingProperty() const noexcept
    {
        return layout_.getRowSpacingProperty();
    }

    void Grid::setRowSpacingProperty(const int value)
    {
        if (value == layout_.getRowSpacingProperty())
        {
            return;
        }
        layout_.setRowSpacingProperty(value);
        InvalidateMeasure();
    }

    const std::shared_ptr<Proportion> &Grid::getDefaultColumnProportionProperty() const noexcept
    {
        return layout_.getDefaultColumnProportionProperty();
    }

    void Grid::setDefaultColumnProportionProperty(std::shared_ptr<Proportion> value)
    {
        layout_.setDefaultColumnProportionProperty(std::move(value));
        InvalidateMeasure();
    }

    const std::shared_ptr<Proportion> &Grid::getDefaultRowProportionProperty() const noexcept
    {
        return layout_.getDefaultRowProportionProperty();
    }

    void Grid::setDefaultRowProportionProperty(std::shared_ptr<Proportion> value)
    {
        layout_.setDefaultRowProportionProperty(std::move(value));
        InvalidateMeasure();
    }

    const ProportionCollection &Grid::getColumnsProportionsProperty() const noexcept
    {
        return layout_.getColumnsProportionsProperty();
    }

    ProportionCollection &Grid::getColumnsProportionsProperty() noexcept
    {
        return layout_.getColumnsProportionsProperty();
    }

    const ProportionCollection &Grid::getRowsProportionsProperty() const noexcept
    {
        return layout_.getRowsProportionsProperty();
    }

    ProportionCollection &Grid::getRowsProportionsProperty() noexcept
    {
        return layout_.getRowsProportionsProperty();
    }

    std::shared_ptr<Graphics2D::IBrush> Grid::getSelectionBackgroundProperty() const
    {
        return selectionBackground_;
    }

    void Grid::setSelectionBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        selectionBackground_ = std::move(value);
    }

    std::shared_ptr<Graphics2D::IBrush> Grid::getSelectionHoverBackgroundProperty() const
    {
        return selectionHoverBackground_;
    }

    void Grid::setSelectionHoverBackgroundProperty(std::shared_ptr<Graphics2D::IBrush> value)
    {
        selectionHoverBackground_ = std::move(value);
    }

    GridSelectionMode Grid::getGridSelectionModeProperty() const noexcept
    {
        return gridSelectionMode_;
    }

    void Grid::setGridSelectionModeProperty(const GridSelectionMode value) noexcept
    {
        gridSelectionMode_ = value;
    }

    bool Grid::getHoverIndexCanBeNullProperty() const noexcept
    {
        return hoverIndexCanBeNull_;
    }

    void Grid::setHoverIndexCanBeNullProperty(const bool value) noexcept
    {
        hoverIndexCanBeNull_ = value;
    }

    bool Grid::getCanSelectNothingProperty() const noexcept
    {
        return canSelectNothing_;
    }

    void Grid::setCanSelectNothingProperty(const bool value) noexcept
    {
        canSelectNothing_ = value;
    }

    const std::vector<int> &Grid::getGridLinesXProperty() const noexcept
    {
        return layout_.getGridLinesXProperty();
    }

    const std::vector<int> &Grid::getGridLinesYProperty() const noexcept
    {
        return layout_.getGridLinesYProperty();
    }

    const std::vector<int> &Grid::getColWidthsProperty() const noexcept
    {
        return layout_.getColWidthsProperty();
    }

    const std::vector<int> &Grid::getRowHeightsProperty() const noexcept
    {
        return layout_.getRowHeightsProperty();
    }

    const std::vector<int> &Grid::getCellLocationsXProperty() const noexcept
    {
        return layout_.getCellLocationsXProperty();
    }

    const std::vector<int> &Grid::getCellLocationsYProperty() const noexcept
    {
        return layout_.getCellLocationsYProperty();
    }

    const std::optional<int> &Grid::getHoverRowIndexProperty() const noexcept
    {
        return hoverRowIndex_;
    }

    void Grid::setHoverRowIndexProperty(std::optional<int> value)
    {
        if (value == hoverRowIndex_)
        {
            return;
        }
        hoverRowIndex_ = std::move(value);
        Utility::EventsExtensions::Invoke(HoverIndexChanged, this, InputEventType::HoverIndexChanged);
    }

    const std::optional<int> &Grid::getHoverColumnIndexProperty() const noexcept
    {
        return hoverColumnIndex_;
    }

    void Grid::setHoverColumnIndexProperty(std::optional<int> value)
    {
        if (value == hoverColumnIndex_)
        {
            return;
        }
        hoverColumnIndex_ = std::move(value);
        Utility::EventsExtensions::Invoke(HoverIndexChanged, this, InputEventType::HoverIndexChanged);
    }

    const std::optional<int> &Grid::getSelectedRowIndexProperty() const noexcept
    {
        return selectedRowIndex_;
    }

    void Grid::setSelectedRowIndexProperty(std::optional<int> value)
    {
        if (value == selectedRowIndex_)
        {
            return;
        }
        selectedRowIndex_ = std::move(value);
        Utility::EventsExtensions::Invoke(SelectedIndexChanged, this, InputEventType::SelectedIndexChanged);
    }

    const std::optional<int> &Grid::getSelectedColumnIndexProperty() const noexcept
    {
        return selectedColumnIndex_;
    }

    void Grid::setSelectedColumnIndexProperty(std::optional<int> value)
    {
        if (value == selectedColumnIndex_)
        {
            return;
        }
        selectedColumnIndex_ = std::move(value);
        Utility::EventsExtensions::Invoke(SelectedIndexChanged, this, InputEventType::SelectedIndexChanged);
    }

    void Grid::OnProportionsCollectionChanged()
    {
        RebuildProportionSubscriptions();
        setHoverRowIndexProperty(std::nullopt);
        setSelectedRowIndexProperty(std::nullopt);
        InvalidateMeasure();
    }

    void Grid::RebuildProportionSubscriptions()
    {
        ClearProportionSubscriptions();

        const auto subscribe = [this](const std::shared_ptr<Proportion> &proportion)
        {
            if (!proportion)
            {
                return;
            }
            const Events::MyraEventHandler::Token token =
                proportion->Changed.Add([this](void *, Events::MyraEventArgs &) { InvalidateMeasure(); });
            proportionSubscriptions_.push_back({proportion, token});
        };
        for (const std::shared_ptr<Proportion> &proportion : layout_.getColumnsProportionsProperty())
        {
            subscribe(proportion);
        }
        for (const std::shared_ptr<Proportion> &proportion : layout_.getRowsProportionsProperty())
        {
            subscribe(proportion);
        }
    }

    void Grid::ClearProportionSubscriptions()
    {
        for (ProportionSubscription &subscription : proportionSubscriptions_)
        {
            if (subscription.proportion)
            {
                static_cast<void>(subscription.proportion->Changed.Remove(subscription.token));
            }
        }
        proportionSubscriptions_.clear();
    }

    void Grid::RenderSelection(Graphics2D::RenderContext &context)
    {
        using Microsoft::Xna::Framework::Color;
        using Microsoft::Xna::Framework::Rectangle;

        const Rectangle bounds = getActualBoundsProperty();
        const auto buildRowRectangle = [this, bounds](const int row)
        {
            const int location = CheckedGridValue(getCellLocationsYProperty(), row,
                                                  "Grid row selection index is outside the arranged rows.");
            const int height = CheckedGridValue(getRowHeightsProperty(), row,
                                                "Grid row selection index is outside the arranged rows.");
            return Rectangle(
                bounds.X,
                CheckedGridCoordinate(static_cast<std::int64_t>(location) + bounds.Y - getRowSpacingProperty() / 2,
                                      "Grid row selection position exceeds the supported integer range."),
                bounds.Width,
                CheckedGridCoordinate(static_cast<std::int64_t>(height) + getRowSpacingProperty(),
                                      "Grid row selection height exceeds the supported integer range."));
        };
        const auto buildColumnRectangle = [this, bounds](const int column)
        {
            const int location = CheckedGridValue(getCellLocationsXProperty(), column,
                                                  "Grid column selection index is outside the arranged columns.");
            const int width = CheckedGridValue(getColWidthsProperty(), column,
                                               "Grid column selection index is outside the arranged columns.");
            return Rectangle(
                CheckedGridCoordinate(static_cast<std::int64_t>(location) + bounds.X - getColumnSpacingProperty() / 2,
                                      "Grid column selection position exceeds the supported integer range."),
                bounds.Y,
                CheckedGridCoordinate(static_cast<std::int64_t>(width) + getColumnSpacingProperty(),
                                      "Grid column selection width exceeds the supported integer range."),
                bounds.Height);
        };
        const auto buildCellRectangle = [this, bounds](const int column, const int row)
        {
            const int locationX = CheckedGridValue(getCellLocationsXProperty(), column,
                                                   "Grid cell selection column is outside the arranged columns.");
            const int locationY = CheckedGridValue(getCellLocationsYProperty(), row,
                                                   "Grid cell selection row is outside the arranged rows.");
            const int width = CheckedGridValue(getColWidthsProperty(), column,
                                               "Grid cell selection column is outside the arranged columns.");
            const int height =
                CheckedGridValue(getRowHeightsProperty(), row, "Grid cell selection row is outside the arranged rows.");
            return Rectangle(
                CheckedGridCoordinate(static_cast<std::int64_t>(locationX) + bounds.X - getColumnSpacingProperty() / 2,
                                      "Grid cell selection X position exceeds the supported integer range."),
                CheckedGridCoordinate(static_cast<std::int64_t>(locationY) + bounds.Y - getRowSpacingProperty() / 2,
                                      "Grid cell selection Y position exceeds the supported integer range."),
                CheckedGridCoordinate(static_cast<std::int64_t>(width) + getColumnSpacingProperty(),
                                      "Grid cell selection width exceeds the supported integer range."),
                CheckedGridCoordinate(static_cast<std::int64_t>(height) + getRowSpacingProperty(),
                                      "Grid cell selection height exceeds the supported integer range."));
        };

        switch (gridSelectionMode_)
        {
        case GridSelectionMode::None:
            break;
        case GridSelectionMode::Row:
        {
            const std::optional<int> hover = hoverRowIndex_;
            const std::optional<int> selectedAtHover = selectedRowIndex_;
            const std::shared_ptr<Graphics2D::IBrush> hoverBrush = selectionHoverBackground_;
            if (hover && hover != selectedAtHover && hoverBrush)
            {
                hoverBrush->Draw(context, buildRowRectangle(*hover), Color::White);
            }

            const std::optional<int> selected = selectedRowIndex_;
            const std::shared_ptr<Graphics2D::IBrush> selectionBrush = selectionBackground_;
            if (selected && selectionBrush)
            {
                selectionBrush->Draw(context, buildRowRectangle(*selected), Color::White);
            }
            break;
        }
        case GridSelectionMode::Column:
        {
            const std::optional<int> hover = hoverColumnIndex_;
            const std::optional<int> selectedAtHover = selectedColumnIndex_;
            const std::shared_ptr<Graphics2D::IBrush> hoverBrush = selectionHoverBackground_;
            if (hover && hover != selectedAtHover && hoverBrush)
            {
                hoverBrush->Draw(context, buildColumnRectangle(*hover), Color::White);
            }

            const std::optional<int> selected = selectedColumnIndex_;
            const std::shared_ptr<Graphics2D::IBrush> selectionBrush = selectionBackground_;
            if (selected && selectionBrush)
            {
                selectionBrush->Draw(context, buildColumnRectangle(*selected), Color::White);
            }
            break;
        }
        case GridSelectionMode::Cell:
        {
            const std::optional<int> hoverRow = hoverRowIndex_;
            const std::optional<int> hoverColumn = hoverColumnIndex_;
            const std::optional<int> selectedRowAtHover = selectedRowIndex_;
            const std::optional<int> selectedColumnAtHover = selectedColumnIndex_;
            const std::shared_ptr<Graphics2D::IBrush> hoverBrush = selectionHoverBackground_;
            if (hoverRow && hoverColumn && (hoverRow != selectedRowAtHover || hoverColumn != selectedColumnAtHover) &&
                hoverBrush)
            {
                hoverBrush->Draw(context, buildCellRectangle(*hoverColumn, *hoverRow), Color::White);
            }

            const std::optional<int> selectedRow = selectedRowIndex_;
            const std::optional<int> selectedColumn = selectedColumnIndex_;
            const std::shared_ptr<Graphics2D::IBrush> selectionBrush = selectionBackground_;
            if (selectedRow && selectedColumn && selectionBrush)
            {
                selectionBrush->Draw(context, buildCellRectangle(*selectedColumn, *selectedRow), Color::White);
            }
            break;
        }
        }
    }

    void Grid::InternalRender(Graphics2D::RenderContext &context)
    {
        using Microsoft::Xna::Framework::Rectangle;

        const Rectangle bounds = getActualBoundsProperty();
        RenderSelection(context);
        Container::InternalRender(context);

        if (!showGridLines_)
        {
            return;
        }
        const Microsoft::Xna::Framework::Color color = gridLinesColor_;
        for (const int line : getGridLinesXProperty())
        {
            const int x = CheckedGridCoordinate(static_cast<std::int64_t>(line) + bounds.X,
                                                "Grid vertical line position exceeds the supported integer range.");
            context.FillRectangle(Rectangle(x, bounds.Y, 1, bounds.Height), color);
        }
        for (const int line : getGridLinesYProperty())
        {
            const int y = CheckedGridCoordinate(static_cast<std::int64_t>(line) + bounds.Y,
                                                "Grid horizontal line position exceeds the supported integer range.");
            context.FillRectangle(Rectangle(bounds.X, y, bounds.Width, 1), color);
        }
    }

    void Grid::UpdateHoverPosition(std::optional<Microsoft::Xna::Framework::Point> position)
    {
        if (gridSelectionMode_ == GridSelectionMode::None)
        {
            return;
        }
        if (!position)
        {
            if (hoverIndexCanBeNull_)
            {
                setHoverRowIndexProperty(std::nullopt);
                setHoverColumnIndexProperty(std::nullopt);
            }
            return;
        }

        const Microsoft::Xna::Framework::Point localPosition = ToLocal(*position);
        const Microsoft::Xna::Framework::Rectangle bounds = getActualBoundsProperty();
        if (gridSelectionMode_ == GridSelectionMode::Column || gridSelectionMode_ == GridSelectionMode::Cell)
        {
            const std::vector<int> &locations = getCellLocationsXProperty();
            const std::vector<int> &widths = getColWidthsProperty();
            for (std::size_t index = 0; index < locations.size(); ++index)
            {
                if (index >= widths.size() || index > static_cast<std::size_t>(std::numeric_limits<int>::max()))
                {
                    throw std::out_of_range("Grid arranged column geometry is inconsistent.");
                }
                const std::int64_t start =
                    static_cast<std::int64_t>(locations[index]) + bounds.X - getColumnSpacingProperty() / 2;
                const std::int64_t end = start + widths[index] + getColumnSpacingProperty() / 2;
                if (localPosition.X >= start && localPosition.X < end)
                {
                    setHoverColumnIndexProperty(static_cast<int>(index));
                    break;
                }
            }
        }

        if (gridSelectionMode_ == GridSelectionMode::Row || gridSelectionMode_ == GridSelectionMode::Cell)
        {
            const std::vector<int> &locations = getCellLocationsYProperty();
            const std::vector<int> &heights = getRowHeightsProperty();
            for (std::size_t index = 0; index < locations.size(); ++index)
            {
                if (index >= heights.size() || index > static_cast<std::size_t>(std::numeric_limits<int>::max()))
                {
                    throw std::out_of_range("Grid arranged row geometry is inconsistent.");
                }
                const std::int64_t start =
                    static_cast<std::int64_t>(locations[index]) + bounds.Y - getRowSpacingProperty() / 2;
                const std::int64_t end = start + heights[index] + getRowSpacingProperty() / 2;
                if (localPosition.Y >= start && localPosition.Y < end)
                {
                    setHoverRowIndexProperty(static_cast<int>(index));
                    break;
                }
            }
        }
    }

    void Grid::OnMouseLeft()
    {
        Widget::OnMouseLeft();
        UpdateHoverPosition(std::nullopt);
    }

    void Grid::OnMouseEntered()
    {
        Widget::OnMouseEntered();
        Desktop *const desktop = getDesktopProperty();
        if (desktop != nullptr)
        {
            UpdateHoverPosition(desktop->getMousePositionProperty());
        }
    }

    void Grid::OnMouseMoved()
    {
        Widget::OnMouseMoved();
        Desktop *const desktop = getDesktopProperty();
        if (desktop != nullptr)
        {
            UpdateHoverPosition(desktop->getMousePositionProperty());
        }
    }

    void Grid::OnTouchDown()
    {
        Widget::OnTouchDown();
        Desktop *const desktop = getDesktopProperty();
        if (desktop == nullptr || !desktop->getTouchPositionProperty())
        {
            return;
        }

        UpdateHoverPosition(desktop->getTouchPositionProperty());
        const std::optional<int> hoverRow = hoverRowIndex_;
        if (hoverRow)
        {
            if (selectedRowIndex_ != hoverRow)
            {
                setSelectedRowIndexProperty(hoverRow);
            }
            else if (canSelectNothing_)
            {
                setSelectedRowIndexProperty(std::nullopt);
            }
        }

        const std::optional<int> hoverColumn = hoverColumnIndex_;
        if (hoverColumn)
        {
            if (selectedColumnIndex_ != hoverColumn)
            {
                setSelectedColumnIndexProperty(hoverColumn);
            }
            else if (canSelectNothing_)
            {
                setSelectedColumnIndexProperty(std::nullopt);
            }
        }
    }

    void Grid::OnChildAdded(Widget &child)
    {
        Container::OnChildAdded(child);
        setHoverRowIndexProperty(std::nullopt);
        setSelectedRowIndexProperty(std::nullopt);
    }

    void Grid::OnChildRemoved(Widget &child)
    {
        Container::OnChildRemoved(child);
        setHoverRowIndexProperty(std::nullopt);
        setSelectedRowIndexProperty(std::nullopt);
    }

    int Grid::GetColumnWidth(const int index) const noexcept
    {
        return layout_.GetColumnWidth(index);
    }
    int Grid::GetRowHeight(const int index) const noexcept
    {
        return layout_.GetRowHeight(index);
    }
    int Grid::GetCellLocationX(const int column) const noexcept
    {
        return layout_.GetCellLocationX(column);
    }
    int Grid::GetCellLocationY(const int row) const noexcept
    {
        return layout_.GetCellLocationY(row);
    }

    Microsoft::Xna::Framework::Rectangle Grid::GetCellRectangle(const int column, const int row) const noexcept
    {
        return layout_.GetCellRectangle(column, row);
    }

    const MML::AttachedPropertyInfo<int> &Grid::getColumnProperty()
    {
        static const MML::AttachedPropertyInfo<int> *property = []
        {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "Column", 0, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(0.0F));
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<int> &Grid::getRowProperty()
    {
        static const MML::AttachedPropertyInfo<int> *property = []
        {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "Row", 0, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(0.0F));
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<int> &Grid::getColumnSpanProperty()
    {
        static const MML::AttachedPropertyInfo<int> *property = []
        {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "ColumnSpan", 1, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(1.0F));
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<int> &Grid::getRowSpanProperty()
    {
        static const MML::AttachedPropertyInfo<int> *property = []
        {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(Grid), "RowSpan", 1, MML::AttachedPropertyOption::AffectsArrange, RangeMetadata(1.0F));
        }();
        return *property;
    }

    int Grid::GetColumn(const Widget &widget)
    {
        return getColumnProperty().GetValue(widget);
    }
    void Grid::SetColumn(Widget &widget, const int value)
    {
        getColumnProperty().SetValue(widget, value);
    }
    int Grid::GetRow(const Widget &widget)
    {
        return getRowProperty().GetValue(widget);
    }
    void Grid::SetRow(Widget &widget, const int value)
    {
        getRowProperty().SetValue(widget, value);
    }
    int Grid::GetColumnSpan(const Widget &widget)
    {
        return getColumnSpanProperty().GetValue(widget);
    }
    void Grid::SetColumnSpan(Widget &widget, const int value)
    {
        getColumnSpanProperty().SetValue(widget, value);
    }
    int Grid::GetRowSpan(const Widget &widget)
    {
        return getRowSpanProperty().GetValue(widget);
    }
    void Grid::SetRowSpan(Widget &widget, const int value)
    {
        getRowSpanProperty().SetValue(widget, value);
    }
} // namespace Myra::Graphics2D::UI

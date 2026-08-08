// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Layouts/GridLayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Layouts/GridLayout.hpp"

#include <algorithm>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    int GridLayout::getColumnSpacingProperty() const noexcept { return columnSpacing_; }
    void GridLayout::setColumnSpacingProperty(const int value) noexcept { columnSpacing_ = value; }
    int GridLayout::getRowSpacingProperty() const noexcept { return rowSpacing_; }
    void GridLayout::setRowSpacingProperty(const int value) noexcept { rowSpacing_ = value; }
    const std::shared_ptr<Proportion>& GridLayout::getDefaultColumnProportionProperty() const noexcept
    {
        return defaultColumnProportion_;
    }

    void GridLayout::setDefaultColumnProportionProperty(std::shared_ptr<Proportion> value)
    {
        defaultColumnProportion_ = std::move(value);
    }

    const std::shared_ptr<Proportion>& GridLayout::getDefaultRowProportionProperty() const noexcept
    {
        return defaultRowProportion_;
    }

    void GridLayout::setDefaultRowProportionProperty(std::shared_ptr<Proportion> value)
    {
        defaultRowProportion_ = std::move(value);
    }

    const ProportionCollection& GridLayout::getColumnsProportionsProperty() const noexcept { return columnsProportions_; }
    ProportionCollection& GridLayout::getColumnsProportionsProperty() noexcept { return columnsProportions_; }
    const ProportionCollection& GridLayout::getRowsProportionsProperty() const noexcept { return rowsProportions_; }
    ProportionCollection& GridLayout::getRowsProportionsProperty() noexcept { return rowsProportions_; }
    const std::vector<int>& GridLayout::getGridLinesXProperty() const noexcept { return gridLinesX_; }
    const std::vector<int>& GridLayout::getGridLinesYProperty() const noexcept { return gridLinesY_; }
    const std::vector<int>& GridLayout::getColWidthsProperty() const noexcept { return columnWidths_; }
    const std::vector<int>& GridLayout::getRowHeightsProperty() const noexcept { return rowHeights_; }
    const std::vector<int>& GridLayout::getCellLocationsXProperty() const noexcept { return cellLocationsX_; }
    const std::vector<int>& GridLayout::getCellLocationsYProperty() const noexcept { return cellLocationsY_; }

    int GridLayout::GetColumnWidth(const int index) const noexcept
    {
        return index < 0 || static_cast<size_t>(index) >= columnWidths_.size() ? 0 : columnWidths_[index];
    }

    int GridLayout::GetRowHeight(const int index) const noexcept
    {
        return index < 0 || static_cast<size_t>(index) >= rowHeights_.size() ? 0 : rowHeights_[index];
    }

    int GridLayout::GetCellLocationX(const int column) const noexcept
    {
        return column < 0 || static_cast<size_t>(column) >= cellLocationsX_.size() ? 0 : cellLocationsX_[column];
    }

    int GridLayout::GetCellLocationY(const int row) const noexcept
    {
        return row < 0 || static_cast<size_t>(row) >= cellLocationsY_.size() ? 0 : cellLocationsY_[row];
    }

    Rectangle GridLayout::GetCellRectangle(const int column, const int row) const noexcept
    {
        if (column < 0 || row < 0 || static_cast<size_t>(column) >= cellLocationsX_.size() ||
            static_cast<size_t>(row) >= cellLocationsY_.size())
        {
            return Rectangle(0, 0, 0, 0);
        }
        return Rectangle(cellLocationsX_[column], cellLocationsY_[row], columnWidths_[column], rowHeights_[row]);
    }

    const std::shared_ptr<Proportion>& GridLayout::GetColumnProportion(const int column) const noexcept
    {
        return column < 0 || column >= columnsProportions_.getCountProperty()
            ? defaultColumnProportion_ : columnsProportions_[column];
    }

    const std::shared_ptr<Proportion>& GridLayout::GetRowProportion(const int row) const noexcept
    {
        return row < 0 || row >= rowsProportions_.getCountProperty()
            ? defaultRowProportion_ : rowsProportions_[row];
    }

    Point GridLayout::GetActualGridPosition(const Widget& child) const
    {
        return Point(Grid::GetColumn(child), Grid::GetRow(child));
    }

    void GridLayout::LayoutProcessFixedPart()
    {
        int size = 0;
        for (size_t index = 0; index < measureColumnWidths_.size(); ++index)
        {
            if (GetColumnProportion(static_cast<int>(index))->getTypeProperty() == ProportionType::Part)
            {
                size = std::max(size, measureColumnWidths_[index]);
            }
        }
        for (size_t index = 0; index < measureColumnWidths_.size(); ++index)
        {
            const std::shared_ptr<Proportion>& proportion = GetColumnProportion(static_cast<int>(index));
            if (proportion->getTypeProperty() == ProportionType::Part)
            {
                measureColumnWidths_[index] = static_cast<int>(static_cast<float>(size) * proportion->getValueProperty());
            }
        }

        size = 0;
        for (size_t index = 0; index < measureRowHeights_.size(); ++index)
        {
            if (GetRowProportion(static_cast<int>(index))->getTypeProperty() == ProportionType::Part)
            {
                size = std::max(size, measureRowHeights_[index]);
            }
        }
        for (size_t index = 0; index < measureRowHeights_.size(); ++index)
        {
            const std::shared_ptr<Proportion>& proportion = GetRowProportion(static_cast<int>(index));
            if (proportion->getTypeProperty() == ProportionType::Part)
            {
                measureRowHeights_[index] = static_cast<int>(static_cast<float>(size) * proportion->getValueProperty());
            }
        }
    }

    Point GridLayout::Measure(const std::vector<std::shared_ptr<Widget>>& widgets, Point availableSize)
    {
        int rows = 0;
        int columns = 0;
        visibleWidgets_.clear();
        for (const std::shared_ptr<Widget>& child : widgets)
        {
            if (!child || !child->getVisibleProperty())
            {
                continue;
            }
            visibleWidgets_.push_back(child);
            const Point position = GetActualGridPosition(*child);
            columns = std::max(columns, position.X + std::max(Grid::GetColumnSpan(*child), 1));
            rows = std::max(rows, position.Y + std::max(Grid::GetRowSpan(*child), 1));
        }
        columns = std::max(columns, static_cast<int>(columnsProportions_.getCountProperty()));
        rows = std::max(rows, static_cast<int>(rowsProportions_.getCountProperty()));

        measureColumnWidths_.assign(static_cast<size_t>(columns), 0);
        measureRowHeights_.assign(static_cast<size_t>(rows), 0);
        widgetsByGridPosition_.assign(static_cast<size_t>(rows),
            std::vector<std::vector<std::shared_ptr<Widget>>>(static_cast<size_t>(columns)));
        for (const std::shared_ptr<Widget>& widget : visibleWidgets_)
        {
            const Point position = GetActualGridPosition(*widget);
            widgetsByGridPosition_[position.Y][position.X].push_back(widget);
        }

        availableSize.X -= (columns - 1) * columnSpacing_;
        availableSize.Y -= (rows - 1) * rowSpacing_;
        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < columns; ++column)
            {
                const std::shared_ptr<Proportion>& rowProportion = GetRowProportion(row);
                const std::shared_ptr<Proportion>& columnProportion = GetColumnProportion(column);
                if (columnProportion->getTypeProperty() == ProportionType::Pixels)
                {
                    measureColumnWidths_[column] = static_cast<int>(columnProportion->getValueProperty());
                }
                if (rowProportion->getTypeProperty() == ProportionType::Pixels)
                {
                    measureRowHeights_[row] = static_cast<int>(rowProportion->getValueProperty());
                }
                for (const std::shared_ptr<Widget>& widget : widgetsByGridPosition_[row][column])
                {
                    Point measuredSize(0, 0);
                    if (rowProportion->getTypeProperty() != ProportionType::Pixels ||
                        columnProportion->getTypeProperty() != ProportionType::Pixels)
                    {
                        measuredSize = widget->Measure(availableSize);
                    }
                    if (Grid::GetColumnSpan(*widget) != 1)
                    {
                        measuredSize.X = 0;
                    }
                    if (Grid::GetRowSpan(*widget) != 1)
                    {
                        measuredSize.Y = 0;
                    }
                    if (columnProportion->getTypeProperty() != ProportionType::Pixels)
                    {
                        measureColumnWidths_[column] = std::max(measureColumnWidths_[column], measuredSize.X);
                    }
                    if (rowProportion->getTypeProperty() != ProportionType::Pixels)
                    {
                        measureRowHeights_[row] = std::max(measureRowHeights_[row], measuredSize.Y);
                    }
                }
            }
        }
        LayoutProcessFixedPart();

        Point result(0, 0);
        for (size_t index = 0; index < measureColumnWidths_.size(); ++index)
        {
            result.X += measureColumnWidths_[index];
            if (index + 1 < measureColumnWidths_.size())
            {
                result.X += columnSpacing_;
            }
        }
        for (size_t index = 0; index < measureRowHeights_.size(); ++index)
        {
            result.Y += measureRowHeights_[index];
            if (index + 1 < measureRowHeights_.size())
            {
                result.Y += rowSpacing_;
            }
        }
        return result;
    }

    void GridLayout::Arrange(const std::vector<std::shared_ptr<Widget>>& widgets, const Rectangle bounds)
    {
        static_cast<void>(Measure(widgets, Point(bounds.Width, bounds.Height)));
        columnWidths_ = measureColumnWidths_;
        rowHeights_ = measureRowHeights_;

        float availableWidth = static_cast<float>(bounds.Width -
            (static_cast<int>(columnWidths_.size()) - 1) * columnSpacing_);
        float totalPart = 0.0F;
        for (size_t column = 0; column < columnWidths_.size(); ++column)
        {
            const std::shared_ptr<Proportion>& proportion = GetColumnProportion(static_cast<int>(column));
            if (proportion->getTypeProperty() == ProportionType::Auto || proportion->getTypeProperty() == ProportionType::Pixels)
            {
                availableWidth -= static_cast<float>(columnWidths_[column]);
            }
            else
            {
                totalPart += proportion->getValueProperty();
            }
        }
        if (totalPart != 0.0F)
        {
            float takenSpace = 0.0F;
            for (size_t column = 0; column < columnWidths_.size(); ++column)
            {
                const std::shared_ptr<Proportion>& proportion = GetColumnProportion(static_cast<int>(column));
                if (proportion->getTypeProperty() == ProportionType::Part)
                {
                    columnWidths_[column] = static_cast<int>(proportion->getValueProperty() * availableWidth / totalPart);
                    takenSpace += static_cast<float>(columnWidths_[column]);
                }
            }
            availableWidth -= takenSpace;
        }
        for (size_t column = 0; column < columnWidths_.size(); ++column)
        {
            if (GetColumnProportion(static_cast<int>(column))->getTypeProperty() == ProportionType::Fill)
            {
                columnWidths_[column] = static_cast<int>(availableWidth);
                break;
            }
        }

        float availableHeight = static_cast<float>(bounds.Height -
            (static_cast<int>(rowHeights_.size()) - 1) * rowSpacing_);
        totalPart = 0.0F;
        for (size_t row = 0; row < rowHeights_.size(); ++row)
        {
            const std::shared_ptr<Proportion>& proportion = GetRowProportion(static_cast<int>(row));
            if (proportion->getTypeProperty() == ProportionType::Auto || proportion->getTypeProperty() == ProportionType::Pixels)
            {
                availableHeight -= static_cast<float>(rowHeights_[row]);
            }
            else
            {
                totalPart += proportion->getValueProperty();
            }
        }
        if (totalPart != 0.0F)
        {
            float takenSpace = 0.0F;
            for (size_t row = 0; row < rowHeights_.size(); ++row)
            {
                const std::shared_ptr<Proportion>& proportion = GetRowProportion(static_cast<int>(row));
                if (proportion->getTypeProperty() == ProportionType::Part)
                {
                    rowHeights_[row] = static_cast<int>(proportion->getValueProperty() * availableHeight / totalPart);
                    takenSpace += static_cast<float>(rowHeights_[row]);
                }
            }
            availableHeight -= takenSpace;
        }
        for (size_t row = 0; row < rowHeights_.size(); ++row)
        {
            if (GetRowProportion(static_cast<int>(row))->getTypeProperty() == ProportionType::Fill)
            {
                rowHeights_[row] = static_cast<int>(availableHeight);
                break;
            }
        }

        gridLinesX_.clear();
        cellLocationsX_.clear();
        int position = 0;
        for (size_t column = 0; column < columnWidths_.size(); ++column)
        {
            cellLocationsX_.push_back(position);
            position += columnWidths_[column];
            if (column + 1 < columnWidths_.size())
            {
                gridLinesX_.push_back(position + columnSpacing_ / 2);
            }
            position += columnSpacing_;
        }
        gridLinesY_.clear();
        cellLocationsY_.clear();
        position = 0;
        for (size_t row = 0; row < rowHeights_.size(); ++row)
        {
            cellLocationsY_.push_back(position);
            position += rowHeights_[row];
            if (row + 1 < rowHeights_.size())
            {
                gridLinesY_.push_back(position + rowSpacing_ / 2);
            }
            position += rowSpacing_;
        }
        for (const std::shared_ptr<Widget>& control : visibleWidgets_)
        {
            LayoutControl(*control, bounds);
        }
    }

    void GridLayout::LayoutControl(Widget& control, const Rectangle bounds)
    {
        const Point position = GetActualGridPosition(control);
        Point cellSize(0, 0);
        const int columnSpan = Grid::GetColumnSpan(control);
        const int rowSpan = Grid::GetRowSpan(control);
        for (int column = position.X; column < position.X + columnSpan; ++column)
        {
            cellSize.X += columnWidths_[column];
            if (column + 1 < position.X + columnSpan)
            {
                cellSize.X += columnSpacing_;
            }
        }
        for (int row = position.Y; row < position.Y + rowSpan; ++row)
        {
            cellSize.Y += rowHeights_[row];
            if (row + 1 < position.Y + rowSpan)
            {
                cellSize.Y += rowSpacing_;
            }
        }
        Rectangle rectangle(bounds.X + cellLocationsX_[position.X], bounds.Y + cellLocationsY_[position.Y],
            cellSize.X, cellSize.Y);
        if (rectangle.getRightProperty() > bounds.getRightProperty())
        {
            rectangle.Width = bounds.getRightProperty() - rectangle.X;
        }
        if (rectangle.Width < 0)
        {
            rectangle.Width = 0;
        }
        if (rectangle.getBottomProperty() > bounds.getBottomProperty())
        {
            rectangle.Height = bounds.getBottomProperty() - rectangle.Y;
        }
        if (rectangle.Height < 0)
        {
            rectangle.Height = 0;
        }
        control.Arrange(rectangle);
    }
}

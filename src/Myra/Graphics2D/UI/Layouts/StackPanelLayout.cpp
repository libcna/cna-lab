// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Layouts/StackPanelLayout.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Layouts/StackPanelLayout.hpp"

#include <utility>

#include "Myra/Graphics2D/UI/Containers/Grid.hpp"
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"

namespace Myra::Graphics2D::UI
{
    StackPanelLayout::StackPanelLayout(const Orientation orientation) : orientation_(orientation)
    {
        setDefaultProportionProperty(Proportion::StackPanelDefault);
    }

    Orientation StackPanelLayout::getOrientationProperty() const noexcept { return orientation_; }

    int StackPanelLayout::getSpacingProperty() const noexcept
    {
        return orientation_ == Orientation::Horizontal ? layout_.getColumnSpacingProperty() : layout_.getRowSpacingProperty();
    }

    void StackPanelLayout::setSpacingProperty(const int value) noexcept
    {
        if (orientation_ == Orientation::Horizontal)
        {
            layout_.setColumnSpacingProperty(value);
        }
        else
        {
            layout_.setRowSpacingProperty(value);
        }
    }

    const Proportion& StackPanelLayout::getDefaultProportionProperty() const noexcept
    {
        return orientation_ == Orientation::Horizontal ? layout_.getDefaultColumnProportionProperty()
                                                       : layout_.getDefaultRowProportionProperty();
    }

    void StackPanelLayout::setDefaultProportionProperty(Proportion value)
    {
        if (orientation_ == Orientation::Horizontal)
        {
            layout_.setDefaultColumnProportionProperty(std::move(value));
        }
        else
        {
            layout_.setDefaultRowProportionProperty(std::move(value));
        }
    }

    const std::vector<Proportion>& StackPanelLayout::getProportionsProperty() const noexcept
    {
        return orientation_ == Orientation::Horizontal ? layout_.getColumnsProportionsProperty()
                                                       : layout_.getRowsProportionsProperty();
    }

    std::vector<Proportion>& StackPanelLayout::getProportionsProperty() noexcept
    {
        return orientation_ == Orientation::Horizontal ? layout_.getColumnsProportionsProperty()
                                                       : layout_.getRowsProportionsProperty();
    }

    const std::vector<int>& StackPanelLayout::getGridLinesXProperty() const noexcept
    {
        return layout_.getGridLinesXProperty();
    }

    const std::vector<int>& StackPanelLayout::getGridLinesYProperty() const noexcept
    {
        return layout_.getGridLinesYProperty();
    }

    void StackPanelLayout::UpdateWidgets(const std::vector<std::shared_ptr<Widget>>& widgets)
    {
        std::vector<Proportion>& proportions = getProportionsProperty();
        proportions.clear();
        int index = 0;
        for (const std::shared_ptr<Widget>& widget : widgets)
        {
            if (!widget)
            {
                continue;
            }
            if (orientation_ == Orientation::Horizontal)
            {
                Grid::SetColumn(*widget, index);
            }
            else
            {
                Grid::SetRow(*widget, index);
            }
            if (StackPanel::getProportionTypeProperty().HasValue(*widget))
            {
                proportions.emplace_back(StackPanel::GetProportionType(*widget), StackPanel::GetProportionValue(*widget));
            }
            else
            {
                proportions.push_back(getDefaultProportionProperty());
            }
            ++index;
        }
    }

    Microsoft::Xna::Framework::Point StackPanelLayout::Measure(
        const std::vector<std::shared_ptr<Widget>>& widgets, const Microsoft::Xna::Framework::Point availableSize)
    {
        UpdateWidgets(widgets);
        return layout_.Measure(widgets, availableSize);
    }

    void StackPanelLayout::Arrange(const std::vector<std::shared_ptr<Widget>>& widgets,
        const Microsoft::Xna::Framework::Rectangle bounds)
    {
        UpdateWidgets(widgets);
        layout_.Arrange(widgets, bounds);
    }

    int StackPanelLayout::GetCellSize(const int index) const noexcept
    {
        return orientation_ == Orientation::Horizontal ? layout_.GetColumnWidth(index) : layout_.GetRowHeight(index);
    }
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Containers/StackPanel.cs, src/Myra/Graphics2D/UI/Containers/HorizontalStackPanel.cs, and src/Myra/Graphics2D/UI/Containers/VerticalStackPanel.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Containers/StackPanel.hpp"

#include <stdexcept>
#include <utility>

#include "Myra/Attributes/RangeAttribute.hpp"

namespace Myra::Graphics2D::UI
{
    StackPanel::StackPanel(const Orientation orientation) : layout_(orientation)
    {
        setChildrenLayoutProperty(&layout_);
        proportions_.CollectionChanged.emplace_back([this](void*, const auto&) { InvalidateProportions(); });
    }

    Orientation StackPanel::getOrientationProperty() const noexcept { return layout_.getOrientationProperty(); }
    int StackPanel::getSpacingProperty() const noexcept { return layout_.getSpacingProperty(); }

    void StackPanel::setSpacingProperty(const int value)
    {
        if (value != layout_.getSpacingProperty())
        {
            layout_.setSpacingProperty(value);
            InvalidateMeasure();
        }
    }

    const std::shared_ptr<Proportion>& StackPanel::getDefaultProportionProperty() const noexcept
    {
        return layout_.getDefaultProportionProperty();
    }

    void StackPanel::setDefaultProportionProperty(std::shared_ptr<Proportion> value)
    {
        layout_.setDefaultProportionProperty(std::move(value));
        InvalidateMeasure();
    }

    const ProportionCollection& StackPanel::getProportionsProperty() const noexcept { return proportions_; }

    ProportionCollection& StackPanel::getProportionsProperty() noexcept
    {
        return proportions_;
    }

    int StackPanel::GetCellSize(const int index) const noexcept { return layout_.GetCellSize(index); }

    const MML::AttachedPropertyInfo<ProportionType>& StackPanel::getProportionTypeProperty()
    {
        static const MML::AttachedPropertyInfo<ProportionType>* property = [] {
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(StackPanel), "ProportionType", ProportionType::Auto, MML::AttachedPropertyOption::AffectsMeasure);
        }();
        return *property;
    }

    const MML::AttachedPropertyInfo<float>& StackPanel::getProportionValueProperty()
    {
        static const MML::AttachedPropertyInfo<float>* property = [] {
            MML::PropertyMetadata metadata;
            metadata.Range = Attributes::RangeAttribute(0.0F);
            return &MML::AttachedPropertiesRegistry::Create(
                typeid(StackPanel), "ProportionValue", 1.0F, MML::AttachedPropertyOption::AffectsMeasure,
                std::move(metadata));
        }();
        return *property;
    }

    ProportionType StackPanel::GetProportionType(const Widget& widget)
    {
        return getProportionTypeProperty().GetValue(widget);
    }

    void StackPanel::SetProportionType(Widget& widget, const ProportionType value)
    {
        getProportionTypeProperty().SetValue(widget, value);
    }

    float StackPanel::GetProportionValue(const Widget& widget)
    {
        return getProportionValueProperty().GetValue(widget);
    }

    void StackPanel::SetProportionValue(Widget& widget, const float value)
    {
        getProportionValueProperty().SetValue(widget, value);
    }

    void StackPanel::UpdateChildren()
    {
        if (!childrenDirty_)
        {
            return;
        }
        size_t index = 0;
        const std::vector<std::shared_ptr<Widget>> snapshot = getChildrenCopyProperty();
        for (const std::shared_ptr<Widget>& widget : snapshot)
        {
            if (index < static_cast<size_t>(proportions_.getCountProperty()))
            {
                const std::shared_ptr<Proportion>& proportion = proportions_[static_cast<SharpRuntime::intcs>(index)];
                if (!proportion)
                {
                    throw std::logic_error("A stack-panel explicit proportion cannot be null.");
                }
                SetProportionType(*widget, proportion->getTypeProperty());
                SetProportionValue(*widget, proportion->getValueProperty());
            }
            ++index;
        }
        childrenDirty_ = false;
    }

    void StackPanel::InvalidateProportions() noexcept
    {
        childrenDirty_ = true;
    }

    Microsoft::Xna::Framework::Point StackPanel::InternalMeasure(
        const Microsoft::Xna::Framework::Point availableSize)
    {
        UpdateChildren();
        return Widget::InternalMeasure(availableSize);
    }

    void StackPanel::InternalArrange()
    {
        UpdateChildren();
        Widget::InternalArrange();
    }

    HorizontalStackPanel::HorizontalStackPanel() : StackPanel(Orientation::Horizontal) {}
    VerticalStackPanel::VerticalStackPanel() : StackPanel(Orientation::Vertical) {}
}

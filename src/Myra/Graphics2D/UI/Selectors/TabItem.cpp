// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Selectors/TabItem.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Selectors/TabItem.hpp"

#include <utility>

#include "Myra/Graphics2D/UI/Widget.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::Graphics2D::UI
{
    TabItem::TabItem(std::optional<std::string> text, std::shared_ptr<Widget> content)
        : content_(std::move(content)), text_(std::move(text))
    {
    }

    const std::optional<std::string> &TabItem::getTextProperty() const noexcept
    {
        return text_;
    }

    void TabItem::setTextProperty(std::optional<std::string> value)
    {
        if (value == text_)
        {
            return;
        }
        text_ = std::move(value);
        FireChanged();
    }

    std::shared_ptr<Widget> TabItem::getContentProperty() const
    {
        return content_;
    }

    void TabItem::setContentProperty(std::shared_ptr<Widget> value)
    {
        if (value == content_)
        {
            return;
        }
        content_ = std::move(value);
        FireChanged();
    }

    const std::any &TabItem::getTagProperty() const noexcept
    {
        return tag_;
    }

    void TabItem::setTagProperty(std::any value)
    {
        tag_ = std::move(value);
    }

    std::shared_ptr<Graphics2D::IImage> TabItem::getImageProperty() const
    {
        return image_;
    }

    void TabItem::setImageProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        image_ = std::move(value);
    }

    int TabItem::getImageTextSpacingProperty() const noexcept
    {
        return imageTextSpacing_;
    }

    void TabItem::setImageTextSpacingProperty(const int value) noexcept
    {
        imageTextSpacing_ = value;
    }

    const std::optional<int> &TabItem::getHeightProperty() const noexcept
    {
        return height_;
    }

    void TabItem::setHeightProperty(std::optional<int> value) noexcept
    {
        height_ = std::move(value);
    }

    bool TabItem::getIsSelectedProperty() const noexcept
    {
        return isSelected_;
    }

    void TabItem::setIsSelectedProperty(const bool value) noexcept
    {
        if (value == isSelected_)
        {
            return;
        }
        isSelected_ = value;
        if (isSelected_)
        {
            Utility::EventsExtensions::Invoke(SelectedChanged, this, InputEventType::SelectionChanged);
        }
    }

    std::string TabItem::ToString() const
    {
        std::string result;
        if (text_ && !text_->empty())
        {
            result = *text_ + " ";
        }
        const std::optional<std::string> &id = getIdProperty();
        if (id && !id->empty())
        {
            result += "(#" + *id + ")";
        }
        return result;
    }

    std::shared_ptr<TabItem> TabItem::Clone() const
    {
        const auto result = std::make_shared<TabItem>(text_, content_ ? content_->Clone() : nullptr);
        result->setTagProperty(tag_);
        result->setImageProperty(image_);
        result->setImageTextSpacingProperty(imageTextSpacing_);
        result->setHeightProperty(height_);
        return result;
    }

    void TabItem::OnIdChanged()
    {
        MML::BaseObject::OnIdChanged();
        FireChanged();
    }

    void TabItem::FireChanged()
    {
        Utility::EventsExtensions::Invoke(Changed, this, InputEventType::ValueChanged);
    }
} // namespace Myra::Graphics2D::UI

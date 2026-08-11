// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/Simple/CheckButtonBase.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/UI/Simple/CheckButtonBase.hpp"

#include <stdexcept>
#include <utility>

namespace Myra::Graphics2D::UI
{
    using Microsoft::Xna::Framework::Input::Keys;

    CheckButtonBase::CheckButtonBase()
        : layout_(Orientation::Horizontal), check_(std::make_shared<CheckImageInternal>())
    {
        check_->setHorizontalAlignmentProperty(HorizontalAlignment::Left);
        check_->setVerticalAlignmentProperty(VerticalAlignment::Center);
        setChildrenLayoutProperty(&layout_);
        UpdateChildren();
    }

    CheckPosition CheckButtonBase::getCheckPositionProperty() const noexcept
    {
        return checkPosition_;
    }

    void CheckButtonBase::setCheckPositionProperty(const CheckPosition value)
    {
        if (value == checkPosition_)
        {
            return;
        }
        checkPosition_ = value;
        UpdateChildren();
    }

    int CheckButtonBase::getCheckContentSpacingProperty() const noexcept
    {
        return layout_.getSpacingProperty();
    }

    void CheckButtonBase::setCheckContentSpacingProperty(const int value)
    {
        if (value == layout_.getSpacingProperty())
        {
            return;
        }
        layout_.setSpacingProperty(value);
        InvalidateMeasure();
    }

    std::shared_ptr<Graphics2D::IImage> CheckButtonBase::getUncheckedImageProperty() const
    {
        return uncheckedImage_;
    }

    void CheckButtonBase::setUncheckedImageProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == uncheckedImage_)
        {
            return;
        }
        uncheckedImage_ = std::move(value);
        UpdateImage();
    }

    std::shared_ptr<Graphics2D::IImage> CheckButtonBase::getCheckedImageProperty() const
    {
        return checkedImage_;
    }

    void CheckButtonBase::setCheckedImageProperty(std::shared_ptr<Graphics2D::IImage> value)
    {
        if (value == checkedImage_)
        {
            return;
        }
        checkedImage_ = std::move(value);
        UpdateImage();
    }

    std::shared_ptr<Widget> CheckButtonBase::getContentProperty() const
    {
        return content_;
    }

    void CheckButtonBase::setContentProperty(std::shared_ptr<Widget> value)
    {
        if (value == content_)
        {
            return;
        }
        content_ = std::move(value);
        UpdateChildren();
    }

    std::shared_ptr<Image> CheckButtonBase::getCheckImageProperty() const noexcept
    {
        return check_;
    }

    void CheckButtonBase::InternalOnTouchUp()
    {
    }

    void CheckButtonBase::InternalOnTouchDown()
    {
        SetIsPressedByUser(!getIsPressedProperty());
    }

    void CheckButtonBase::OnPressedChanged()
    {
        ButtonBase::OnPressedChanged();
        check_->setIsPressedProperty(getIsPressedProperty());
        UpdateImage();
    }

    void CheckButtonBase::OnKeyDown(const Keys key)
    {
        ButtonBase::OnKeyDown(key);
        if (!getEnabledProperty())
        {
            return;
        }
        if (key == Keys::Space)
        {
            SetIsPressedByUser(!getIsPressedProperty());
        }
    }

    void CheckButtonBase::CopyFrom(const Widget& source)
    {
        ButtonBase::CopyFrom(source);
        const auto* const checkButton = dynamic_cast<const CheckButtonBase*>(&source);
        if (checkButton == nullptr)
        {
            throw std::invalid_argument("CheckButtonBase copy source must be a CheckButtonBase.");
        }

        setCheckPositionProperty(checkButton->checkPosition_);
        setCheckContentSpacingProperty(checkButton->getCheckContentSpacingProperty());
        checkedImage_ = checkButton->checkedImage_;
        uncheckedImage_ = checkButton->uncheckedImage_;
        check_->CopyFromImage(*checkButton->check_);
    }

    void CheckButtonBase::UpdateChildren()
    {
        const std::shared_ptr<CheckImageInternal> check = check_;
        const std::shared_ptr<Widget> content = content_;
        ClearChildren();

        switch (checkPosition_)
        {
            case CheckPosition::Left:
                AddChild(check);
                if (content)
                {
                    AddChild(content);
                }
                break;
            case CheckPosition::Right:
                if (content)
                {
                    AddChild(content);
                }
                AddChild(check);
                break;
        }
    }

    void CheckButtonBase::UpdateImage()
    {
        check_->setRenderableProperty(getIsPressedProperty() ? checkedImage_ : uncheckedImage_);
    }

    void CheckButtonBase::CheckImageInternal::CopyFromImage(const Image& source)
    {
        Image::CopyFrom(source);
    }

    std::shared_ptr<Widget> CheckButtonBase::CheckImageInternal::CreateCloneInstance() const
    {
        return std::make_shared<CheckImageInternal>();
    }
}

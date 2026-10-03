// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/BaseObject.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/MML/BaseObject.hpp"

#include <utility>

#include "Myra/Graphics2D/UI/InputEventType.hpp"
#include "Myra/Utility/EventsExtensions.hpp"

namespace Myra::MML
{
    const std::optional<std::string>& BaseObject::getIdProperty() const noexcept
    {
        return id_;
    }

    void BaseObject::setIdProperty(std::optional<std::string> value)
    {
        if (value == id_)
        {
            return;
        }

        id_ = std::move(value);
        OnIdChanged();
    }

    void BaseObject::setIdProperty(std::string value)
    {
        setIdProperty(std::optional<std::string>(std::move(value)));
    }

    const std::unordered_map<std::string, std::string>& BaseObject::getUserDataProperty() const noexcept
    {
        return userData_;
    }

    std::unordered_map<std::string, std::string>& BaseObject::getUserDataProperty() noexcept
    {
        return userData_;
    }

    void BaseObject::OnAttachedPropertyChanged(const BaseAttachedPropertyInfo& /*propertyInfo*/)
    {
    }

    void BaseObject::OnAttachedPropertyLayoutChanged(const AttachedPropertyOption /*option*/)
    {
    }

    void BaseObject::OnIdChanged()
    {
        Utility::EventsExtensions::Invoke(IdChanged, this, Graphics2D::UI::InputEventType::ValueChanged);
    }
}

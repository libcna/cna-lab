// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Attributes/XmlNameAttribute.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace Myra::Attributes
{
    /** @brief Specifies an alternative XML name for a registry type or property. */
    class XmlNameAttribute final
    {
    public:
        explicit XmlNameAttribute(std::string xmlName) : xmlName_(std::move(xmlName))
        {
            if (xmlName_.empty())
            {
                throw std::invalid_argument("XmlNameAttribute XML name cannot be empty.");
            }
        }

        [[nodiscard]] const std::string& getXmlNameProperty() const noexcept
        {
            return xmlName_;
        }

    private:
        std::string xmlName_;
    };
}

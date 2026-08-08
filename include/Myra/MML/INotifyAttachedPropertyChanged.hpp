// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/AttachedPropertiesRegistry.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

namespace Myra::MML
{
    class BaseAttachedPropertyInfo;

    /** @brief Receives a notification after an attached property changes. */
    class INotifyAttachedPropertyChanged
    {
    public:
        virtual ~INotifyAttachedPropertyChanged() = default;

        virtual void OnAttachedPropertyChanged(const BaseAttachedPropertyInfo& propertyInfo) = 0;
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Attributes/FilePathAttribute.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string>
#include <utility>

#include "Myra/Graphics2D/UI/File/FileDialogMode.hpp"

namespace Myra::Attributes
{
    /** @brief Describes file-dialog metadata for a string registry property. */
    class FilePathAttribute final
    {
    public:
        explicit FilePathAttribute(Graphics2D::UI::File::FileDialogMode dialogMode,
            std::string filter = {}, bool showPath = false)
            : dialogMode_(dialogMode), filter_(std::move(filter)), showPath_(showPath)
        {
        }

        [[nodiscard]] Graphics2D::UI::File::FileDialogMode getDialogModeProperty() const noexcept
        {
            return dialogMode_;
        }

        [[nodiscard]] const std::string& getFilterProperty() const noexcept
        {
            return filter_;
        }

        [[nodiscard]] bool getShowPathProperty() const noexcept
        {
            return showPath_;
        }

    private:
        Graphics2D::UI::File::FileDialogMode dialogMode_;
        std::string filter_;
        bool showPath_;
    };
}

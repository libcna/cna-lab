// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/UI/File/FileDialogMode.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

namespace Myra::Graphics2D::UI::File
{
    /** @brief Specifies the intended operation of a file dialog. */
    enum class FileDialogMode
    {
        OpenFile,
        SaveFile,
        ChooseFolder
    };
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra and MonoGame.Extended, MIT Licenses,
// Copyright (c) 2017-2020 The Myra Team and Copyright (c) 2015 Dylan Wilson.
// Ported from: src/Myra/Utility/InputExtension.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md, THIRD_PARTY_NOTICES.md, and UPSTREAM_MANIFEST.md.
#include "Myra/Utility/InputExtension.hpp"

namespace Myra::Utility
{
    std::optional<char> InputExtension::ToChar(
        const Microsoft::Xna::Framework::Input::Keys key, const bool isShiftDown) noexcept
    {
        using Microsoft::Xna::Framework::Input::Keys;

        const int value = static_cast<int>(key);
        if (value >= static_cast<int>(Keys::A) && value <= static_cast<int>(Keys::Z))
        {
            const char first = isShiftDown ? 'A' : 'a';
            return static_cast<char>(first + value - static_cast<int>(Keys::A));
        }
        if (value >= static_cast<int>(Keys::D0) && value <= static_cast<int>(Keys::D9))
        {
            if (!isShiftDown)
            {
                return static_cast<char>('0' + value - static_cast<int>(Keys::D0));
            }
            constexpr char shiftedDigits[] = ")!@#$%^&*(";
            return shiftedDigits[value - static_cast<int>(Keys::D0)];
        }
        if (value >= static_cast<int>(Keys::NumPad0) && value <= static_cast<int>(Keys::NumPad9))
        {
            return static_cast<char>('0' + value - static_cast<int>(Keys::NumPad0));
        }

        switch (key)
        {
        case Keys::Space:
            return ' ';
        case Keys::Tab:
            return '\t';
        case Keys::Enter:
            return static_cast<char>(13);
        case Keys::Back:
            return static_cast<char>(8);
        case Keys::Add:
            return '+';
        case Keys::Decimal:
            return '.';
        case Keys::Divide:
            return '/';
        case Keys::Multiply:
            return '*';
        case Keys::Subtract:
            return '-';
        case Keys::OemBackslash:
            return '\\';
        case Keys::OemComma:
            return isShiftDown ? '<' : ',';
        case Keys::OemOpenBrackets:
            return isShiftDown ? '{' : '[';
        case Keys::OemCloseBrackets:
            return isShiftDown ? '}' : ']';
        case Keys::OemPeriod:
            return isShiftDown ? '>' : '.';
        case Keys::OemPipe:
            return isShiftDown ? '|' : '\\';
        case Keys::OemPlus:
            return isShiftDown ? '+' : '=';
        case Keys::OemMinus:
            return isShiftDown ? '_' : '-';
        case Keys::OemQuestion:
            return isShiftDown ? '?' : '/';
        case Keys::OemQuotes:
            return isShiftDown ? '"' : '\'';
        case Keys::OemSemicolon:
            return isShiftDown ? ':' : ';';
        case Keys::OemTilde:
            return isShiftDown ? '~' : '`';
        default:
            return std::nullopt;
        }
    }
}

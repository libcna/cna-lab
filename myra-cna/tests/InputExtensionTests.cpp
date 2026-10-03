// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Utility/InputExtension.hpp"

#include <gtest/gtest.h>

#include <array>
#include <optional>
#include <string_view>

namespace
{
    using Microsoft::Xna::Framework::Input::Keys;
    using Myra::Utility::InputExtension;

    TEST(InputExtensionTests, MapsEveryLetterAndDigitForBothShiftStates)
    {
        for (int index = 0; index < 26; ++index)
        {
            SCOPED_TRACE(index);
            const auto key = static_cast<Keys>(static_cast<int>(Keys::A) + index);
            EXPECT_EQ(InputExtension::ToChar(key, false),
                std::optional<char>(static_cast<char>('a' + index)));
            EXPECT_EQ(InputExtension::ToChar(key, true),
                std::optional<char>(static_cast<char>('A' + index)));
        }

        constexpr std::string_view shifted = ")!@#$%^&*(";
        for (int index = 0; index < 10; ++index)
        {
            SCOPED_TRACE(index);
            const auto digit = static_cast<Keys>(static_cast<int>(Keys::D0) + index);
            const auto numberPad = static_cast<Keys>(static_cast<int>(Keys::NumPad0) + index);
            EXPECT_EQ(InputExtension::ToChar(digit, false),
                std::optional<char>(static_cast<char>('0' + index)));
            EXPECT_EQ(InputExtension::ToChar(digit, true), std::optional<char>(shifted[index]));
            EXPECT_EQ(InputExtension::ToChar(numberPad, false),
                std::optional<char>(static_cast<char>('0' + index)));
            EXPECT_EQ(InputExtension::ToChar(numberPad, true),
                std::optional<char>(static_cast<char>('0' + index)));
        }
    }

    TEST(InputExtensionTests, MapsControlArithmeticAndUsOemKeysExactly)
    {
        struct Mapping
        {
            Keys Key;
            char Plain;
            char Shifted;
        };
        constexpr std::array mappings{
            Mapping{Keys::Space, ' ', ' '},
            Mapping{Keys::Tab, '\t', '\t'},
            Mapping{Keys::Enter, static_cast<char>(13), static_cast<char>(13)},
            Mapping{Keys::Back, static_cast<char>(8), static_cast<char>(8)},
            Mapping{Keys::Add, '+', '+'},
            Mapping{Keys::Decimal, '.', '.'},
            Mapping{Keys::Divide, '/', '/'},
            Mapping{Keys::Multiply, '*', '*'},
            Mapping{Keys::Subtract, '-', '-'},
            Mapping{Keys::OemBackslash, '\\', '\\'},
            Mapping{Keys::OemComma, ',', '<'},
            Mapping{Keys::OemOpenBrackets, '[', '{'},
            Mapping{Keys::OemCloseBrackets, ']', '}'},
            Mapping{Keys::OemPeriod, '.', '>'},
            Mapping{Keys::OemPipe, '\\', '|'},
            Mapping{Keys::OemPlus, '=', '+'},
            Mapping{Keys::OemMinus, '-', '_'},
            Mapping{Keys::OemQuestion, '/', '?'},
            Mapping{Keys::OemQuotes, '\'', '"'},
            Mapping{Keys::OemSemicolon, ';', ':'},
            Mapping{Keys::OemTilde, '`', '~'},
        };

        for (const Mapping& mapping : mappings)
        {
            SCOPED_TRACE(static_cast<int>(mapping.Key));
            EXPECT_EQ(InputExtension::ToChar(mapping.Key, false),
                std::optional<char>(mapping.Plain));
            EXPECT_EQ(InputExtension::ToChar(mapping.Key, true),
                std::optional<char>(mapping.Shifted));
        }
    }

    TEST(InputExtensionTests, ReturnsNoCharacterForUnsupportedKeys)
    {
        EXPECT_EQ(InputExtension::ToChar(Keys::None, false), std::nullopt);
        EXPECT_EQ(InputExtension::ToChar(Keys::Escape, true), std::nullopt);
        EXPECT_EQ(InputExtension::ToChar(Keys::F1, false), std::nullopt);
        EXPECT_EQ(InputExtension::ToChar(Keys::LeftShift, true), std::nullopt);
    }
}

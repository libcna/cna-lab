// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Myra.hpp"

#include <iostream>

int main()
{
    std::cout << "Myra-CNA " << Myra::Version::GetString() << '\n';
    return 0;
}

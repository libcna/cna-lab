// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Myra.hpp"

#include <iostream>

#if !defined(MYRA_CNA_HAS_CNA_TARGET)
#error "The parent-provided CNA target must select linked consumer mode"
#endif

int main()
{
    std::cout << "parent target -> Myra::CNA -> " << Myra::Version::GetString() << '\n';
    return 0;
}

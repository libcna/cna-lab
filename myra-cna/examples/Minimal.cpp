// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Myra.hpp"

#include <iostream>
#if defined(MYRA_CNA_HAS_CNA_TARGET)
#include <memory>
#endif

int main()
{
#if defined(MYRA_CNA_HAS_CNA_TARGET)
    auto root = std::make_shared<Myra::Graphics2D::UI::VerticalStackPanel>();
    auto button = std::make_shared<Myra::Graphics2D::UI::Button>();
    button->setWidthProperty(120);
    button->setHeightProperty(32);
    root->AddWidget(button);

    std::cout << "Myra-CNA " << Myra::Version::GetString() << ": " << root->getWidgetsProperty().size()
              << " logical widget\n";
#else
    std::cout << "Myra-CNA " << Myra::Version::GetString() << " (headers-only compile-check mode)\n";
#endif
    return 0;
}

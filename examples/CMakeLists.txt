# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
# See NOTICE.md and THIRD_PARTY_NOTICES.md.
add_executable(myra_cna_minimal Minimal.cpp)
target_link_libraries(myra_cna_minimal PRIVATE Myra::CNA)
myra_cna_enable_strict_warnings(myra_cna_minimal)

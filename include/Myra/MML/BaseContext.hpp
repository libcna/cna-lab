// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/MML/BaseContext.cs at 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <string_view>
#include <typeindex>
#include <vector>

#include "Myra/MML/TypeRegistry.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"

namespace Myra::MML
{
    /** @brief Registry-backed property split consumed by MML load and save contexts. */
    struct ParsedProperties final
    {
        std::vector<const PropertyDescriptor*> Complex;
        std::vector<const PropertyDescriptor*> Simple;
    };

    /** @brief Shared explicit-metadata foundation for MML loading and saving. */
    class BaseContext
    {
    public:
        static constexpr std::string_view IdName = "Id";

        BaseContext(const TypeRegistry& typeRegistry, const ValueCodecRegistry& valueCodecs) noexcept;
        virtual ~BaseContext() = default;

        [[nodiscard]] const ITypeSerializer* FindSerializer(std::type_index type) const noexcept;
        [[nodiscard]] static bool IsPropertyExternalAsset(const PropertyDescriptor& property) noexcept;
        [[nodiscard]] ParsedProperties ParseProperties(std::type_index type, bool isSave) const;

    protected:
        [[nodiscard]] const TypeRegistry& getTypeRegistryProperty() const noexcept;
        [[nodiscard]] const ValueCodecRegistry& getValueCodecsProperty() const noexcept;

    private:
        const TypeRegistry& typeRegistry_;
        const ValueCodecRegistry& valueCodecs_;
    };
}

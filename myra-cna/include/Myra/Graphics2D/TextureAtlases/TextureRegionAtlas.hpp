// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TextureRegionAtlas.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "Myra/Graphics2D/TextureAtlases/TextureRegion.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace Myra::Graphics2D::TextureAtlases
{
    /** @brief Retained collection of named regions cut from one CNA texture. */
    class TextureRegionAtlas final
    {
    public:
        using RegionMap = std::unordered_map<std::string, std::shared_ptr<TextureRegion>>;
        using TextureGetter = std::function<
            std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>(
                const std::string&)>;

        TextureRegionAtlas() = default;

        [[nodiscard]] const std::optional<std::string>& getNameProperty() const noexcept;
        void setNameProperty(std::optional<std::string> value);
        [[nodiscard]] const std::optional<std::string>& getImageProperty() const noexcept;
        void setImageProperty(std::optional<std::string> value);

        [[nodiscard]] const RegionMap& getRegionsProperty() const noexcept;
        [[nodiscard]] RegionMap& getRegionsProperty() noexcept;
        [[nodiscard]] std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>
            getTextureProperty() const;

        /** @brief Implements the selected-upstream string indexer getter. */
        [[nodiscard]] std::shared_ptr<TextureRegion> getItemProperty(
            const std::string& name) const;
        /** @brief Implements the selected-upstream string indexer setter. */
        void setItemProperty(std::string name, std::shared_ptr<TextureRegion> value);

        /** @brief Resolves and retains a non-null region, or throws if it is unavailable. */
        [[nodiscard]] std::shared_ptr<TextureRegion> EnsureRegion(const std::string& id) const;

        /** @brief Splits `atlas:region` references using the upstream separator rule. */
        static bool TryGetRegionName(
            std::string& assetName, std::optional<std::string>& regionName);

        [[nodiscard]] std::string ToXml() const;
        [[nodiscard]] static TextureRegionAtlas FromXml(
            const std::string& xml, const TextureGetter& textureGetter);
        [[nodiscard]] std::string ToString() const;

    private:
        static constexpr char Separator = ':';

        std::optional<std::string> name_;
        std::optional<std::string> image_;
        RegionMap regions_;
        std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D> texture_;
    };
}

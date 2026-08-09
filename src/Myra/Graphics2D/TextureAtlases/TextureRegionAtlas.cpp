// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// Portions derived from MyraUI/Myra, MIT License,
// Copyright (c) 2017-2020 The Myra Team.
// Ported from: src/Myra/Graphics2D/TextureAtlases/TextureRegionAtlas.cs at
// 0d79b939310bfe1d00b21803fe15e291caf60aa1.
// See NOTICE.md and UPSTREAM_MANIFEST.md.
#include "Myra/Graphics2D/TextureAtlases/TextureRegionAtlas.hpp"

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <utility>

#include "Myra/Graphics2D/TextureAtlases/NinePatchRegion.hpp"
#include "Myra/MML/BaseContext.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Char.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlNodeList.hpp"
#include "System/Xml/XmlNodeType.hpp"

namespace Myra::Graphics2D::TextureAtlases
{
    namespace
    {
        constexpr std::string_view TextureAtlasName = "TextureAtlas";
        constexpr std::string_view ImageName = "Image";
        constexpr std::string_view TextureRegionName = "TextureRegion";
        constexpr std::string_view NinePatchRegionName = "NinePatchRegion";
        constexpr std::string_view LeftName = "Left";
        constexpr std::string_view TopName = "Top";
        constexpr std::string_view WidthName = "Width";
        constexpr std::string_view HeightName = "Height";
        constexpr std::string_view NinePatchLeftName = "NinePatchLeft";
        constexpr std::string_view NinePatchTopName = "NinePatchTop";
        constexpr std::string_view NinePatchRightName = "NinePatchRight";
        constexpr std::string_view NinePatchBottomName = "NinePatchBottom";

        [[nodiscard]] std::string Trim(const std::string& value)
        {
            const std::size_t first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
            {
                return {};
            }
            return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        }

        [[nodiscard]] std::string RequireAttribute(
            const System::Xml::XmlElement& element, const std::string_view name)
        {
            const std::string ownedName(name);
            if (!element.HasAttribute(ownedName))
            {
                throw std::invalid_argument(
                    "Texture atlas element '" + element.getNameProperty()
                    + "' is missing mandatory attribute '" + ownedName + "'.");
            }
            return element.GetAttribute(ownedName);
        }

        [[nodiscard]] int ParseIntegerAttribute(
            const System::Xml::XmlElement& element, const std::string_view name)
        {
            std::string text = Trim(RequireAttribute(element, name));
            std::string_view value(text);
            if (!value.empty() && value.front() == '+')
            {
                value.remove_prefix(1);
            }

            int result = 0;
            const auto [position, error] =
                std::from_chars(value.data(), value.data() + value.size(), result);
            if (value.empty() || error != std::errc{}
                || position != value.data() + value.size())
            {
                throw std::invalid_argument(
                    "Texture atlas attribute '" + std::string(name)
                    + "' on region '" + element.getNameProperty()
                    + "' is not a valid invariant-culture integer.");
            }
            return result;
        }

        void SetIntegerAttribute(
            System::Xml::XmlElement& element, const std::string_view name, const int value)
        {
            element.SetAttribute(std::string(name), std::to_string(value));
        }
    }

    const std::optional<std::string>& TextureRegionAtlas::getNameProperty() const noexcept
    {
        return name_;
    }

    void TextureRegionAtlas::setNameProperty(std::optional<std::string> value)
    {
        name_ = std::move(value);
    }

    const std::optional<std::string>& TextureRegionAtlas::getImageProperty() const noexcept
    {
        return image_;
    }

    void TextureRegionAtlas::setImageProperty(std::optional<std::string> value)
    {
        image_ = std::move(value);
    }

    const TextureRegionAtlas::RegionMap& TextureRegionAtlas::getRegionsProperty() const noexcept
    {
        return regions_;
    }

    TextureRegionAtlas::RegionMap& TextureRegionAtlas::getRegionsProperty() noexcept
    {
        return regions_;
    }

    std::shared_ptr<Microsoft::Xna::Framework::Graphics::Texture2D>
        TextureRegionAtlas::getTextureProperty() const
    {
        return texture_;
    }

    std::shared_ptr<TextureRegion> TextureRegionAtlas::getItemProperty(
        const std::string& name) const
    {
        return regions_.at(name);
    }

    void TextureRegionAtlas::setItemProperty(
        std::string name, std::shared_ptr<TextureRegion> value)
    {
        if (value == nullptr)
        {
            throw std::invalid_argument("A texture atlas region cannot be null.");
        }
        regions_.insert_or_assign(std::move(name), std::move(value));
    }

    std::shared_ptr<TextureRegion> TextureRegionAtlas::EnsureRegion(const std::string& id) const
    {
        const auto iterator = regions_.find(id);
        if (iterator == regions_.end() || iterator->second == nullptr)
        {
            throw std::out_of_range("Could not resolve region '" + id + "'.");
        }
        return iterator->second;
    }

    bool TextureRegionAtlas::TryGetRegionName(
        std::string& assetName, std::optional<std::string>& regionName)
    {
        regionName.reset();
        for (std::size_t index = 0; index + 1 < assetName.size(); ++index)
        {
            const auto next = static_cast<SharpRuntime::charcs>(
                static_cast<unsigned char>(assetName[index + 1]));
            if (assetName[index] != Separator || !System::Char::IsLetterOrDigit(next))
            {
                continue;
            }

            regionName = Trim(assetName.substr(index + 1));
            assetName = Trim(assetName.substr(0, index));
            return true;
        }
        return false;
    }

    std::string TextureRegionAtlas::ToXml() const
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root =
            document.CreateElement(std::string(TextureAtlasName));
        if (image_)
        {
            root->SetAttribute(std::string(ImageName), *image_);
        }
        document.AppendChild(root);

        for (const auto& [key, region] : regions_)
        {
            (void) key;
            if (region == nullptr)
            {
                throw std::invalid_argument("A texture atlas cannot serialize a null region.");
            }

            const auto* ninePatch = dynamic_cast<const NinePatchRegion*>(region.get());
            System::Xml::XmlElement* entry = document.CreateElement(std::string(
                ninePatch == nullptr ? TextureRegionName : NinePatchRegionName));
            if (region->getNameProperty())
            {
                entry->SetAttribute(std::string(MML::BaseContext::IdName),
                    *region->getNameProperty());
            }

            const Microsoft::Xna::Framework::Rectangle bounds = region->getBoundsProperty();
            SetIntegerAttribute(*entry, LeftName, bounds.X);
            SetIntegerAttribute(*entry, TopName, bounds.Y);
            SetIntegerAttribute(*entry, WidthName, bounds.Width);
            SetIntegerAttribute(*entry, HeightName, bounds.Height);

            if (ninePatch != nullptr)
            {
                const Thickness info = ninePatch->getInfoProperty();
                SetIntegerAttribute(*entry, NinePatchLeftName, info.Left);
                SetIntegerAttribute(*entry, NinePatchTopName, info.Top);
                SetIntegerAttribute(*entry, NinePatchRightName, info.Right);
                SetIntegerAttribute(*entry, NinePatchBottomName, info.Bottom);
            }
            root->AppendChild(entry);
        }
        return document.getOuterXmlProperty();
    }

    TextureRegionAtlas TextureRegionAtlas::FromXml(
        const std::string& xml, const TextureGetter& textureGetter)
    {
        if (!textureGetter)
        {
            throw std::invalid_argument("TextureRegionAtlas requires a texture getter.");
        }

        System::Xml::XmlDocument document;
        document.LoadXml(xml);
        System::Xml::XmlElement* root = document.getDocumentElementProperty();
        if (root == nullptr)
        {
            throw std::invalid_argument("Texture atlas XML has no root element.");
        }

        TextureRegionAtlas result;
        result.image_ = RequireAttribute(*root, ImageName);
        result.texture_ = textureGetter(*result.image_);
        if (result.texture_ == nullptr)
        {
            throw std::invalid_argument("The texture getter returned a null atlas texture.");
        }

        System::Xml::XmlNodeList* children = root->getChildNodesProperty();
        for (SharpRuntime::intcs index = 0; index < children->getCountProperty(); ++index)
        {
            System::Xml::XmlNode* node = children->Item(index);
            if (node == nullptr
                || node->getNodeTypeProperty() != System::Xml::XmlNodeType::Element)
            {
                continue;
            }
            const auto& entry = *static_cast<System::Xml::XmlElement*>(node);
            const std::string id = RequireAttribute(entry, MML::BaseContext::IdName);
            const Microsoft::Xna::Framework::Rectangle bounds(
                ParseIntegerAttribute(entry, LeftName),
                ParseIntegerAttribute(entry, TopName),
                ParseIntegerAttribute(entry, WidthName),
                ParseIntegerAttribute(entry, HeightName));

            std::shared_ptr<TextureRegion> region;
            if (entry.getNameProperty() != NinePatchRegionName)
            {
                region = std::make_shared<TextureRegion>(result.texture_, bounds);
            }
            else
            {
                const Thickness info(
                    ParseIntegerAttribute(entry, NinePatchLeftName),
                    ParseIntegerAttribute(entry, NinePatchTopName),
                    ParseIntegerAttribute(entry, NinePatchRightName),
                    ParseIntegerAttribute(entry, NinePatchBottomName));
                region = std::make_shared<NinePatchRegion>(result.texture_, bounds, info);
            }

            region->setNameProperty(id);
            result.regions_.insert_or_assign(id, std::move(region));
        }
        return result;
    }

    std::string TextureRegionAtlas::ToString() const
    {
        return name_.value_or(std::string());
    }
}

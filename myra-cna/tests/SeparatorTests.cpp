// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/HorizontalSeparator.hpp"
#include "Myra/Graphics2D/UI/Simple/VerticalSeparator.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/UI/Simple/SeparatorWidget.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::HorizontalSeparator;
    using Myra::Graphics2D::UI::ImageResizeMode;
    using Myra::Graphics2D::UI::Orientation;
    using Myra::Graphics2D::UI::SeparatorWidget;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::Graphics2D::UI::VerticalSeparator;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    struct SeparatorDraw
    {
        Rectangle destination;
        Color color;
    };

    class RecordingImage final : public IImage
    {
    public:
        explicit RecordingImage(std::vector<SeparatorDraw>& draws) : draws_(draws) {}

        [[nodiscard]] Point getSizeProperty() const override { return Point(9, 7); }

        void Draw(RenderContext&, const Rectangle destination, const Color color) const override
        {
            draws_.push_back({destination, color});
        }

    private:
        std::vector<SeparatorDraw>& draws_;
    };

    TEST(SeparatorTests, ConcreteDefaultsAndOrientationMatchTheSelectedUpstream)
    {
        HorizontalSeparator horizontal;
        EXPECT_EQ(horizontal.getThicknessProperty(), 0);
        EXPECT_EQ(horizontal.getOrientationProperty(), Orientation::Horizontal);
        EXPECT_EQ(horizontal.getHorizontalAlignmentProperty(), HorizontalAlignment::Stretch);
        EXPECT_EQ(horizontal.getVerticalAlignmentProperty(), VerticalAlignment::Center);
        EXPECT_EQ(horizontal.Measure(Point(100, 80)), Point(0, 0));

        VerticalSeparator vertical;
        EXPECT_EQ(vertical.getThicknessProperty(), 0);
        EXPECT_EQ(vertical.getOrientationProperty(), Orientation::Vertical);
        EXPECT_EQ(vertical.getHorizontalAlignmentProperty(), HorizontalAlignment::Center);
        EXPECT_EQ(vertical.getVerticalAlignmentProperty(), VerticalAlignment::Stretch);
        EXPECT_EQ(vertical.Measure(Point(100, 80)), Point(0, 0));
    }

    TEST(SeparatorTests, ThicknessChangesInvalidateCachedCrossAxisMeasurement)
    {
        HorizontalSeparator horizontal;
        EXPECT_EQ(horizontal.Measure(Point(100, 80)), Point(0, 0));
        horizontal.setThicknessProperty(6);
        EXPECT_EQ(horizontal.Measure(Point(100, 80)), Point(0, 6));
        horizontal.setThicknessProperty(-2);
        EXPECT_EQ(horizontal.Measure(Point(100, 80)), Point(0, -2));

        VerticalSeparator vertical;
        EXPECT_EQ(vertical.Measure(Point(100, 80)), Point(0, 0));
        vertical.setThicknessProperty(5);
        EXPECT_EQ(vertical.Measure(Point(100, 80)), Point(5, 0));
    }

    TEST(SeparatorTests, UsesInheritedImageRenderingInsideItsMeasuredLine)
    {
        Game game;
        RenderContext context(game.getGraphicsDeviceProperty());
        std::vector<SeparatorDraw> draws;
        auto image = std::make_shared<RecordingImage>(draws);

        HorizontalSeparator separator;
        separator.setThicknessProperty(4);
        separator.setRenderableProperty(image);
        separator.setColorProperty(Color(10, 20, 30, 40));
        separator.Arrange(Rectangle(10, 20, 30, 12));
        separator.InternalRender(context);

        ASSERT_EQ(draws.size(), 1U);
        EXPECT_EQ(draws.front().destination, Rectangle(0, 0, 30, 4));
        EXPECT_EQ(draws.front().color, Color(10, 20, 30, 40));
    }

    TEST(SeparatorTests, ClonePreservesExactConcreteTypeAndSeparatorImageState)
    {
        std::vector<SeparatorDraw> draws;
        auto image = std::make_shared<RecordingImage>(draws);

        HorizontalSeparator horizontal;
        horizontal.setThicknessProperty(8);
        horizontal.setRenderableProperty(image);
        horizontal.setColorProperty(Color(1, 2, 3, 4));
        horizontal.setResizeModeProperty(ImageResizeMode::KeepAspectRatio);
        horizontal.setVerticalAlignmentProperty(VerticalAlignment::Bottom);

        const std::shared_ptr<HorizontalSeparator> horizontalClone =
            std::dynamic_pointer_cast<HorizontalSeparator>(horizontal.Clone());
        ASSERT_NE(horizontalClone, nullptr);
        EXPECT_EQ(horizontalClone->getThicknessProperty(), 8);
        EXPECT_EQ(horizontalClone->getRenderableProperty(), image);
        EXPECT_EQ(horizontalClone->getColorProperty(), Color(1, 2, 3, 4));
        EXPECT_EQ(horizontalClone->getResizeModeProperty(), ImageResizeMode::KeepAspectRatio);
        EXPECT_EQ(horizontalClone->getVerticalAlignmentProperty(), VerticalAlignment::Bottom);
        EXPECT_EQ(horizontalClone->getOrientationProperty(), Orientation::Horizontal);

        VerticalSeparator vertical;
        vertical.setThicknessProperty(3);
        const std::shared_ptr<VerticalSeparator> verticalClone =
            std::dynamic_pointer_cast<VerticalSeparator>(vertical.Clone());
        ASSERT_NE(verticalClone, nullptr);
        EXPECT_EQ(verticalClone->getThicknessProperty(), 3);
        EXPECT_EQ(verticalClone->getOrientationProperty(), Orientation::Vertical);
    }

    TEST(SeparatorTests, RegistersAbstractAndConcreteMmlMetadataAndDefaults)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* separator = registry.FindByType(typeid(SeparatorWidget));
        const TypeDescriptor* horizontal = registry.FindByType(typeid(HorizontalSeparator));
        const TypeDescriptor* vertical = registry.FindByType(typeid(VerticalSeparator));
        ASSERT_NE(separator, nullptr);
        ASSERT_NE(horizontal, nullptr);
        ASSERT_NE(vertical, nullptr);
        EXPECT_FALSE(separator->getCanCreateProperty());
        EXPECT_TRUE(horizontal->getCanCreateProperty());
        EXPECT_TRUE(vertical->getCanCreateProperty());

        const PropertyDescriptor* thickness =
            registry.FindPropertyByName(typeid(HorizontalSeparator), "Thickness");
        const PropertyDescriptor* orientation =
            registry.FindPropertyByName(typeid(HorizontalSeparator), "Orientation");
        ASSERT_NE(thickness, nullptr);
        ASSERT_NE(orientation, nullptr);
        EXPECT_EQ(std::any_cast<int>(*thickness->getDefaultValueProperty()), 0);
        EXPECT_TRUE(orientation->getMetadataProperty().XmlIgnore);
        EXPECT_TRUE(orientation->getCanReadProperty());
        EXPECT_FALSE(orientation->getCanWriteProperty());

        const PropertyDescriptor* horizontalAlignment =
            registry.FindPropertyByName(typeid(HorizontalSeparator), "HorizontalAlignment");
        const PropertyDescriptor* horizontalVerticalAlignment =
            registry.FindPropertyByName(typeid(HorizontalSeparator), "VerticalAlignment");
        const PropertyDescriptor* verticalHorizontalAlignment =
            registry.FindPropertyByName(typeid(VerticalSeparator), "HorizontalAlignment");
        const PropertyDescriptor* verticalAlignment =
            registry.FindPropertyByName(typeid(VerticalSeparator), "VerticalAlignment");
        ASSERT_NE(horizontalAlignment, nullptr);
        ASSERT_NE(horizontalVerticalAlignment, nullptr);
        ASSERT_NE(verticalHorizontalAlignment, nullptr);
        ASSERT_NE(verticalAlignment, nullptr);
        EXPECT_EQ(std::any_cast<HorizontalAlignment>(
                      *horizontalAlignment->getDefaultValueProperty()),
            HorizontalAlignment::Stretch);
        EXPECT_EQ(std::any_cast<VerticalAlignment>(
                      *horizontalVerticalAlignment->getDefaultValueProperty()),
            VerticalAlignment::Center);
        EXPECT_EQ(std::any_cast<HorizontalAlignment>(
                      *verticalHorizontalAlignment->getDefaultValueProperty()),
            HorizontalAlignment::Center);
        EXPECT_EQ(std::any_cast<VerticalAlignment>(
                      *verticalAlignment->getDefaultValueProperty()),
            VerticalAlignment::Stretch);

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml(
            "<VerticalSeparator Thickness=\"7\" HorizontalAlignment=\"Right\" />");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(VerticalSeparator));
        const auto* loadedVertical = static_cast<const VerticalSeparator*>(loaded.Value.get());
        EXPECT_EQ(loadedVertical->getThicknessProperty(), 7);
        EXPECT_EQ(loadedVertical->getHorizontalAlignmentProperty(), HorizontalAlignment::Right);
        EXPECT_EQ(loadedVertical->getVerticalAlignmentProperty(), VerticalAlignment::Stretch);

        const SaveContext saver(registry, codecs);
        const std::string xml = saver.ToXml(loadedVertical, typeid(VerticalSeparator));
        EXPECT_NE(xml.find("Thickness=\"7\""), std::string::npos);
        EXPECT_NE(xml.find("HorizontalAlignment=\"Right\""), std::string::npos);
        EXPECT_EQ(xml.find("VerticalAlignment="), std::string::npos);
        EXPECT_EQ(xml.find("Orientation="), std::string::npos);

        const HorizontalSeparator defaultHorizontal;
        const std::string defaultXml =
            saver.ToXml(&defaultHorizontal, typeid(HorizontalSeparator));
        EXPECT_EQ(defaultXml.find("HorizontalAlignment="), std::string::npos);
        EXPECT_EQ(defaultXml.find("VerticalAlignment="), std::string::npos);
        EXPECT_EQ(defaultXml.find("Thickness="), std::string::npos);
    }
}

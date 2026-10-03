// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/Graphics2D/UI/Simple/Image.hpp"

#include <gtest/gtest.h>

#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Myra/Graphics2D/IImage.hpp"
#include "Myra/Graphics2D/RenderContext.hpp"
#include "Myra/Graphics2D/Thickness.hpp"
#include "Myra/Graphics2D/UI/Enums.hpp"
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/RegisterMyraTypes.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/ValueCodecRegistry.hpp"
#include "Myra/MyraEnvironment.hpp"
#include "System/Xml/XmlDocument.hpp"

namespace
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Game;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Myra::Graphics2D::IImage;
    using Myra::Graphics2D::RenderContext;
    using Myra::Graphics2D::Thickness;
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::Graphics2D::UI::Image;
    using Myra::Graphics2D::UI::ImageResizeMode;
    using Myra::Graphics2D::UI::VerticalAlignment;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;
    using Myra::MyraEnvironment;

    struct ImageDraw
    {
        std::string name;
        Rectangle destination;
        Color color;
    };

    class RecordingImage final : public IImage
    {
    public:
        RecordingImage(std::string name, const Point size, std::vector<ImageDraw>& draws)
            : name_(std::move(name)), size_(size), draws_(draws)
        {
        }

        [[nodiscard]] Point getSizeProperty() const override
        {
            ++sizeQueries;
            return size_;
        }

        void Draw(RenderContext&, const Rectangle destination, const Color color) const override
        {
            draws_.push_back({name_, destination, color});
        }

        mutable int sizeQueries = 0;

    private:
        std::string name_;
        Point size_;
        std::vector<ImageDraw>& draws_;
    };

    class HoverImage final : public Image
    {
    public:
        bool hovered = false;

    protected:
        [[nodiscard]] bool UseOverBackground() const noexcept override { return hovered; }
    };

    class ImageTests : public testing::Test
    {
    protected:
        void SetUp() override
        {
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
        }

        void TearDown() override
        {
            MyraEnvironment::setDrawWidgetsFramesProperty(false);
            MyraEnvironment::setDrawKeyboardFocusedWidgetFrameProperty(false);
            MyraEnvironment::setDrawMouseHoveredWidgetFrameProperty(false);
            MyraEnvironment::setDisableClippingProperty(false);
            MyraEnvironment::ClearGame();
        }

        static RenderContext MakeContext(Game& game)
        {
            return RenderContext(game.getGraphicsDeviceProperty());
        }
    };

    static_assert(static_cast<int>(ImageResizeMode::Stretch) == 0);
    static_assert(static_cast<int>(ImageResizeMode::KeepAspectRatio) == 1);

    TEST_F(ImageTests, DefaultsMatchTheSelectedFnaSurface)
    {
        Image image;

        EXPECT_EQ(image.getRenderableProperty(), nullptr);
        EXPECT_EQ(image.getDisabledRenderableProperty(), nullptr);
        EXPECT_EQ(image.getOverRenderableProperty(), nullptr);
        EXPECT_EQ(image.getFocusedRenderableProperty(), nullptr);
        EXPECT_EQ(image.getPressedRenderableProperty(), nullptr);
        EXPECT_EQ(image.getColorProperty(), Color::White);
        EXPECT_EQ(image.getResizeModeProperty(), ImageResizeMode::Stretch);
        EXPECT_EQ(image.Measure(Point(100, 100)), Point(0, 0));
    }

    TEST_F(ImageTests, MeasuresEveryVisualAndInvalidatesOnlyWhenAHandleChanges)
    {
        std::vector<ImageDraw> draws;
        auto normal = std::make_shared<RecordingImage>("normal", Point(10, 20), draws);
        auto disabled = std::make_shared<RecordingImage>("disabled", Point(30, 5), draws);
        auto over = std::make_shared<RecordingImage>("over", Point(15, 40), draws);
        auto focused = std::make_shared<RecordingImage>("focused", Point(25, 35), draws);
        auto pressed = std::make_shared<RecordingImage>("pressed", Point(50, 10), draws);

        Image image;
        image.setRenderableProperty(normal);
        image.setDisabledRenderableProperty(disabled);
        image.setOverRenderableProperty(over);
        image.setFocusedRenderableProperty(focused);
        image.setPressedRenderableProperty(pressed);

        EXPECT_EQ(image.Measure(Point(100, 100)), Point(50, 40));
        EXPECT_EQ(normal->sizeQueries, 1);
        EXPECT_EQ(disabled->sizeQueries, 1);
        EXPECT_EQ(over->sizeQueries, 1);
        EXPECT_EQ(focused->sizeQueries, 1);
        EXPECT_EQ(pressed->sizeQueries, 1);

        EXPECT_EQ(image.Measure(Point(100, 100)), Point(50, 40));
        image.setRenderableProperty(normal);
        EXPECT_EQ(image.Measure(Point(100, 100)), Point(50, 40));
        EXPECT_EQ(normal->sizeQueries, 1);

        auto replacement = std::make_shared<RecordingImage>("replacement", Point(60, 12), draws);
        std::weak_ptr<RecordingImage> retained = replacement;
        image.setRenderableProperty(replacement);
        EXPECT_EQ(image.Measure(Point(100, 100)), Point(60, 40));
        replacement.reset();
        EXPECT_FALSE(retained.expired());
        image.setRenderableProperty(nullptr);
        EXPECT_TRUE(retained.expired());
    }

    TEST_F(ImageTests, SelectsVisualsUsingTheWidgetStatePriority)
    {
        Game game;
        RenderContext context = MakeContext(game);
        std::vector<ImageDraw> draws;
        auto normal = std::make_shared<RecordingImage>("normal", Point(5, 5), draws);
        auto disabled = std::make_shared<RecordingImage>("disabled", Point(5, 5), draws);
        auto over = std::make_shared<RecordingImage>("over", Point(5, 5), draws);
        auto focused = std::make_shared<RecordingImage>("focused", Point(5, 5), draws);
        auto pressed = std::make_shared<RecordingImage>("pressed", Point(5, 5), draws);

        HoverImage image;
        image.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        image.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        image.setRenderableProperty(normal);
        image.setDisabledRenderableProperty(disabled);
        image.setOverRenderableProperty(over);
        image.setFocusedRenderableProperty(focused);
        image.setPressedRenderableProperty(pressed);
        image.Arrange(Rectangle(0, 0, 20, 20));

        const auto expectDraw = [&](const std::string& expected) {
            draws.clear();
            image.InternalRender(context);
            ASSERT_EQ(draws.size(), 1U);
            EXPECT_EQ(draws.front().name, expected);
        };
        expectDraw("normal");
        image.hovered = true;
        expectDraw("over");
        image.OnGotKeyboardFocus();
        expectDraw("focused");
        image.setIsPressedProperty(true);
        expectDraw("pressed");
        image.setIsPressedProperty(false);
        image.OnLostKeyboardFocus();
        image.hovered = false;
        image.setEnabledProperty(false);
        expectDraw("disabled");
        image.setDisabledRenderableProperty(nullptr);
        expectDraw("over");
    }

    TEST_F(ImageTests, RendersTintAndPreservesTheSelectedUpstreamAspectFormula)
    {
        Game game;
        RenderContext context = MakeContext(game);
        std::vector<ImageDraw> draws;
        auto renderable = std::make_shared<RecordingImage>("image", Point(40, 20), draws);

        Image image;
        image.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        image.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        image.setPaddingProperty(Thickness(2, 3, 4, 5));
        image.setRenderableProperty(renderable);
        image.setColorProperty(Color(10, 20, 30, 40));
        image.Arrange(Rectangle(0, 0, 100, 80));

        image.InternalRender(context);
        ASSERT_EQ(draws.size(), 1U);
        EXPECT_EQ(draws.back().destination, Rectangle(2, 3, 94, 72));
        EXPECT_EQ(draws.back().color, Color(10, 20, 30, 40));

        image.setResizeModeProperty(ImageResizeMode::KeepAspectRatio);
        image.InternalRender(context);
        ASSERT_EQ(draws.size(), 2U);
        EXPECT_EQ(draws.back().destination, Rectangle(2, 3, 94, 188));
        EXPECT_EQ(draws.back().color, Color(10, 20, 30, 40));
    }

    TEST_F(ImageTests, RejectsUndefinedAspectConversionsDeterministically)
    {
        Game game;
        RenderContext context = MakeContext(game);
        std::vector<ImageDraw> draws;

        Image zeroHeight;
        zeroHeight.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        zeroHeight.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        zeroHeight.setRenderableProperty(
            std::make_shared<RecordingImage>("zero", Point(10, 0), draws));
        zeroHeight.setResizeModeProperty(ImageResizeMode::KeepAspectRatio);
        zeroHeight.Arrange(Rectangle(0, 0, 20, 20));
        EXPECT_THROW(zeroHeight.InternalRender(context), std::invalid_argument);

        Image overflow;
        overflow.setHorizontalAlignmentProperty(HorizontalAlignment::Stretch);
        overflow.setVerticalAlignmentProperty(VerticalAlignment::Stretch);
        overflow.setRenderableProperty(std::make_shared<RecordingImage>(
            "overflow", Point(std::numeric_limits<int>::max(), 1), draws));
        overflow.setResizeModeProperty(ImageResizeMode::KeepAspectRatio);
        overflow.Arrange(Rectangle(0, 0, std::numeric_limits<int>::max(), 1));
        EXPECT_THROW(overflow.InternalRender(context), std::overflow_error);
        EXPECT_TRUE(draws.empty());
    }

    TEST_F(ImageTests, ClonePreservesStateAndSharesManagedRenderableHandles)
    {
        std::vector<ImageDraw> draws;
        auto normal = std::make_shared<RecordingImage>("normal", Point(10, 20), draws);
        auto disabled = std::make_shared<RecordingImage>("disabled", Point(20, 10), draws);
        auto over = std::make_shared<RecordingImage>("over", Point(15, 15), draws);
        auto focused = std::make_shared<RecordingImage>("focused", Point(12, 18), draws);
        auto pressed = std::make_shared<RecordingImage>("pressed", Point(18, 12), draws);

        Image source;
        source.setRenderableProperty(normal);
        source.setDisabledRenderableProperty(disabled);
        source.setOverRenderableProperty(over);
        source.setFocusedRenderableProperty(focused);
        source.setPressedRenderableProperty(pressed);
        source.setColorProperty(Color(1, 2, 3, 4));
        source.setResizeModeProperty(ImageResizeMode::KeepAspectRatio);

        const std::shared_ptr<Image> clone = std::dynamic_pointer_cast<Image>(source.Clone());

        ASSERT_NE(clone, nullptr);
        EXPECT_EQ(clone->getRenderableProperty(), normal);
        EXPECT_EQ(clone->getDisabledRenderableProperty(), disabled);
        EXPECT_EQ(clone->getOverRenderableProperty(), over);
        EXPECT_EQ(clone->getFocusedRenderableProperty(), focused);
        EXPECT_EQ(clone->getPressedRenderableProperty(), pressed);
        EXPECT_EQ(clone->getColorProperty(), Color(1, 2, 3, 4));
        EXPECT_EQ(clone->getResizeModeProperty(), ImageResizeMode::KeepAspectRatio);
    }

    TEST_F(ImageTests, RegistersResizeAndExternalRenderableMmlMetadata)
    {
        const TypeRegistry registry = Myra::MML::CreateMyraTypeRegistry();
        const TypeDescriptor* descriptor = registry.FindByType(typeid(Image));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_TRUE(descriptor->getCanCreateProperty());

        const std::unordered_map<std::string, std::string> stylePaths{
            {"Renderable", "Image"},
            {"DisabledRenderable", "DisabledImage"},
            {"OverRenderable", "OverImage"},
            {"FocusedRenderable", "FocusedImage"},
            {"PressedRenderable", "PressedImage"},
        };
        for (const auto& [name, stylePath] : stylePaths)
        {
            const PropertyDescriptor* property = registry.FindPropertyByName(typeid(Image), name);
            ASSERT_NE(property, nullptr);
            EXPECT_EQ(property->getValueTypeProperty(), typeid(std::shared_ptr<IImage>));
            EXPECT_TRUE(property->getMetadataProperty().ExternalAsset);
            EXPECT_EQ(property->getMetadataProperty().StylePropertyPath, stylePath);
            EXPECT_TRUE(property->getCanBeNullProperty());
        }

        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(registry, codecs);
        std::vector<ImageDraw> draws;
        const std::shared_ptr<IImage> normal =
            std::make_shared<RecordingImage>("normal", Point(10, 20), draws);
        const std::shared_ptr<IImage> pressed =
            std::make_shared<RecordingImage>("pressed", Point(20, 10), draws);
        const std::unordered_map<std::string, std::shared_ptr<IImage>> assets{
            {"atlas:normal", normal}, {"atlas:pressed", pressed}};
        loader.LoadExternalAsset = [&](const PropertyDescriptor& property,
                                       const std::string& assetName) -> std::any {
            EXPECT_TRUE(property.getMetadataProperty().ExternalAsset);
            return assets.at(assetName);
        };

        System::Xml::XmlDocument document;
        document.LoadXml(
            "<Image ResizeMode=\"KeepAspectRatio\" Renderable=\"atlas:normal\" "
            "PressedRenderable=\"atlas:pressed\" />");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*document.getDocumentElementProperty());
        const auto* image = static_cast<const Image*>(loaded.Value.get());
        EXPECT_EQ(image->getResizeModeProperty(), ImageResizeMode::KeepAspectRatio);
        EXPECT_EQ(image->getRenderableProperty(), normal);
        EXPECT_EQ(image->getPressedRenderableProperty(), pressed);

        SaveContext saver(registry, codecs);
        saver.SaveExternalAsset = [&](const PropertyDescriptor&, const std::any& value) {
            const std::shared_ptr<IImage>& renderable =
                std::any_cast<const std::shared_ptr<IImage>&>(value);
            return renderable == normal ? std::string("atlas:normal") : std::string("atlas:pressed");
        };
        const std::string xml = saver.ToXml(image, typeid(Image));
        EXPECT_NE(xml.find("ResizeMode=\"KeepAspectRatio\""), std::string::npos);
        EXPECT_NE(xml.find("Renderable=\"atlas:normal\""), std::string::npos);
        EXPECT_NE(xml.find("PressedRenderable=\"atlas:pressed\""), std::string::npos);
        EXPECT_EQ(xml.find("DisabledRenderable="), std::string::npos);
        EXPECT_EQ(xml.find("OverRenderable="), std::string::npos);
        EXPECT_EQ(xml.find("FocusedRenderable="), std::string::npos);
    }
}

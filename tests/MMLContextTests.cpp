// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/LoadContext.hpp"
#include "Myra/MML/SaveContext.hpp"
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Myra/Graphics2D/UI/Enums.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlException.hpp"

namespace
{
    using Myra::Graphics2D::UI::HorizontalAlignment;
    using Myra::MML::LoadContext;
    using Myra::MML::PropertyDescriptor;
    using Myra::MML::PropertyMetadata;
    using Myra::MML::SaveContext;
    using Myra::MML::TypeDescriptor;
    using Myra::MML::TypeRegistry;
    using Myra::MML::ValueCodecRegistry;

    struct AssetReference
    {
        std::string Name;
        bool operator==(const AssetReference&) const = default;
    };

    struct ScalarModel
    {
        int Count = 0;
        std::optional<int> Limit;
        HorizontalAlignment Alignment = HorizontalAlignment::Left;
        std::string Label = "default";
        int LoadOnly = 0;
        int SaveOnly = 0;
        AssetReference Image;
    };

    struct NestedModel
    {
        virtual ~NestedModel() = default;

        std::string Id;
        int Number = 0;
    };

    struct NestedPrefix
    {
        virtual ~NestedPrefix() = default;
        int Prefix = 0;
    };

    struct DerivedNestedModel final : NestedPrefix, NestedModel
    {
        int Extra = 0;
    };

    struct ComplexModel
    {
        NestedModel Child;
    };

    struct UnsupportedComplexModel
    {
        NestedModel Child;
    };

    struct CollectionModel
    {
        std::shared_ptr<NestedModel> Selected;
        std::vector<std::shared_ptr<NestedModel>> Items;
        std::unordered_map<std::string, std::shared_ptr<NestedModel>> Resources;
        std::vector<std::shared_ptr<NestedModel>> Children;
    };

    struct AttachedOwner
    {
    };

    class AttachedModel final : public Myra::MML::BaseObject
    {
    };

    const Myra::MML::AttachedPropertyInfo<int>& GetPositionProperty()
    {
        static const Myra::MML::AttachedPropertyInfo<int>* property = [] {
            return &Myra::MML::AttachedPropertiesRegistry::Create(
                typeid(AttachedOwner), "Position", 0, Myra::MML::AttachedPropertyOption::AffectsArrange);
        }();
        return *property;
    }

    const Myra::MML::AttachedPropertyInfo<AssetReference>& GetAttachedImageProperty()
    {
        static const Myra::MML::AttachedPropertyInfo<AssetReference>* property = [] {
            PropertyMetadata metadata;
            metadata.ExternalAsset = true;
            return &Myra::MML::AttachedPropertiesRegistry::Create(
                typeid(AttachedOwner), "Image", AssetReference(),
                Myra::MML::AttachedPropertyOption::None, std::move(metadata));
        }();
        return *property;
    }

    template<typename T>
    PropertyDescriptor::Equality EqualValues()
    {
        return [](const std::any& left, const std::any& right) {
            return std::any_cast<const T&>(left) == std::any_cast<const T&>(right);
        };
    }

    PropertyDescriptor::NullCheck OptionalIntIsNull()
    {
        return [](const std::any& value) {
            return !std::any_cast<const std::optional<int>&>(value).has_value();
        };
    }

    std::shared_ptr<NestedModel> AsNested(const std::shared_ptr<void>& value)
    {
        return std::shared_ptr<NestedModel>(value, static_cast<NestedModel*>(value.get()));
    }

    std::vector<Myra::MML::RegisteredObjectView> EnumerateNested(
        const std::vector<std::shared_ptr<NestedModel>>& values)
    {
        std::vector<Myra::MML::RegisteredObjectView> result;
        result.reserve(values.size());
        for (const std::shared_ptr<NestedModel>& value : values)
        {
            if (value)
            {
                result.emplace_back(dynamic_cast<const void*>(value.get()), typeid(*value));
            }
        }
        return result;
    }

    TypeRegistry CreateRegistry()
    {
        static_cast<void>(GetPositionProperty());
        static_cast<void>(GetAttachedImageProperty());

        TypeDescriptor scalar("ScalarModel", typeid(ScalarModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<ScalarModel>());
        }, std::nullopt, std::string("Scalar"));
        scalar.AddProperty(PropertyDescriptor("Count", typeid(int),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->Count); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->Count = std::any_cast<int>(value);
            }, std::any(0), {}, EqualValues<int>()));
        scalar.AddProperty(PropertyDescriptor("Limit", typeid(std::optional<int>),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->Limit); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->Limit = std::any_cast<std::optional<int>>(value);
            }, std::any(std::optional<int>()), {}, EqualValues<std::optional<int>>(), OptionalIntIsNull()));
        scalar.AddProperty(PropertyDescriptor("Alignment", typeid(HorizontalAlignment),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->Alignment); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->Alignment = std::any_cast<HorizontalAlignment>(value);
            }, std::any(HorizontalAlignment::Left), {}, EqualValues<HorizontalAlignment>()));

        PropertyMetadata labelMetadata;
        labelMetadata.XmlName = "Title";
        scalar.AddProperty(PropertyDescriptor("Label", typeid(std::string),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->Label); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->Label = std::any_cast<std::string>(value);
            }, std::any(std::string("default")), std::move(labelMetadata), EqualValues<std::string>()));

        PropertyMetadata loadOnlyMetadata;
        loadOnlyMetadata.SkipSave = true;
        scalar.AddProperty(PropertyDescriptor("LoadOnly", typeid(int),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->LoadOnly); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->LoadOnly = std::any_cast<int>(value);
            }, std::any(0), std::move(loadOnlyMetadata), EqualValues<int>()));
        PropertyMetadata saveOnlyMetadata;
        saveOnlyMetadata.SkipLoad = true;
        scalar.AddProperty(PropertyDescriptor("SaveOnly", typeid(int),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->SaveOnly); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->SaveOnly = std::any_cast<int>(value);
            }, std::any(0), std::move(saveOnlyMetadata), EqualValues<int>()));

        PropertyMetadata assetMetadata;
        assetMetadata.ExternalAsset = true;
        scalar.AddProperty(PropertyDescriptor("Image", typeid(AssetReference),
            [](const void* object) { return std::any(static_cast<const ScalarModel*>(object)->Image); },
            [](void* object, const std::any& value) {
                static_cast<ScalarModel*>(object)->Image = std::any_cast<AssetReference>(value);
            }, std::any(AssetReference()), std::move(assetMetadata), EqualValues<AssetReference>()));

        TypeDescriptor nested("NestedModel", typeid(NestedModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<NestedModel>());
        }, std::nullopt, std::string("Nested"));
        nested.AddProperty(PropertyDescriptor("Id", typeid(std::string),
            [](const void* object) { return std::any(static_cast<const NestedModel*>(object)->Id); },
            [](void* object, const std::any& value) {
                static_cast<NestedModel*>(object)->Id = std::any_cast<std::string>(value);
            }, std::any(std::string()), {}, EqualValues<std::string>()));
        nested.AddProperty(PropertyDescriptor("Number", typeid(int),
            [](const void* object) { return std::any(static_cast<const NestedModel*>(object)->Number); },
            [](void* object, const std::any& value) {
                static_cast<NestedModel*>(object)->Number = std::any_cast<int>(value);
            }, std::any(0), {}, EqualValues<int>()));
        TypeDescriptor derivedNested("DerivedNestedModel", typeid(DerivedNestedModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<DerivedNestedModel>());
        }, typeid(NestedModel), std::string("DerivedNested"));
        derivedNested.EnableBaseTypeAccess<DerivedNestedModel, NestedModel>();
        derivedNested.AddProperty(PropertyDescriptor("Extra", typeid(int),
            [](const void* object) { return std::any(static_cast<const DerivedNestedModel*>(object)->Extra); },
            [](void* object, const std::any& value) {
                static_cast<DerivedNestedModel*>(object)->Extra = std::any_cast<int>(value);
            }, std::any(0), {}, EqualValues<int>()));
        TypeDescriptor complex("ComplexModel", typeid(ComplexModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<ComplexModel>());
        });
        complex.AddProperty(PropertyDescriptor("Child", typeid(NestedModel),
            [](const void* object) { return std::any(static_cast<const ComplexModel*>(object)->Child); }, {},
            std::nullopt, {}, {}, {}, Myra::MML::ComplexPropertyAdapter::SingleReadOnly(
                typeid(NestedModel),
                [](void* object) { return &static_cast<ComplexModel*>(object)->Child; },
                [](const void* object) {
                    return std::vector<Myra::MML::RegisteredObjectView>{
                        {&static_cast<const ComplexModel*>(object)->Child, typeid(NestedModel)}};
                })));

        TypeDescriptor unsupportedComplex("UnsupportedComplexModel", typeid(UnsupportedComplexModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<UnsupportedComplexModel>());
        });
        unsupportedComplex.AddProperty(PropertyDescriptor("Child", typeid(NestedModel),
            [](const void* object) {
                return std::any(static_cast<const UnsupportedComplexModel*>(object)->Child);
            }));

        TypeDescriptor collection("CollectionModel", typeid(CollectionModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<CollectionModel>());
        }, std::nullopt, std::string("Collection"));
        collection.AddProperty(PropertyDescriptor("Selected", typeid(std::shared_ptr<NestedModel>), {}, {},
            std::nullopt, {}, {}, {}, Myra::MML::ComplexPropertyAdapter::SingleWritable(
                typeid(NestedModel),
                [](void* object, const std::shared_ptr<void>& value) {
                    static_cast<CollectionModel*>(object)->Selected = AsNested(value);
                },
                [](const void* object) {
                    const std::shared_ptr<NestedModel>& value =
                        static_cast<const CollectionModel*>(object)->Selected;
                    return value
                        ? std::vector<Myra::MML::RegisteredObjectView>{{value.get(), typeid(NestedModel)}}
                        : std::vector<Myra::MML::RegisteredObjectView>();
                })));
        collection.AddProperty(PropertyDescriptor("Items", typeid(std::vector<std::shared_ptr<NestedModel>>),
            {}, {}, std::nullopt, {}, {}, {}, Myra::MML::ComplexPropertyAdapter::Sequence(
                typeid(NestedModel),
                [](void* object, const std::shared_ptr<void>& value) {
                    static_cast<CollectionModel*>(object)->Items.push_back(AsNested(value));
                },
                [](const void* object) {
                    return EnumerateNested(static_cast<const CollectionModel*>(object)->Items);
                })));
        collection.AddProperty(PropertyDescriptor("Resources",
            typeid(std::unordered_map<std::string, std::shared_ptr<NestedModel>>), {}, {}, std::nullopt, {}, {}, {},
            Myra::MML::ComplexPropertyAdapter::Dictionary(
                typeid(NestedModel),
                [](void* object, const std::string& key, const std::shared_ptr<void>& value) {
                    const bool inserted = static_cast<CollectionModel*>(object)->Resources.emplace(
                        key, AsNested(value)).second;
                    if (!inserted)
                    {
                        throw std::invalid_argument("duplicate test resource key");
                    }
                },
                [](const void* object) {
                    std::vector<Myra::MML::RegisteredObjectView> result;
                    for (const auto& [key, value] : static_cast<const CollectionModel*>(object)->Resources)
                    {
                        static_cast<void>(key);
                        if (value)
                        {
                            result.emplace_back(value.get(), typeid(NestedModel));
                        }
                    }
                    return result;
                })));
        PropertyMetadata childrenMetadata;
        childrenMetadata.Content = true;
        collection.AddProperty(PropertyDescriptor("Children", typeid(std::vector<std::shared_ptr<NestedModel>>),
            {}, {}, std::nullopt, std::move(childrenMetadata), {}, {},
            Myra::MML::ComplexPropertyAdapter::Sequence(
                typeid(NestedModel),
                [](void* object, const std::shared_ptr<void>& value) {
                    static_cast<CollectionModel*>(object)->Children.push_back(AsNested(value));
                },
                [](const void* object) {
                    return EnumerateNested(static_cast<const CollectionModel*>(object)->Children);
                })));

        TypeDescriptor attached("AttachedModel", typeid(AttachedModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<AttachedModel>());
        }, std::nullopt, std::string("Attached"));
        attached.EnableBaseObjectAccess<AttachedModel>();

        TypeRegistry registry;
        registry.Register(std::move(scalar));
        registry.Register(std::move(nested));
        registry.Register(std::move(derivedNested));
        registry.Register(std::move(complex));
        registry.Register(std::move(unsupportedComplex));
        registry.Register(std::move(collection));
        registry.Register(TypeDescriptor("AttachedOwner", typeid(AttachedOwner)));
        registry.Register(std::move(attached));
        return registry;
    }

    TEST(MMLContextTests, CreatesRegisteredObjectsAndLoadsScalarAttributes)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext context(types, codecs);
        context.LegacyClassNames.emplace("OldScalar", "Scalar");
        context.LegacyPropertyNames.emplace("OldTitle", "Title");

        System::Xml::XmlDocument document;
        document.LoadXml(
            "<OldScalar Count=\"7\" Limit=\"12\" Alignment=\"Stretch\" OldTitle=\"hello\" "
            "LoadOnly=\"3\" SaveOnly=\"99\" Unknown=\"ignored\" />");
        const Myra::MML::LoadedObject loaded = context.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(loaded.Type, typeid(ScalarModel));
        const auto* model = static_cast<const ScalarModel*>(loaded.Value.get());
        EXPECT_EQ(model->Count, 7);
        EXPECT_EQ(model->Limit, 12);
        EXPECT_EQ(model->Alignment, HorizontalAlignment::Stretch);
        EXPECT_EQ(model->Label, "hello");
        EXPECT_EQ(model->LoadOnly, 3);
        EXPECT_EQ(model->SaveOnly, 0);
        ASSERT_EQ(context.ObjectsNodes.size(), 1U);
        EXPECT_EQ(context.ObjectsNodes.front().Object, loaded.Value.get());
        EXPECT_EQ(context.ObjectsNodes.front().Type, typeid(ScalarModel));
        EXPECT_EQ(context.ObjectsNodes.front().RetainedValue, loaded.Value);
    }

    TEST(MMLContextTests, SavesDefaultsNullsSkipsAndXmlNamesWithoutLosingEmptyOverrides)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        const SaveContext context(types, codecs);
        ScalarModel model;
        model.Count = 7;
        model.Label.clear();
        model.LoadOnly = 5;
        model.SaveOnly = 9;

        const std::string xml = context.ToXml(&model, typeid(ScalarModel));
        System::Xml::XmlDocument document;
        document.LoadXml(xml);
        const System::Xml::XmlElement* root = document.getDocumentElementProperty();
        ASSERT_NE(root, nullptr);
        EXPECT_EQ(root->getNameProperty(), "Scalar");
        EXPECT_EQ(root->GetAttribute("Count"), "7");
        EXPECT_TRUE(root->HasAttribute("Title"));
        EXPECT_EQ(root->GetAttribute("Title"), "");
        EXPECT_EQ(root->GetAttribute("SaveOnly"), "9");
        EXPECT_FALSE(root->HasAttribute("Limit"));
        EXPECT_FALSE(root->HasAttribute("Alignment"));
        EXPECT_FALSE(root->HasAttribute("LoadOnly"));
        EXPECT_FALSE(root->HasAttribute("Image"));
    }

    TEST(MMLContextTests, UsesExplicitExternalAssetAdapters)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();

        System::Xml::XmlDocument document;
        document.LoadXml("<Scalar Image=\"icon-star\" />");
        LoadContext loader(types, codecs);
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(*document.getDocumentElementProperty())),
            std::logic_error);
        loader.LoadExternalAsset = [](const PropertyDescriptor& property, const std::string& name) {
            EXPECT_EQ(property.getNameProperty(), "Image");
            return std::any(AssetReference{name});
        };
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        EXPECT_EQ(static_cast<const ScalarModel*>(loaded.Value.get())->Image.Name, "icon-star");

        ScalarModel model;
        model.Image.Name = "icon-star";
        SaveContext saver(types, codecs);
        EXPECT_THROW(static_cast<void>(saver.ToXml(&model, typeid(ScalarModel))), std::logic_error);
        saver.SaveExternalAsset = [](const PropertyDescriptor& property, const std::any& value) {
            EXPECT_EQ(property.getNameProperty(), "Image");
            return std::any_cast<const AssetReference&>(value).Name;
        };
        const std::string saved = saver.ToXml(&model, typeid(ScalarModel));
        EXPECT_NE(saved.find("Image=\"icon-star\""), std::string::npos);

        System::Xml::XmlDocument attachedDocument;
        attachedDocument.LoadXml("<Attached AttachedOwner.Image=\"attached-star\" />");
        LoadContext attachedLoader(types, codecs);
        EXPECT_THROW(static_cast<void>(attachedLoader.CreateAndLoad(
            *attachedDocument.getDocumentElementProperty())), std::logic_error);
        attachedLoader.LoadAttachedExternalAsset = [](
            const Myra::MML::BaseAttachedPropertyInfo& property, const std::string& name) {
            EXPECT_EQ(property.getNameProperty(), "Image");
            return std::any(AssetReference{name});
        };
        const Myra::MML::LoadedObject attachedLoaded = attachedLoader.CreateAndLoad(
            *attachedDocument.getDocumentElementProperty());
        const auto* attachedModel = static_cast<const AttachedModel*>(attachedLoaded.Value.get());
        EXPECT_EQ(GetAttachedImageProperty().GetValue(*attachedModel).Name, "attached-star");

        SaveContext attachedSaver(types, codecs);
        EXPECT_THROW(static_cast<void>(attachedSaver.ToXml(
            attachedModel, typeid(AttachedModel), false, std::nullopt, typeid(AttachedOwner))),
            std::logic_error);
        attachedSaver.SaveAttachedExternalAsset = [](
            const Myra::MML::BaseAttachedPropertyInfo& property, const std::any& value) {
            EXPECT_EQ(property.getNameProperty(), "Image");
            return std::any_cast<const AssetReference&>(value).Name;
        };
        const std::string attachedSaved = attachedSaver.ToXml(
            attachedModel, typeid(AttachedModel), false, std::nullopt, typeid(AttachedOwner));
        EXPECT_NE(attachedSaved.find("AttachedOwner.Image=\"attached-star\""), std::string::npos);
    }

    TEST(MMLContextTests, RoundTripsAttachedPropertiesAndLoadsBaseObjectUserData)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(types, codecs);

        System::Xml::XmlDocument document;
        document.LoadXml("<Attached AttachedOwner.Position=\"4\" _note=\"hello\" />");
        const Myra::MML::LoadedObject loaded = loader.CreateAndLoad(*document.getDocumentElementProperty());
        const auto* model = static_cast<const AttachedModel*>(loaded.Value.get());
        EXPECT_EQ(GetPositionProperty().GetValue(*model), 4);
        ASSERT_TRUE(model->getUserDataProperty().contains("_note"));
        EXPECT_EQ(model->getUserDataProperty().at("_note"), "hello");

        const SaveContext saver(types, codecs);
        const std::string saved = saver.ToXml(
            model, typeid(AttachedModel), false, std::nullopt, typeid(AttachedOwner));
        System::Xml::XmlDocument savedDocument;
        savedDocument.LoadXml(saved);
        const System::Xml::XmlElement* root = savedDocument.getDocumentElementProperty();
        ASSERT_NE(root, nullptr);
        EXPECT_EQ(root->GetAttribute("AttachedOwner.Position"), "4");
        EXPECT_FALSE(root->HasAttribute("_note"));

        AttachedModel defaults;
        const std::string defaultXml = saver.ToXml(
            &defaults, typeid(AttachedModel), false, std::nullopt, typeid(AttachedOwner));
        System::Xml::XmlDocument defaultDocument;
        defaultDocument.LoadXml(defaultXml);
        ASSERT_NE(defaultDocument.getDocumentElementProperty(), nullptr);
        EXPECT_FALSE(defaultDocument.getDocumentElementProperty()->HasAttribute("AttachedOwner.Position"));

        EXPECT_EQ(Myra::MML::AttachedPropertiesRegistry::FindProperty(
            typeid(AttachedOwner), "Position", types), &GetPositionProperty());
    }

    TEST(MMLContextTests, RoundTripsSingleSequenceDictionaryAndImplicitContentProperties)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(types, codecs);
        SaveContext saver(types, codecs);

        System::Xml::XmlDocument singleDocument;
        singleDocument.LoadXml("<ComplexModel><ComplexModel.Child Id=\"child\" Number=\"8\" /></ComplexModel>");
        const Myra::MML::LoadedObject single =
            loader.CreateAndLoad(*singleDocument.getDocumentElementProperty());
        const auto* complex = static_cast<const ComplexModel*>(single.Value.get());
        EXPECT_EQ(complex->Child.Id, "child");
        EXPECT_EQ(complex->Child.Number, 8);
        const std::string singleXml = saver.ToXml(complex, typeid(ComplexModel));
        EXPECT_NE(singleXml.find("ComplexModel.Child"), std::string::npos);
        EXPECT_NE(singleXml.find("Number=\"8\""), std::string::npos);

        System::Xml::XmlDocument collectionDocument;
        collectionDocument.LoadXml(
            "<Collection>"
            "<CollectionModel.Selected Id=\"selected\" Number=\"1\" />"
            "<CollectionModel.Items><Nested Id=\"first\" Number=\"2\" />"
            "<DerivedNested Id=\"second\" Number=\"3\" Extra=\"9\" /></CollectionModel.Items>"
            "<CollectionModel.Resources><Nested Id=\"resource\" Number=\"4\" />"
            "</CollectionModel.Resources>"
            "<Nested Id=\"content\" Number=\"5\" />"
            "</Collection>");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*collectionDocument.getDocumentElementProperty());
        const auto* collection = static_cast<const CollectionModel*>(loaded.Value.get());
        ASSERT_NE(collection->Selected, nullptr);
        EXPECT_EQ(collection->Selected->Id, "selected");
        ASSERT_EQ(collection->Items.size(), 2U);
        EXPECT_EQ(collection->Items[0]->Number, 2);
        EXPECT_EQ(collection->Items[1]->Number, 3);
        const auto* derived = dynamic_cast<const DerivedNestedModel*>(collection->Items[1].get());
        ASSERT_NE(derived, nullptr);
        EXPECT_EQ(derived->Extra, 9);
        ASSERT_TRUE(collection->Resources.contains("resource"));
        EXPECT_EQ(collection->Resources.at("resource")->Number, 4);
        ASSERT_EQ(collection->Children.size(), 1U);
        EXPECT_EQ(collection->Children.front()->Id, "content");

        const std::string saved = saver.ToXml(collection, typeid(CollectionModel));
        EXPECT_NE(saved.find("CollectionModel.Selected"), std::string::npos);
        EXPECT_NE(saved.find("CollectionModel.Items"), std::string::npos);
        EXPECT_NE(saved.find("CollectionModel.Resources"), std::string::npos);
        EXPECT_NE(saved.find("DerivedNested"), std::string::npos);
        EXPECT_NE(saved.find("Extra=\"9\""), std::string::npos);
        System::Xml::XmlDocument roundTripDocument;
        roundTripDocument.LoadXml(saved);
        const Myra::MML::LoadedObject roundTrip =
            loader.CreateAndLoad(*roundTripDocument.getDocumentElementProperty());
        const auto* roundTripCollection = static_cast<const CollectionModel*>(roundTrip.Value.get());
        ASSERT_NE(roundTripCollection->Selected, nullptr);
        EXPECT_EQ(roundTripCollection->Selected->Number, 1);
        EXPECT_EQ(roundTripCollection->Items.size(), 2U);
        const auto* roundTripDerived = dynamic_cast<const DerivedNestedModel*>(
            roundTripCollection->Items[1].get());
        ASSERT_NE(roundTripDerived, nullptr);
        EXPECT_EQ(roundTripDerived->Number, 3);
        EXPECT_EQ(roundTripDerived->Extra, 9);
        ASSERT_TRUE(roundTripCollection->Resources.contains("resource"));
        EXPECT_EQ(roundTripCollection->Children.size(), 1U);

        SaveContext shortNameSaver(types, codecs);
        shortNameSaver.PrependNamespace = false;
        const std::string shortNames = shortNameSaver.ToXml(collection, typeid(CollectionModel));
        EXPECT_NE(shortNames.find("<Selected"), std::string::npos);
        EXPECT_NE(shortNames.find("<Items>"), std::string::npos);
        EXPECT_EQ(shortNames.find("CollectionModel.Items"), std::string::npos);

        SaveContext filteredSaver(types, codecs);
        filteredSaver.ShouldSerializeProperty = [](
            const void*, const std::type_index, const PropertyDescriptor& property) {
            return property.getNameProperty() != "Items";
        };
        const std::string filtered = filteredSaver.ToXml(collection, typeid(CollectionModel));
        EXPECT_EQ(filtered.find("CollectionModel.Items"), std::string::npos);
        EXPECT_NE(filtered.find("CollectionModel.Resources"), std::string::npos);
    }

    TEST(MMLContextTests, RejectsUnsupportedStructureAndMalformedRegisteredValuesExplicitly)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(types, codecs);

        System::Xml::XmlDocument malformedXml;
        EXPECT_THROW(malformedXml.LoadXml("<Scalar><Unclosed></Scalar>"), System::Xml::XmlException);

        System::Xml::XmlDocument malformed;
        malformed.LoadXml("<Scalar Count=\"bad\" />");
        try
        {
            static_cast<void>(loader.CreateAndLoad(*malformed.getDocumentElementProperty()));
            FAIL() << "A malformed registered scalar value must be rejected.";
        }
        catch (const std::invalid_argument& error)
        {
            EXPECT_NE(std::string(error.what()).find("Count"), std::string::npos);
            EXPECT_NE(std::string(error.what()).find("Scalar"), std::string::npos);
        }

        System::Xml::XmlDocument attached;
        attached.LoadXml("<Scalar Grid.Row=\"1\" />");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(*attached.getDocumentElementProperty())),
            std::logic_error);

        System::Xml::XmlDocument attachedWithoutAdapter;
        attachedWithoutAdapter.LoadXml("<Scalar AttachedOwner.Position=\"1\" />");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(*attachedWithoutAdapter.getDocumentElementProperty())),
            std::logic_error);

        System::Xml::XmlDocument malformedAttached;
        malformedAttached.LoadXml("<Attached AttachedOwner.Position.More=\"1\" />");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(*malformedAttached.getDocumentElementProperty())),
            std::invalid_argument);

        System::Xml::XmlDocument complex;
        complex.LoadXml("<Scalar><Child /></Scalar>");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(*complex.getDocumentElementProperty())),
            std::logic_error);
        loader.NodesToIgnore.emplace("Child");
        EXPECT_NO_THROW(static_cast<void>(loader.CreateAndLoad(*complex.getDocumentElementProperty())));

        System::Xml::XmlDocument unknown;
        unknown.LoadXml("<Missing />");
        try
        {
            static_cast<void>(loader.CreateAndLoad(*unknown.getDocumentElementProperty()));
            FAIL() << "An unknown MML root type must be rejected.";
        }
        catch (const std::out_of_range& error)
        {
            EXPECT_NE(std::string(error.what()).find("Missing"), std::string::npos);
        }

        System::Xml::XmlDocument unknownComplexProperty;
        unknownComplexProperty.LoadXml("<Collection><CollectionModel.Missing /></Collection>");
        try
        {
            static_cast<void>(loader.CreateAndLoad(
                *unknownComplexProperty.getDocumentElementProperty()));
            FAIL() << "An unknown complex MML property must be rejected.";
        }
        catch (const std::out_of_range& error)
        {
            EXPECT_NE(std::string(error.what()).find("CollectionModel.Missing"), std::string::npos);
        }

        System::Xml::XmlDocument incompatibleItem;
        incompatibleItem.LoadXml(
            "<Collection><CollectionModel.Items><Attached /></CollectionModel.Items></Collection>");
        try
        {
            static_cast<void>(loader.CreateAndLoad(*incompatibleItem.getDocumentElementProperty()));
            FAIL() << "An incompatible nested MML item must be rejected.";
        }
        catch (const std::invalid_argument& error)
        {
            EXPECT_NE(std::string(error.what()).find("Attached"), std::string::npos);
            EXPECT_NE(std::string(error.what()).find("NestedModel"), std::string::npos);
        }

        System::Xml::XmlDocument duplicateDictionaryKey;
        duplicateDictionaryKey.LoadXml(
            "<Collection><CollectionModel.Resources><Nested Id=\"same\" />"
            "<Nested Id=\"same\" /></CollectionModel.Resources></Collection>");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(
            *duplicateDictionaryKey.getDocumentElementProperty())), std::invalid_argument);

        UnsupportedComplexModel model;
        const SaveContext saver(types, codecs);
        EXPECT_THROW(static_cast<void>(saver.ToXml(&model, typeid(UnsupportedComplexModel))), std::logic_error);
        EXPECT_NO_THROW(static_cast<void>(saver.ToXml(&model, typeid(UnsupportedComplexModel), true)));

        System::Xml::XmlDocument noContent;
        noContent.LoadXml("<Scalar><Nested Number=\"1\" /></Scalar>");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(*noContent.getDocumentElementProperty())),
            std::logic_error);
        loader.DemandContentProperty = false;
        EXPECT_NO_THROW(static_cast<void>(loader.CreateAndLoad(*noContent.getDocumentElementProperty())));
    }

    TEST(MMLContextTests, RollsBackObjectNodeMappingsWhenNestedLoadingFails)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(types, codecs);

        System::Xml::XmlDocument failingDocument;
        failingDocument.LoadXml(
            "<Collection><CollectionModel.Items>"
            "<Nested Id=\"valid-before-failure\" Number=\"1\" />"
            "<Nested Id=\"invalid\" Number=\"bad\" />"
            "</CollectionModel.Items></Collection>");
        EXPECT_THROW(static_cast<void>(loader.CreateAndLoad(
            *failingDocument.getDocumentElementProperty())), std::invalid_argument);
        EXPECT_TRUE(loader.ObjectsNodes.empty());

        System::Xml::XmlDocument validDocument;
        validDocument.LoadXml("<Scalar Count=\"2\" />");
        const Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*validDocument.getDocumentElementProperty());
        ASSERT_EQ(loader.ObjectsNodes.size(), 1U);
        EXPECT_EQ(loader.ObjectsNodes.front().Object, loaded.Value.get());
        EXPECT_EQ(loader.ObjectsNodes.front().Type, typeid(ScalarModel));
        EXPECT_EQ(loader.ObjectsNodes.front().Node, validDocument.getDocumentElementProperty());
        EXPECT_EQ(loader.ObjectsNodes.front().RetainedValue, loaded.Value);
    }

    TEST(MMLContextTests, RetainsCreatedObjectsUntilTheirMappingsAreReleased)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        LoadContext loader(types, codecs);
        System::Xml::XmlDocument document;
        document.LoadXml("<Scalar Count=\"7\" />");

        Myra::MML::LoadedObject loaded =
            loader.CreateAndLoad(*document.getDocumentElementProperty());
        std::weak_ptr<void> objectObserver = loaded.Value;
        loaded.Value.reset();

        ASSERT_EQ(loader.ObjectsNodes.size(), 1U);
        EXPECT_FALSE(objectObserver.expired());
        EXPECT_NE(loader.ObjectsNodes.front().RetainedValue, nullptr);

        loader.ObjectsNodes.clear();
        EXPECT_TRUE(objectObserver.expired());

        ScalarModel external;
        loader.Load(&external, typeid(ScalarModel), *document.getDocumentElementProperty());
        ASSERT_EQ(loader.ObjectsNodes.size(), 1U);
        EXPECT_EQ(loader.ObjectsNodes.front().Object, &external);
        EXPECT_EQ(loader.ObjectsNodes.front().Type, typeid(ScalarModel));
        EXPECT_EQ(loader.ObjectsNodes.front().RetainedValue, nullptr);
    }

    TEST(MMLContextTests, OwnedDocumentResultOutlivesContextAndRetainsDetachedCreatedObjects)
    {
        const TypeRegistry types = CreateRegistry();
        const ValueCodecRegistry codecs = ValueCodecRegistry::CreateDefault();
        std::weak_ptr<System::Xml::XmlDocument> documentObserver;

        {
            Myra::MML::LoadedDocument loaded = [&] {
                LoadContext loader(types, codecs);
                loader.DemandContentProperty = false;
                Myra::MML::LoadedDocument result = loader.CreateAndLoadDocument(
                    "<Scalar Count=\"9\"><Nested Id=\"detached\" Number=\"3\" /></Scalar>");
                EXPECT_TRUE(loader.ObjectsNodes.empty());
                return result;
            }();

            documentObserver = loaded.Document;
            ASSERT_NE(loaded.Document, nullptr);
            EXPECT_EQ(loaded.Root.Type, typeid(ScalarModel));
            ASSERT_EQ(loaded.ObjectsNodes.size(), 2U);
            EXPECT_EQ(loaded.ObjectsNodes[0].Object, loaded.Root.Value.get());
            EXPECT_EQ(loaded.ObjectsNodes[0].Type, typeid(ScalarModel));
            EXPECT_EQ(loaded.ObjectsNodes[0].Node->getNameProperty(), "Scalar");
            EXPECT_EQ(loaded.ObjectsNodes[1].Type, typeid(NestedModel));
            EXPECT_EQ(loaded.ObjectsNodes[1].Node->getNameProperty(), "Nested");
            EXPECT_NE(loaded.ObjectsNodes[0].RetainedValue, nullptr);
            EXPECT_NE(loaded.ObjectsNodes[1].RetainedValue, nullptr);

            std::weak_ptr<void> detachedObserver = loaded.ObjectsNodes[1].RetainedValue;
            loaded.ObjectsNodes.erase(loaded.ObjectsNodes.begin() + 1);
            EXPECT_TRUE(detachedObserver.expired());
            EXPECT_FALSE(documentObserver.expired());

            std::weak_ptr<void> rootObserver = loaded.Root.Value;
            loaded.Root.Value.reset();
            EXPECT_FALSE(rootObserver.expired());
            loaded.ObjectsNodes.clear();
            EXPECT_TRUE(rootObserver.expired());
            EXPECT_FALSE(documentObserver.expired());
        }

        EXPECT_TRUE(documentObserver.expired());
    }
}

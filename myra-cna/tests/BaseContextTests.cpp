// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/BaseContext.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <utility>

namespace
{
    struct NestedValue
    {
        int Number = 0;
    };

    struct ExternalValue
    {
        std::string Name;
    };

    struct ContextModel
    {
        int Count = 0;
        std::optional<int> Limit;
        NestedValue Child;
        ExternalValue Image;
        int LoadOnly = 0;
        int SaveOnly = 0;
        int Ignored = 0;
        int Old = 0;
    };

    Myra::MML::PropertyDescriptor IntegerProperty(
        std::string name, int ContextModel::* member, Myra::MML::PropertyMetadata metadata = {})
    {
        return Myra::MML::PropertyDescriptor(std::move(name), typeid(int),
            [member](const void* instance) { return std::any(static_cast<const ContextModel*>(instance)->*member); },
            [member](void* instance, const std::any& value) {
                static_cast<ContextModel*>(instance)->*member = std::any_cast<int>(value);
            }, std::any(0), std::move(metadata));
    }

    Myra::MML::TypeRegistry CreateTypeRegistry()
    {
        Myra::MML::TypeDescriptor nested("NestedValue", typeid(NestedValue), [] {
            return std::static_pointer_cast<void>(std::make_shared<NestedValue>());
        });

        Myra::MML::TypeDescriptor model("ContextModel", typeid(ContextModel), [] {
            return std::static_pointer_cast<void>(std::make_shared<ContextModel>());
        });
        model.AddProperty(IntegerProperty("Count", &ContextModel::Count));
        model.AddProperty(Myra::MML::PropertyDescriptor("Limit", typeid(std::optional<int>),
            [](const void* instance) { return std::any(static_cast<const ContextModel*>(instance)->Limit); },
            [](void* instance, const std::any& value) {
                static_cast<ContextModel*>(instance)->Limit = std::any_cast<std::optional<int>>(value);
            }));
        model.AddProperty(Myra::MML::PropertyDescriptor("Child", typeid(NestedValue),
            [](const void* instance) { return std::any(static_cast<const ContextModel*>(instance)->Child); }));

        Myra::MML::PropertyMetadata external;
        external.ExternalAsset = true;
        model.AddProperty(Myra::MML::PropertyDescriptor("Image", typeid(ExternalValue),
            [](const void* instance) { return std::any(static_cast<const ContextModel*>(instance)->Image); },
            {}, std::nullopt, std::move(external)));

        Myra::MML::PropertyMetadata skipSave;
        skipSave.SkipSave = true;
        model.AddProperty(IntegerProperty("LoadOnly", &ContextModel::LoadOnly, std::move(skipSave)));
        Myra::MML::PropertyMetadata skipLoad;
        skipLoad.SkipLoad = true;
        model.AddProperty(IntegerProperty("SaveOnly", &ContextModel::SaveOnly, std::move(skipLoad)));
        Myra::MML::PropertyMetadata ignored;
        ignored.XmlIgnore = true;
        model.AddProperty(IntegerProperty("Ignored", &ContextModel::Ignored, std::move(ignored)));
        Myra::MML::PropertyMetadata obsolete;
        obsolete.Obsolete = true;
        model.AddProperty(IntegerProperty("Old", &ContextModel::Old, std::move(obsolete)));
        model.AddProperty(Myra::MML::PropertyDescriptor("WriteOnly", typeid(int), {},
            [](void*, const std::any&) {}));

        Myra::MML::TypeRegistry registry;
        registry.Register(std::move(nested));
        registry.Register(std::move(model));
        return registry;
    }

    TEST(BaseContextTests, ClassifiesRegisteredScalarComplexAndExternalProperties)
    {
        const Myra::MML::TypeRegistry types = CreateTypeRegistry();
        const Myra::MML::ValueCodecRegistry codecs = Myra::MML::ValueCodecRegistry::CreateDefault();
        const Myra::MML::BaseContext context(types, codecs);

        const Myra::MML::ParsedProperties properties = context.ParseProperties(typeid(ContextModel), false);
        ASSERT_EQ(properties.Simple.size(), 5U);
        EXPECT_EQ(properties.Simple[0]->getNameProperty(), "Count");
        EXPECT_EQ(properties.Simple[1]->getNameProperty(), "Limit");
        EXPECT_EQ(properties.Simple[2]->getNameProperty(), "Image");
        EXPECT_EQ(properties.Simple[3]->getNameProperty(), "LoadOnly");
        EXPECT_EQ(properties.Simple[4]->getNameProperty(), "Old");
        ASSERT_EQ(properties.Complex.size(), 1U);
        EXPECT_EQ(properties.Complex.front()->getNameProperty(), "Child");
        EXPECT_NE(context.FindSerializer(typeid(std::optional<int>)), nullptr);
        EXPECT_TRUE(Myra::MML::BaseContext::IsPropertyExternalAsset(*properties.Simple[2]));
        EXPECT_EQ(Myra::MML::BaseContext::IdName, "Id");
    }

    TEST(BaseContextTests, AppliesTheDistinctUpstreamLoadAndSaveFilters)
    {
        const Myra::MML::TypeRegistry types = CreateTypeRegistry();
        const Myra::MML::ValueCodecRegistry codecs = Myra::MML::ValueCodecRegistry::CreateDefault();
        const Myra::MML::BaseContext context(types, codecs);

        const Myra::MML::ParsedProperties load = context.ParseProperties(typeid(ContextModel), false);
        const Myra::MML::ParsedProperties save = context.ParseProperties(typeid(ContextModel), true);
        ASSERT_EQ(load.Simple.size(), 5U);
        ASSERT_EQ(save.Simple.size(), 4U);
        EXPECT_EQ(load.Simple[3]->getNameProperty(), "LoadOnly");
        EXPECT_EQ(load.Simple[4]->getNameProperty(), "Old");
        EXPECT_EQ(save.Simple[3]->getNameProperty(), "SaveOnly");
        EXPECT_THROW(static_cast<void>(context.ParseProperties(typeid(ExternalValue), false)), std::out_of_range);
    }
}

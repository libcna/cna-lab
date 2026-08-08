// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/TypeRegistry.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <vector>

namespace
{
    struct Animal
    {
        virtual ~Animal() = default;
        int Age = 0;
    };

    struct Dog final : Animal
    {
        std::string Name;
    };

    struct BaseSettings
    {
        virtual ~BaseSettings() = default;
        int Value = 1;
    };

    struct DerivedSettings final : BaseSettings
    {
    };

    Myra::MML::TypeRegistry CreateRegistry()
    {
        Myra::MML::TypeDescriptor animal("Animal", typeid(Animal), [] {
            return std::static_pointer_cast<void>(std::make_shared<Animal>());
        });
        animal.AddProperty(Myra::MML::PropertyDescriptor(
            "Age", typeid(int),
            [](const void* instance) { return std::any(static_cast<const Animal*>(instance)->Age); },
            [](void* instance, const std::any& value) {
                static_cast<Animal*>(instance)->Age = std::any_cast<int>(value);
            },
            std::any(0), {}, [](const std::any& left, const std::any& right) {
                return std::any_cast<int>(left) == std::any_cast<int>(right);
            }));

        Myra::MML::TypeDescriptor dog("Dog", typeid(Dog), [] {
            return std::static_pointer_cast<void>(std::make_shared<Dog>());
        }, typeid(Animal), std::string("Canine"));
        dog.EnableBaseTypeAccess<Dog, Animal>();
        Myra::MML::PropertyMetadata nameMetadata;
        nameMetadata.Content = true;
        nameMetadata.XmlName = "Label";
        dog.AddProperty(Myra::MML::PropertyDescriptor(
            "Name", typeid(std::string),
            [](const void* instance) { return std::any(static_cast<const Dog*>(instance)->Name); },
            [](void* instance, const std::any& value) {
                static_cast<Dog*>(instance)->Name = std::any_cast<std::string>(value);
            },
            std::any(std::string()), std::move(nameMetadata)));

        Myra::MML::TypeRegistry registry;
        registry.Register(std::move(animal));
        registry.Register(std::move(dog));
        return registry;
    }

    TEST(TypeRegistryTests, RegistersFactoriesNamesAndInheritedPropertiesWithoutReflection)
    {
        const Myra::MML::TypeRegistry registry = CreateRegistry();
        const auto* dog = registry.FindByType(typeid(Dog));
        ASSERT_NE(dog, nullptr);
        EXPECT_EQ(dog->getXmlNameProperty(), "Canine");
        EXPECT_EQ(registry.FindByXmlName("Canine"), dog);
        EXPECT_EQ(registry.FindByName("Dog"), dog);

        const auto hierarchy = registry.GetTypesIncludingBase(typeid(Dog));
        ASSERT_EQ(hierarchy.size(), 2U);
        EXPECT_EQ(hierarchy[0], typeid(Dog));
        EXPECT_EQ(hierarchy[1], typeid(Animal));

        const auto properties = registry.GetPropertiesIncludingBase(typeid(Dog));
        ASSERT_EQ(properties.size(), 2U);
        EXPECT_EQ(properties[0]->getNameProperty(), "Age");
        EXPECT_EQ(properties[1]->getNameProperty(), "Name");
        ASSERT_TRUE(properties[0]->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*properties[0]->getDeclaringTypeProperty(), typeid(Animal));
        ASSERT_TRUE(properties[1]->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*properties[1]->getDeclaringTypeProperty(), typeid(Dog));
        EXPECT_EQ(properties[1]->getXmlNameProperty(), "Label");
        EXPECT_TRUE(properties[1]->getMetadataProperty().Content);
        EXPECT_EQ(registry.FindPropertyByName(typeid(Dog), "Age"), properties[0]);
        EXPECT_EQ(registry.FindPropertyByName(typeid(Dog), "Name"), properties[1]);
        EXPECT_EQ(registry.FindPropertyByXmlName(typeid(Dog), "Label"), properties[1]);
        EXPECT_EQ(registry.FindPropertyByXmlName(typeid(Dog), "Missing"), nullptr);

        const std::shared_ptr<void> created = registry.Create("Canine");
        auto* createdDog = static_cast<Dog*>(created.get());
        EXPECT_EQ(registry.CastObject(createdDog, typeid(Dog), typeid(Animal)),
            static_cast<Animal*>(createdDog));
        EXPECT_EQ(registry.CastObject(static_cast<const void*>(createdDog), typeid(Dog), typeid(Animal)),
            static_cast<const Animal*>(createdDog));
        const std::shared_ptr<void> asAnimal = registry.CastObject(created, typeid(Dog), typeid(Animal));
        EXPECT_EQ(asAnimal.get(), static_cast<Animal*>(createdDog));
        EXPECT_TRUE(registry.IsTypeOrDerivedFrom(typeid(Dog), typeid(Animal)));
        EXPECT_FALSE(registry.IsTypeOrDerivedFrom(typeid(Animal), typeid(Dog)));
        registry.SetPropertyValue(createdDog, typeid(Dog), *properties[0], std::any(4));
        registry.SetPropertyValue(createdDog, typeid(Dog), *properties[1], std::any(std::string("Mira")));
        EXPECT_EQ(createdDog->Age, 4);
        EXPECT_EQ(createdDog->Name, "Mira");
        EXPECT_EQ(std::any_cast<int>(registry.GetPropertyValue(createdDog, typeid(Dog), *properties[0])), 4);
        EXPECT_FALSE(properties[0]->IsDefaultValue(
            registry.GetPropertyValue(createdDog, typeid(Dog), *properties[0])));
        createdDog->Age = 0;
        EXPECT_TRUE(properties[0]->IsDefaultValue(
            registry.GetPropertyValue(createdDog, typeid(Dog), *properties[0])));
        EXPECT_THROW(registry.SetPropertyValue(
            createdDog, typeid(Dog), *properties[0], std::any(std::string("wrong"))), std::invalid_argument);
    }

    TEST(TypeRegistryTests, RejectsInvalidRegistrationsAndPropertyValues)
    {
        Myra::MML::TypeRegistry registry;
        EXPECT_THROW(registry.Register(Myra::MML::TypeDescriptor("Dog", typeid(Dog), {}, typeid(Animal))),
            std::invalid_argument);

        Myra::MML::TypeDescriptor animal("Animal", typeid(Animal));
        animal.AddProperty(Myra::MML::PropertyDescriptor("Age", typeid(int)));
        EXPECT_THROW(animal.AddProperty(Myra::MML::PropertyDescriptor("Age", typeid(int))), std::invalid_argument);
        registry.Register(std::move(animal));
        EXPECT_THROW(registry.Register(Myra::MML::TypeDescriptor(
            "Dog", typeid(Dog), {}, typeid(Animal))), std::invalid_argument);
        EXPECT_THROW(registry.Register(Myra::MML::TypeDescriptor("Other", typeid(Animal))), std::invalid_argument);

        const auto* descriptor = registry.FindByType(typeid(Animal));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_THROW(descriptor->getPropertiesProperty().front().Set(nullptr, std::any(std::string("wrong"))),
            std::logic_error);
        EXPECT_THROW(static_cast<void>(registry.Create("Missing")), std::out_of_range);
        EXPECT_THROW(static_cast<void>(registry.FindPropertyByName(typeid(Dog), "Name")), std::out_of_range);
        EXPECT_THROW(static_cast<void>(registry.FindPropertyByXmlName(typeid(Dog), "Label")), std::out_of_range);

        EXPECT_THROW(static_cast<void>(Myra::MML::PropertyDescriptor(
            "BadDefault", typeid(int), {}, {}, std::any(std::string("wrong")))), std::invalid_argument);
        const Myra::MML::PropertyDescriptor badGetter("BadGetter", typeid(int),
            [](const void*) { return std::any(std::string("wrong")); });
        EXPECT_THROW(static_cast<void>(badGetter.Get(nullptr)), std::invalid_argument);
    }

    TEST(TypeRegistryTests, RejectsNullCallbackOwnersAndFactoryResultsBeforeAdaptersRun)
    {
        bool getterCalled = false;
        bool setterCalled = false;
        const Myra::MML::PropertyDescriptor property("Guarded", typeid(int),
            [&getterCalled](const void*) {
                getterCalled = true;
                return std::any(1);
            },
            [&setterCalled](void*, const std::any&) { setterCalled = true; });
        EXPECT_THROW(static_cast<void>(property.Get(nullptr)), std::invalid_argument);
        EXPECT_THROW(property.Set(nullptr, std::any(1)), std::invalid_argument);
        EXPECT_FALSE(getterCalled);
        EXPECT_FALSE(setterCalled);

        bool appenderCalled = false;
        bool enumeratorCalled = false;
        const Myra::MML::ComplexPropertyAdapter adapter = Myra::MML::ComplexPropertyAdapter::Sequence(
            typeid(Animal),
            [&appenderCalled](void*, const std::shared_ptr<void>&) { appenderCalled = true; },
            [&enumeratorCalled](const void*) {
                enumeratorCalled = true;
                return std::vector<Myra::MML::RegisteredObjectView>();
            });
        EXPECT_THROW(adapter.AppendObject(nullptr, {}), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(adapter.EnumerateObjects(nullptr)), std::invalid_argument);
        EXPECT_FALSE(appenderCalled);
        EXPECT_FALSE(enumeratorCalled);

        Myra::MML::TypeRegistry registry;
        registry.Register(Myra::MML::TypeDescriptor("NullFactory", typeid(Animal), [] {
            return std::shared_ptr<void>();
        }));
        EXPECT_THROW(static_cast<void>(registry.Create("NullFactory")), std::runtime_error);
    }

    TEST(TypeRegistryTests, SupportsExplicitNullableAndDefaultValuePolicies)
    {
        const Myra::MML::PropertyDescriptor property(
            "Limit", typeid(std::optional<int>), {}, {}, std::any(std::optional<int>()), {},
            [](const std::any& left, const std::any& right) {
                return std::any_cast<const std::optional<int>&>(left) ==
                    std::any_cast<const std::optional<int>&>(right);
            },
            [](const std::any& value) {
                return !std::any_cast<const std::optional<int>&>(value).has_value();
            });

        EXPECT_TRUE(property.getCanCompareDefaultProperty());
        EXPECT_TRUE(property.getCanBeNullProperty());
        EXPECT_TRUE(property.IsDefaultValue(std::any(std::optional<int>())));
        EXPECT_TRUE(property.IsNull(std::any(std::optional<int>())));
        EXPECT_FALSE(property.IsDefaultValue(std::any(std::optional<int>(4))));
        EXPECT_FALSE(property.IsNull(std::any(std::optional<int>(4))));
        EXPECT_THROW(static_cast<void>(property.IsDefaultValue(std::any(4))), std::invalid_argument);
        EXPECT_THROW(static_cast<void>(property.IsNull(std::any(4))), std::invalid_argument);
    }

    TEST(TypeRegistryTests, DerivedPropertyMetadataReplacesTheInheritedDescriptor)
    {
        Myra::MML::TypeDescriptor base("BaseSettings", typeid(BaseSettings));
        base.AddProperty(Myra::MML::PropertyDescriptor("Value", typeid(int),
            [](const void* object) { return std::any(static_cast<const BaseSettings*>(object)->Value); },
            [](void* object, const std::any& value) {
                static_cast<BaseSettings*>(object)->Value = std::any_cast<int>(value);
            }, std::any(1), {}, [](const std::any& left, const std::any& right) {
                return std::any_cast<int>(left) == std::any_cast<int>(right);
            }));
        Myra::MML::TypeDescriptor derived(
            "DerivedSettings", typeid(DerivedSettings), {}, typeid(BaseSettings));
        derived.EnableBaseTypeAccess<DerivedSettings, BaseSettings>();
        derived.AddProperty(Myra::MML::PropertyDescriptor("Value", typeid(int),
            [](const void* object) { return std::any(static_cast<const DerivedSettings*>(object)->Value); },
            [](void* object, const std::any& value) {
                static_cast<DerivedSettings*>(object)->Value = std::any_cast<int>(value);
            }, std::any(2), {}, [](const std::any& left, const std::any& right) {
                return std::any_cast<int>(left) == std::any_cast<int>(right);
            }));

        Myra::MML::TypeRegistry registry;
        registry.Register(std::move(base));
        registry.Register(std::move(derived));
        const auto properties = registry.GetPropertiesIncludingBase(typeid(DerivedSettings));
        ASSERT_EQ(properties.size(), 1U);
        ASSERT_TRUE(properties.front()->getDeclaringTypeProperty().has_value());
        EXPECT_EQ(*properties.front()->getDeclaringTypeProperty(), typeid(DerivedSettings));
        EXPECT_EQ(std::any_cast<int>(*properties.front()->getDefaultValueProperty()), 2);
    }

    TEST(TypeRegistryTests, RejectsAmbiguousCppAndXmlPropertyNameCollisions)
    {
        Myra::MML::PropertyMetadata emptyXmlName;
        emptyXmlName.XmlName = "";
        EXPECT_THROW(static_cast<void>(Myra::MML::PropertyDescriptor(
            "Value", typeid(int), {}, {}, std::nullopt, std::move(emptyXmlName))),
            std::invalid_argument);

        Myra::MML::PropertyMetadata aliasMetadata;
        aliasMetadata.XmlName = "Alias";
        Myra::MML::TypeDescriptor local("Local", typeid(Animal));
        local.AddProperty(Myra::MML::PropertyDescriptor(
            "Value", typeid(int), {}, {}, std::nullopt, aliasMetadata));
        EXPECT_THROW(local.AddProperty(Myra::MML::PropertyDescriptor("Alias", typeid(int))),
            std::invalid_argument);

        Myra::MML::TypeDescriptor base("BaseSettings", typeid(BaseSettings));
        base.AddProperty(Myra::MML::PropertyDescriptor(
            "Value", typeid(int), {}, {}, std::nullopt, std::move(aliasMetadata)));
        Myra::MML::TypeDescriptor derived(
            "DerivedSettings", typeid(DerivedSettings), {}, typeid(BaseSettings));
        derived.EnableBaseTypeAccess<DerivedSettings, BaseSettings>();
        Myra::MML::PropertyMetadata otherMetadata;
        otherMetadata.XmlName = "Other";
        derived.AddProperty(Myra::MML::PropertyDescriptor(
            "Alias", typeid(int), {}, {}, std::nullopt, std::move(otherMetadata)));

        Myra::MML::TypeRegistry registry;
        registry.Register(std::move(base));
        EXPECT_THROW(registry.Register(std::move(derived)), std::invalid_argument);

        Myra::MML::PropertyMetadata firstMetadata;
        firstMetadata.XmlName = "FirstXml";
        Myra::MML::PropertyMetadata secondMetadata;
        secondMetadata.XmlName = "SecondXml";
        Myra::MML::TypeDescriptor twoPropertyBase("TwoPropertyBase", typeid(BaseSettings));
        twoPropertyBase.AddProperty(Myra::MML::PropertyDescriptor(
            "First", typeid(int), {}, {}, std::nullopt, std::move(firstMetadata)));
        twoPropertyBase.AddProperty(Myra::MML::PropertyDescriptor(
            "Second", typeid(int), {}, {}, std::nullopt, std::move(secondMetadata)));
        Myra::MML::TypeDescriptor mergingDerived(
            "MergingDerived", typeid(DerivedSettings), {}, typeid(BaseSettings));
        mergingDerived.EnableBaseTypeAccess<DerivedSettings, BaseSettings>();
        Myra::MML::PropertyMetadata mergingMetadata;
        mergingMetadata.XmlName = "SecondXml";
        mergingDerived.AddProperty(Myra::MML::PropertyDescriptor(
            "First", typeid(int), {}, {}, std::nullopt, std::move(mergingMetadata)));

        Myra::MML::TypeRegistry mergingRegistry;
        mergingRegistry.Register(std::move(twoPropertyBase));
        EXPECT_THROW(mergingRegistry.Register(std::move(mergingDerived)), std::invalid_argument);
    }
}

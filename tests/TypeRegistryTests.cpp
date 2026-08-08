// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/TypeRegistry.hpp"

#include <gtest/gtest.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>

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
            std::any(0)));

        Myra::MML::TypeDescriptor dog("Dog", typeid(Dog), [] {
            return std::static_pointer_cast<void>(std::make_shared<Dog>());
        }, typeid(Animal), std::string("Canine"));
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

        const auto hierarchy = registry.GetTypesIncludingBase(typeid(Dog));
        ASSERT_EQ(hierarchy.size(), 2U);
        EXPECT_EQ(hierarchy[0], typeid(Dog));
        EXPECT_EQ(hierarchy[1], typeid(Animal));

        const auto properties = registry.GetPropertiesIncludingBase(typeid(Dog));
        ASSERT_EQ(properties.size(), 2U);
        EXPECT_EQ(properties[0]->getNameProperty(), "Age");
        EXPECT_EQ(properties[1]->getNameProperty(), "Name");
        EXPECT_EQ(properties[1]->getXmlNameProperty(), "Label");
        EXPECT_TRUE(properties[1]->getMetadataProperty().Content);

        const std::shared_ptr<void> created = registry.Create("Canine");
        auto* createdDog = static_cast<Dog*>(created.get());
        properties[0]->Set(createdDog, std::any(4));
        properties[1]->Set(createdDog, std::any(std::string("Mira")));
        EXPECT_EQ(createdDog->Age, 4);
        EXPECT_EQ(createdDog->Name, "Mira");
        EXPECT_EQ(std::any_cast<int>(properties[0]->Get(createdDog)), 4);
        EXPECT_THROW(properties[0]->Set(createdDog, std::any(std::string("wrong"))), std::invalid_argument);
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
        EXPECT_THROW(registry.Register(Myra::MML::TypeDescriptor("Other", typeid(Animal))), std::invalid_argument);

        const auto* descriptor = registry.FindByType(typeid(Animal));
        ASSERT_NE(descriptor, nullptr);
        EXPECT_THROW(descriptor->getPropertiesProperty().front().Set(nullptr, std::any(std::string("wrong"))),
            std::logic_error);
        EXPECT_THROW(static_cast<void>(registry.Create("Missing")), std::out_of_range);
    }
}

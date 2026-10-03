// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Robert Vokáč and Myra-CNA contributors.
// See NOTICE.md and THIRD_PARTY_NOTICES.md.
#include "Myra/MML/AttachedPropertiesRegistry.hpp"

#include <gtest/gtest.h>

#include <any>
#include <array>
#include <barrier>
#include <string>
#include <thread>
#include <typeindex>
#include <utility>
#include <vector>

namespace
{
    struct AttachedBaseOwner
    {
    };

    struct AttachedDerivedOwner : AttachedBaseOwner
    {
    };

    struct ConcurrentAttachedOwner
    {
    };

    class TrackingObject final : public Myra::MML::BaseObject
    {
    public:
        std::vector<std::string> Notifications;
        const Myra::MML::BaseAttachedPropertyInfo* LastProperty = nullptr;

        void OnAttachedPropertyChanged(const Myra::MML::BaseAttachedPropertyInfo& propertyInfo) override
        {
            Notifications.emplace_back("changed");
            LastProperty = &propertyInfo;
        }

        void OnAttachedPropertyLayoutChanged(const Myra::MML::AttachedPropertyOption option) override
        {
            Notifications.emplace_back(option == Myra::MML::AttachedPropertyOption::AffectsMeasure ? "measure" : "other");
        }
    };

    const Myra::MML::AttachedPropertyInfo<int>& GetValueProperty()
    {
        static const Myra::MML::AttachedPropertyInfo<int>* property = [] {
            Myra::MML::PropertyMetadata metadata;
            metadata.Range = Myra::Attributes::RangeAttribute(0);
            return &Myra::MML::AttachedPropertiesRegistry::Create(
                typeid(AttachedBaseOwner), "Value", 3, Myra::MML::AttachedPropertyOption::AffectsMeasure,
                std::move(metadata));
        }();
        return *property;
    }

    Myra::MML::TypeRegistry CreateOwnerRegistry()
    {
        Myra::MML::TypeRegistry registry;
        registry.Register(Myra::MML::TypeDescriptor("AttachedBaseOwner", typeid(AttachedBaseOwner)));
        Myra::MML::TypeDescriptor derived(
            "AttachedDerivedOwner", typeid(AttachedDerivedOwner), {}, typeid(AttachedBaseOwner));
        derived.EnableBaseTypeAccess<AttachedDerivedOwner, AttachedBaseOwner>();
        registry.Register(std::move(derived));
        return registry;
    }

    TEST(AttachedPropertiesRegistryTests, StoresDefaultsTypedValuesAndNotifiesInUpstreamOrder)
    {
        const Myra::MML::AttachedPropertyInfo<int>& property = GetValueProperty();
        TrackingObject object;

        EXPECT_FALSE(property.HasValue(object));
        EXPECT_EQ(property.GetValue(object), 3);
        EXPECT_EQ(property.getPropertyTypeProperty(), typeid(int));
        ASSERT_TRUE(property.getMetadataProperty().Range.has_value());
        EXPECT_EQ(property.getMetadataProperty().Range->getMinimumProperty(), 0.0F);

        property.SetValue(object, 8);
        EXPECT_TRUE(property.HasValue(object));
        EXPECT_EQ(property.GetValue(object), 8);
        ASSERT_EQ(object.Notifications.size(), 2U);
        EXPECT_EQ(object.Notifications[0], "measure");
        EXPECT_EQ(object.Notifications[1], "changed");
        EXPECT_EQ(object.LastProperty, &property);

        property.SetValue(object, 8);
        EXPECT_EQ(object.Notifications.size(), 2U);
        EXPECT_THROW(property.SetValueObject(object, std::any(std::string("wrong"))), std::invalid_argument);
    }

    TEST(AttachedPropertiesRegistryTests, EnumeratesPropertiesDeclaredByRegisteredBaseTypes)
    {
        const Myra::MML::AttachedPropertyInfo<int>& property = GetValueProperty();
        const Myra::MML::TypeRegistry registry = CreateOwnerRegistry();

        const std::vector<const Myra::MML::BaseAttachedPropertyInfo*> properties =
            Myra::MML::AttachedPropertiesRegistry::GetPropertiesOfType(typeid(AttachedDerivedOwner), registry);
        ASSERT_EQ(properties.size(), 1U);
        EXPECT_EQ(properties.front(), &property);
        EXPECT_EQ(properties.front()->getOwnerTypeProperty(), typeid(AttachedBaseOwner));
        EXPECT_EQ(properties.front()->getDefaultValueObjectProperty().type(), typeid(int));
    }

    TEST(AttachedPropertiesRegistryTests, RejectsDuplicateDeclarationsAndSupportsConcurrentRegistration)
    {
        static_cast<void>(GetValueProperty());
        EXPECT_THROW(static_cast<void>(Myra::MML::AttachedPropertiesRegistry::Create(
            typeid(AttachedBaseOwner), "Value", 0, Myra::MML::AttachedPropertyOption::None)),
            std::invalid_argument);

        std::array<const Myra::MML::AttachedPropertyInfo<int>*, 2> registered{};
        std::barrier start(2);
        std::array<std::thread, 2> threads;
        for (size_t index = 0; index < threads.size(); ++index)
        {
            threads[index] = std::thread([index, &registered, &start] {
                start.arrive_and_wait();
                registered[index] = &Myra::MML::AttachedPropertiesRegistry::Create(
                    typeid(ConcurrentAttachedOwner), "Concurrent" + std::to_string(index),
                    static_cast<int>(index), Myra::MML::AttachedPropertyOption::None);
            });
        }
        for (std::thread& thread : threads)
        {
            thread.join();
        }

        ASSERT_NE(registered[0], nullptr);
        ASSERT_NE(registered[1], nullptr);
        EXPECT_NE(registered[0]->getIdProperty(), registered[1]->getIdProperty());

        Myra::MML::TypeRegistry registry;
        registry.Register(Myra::MML::TypeDescriptor(
            "ConcurrentAttachedOwner", typeid(ConcurrentAttachedOwner)));
        const std::vector<const Myra::MML::BaseAttachedPropertyInfo*> properties =
            Myra::MML::AttachedPropertiesRegistry::GetPropertiesOfType(
                typeid(ConcurrentAttachedOwner), registry);
        ASSERT_EQ(properties.size(), 2U);
        EXPECT_EQ(Myra::MML::AttachedPropertiesRegistry::FindProperty(
            typeid(ConcurrentAttachedOwner), "Concurrent0", registry), registered[0]);
        EXPECT_EQ(Myra::MML::AttachedPropertiesRegistry::FindProperty(
            typeid(ConcurrentAttachedOwner), "Concurrent1", registry), registered[1]);
    }
}

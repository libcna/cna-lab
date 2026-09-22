// SPDX-License-Identifier: MS-PL
#include "CNA/Studio/Core/ComponentDescriptor.hpp"

#include "CNA/Studio/Core/Json.hpp"

#include <algorithm>

namespace CNA::Studio
{
    const PropertyDescriptor* ComponentDescriptor::findProperty(std::string_view name) const
    {
        const auto found = std::find_if(properties.begin(), properties.end(),
                                        [&](const PropertyDescriptor& property) { return property.name == name; });
        return found == properties.end() ? nullptr : &*found;
    }

    bool ComponentRegistry::registerComponent(ComponentDescriptor descriptor)
    {
        if (descriptor.typeId.empty()) { return false; }
        const std::string key = descriptor.typeId;
        descriptors_[key] = std::move(descriptor);
        return true;
    }

    const ComponentDescriptor* ComponentRegistry::find(std::string_view typeId) const
    {
        const auto found = descriptors_.find(std::string{typeId});
        return found == descriptors_.end() ? nullptr : &found->second;
    }

    std::vector<std::string> ComponentRegistry::getTypeIds() const
    {
        std::vector<std::string> ids;
        ids.reserve(descriptors_.size());
        for (const auto& [typeId, descriptor] : descriptors_) { ids.push_back(typeId); }
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    bool ComponentRegistry::unregisterComponent(std::string_view typeId)
    {
        return descriptors_.erase(std::string{typeId}) > 0;
    }
}

namespace CNA::Studio
{
    PropertyValue propertyValueFromJson(const JsonValue& json, const PropertyDescriptor& descriptor,
                                        std::vector<std::string>* changes, std::string_view path);

    namespace
    {
        /** @brief What a JSON value is, in words a message can use. */
        const char* describeJson(const JsonValue& json)
        {
            switch (json.getType())
            {
                case JsonType::Null: return "nothing";
                case JsonType::Boolean: return "true or false";
                case JsonType::Number: return "a number";
                case JsonType::String: return "text";
                case JsonType::Array: return "a list";
                case JsonType::Object: return "a group of fields";
            }
            return "something else";
        }

        /** @brief How many numbers a fixed-width type needs, or zero when it is not one. */
        std::size_t fixedElementCount(PropertyType type)
        {
            switch (type)
            {
                case PropertyType::Vector2: return 2;
                case PropertyType::Vector3: return 3;
                case PropertyType::Vector4:
                case PropertyType::Quaternion:
                case PropertyType::Color:
                case PropertyType::Rectangle: return 4;
                default: return 0;
            }
        }

        /** @brief Appends "this was replaced" to @p changes, when anyone is listening. */
        void noteReplacement(std::vector<std::string>* changes, std::string_view path,
                             const PropertyDescriptor& descriptor, const JsonValue& json)
        {
            if (changes == nullptr) { return; }

            std::string where = path.empty() ? std::string{descriptor.name} : std::string{path};
            changes->push_back(where + " holds " + describeJson(json) + " where "
                               + toString(descriptor.type)
                               + " was expected; the default was used, and saving will write it");
        }

        /** @brief Reads one structure from @p json against @p fields. */
        PropertyValue::StructureValue readStructure(const JsonValue& json,
                                                    const std::vector<PropertyDescriptor>& fields,
                                                    std::vector<std::string>* changes = nullptr,
                                                    std::string_view path = {})
        {
            PropertyValue::StructureValue structure;

            // Driven by the *schema*, not by the JSON. Walking the JSON instead would carry
            // through whatever a file happened to contain, including fields no descriptor
            // declares -- and the next save would write out a shape nothing can read back.
            for (const PropertyDescriptor& field : fields)
            {
                const JsonValue& fieldJson = json[std::string_view{field.name}];

                // Absent is not the same as present-and-empty. A field the document never carried
                // takes the descriptor's declared default -- which is what lets a structure gain a
                // field later without every document already written becoming one with a hole in
                // it. A field that *is* there is read even when its value is empty, because an
                // empty string somebody typed is a choice they made.
                if (fieldJson.isNull())
                {
                    structure.set(field.name, field.defaultValue);
                    continue;
                }

                const std::string fieldPath =
                    path.empty() ? field.name : std::string{path} + "." + field.name;
                structure.set(field.name,
                              propertyValueFromJson(fieldJson, field, changes, fieldPath));
            }

            return structure;
        }
    }

    bool studioJsonMatchesPropertyType(const JsonValue& json, const PropertyDescriptor& descriptor)
    {
        // Absent is not a mismatch. It means "use the default", which is the contract that lets a
        // component gain a property without every document already written becoming one with a
        // hole in it.
        if (json.isNull()) { return true; }

        switch (descriptor.type)
        {
            case PropertyType::None: return true;
            case PropertyType::Boolean: return json.getType() == JsonType::Boolean;
            case PropertyType::Integer:
            case PropertyType::Float: return json.getType() == JsonType::Number;

            // A reference is a string; whether it names anything is a *different* question, and
            // scene validation answers it with a better message than this could.
            case PropertyType::String:
            case PropertyType::Enum:
            case PropertyType::AssetReference:
            case PropertyType::EntityReference: return json.getType() == JsonType::String;

            case PropertyType::Structure: return json.isObject();
            case PropertyType::List: return json.isArray();

            default: break;
        }

        // The fixed-width numeric types. Too *few* elements counts, because the reader fills the
        // rest from a default and the document said something shorter -- which is the loss this
        // exists to notice. Too many does not: the extra is ignored and nothing the type can hold
        // was dropped.
        const std::size_t needed = fixedElementCount(descriptor.type);
        if (needed == 0) { return true; }
        return json.isArray() && json.getElements().size() >= needed;
    }

    PropertyValue propertyValueFromJson(const JsonValue& json, const PropertyDescriptor& descriptor,
                                        std::vector<std::string>* changes, std::string_view path)
    {
        // Said once, here, rather than at each of the three readings below: every one of them
        // falls back, and the fallback is the change `STUDIO-31011` forbids doing quietly.
        if (!studioJsonMatchesPropertyType(json, descriptor))
        {
            noteReplacement(changes, path, descriptor, json);
        }

        if (descriptor.type == PropertyType::Structure)
        {
            // An absent or non-object value reads as the declared defaults rather than as an
            // empty structure: a field missing from a document is a field the document was written
            // before, and defaulting it is the only reading that keeps old files opening.
            if (!json.isObject())
            {
                return PropertyValue{
                    readStructure(JsonValue{}, descriptor.structureFields, nullptr, path)};
            }
            return PropertyValue{readStructure(json, descriptor.structureFields, changes, path)};
        }

        if (descriptor.type == PropertyType::List
            && descriptor.elementType == PropertyType::Structure)
        {
            PropertyValue::ListValue list;
            std::size_t index = 0;
            for (const JsonValue& element : json.getElements())
            {
                const std::string elementPath = (path.empty() ? std::string{descriptor.name}
                                                              : std::string{path})
                                              + "[" + std::to_string(index) + "]";
                list.items.push_back(PropertyValue{
                    readStructure(element, descriptor.structureFields, changes, elementPath)});
                ++index;
            }
            return PropertyValue{std::move(list)};
        }

        return PropertyValue::fromJson(json, descriptor.type, descriptor.elementType);
    }

    std::string studioPropertyConditionText(const PropertyValue& value)
    {
        switch (value.getType())
        {
            case PropertyType::Enum:
                return value.get<PropertyValue::EnumValue>().name;

            // "true" and "false" as the language spells them, not "1" and "0": a condition is
            // read by a person writing a descriptor, and `{"castShadows", {"true"}}` is the only
            // spelling that does not need looking up.
            case PropertyType::Boolean:
                return value.get<bool>() ? "true" : "false";

            default:
                return {};
        }
    }

    bool studioPropertyConditionMet(const PropertyDescriptor& descriptor,
                                    const PropertyValue* sibling)
    {
        if (descriptor.appliesWhen.isAlways()) { return true; }

        // Fails open, for the reason the header gives: a condition naming a property that is not
        // there is a descriptor's mistake, and greying a field a user needs with no explanation is
        // the more expensive way to be wrong.
        if (sibling == nullptr) { return true; }

        const std::string actual = studioPropertyConditionText(*sibling);
        if (actual.empty()) { return true; }

        for (const std::string& allowed : descriptor.appliesWhen.values)
        {
            if (allowed == actual) { return true; }
        }
        return false;
    }
}

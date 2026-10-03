// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for the five ContentSerializer attribute types,
// transcribed from the CIL of the hash-registered
// `Microsoft.Xna.Framework.dll` (SHA-256 38e7093f…a130). The one BCL fact they
// inherit -- `IsDefaultAttribute` -- comes from the admitted `mscorlib` and is
// asserted alongside, because it is what the base identity buys.
extension PureValueTests {

    // ------------------------------------------------------------------
    // The base identity, which is most of what the base is for.
    // ------------------------------------------------------------------

    func testEveryContentSerializerAttributeIsAnAttribute() throws {
        let attributes: [CNAAttribute] = [
            Microsoft.Xna.Framework.Content.ContentSerializerAttribute(),
            try Microsoft.Xna.Framework.Content
                .ContentSerializerCollectionItemNameAttribute(
                    collectionItemName: "Entry"),
            Microsoft.Xna.Framework.Content.ContentSerializerIgnoreAttribute(),
            try Microsoft.Xna.Framework.Content
                .ContentSerializerRuntimeTypeAttribute(runtimeType: "System.Int32"),
            Microsoft.Xna.Framework.Content
                .ContentSerializerTypeVersionAttribute(typeVersion: 2),
        ]
        XCTAssertEqual(attributes.count, 5)
        // `Attribute.IsDefaultAttribute()` is `return false`, and none of the
        // five overrides it.
        for attribute in attributes {
            XCTAssertFalse(attribute.IsDefaultAttribute())
        }
        // They are attributes, and they are not each other.
        let first: Any = attributes[0]
        XCTAssertFalse(
            first is Microsoft.Xna.Framework.Content
                .ContentSerializerIgnoreAttribute)
        // ... and an attribute is not an exception, which is the other support
        // hierarchy this binding carries.
        XCTAssertFalse(first is CNAException)
    }

    // ------------------------------------------------------------------
    // ContentSerializerAttribute — the defaults.
    // ------------------------------------------------------------------

    // `.ctor()` sets `allowNull = true` before calling the base and leaves
    // every other field at its zero value. A reimplementation that made the
    // four Booleans uniform would get exactly one of them wrong.
    func testContentSerializerAttributeDefaults() {
        let attribute =
            Microsoft.Xna.Framework.Content.ContentSerializerAttribute()
        XCTAssertNil(attribute.ElementName)
        XCTAssertFalse(attribute.FlattenContent)
        XCTAssertFalse(attribute.Optional)
        XCTAssertTrue(attribute.AllowNull, "AllowNull is the one that starts true")
        XCTAssertFalse(attribute.SharedResource)
        XCTAssertEqual(attribute.CollectionItemName, "Item")
        XCTAssertFalse(attribute.HasCollectionItemName)
    }

    func testContentSerializerAttributeSettersRoundTrip() {
        let attribute =
            Microsoft.Xna.Framework.Content.ContentSerializerAttribute()
        attribute.ElementName = "Node"
        attribute.FlattenContent = true
        attribute.Optional = true
        attribute.AllowNull = false
        attribute.SharedResource = true
        XCTAssertEqual(attribute.ElementName, "Node")
        XCTAssertTrue(attribute.FlattenContent)
        XCTAssertTrue(attribute.Optional)
        XCTAssertFalse(attribute.AllowNull)
        XCTAssertTrue(attribute.SharedResource)
        // `ElementName` is the one string with no substituted default, so it
        // can be put back to nil.
        attribute.ElementName = nil
        XCTAssertNil(attribute.ElementName)
    }

    // ------------------------------------------------------------------
    // The CollectionItemName trio, which is the whole interesting behaviour.
    // ------------------------------------------------------------------

    // The getter substitutes `"Item"` for a null-or-empty field; the setter
    // REFUSES a null-or-empty value; and `HasCollectionItemName` reports the
    // raw field, not what the getter answers. All three differ, and a
    // projection that folded them together would be wrong three ways.
    func testCollectionItemNameSubstitutesRefusesAndReportsSeparately() throws {
        let attribute =
            Microsoft.Xna.Framework.Content.ContentSerializerAttribute()
        XCTAssertEqual(attribute.CollectionItemName, "Item")
        XCTAssertFalse(attribute.HasCollectionItemName)

        assertProjected(
            CNAArgumentNullException.self,
            message: composedArgumentMessage("Value cannot be null.", paramName: "value"),
            paramName: "value",
            hResult: Int32(bitPattern: 0x8000_4003)
        ) {
            try attribute.SetCollectionItemName("")
        }
        XCTAssertFalse(
            attribute.HasCollectionItemName,
            "a refused set must leave the raw field alone")

        try attribute.SetCollectionItemName("Entry")
        XCTAssertEqual(attribute.CollectionItemName, "Entry")
        XCTAssertTrue(attribute.HasCollectionItemName)

        // Setting the substituted default explicitly is a real store, so the
        // flag flips even though the reported name does not change.
        let explicitDefault =
            Microsoft.Xna.Framework.Content.ContentSerializerAttribute()
        try explicitDefault.SetCollectionItemName("Item")
        XCTAssertEqual(explicitDefault.CollectionItemName, "Item")
        XCTAssertTrue(explicitDefault.HasCollectionItemName)
    }

    // `Clone` copies the six FIELDS, not the properties. Cloning through the
    // properties would have turned the substituted `"Item"` into a stored
    // value and flipped `HasCollectionItemName` on the copy.
    func testCloneCopiesTheFieldsAndNotTheSubstitutedDefaults() throws {
        let source =
            Microsoft.Xna.Framework.Content.ContentSerializerAttribute()
        source.ElementName = "Node"
        source.FlattenContent = true
        source.AllowNull = false

        let clone = source.Clone()
        XCTAssertFalse(clone === source, "Clone must be a fresh instance")
        XCTAssertEqual(clone.ElementName, "Node")
        XCTAssertTrue(clone.FlattenContent)
        XCTAssertFalse(clone.AllowNull)
        XCTAssertEqual(clone.CollectionItemName, "Item")
        XCTAssertFalse(
            clone.HasCollectionItemName,
            "the raw null field is carried on, not the substituted default")

        // The clone is independent.
        clone.ElementName = "Other"
        XCTAssertEqual(source.ElementName, "Node")

        // A set collection item name does clone across.
        try source.SetCollectionItemName("Entry")
        let second = source.Clone()
        XCTAssertEqual(second.CollectionItemName, "Entry")
        XCTAssertTrue(second.HasCollectionItemName)
    }

    // ------------------------------------------------------------------
    // The three constructor-validating attributes.
    // ------------------------------------------------------------------

    func testCollectionItemNameAttributeRefusesAnEmptyName() throws {
        let attribute = try Microsoft.Xna.Framework.Content
            .ContentSerializerCollectionItemNameAttribute(
                collectionItemName: "Entry")
        XCTAssertEqual(attribute.CollectionItemName, "Entry")

        assertProjected(
            CNAArgumentNullException.self,
            message: composedArgumentMessage(
                "Value cannot be null.", paramName: "collectionItemName"),
            paramName: "collectionItemName",
            hResult: Int32(bitPattern: 0x8000_4003)
        ) {
            _ = try Microsoft.Xna.Framework.Content
                .ContentSerializerCollectionItemNameAttribute(
                    collectionItemName: "")
        }
    }

    func testRuntimeTypeAttributeRefusesAnEmptyTypeName() throws {
        let attribute = try Microsoft.Xna.Framework.Content
            .ContentSerializerRuntimeTypeAttribute(
                runtimeType: "System.Collections.Generic.List`1")
        XCTAssertEqual(
            attribute.RuntimeType, "System.Collections.Generic.List`1",
            "the name is carried verbatim; nothing here resolves it")

        assertProjected(
            CNAArgumentNullException.self,
            message: composedArgumentMessage(
                "Value cannot be null.", paramName: "runtimeType"),
            paramName: "runtimeType",
            hResult: Int32(bitPattern: 0x8000_4003)
        ) {
            _ = try Microsoft.Xna.Framework.Content
                .ContentSerializerRuntimeTypeAttribute(runtimeType: "")
        }
    }

    // The one constructor of the five that validates nothing: the IL is
    // `base..ctor()` then a plain store, so a negative or zero version is
    // accepted exactly as XNA accepts it.
    func testTypeVersionAttributeValidatesNothing() {
        for version: Int32 in [-1, 0, 7, Int32.max] {
            let attribute = Microsoft.Xna.Framework.Content
                .ContentSerializerTypeVersionAttribute(typeVersion: version)
            XCTAssertEqual(attribute.TypeVersion, version)
        }
    }

    // ------------------------------------------------------------------
    // The support base itself.
    // ------------------------------------------------------------------

    // The recorded widening: `mscorlib` declares `System.Attribute` abstract
    // with a protected constructor, and Swift has neither, so this compiles
    // where `new Attribute()` does not. Asserting it here keeps the divergence
    // a stated fact rather than a discovery.
    func testCNAAttributeIsConstructibleWhichTheCLRForbids() {
        let bare = CNAAttribute()
        XCTAssertFalse(bare.IsDefaultAttribute())

        // It is `open`, so a consumer can define their own attribute.
        final class ConsumerAttribute: CNAAttribute {
            override func IsDefaultAttribute() -> Bool { true }
        }
        XCTAssertTrue(ConsumerAttribute().IsDefaultAttribute())
        let consumer: Any = ConsumerAttribute()
        XCTAssertTrue(consumer is CNAAttribute)
    }
}

// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

// Pure XNA-derived behaviour for `GameServiceContainer`, transcribed from the
// CIL of the hash-registered `Microsoft.Xna.Framework.Game.dll`
// (SHA-256 b5dffdd8…a1f0). The container needs no host, so this is counted in
// the pure managed corpus.

private protocol ProbeService {}
private class ProbeBase {}
private final class ProbeDerived: ProbeBase, ProbeService {}
private final class ProbeUnrelated {}

extension PureValueTests {

    private func makeContainer() -> Microsoft.Xna.Framework.GameServiceContainer {
        Microsoft.Xna.Framework.GameServiceContainer()
    }

    // `.ctor()` allocates the dictionary and nothing else, and `GetService`
    // returns NULL for an absent type rather than raising -- which is why the
    // projection is Optional.
    func testAnEmptyContainerReturnsNilRatherThanFailing() {
        let container = makeContainer()
        XCTAssertNil(container.GetService(ProbeService.self))
        XCTAssertNil(container.GetService(ProbeBase.self))
    }

    func testAddedServicesComeBackByTheirRegisteredType() throws {
        let container = makeContainer()
        let service = ProbeDerived()
        try container.AddService(ProbeService.self, provider: service)

        XCTAssertTrue(container.GetService(ProbeService.self) as AnyObject === service)
        // ... and only by the type it was registered under: the dictionary is
        // keyed by the type token, not by what the provider happens to be.
        XCTAssertNil(container.GetService(ProbeDerived.self))
        XCTAssertNil(container.GetService(ProbeBase.self))
    }

    // `services.ContainsKey(type)` guards the insert, with the exact
    // `ServiceAlreadyPresent` string.
    func testAddingTheSameTypeTwiceIsRefused() throws {
        let container = makeContainer()
        try container.AddService(ProbeService.self, provider: ProbeDerived())
        // `ArgumentException(message, paramName)`: the resource string is the
        // message and "type" is the parameter name, so Message composes both.
        assertProjected(
            CNAArgumentException.self,
            message: composedArgumentMessage(
                "Container already contains a service of this type.",
                paramName: "type"),
            paramName: "type",
            hResult: Int32(bitPattern: 0x8007_0057)
        ) {
            try container.AddService(ProbeService.self, provider: ProbeDerived())
        }
    }

    // `System.Object` projects to `Any?`, so the null-provider branch is
    // reachable and is reproduced. The null-TYPE branch is not: a Swift
    // metatype cannot be nil.
    func testANullProviderIsRefused() {
        let container = makeContainer()
        // `ArgumentNullException(paramName, message)`: transposed with
        // respect to ArgumentException's, and getting the order wrong would
        // swap the two halves of this Message.
        assertProjected(
            CNAArgumentNullException.self,
            message: composedArgumentMessage(
                "The service provider instance cannot be null.",
                paramName: "provider"),
            paramName: "provider",
            hResult: Int32(bitPattern: 0x8000_4003)
        ) {
            try container.AddService(ProbeService.self, provider: nil)
        }
    }

    // The interesting check: `type.IsAssignableFrom(provider.GetType())`.
    // Registering an implementation under an interface type is what every real
    // use of this container does, and identity alone would have refused it.
    func testAssignabilityAcceptsProtocolsAndBaseClassesAndRefusesTheRest() throws {
        let container = makeContainer()
        let derived = ProbeDerived()

        // A protocol the provider conforms to.
        XCTAssertNoThrow(try container.AddService(ProbeService.self, provider: derived))
        // A base class the provider inherits.
        XCTAssertNoThrow(try container.AddService(ProbeBase.self, provider: derived))
        // Its own type.
        XCTAssertNoThrow(try container.AddService(ProbeDerived.self, provider: derived))

        // ... and a type it is not.
        let refused = makeContainer()
        XCTAssertThrowsError(
            try refused.AddService(ProbeDerived.self, provider: ProbeBase())
        ) {
            guard let failure = $0 as? CNAArgumentException else {
                return XCTFail("wrong error: \($0)")
            }
            XCTAssertTrue(Swift.type(of: failure) == CNAArgumentException.self)
            // The assignability failure selects the MESSAGE-ONLY constructor,
            // so unlike the two above it carries no parameter name and its
            // Message is not composed.
            XCTAssertNil(failure.ParamName)
            XCTAssertEqual(failure.HResult, Int32(bitPattern: 0x8007_0057))
            let message = failure.Message
            // The template is XNA's own, read out of its resource table; the
            // substituted names are Swift's, because Type.FullName is not part
            // of the Any.Type projection.
            XCTAssertTrue(message.hasPrefix("Service provider object of type "))
            XCTAssertTrue(message.hasSuffix("."))
            XCTAssertTrue(message.contains(" must be assignable to service type "))
        }
        XCTAssertThrowsError(
            try refused.AddService(ProbeService.self, provider: ProbeUnrelated())
        )
        XCTAssertNil(refused.GetService(ProbeDerived.self),
                     "a refused registration must store nothing")
    }

    // `RemoveService` discards `Dictionary.Remove`'s result, so removing a type
    // the container does not hold is a no-op rather than a failure.
    func testRemoveServiceIsIdempotent() throws {
        let container = makeContainer()
        try container.AddService(ProbeService.self, provider: ProbeDerived())
        container.RemoveService(ProbeService.self)
        XCTAssertNil(container.GetService(ProbeService.self))
        container.RemoveService(ProbeService.self)
        container.RemoveService(ProbeUnrelated.self)

        // ... and the type can be registered again afterwards.
        XCTAssertNoThrow(
            try container.AddService(ProbeService.self, provider: ProbeDerived()))
    }

    // The container is keyed by type IDENTITY, which is what the CLR's default
    // comparer gives for a runtime Type. Two distinct metatypes never collide.
    func testDistinctTypesAreDistinctKeys() throws {
        let container = makeContainer()
        let first = ProbeDerived()
        let second = ProbeUnrelated()
        try container.AddService(ProbeDerived.self, provider: first)
        try container.AddService(ProbeUnrelated.self, provider: second)
        XCTAssertTrue(container.GetService(ProbeDerived.self) as AnyObject === first)
        XCTAssertTrue(container.GetService(ProbeUnrelated.self) as AnyObject === second)
    }
}

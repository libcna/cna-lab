// SPDX-License-Identifier: MIT

// BCL attribute support projection.
//
// The five `Microsoft.Xna.Framework.Content.ContentSerializer*` types are all
// declared on `System.Attribute`. Unlike the collection and dictionary
// families, almost nothing usable is inherited here — what the base carries is
// the IDENTITY. Dropping it would leave five ordinary classes that a consumer
// could not treat uniformly as attributes, and it would be the silent
// base-dropping the Foundation 27 rule exists to stop.
//
//     System.Attribute  ->  CNAAttribute
//
// ## What is projected, and what a CLR attribute mostly is
//
// `System.Attribute`'s public surface is overwhelmingly REFLECTION: eight
// `GetCustomAttribute` overloads, sixteen `GetCustomAttributes` overloads and
// eight `IsDefined` overloads over `MemberInfo`, `Assembly`, `Module` and
// `ParameterInfo`, plus `Equals`, `GetHashCode` and `Match`, which compare two
// attributes field by field through reflection, and `TypeId`, which returns
// `GetType()`. None of that is reconstructible without `System.Reflection` and
// `System.Type`, so every one of them is FORBIDDEN by the verifier rather than
// answered with something plausible.
//
// What is left, and what is projected, is the constructor and
// `IsDefaultAttribute()`, whose base body is a plain `return false`.
//
// ## The one widening
//
// `mscorlib` declares `System.Attribute` **abstract** with a **protected**
// constructor: a consumer may derive from it but may not construct one.
// Swift has no abstract class, so `CNAAttribute()` is constructible here where
// the CLR refuses it. That is stated rather than papered over with an invented
// runtime trap, and it is recorded on the support type's measured shape. The
// protected-to-public widening is the same one `CNACollection.Items` and
// `CNAException.HResult` already make, for the same reason: Swift has no
// `protected`.
//
// It lives outside `Microsoft.Xna.Framework`, is counted in no XNA scoreboard,
// and no `::System` namespace is fabricated for it. It does not conform to
// `Sendable`: it is `open`, so a subclass anywhere may add mutable stored
// state, and `mscorlib` makes no thread-safety promise for an attribute
// either.

/// The `System.Attribute` projection.
open class CNAAttribute {
    /// `Attribute..ctor()`.
    ///
    /// The CLR declares this `protected` on an `abstract` class. Swift has
    /// neither, so it is a public initializer on an `open` class — the
    /// widening documented above.
    public init() {}

    /// `Attribute.IsDefaultAttribute()`.
    ///
    /// The base body is `ldc.i4.0; ret` — a plain `false`. It is `virtual` and
    /// not `final`, so it is `open`; none of the five XNA attribute types
    /// overrides it, so all five report `false`.
    open func IsDefaultAttribute() -> Bool { false }
}

# C++ deviations from upstream Myra

This document records only necessary, tested differences from the selected
upstream `Myra.FNA.Core` behavior. A difference is not accepted merely because
it is more convenient in C++.

| ID | Upstream behavior | C++ mapping | Rationale | Test | Status |
| --- | --- | --- | --- | --- | --- |
| DEV-001 | C# properties | `getXProperty()` / `setXProperty()` | Established CNA/sharp-runtime C++ surface convention; behavior remains equivalent. | API inventory | Planned |
| DEV-002 | Runtime reflection drives MML/PropertyGrid/DataGrid | Explicit Myra type/property registry | sharp-runtime intentionally has no usable .NET reflection. | MML/PropertyGrid tests | Planned |
| DEV-003 | `Thickness.FromString(null)` returns `Thickness.Zero`; invalid numeric parts throw .NET format exceptions. | C++ accepts `const std::string&` (not null) and throws `std::invalid_argument` for all invalid text. | C++ has no null string value and does not expose .NET exception types. Valid parse and layout behavior remain identical. | `ThicknessTests.*` | Accepted |
| DEV-004 | Myra event delegates accept `object sender`; C# classes can be passed and event-argument objects remain mutable to handlers. | `MyraEventHandler` uses sharp-runtime `System::MulticastAction<void*, T&>`. | CNA/Myra classes do not derive from sharp-runtime's abstract `System::Object`; a raw non-owning sender preserves unconstrained C# sender behavior, and `T&` preserves cancellation/value mutation. | `EventsTests.GenericHandlerSupportsMutationAndTokenRemoval` | Accepted |
| DEV-005 | C# permits `CancellableEventArgs` and `CancellableEventArgs<T>` in one namespace. | The generic C++ type is `CancellableEventArgsT<T>`. | C++ cannot declare a class and a class template with the same name in one namespace. | `EventsTests.ArgumentsPreserveUpstreamValuesAndMutability` | Accepted |
| DEV-006 | `Mathematics.PointZero` is a static initialized C# field. | `Mathematics::getPointZeroProperty()` constructs and returns the zero `Point` on demand. | Avoids a CNA `Point` linker dependency merely from including an otherwise header-only utility header in headers-only CMake mode. | Header compile via `myra_cna_minimal` | Accepted |
| DEV-007 | `ColorHSV.Equals` compares `a.S` with `b.V`, making many values unequal to themselves. | The same comparison is retained. | This is an observable upstream defect; changing it would break strict behavior parity and must be a separately approved compatibility change. | `ColorHSVTests.PreservesTheSelectedUpstreamEqualityBehavior` | Accepted upstream defect |

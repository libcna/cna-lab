// SPDX-License-Identifier: MIT

extension Microsoft.Xna.Framework.Input.Touch {
    // Pinned in Microsoft.Xna.Framework.Input.Touch.dll as
    // `.class public sequential ansi sealed beforefieldinit ... extends
    // [mscorlib]System.ValueType`. Both members are compiler-generated auto
    // properties whose setters are `private`, so the public contract is two
    // get-only properties and nothing else: no constructor, no equality, no
    // `ToString`.
    //
    // The only producer is the `assembly` static `GetCaps()`, which is not
    // part of the public contract. This projection is the value struct alone;
    // `TouchPanel` is not implemented and no capability is claimed, queried,
    // or invented here.
    public struct TouchPanelCapabilities {
        private let isConnected: Bool
        private let maximumTouchCount: Int32

        internal init(isConnected: Bool, maximumTouchCount: Int32) {
            self.isConnected = isConnected
            self.maximumTouchCount = maximumTouchCount
        }

        public var IsConnected: Bool { isConnected }

        public var MaximumTouchCount: Int32 { maximumTouchCount }
    }
}

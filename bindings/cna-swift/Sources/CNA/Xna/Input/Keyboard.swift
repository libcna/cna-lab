// SPDX-License-Identifier: MIT

import CNAShim

extension Microsoft.Xna.Framework.Input {
    public enum KeyState: Int32 {
        case Up = 0
        case Down = 1
    }

    public enum Keys: Int32 {
        case A = 65
        case Add = 107
        case Apps = 93
        case Attn = 246
        case B = 66
        case Back = 8
        case BrowserBack = 166
        case BrowserFavorites = 171
        case BrowserForward = 167
        case BrowserHome = 172
        case BrowserRefresh = 168
        case BrowserSearch = 170
        case BrowserStop = 169
        case C = 67
        case CapsLock = 20
        case Crsel = 247
        case D = 68
        case D0 = 48
        case D1 = 49
        case D2 = 50
        case D3 = 51
        case D4 = 52
        case D5 = 53
        case D6 = 54
        case D7 = 55
        case D8 = 56
        case D9 = 57
        case Decimal = 110
        case Delete = 46
        case Divide = 111
        case Down = 40
        case E = 69
        case End = 35
        case Enter = 13
        case EraseEof = 249
        case Escape = 27
        case Execute = 43
        case Exsel = 248
        case F = 70
        case F1 = 112
        case F10 = 121
        case F11 = 122
        case F12 = 123
        case F13 = 124
        case F14 = 125
        case F15 = 126
        case F16 = 127
        case F17 = 128
        case F18 = 129
        case F19 = 130
        case F2 = 113
        case F20 = 131
        case F21 = 132
        case F22 = 133
        case F23 = 134
        case F24 = 135
        case F3 = 114
        case F4 = 115
        case F5 = 116
        case F6 = 117
        case F7 = 118
        case F8 = 119
        case F9 = 120
        case G = 71
        case H = 72
        case Help = 47
        case Home = 36
        case I = 73
        case ImeConvert = 28
        case ImeNoConvert = 29
        case Insert = 45
        case J = 74
        case K = 75
        case Kana = 21
        case Kanji = 25
        case L = 76
        case LaunchApplication1 = 182
        case LaunchApplication2 = 183
        case LaunchMail = 180
        case LeftControl = 162
        case Left = 37
        case LeftAlt = 164
        case LeftShift = 160
        case LeftWindows = 91
        case M = 77
        case MediaNextTrack = 176
        case MediaPlayPause = 179
        case MediaPreviousTrack = 177
        case MediaStop = 178
        case Multiply = 106
        case N = 78
        case None = 0
        case NumLock = 144
        case NumPad0 = 96
        case NumPad1 = 97
        case NumPad2 = 98
        case NumPad3 = 99
        case NumPad4 = 100
        case NumPad5 = 101
        case NumPad6 = 102
        case NumPad7 = 103
        case NumPad8 = 104
        case NumPad9 = 105
        case O = 79
        case OemAuto = 243
        case OemCopy = 242
        case OemEnlW = 244
        case OemSemicolon = 186
        case OemBackslash = 226
        case OemQuestion = 191
        case OemTilde = 192
        case OemOpenBrackets = 219
        case OemPipe = 220
        case OemCloseBrackets = 221
        case OemQuotes = 222
        case Oem8 = 223
        case OemClear = 254
        case OemComma = 188
        case OemMinus = 189
        case OemPeriod = 190
        case OemPlus = 187
        case P = 80
        case Pa1 = 253
        case PageDown = 34
        case PageUp = 33
        case Pause = 19
        case Play = 250
        case Print = 42
        case PrintScreen = 44
        case ProcessKey = 229
        case Q = 81
        case R = 82
        case RightControl = 163
        case Right = 39
        case RightAlt = 165
        case RightShift = 161
        case RightWindows = 92
        case S = 83
        case Scroll = 145
        case Select = 41
        case SelectMedia = 181
        case Separator = 108
        case Sleep = 95
        case Space = 32
        case Subtract = 109
        case T = 84
        case Tab = 9
        case U = 85
        case Up = 38
        case V = 86
        case VolumeDown = 174
        case VolumeMute = 173
        case VolumeUp = 175
        case W = 87
        case X = 88
        case Y = 89
        case Z = 90
        case Zoom = 251
        case ChatPadGreen = 202
        case ChatPadOrange = 203
    }

    public struct KeyboardState {
        private var words: (UInt64, UInt64, UInt64, UInt64)

        public init(_ keys: [Keys]) {
            var values = (UInt64(0), UInt64(0), UInt64(0), UInt64(0))
            for key in keys {
                let raw = UInt32(bitPattern: key.rawValue)
                guard raw < 256 else { continue }
                let bit = UInt64(1) << UInt64(raw & 63)
                switch raw >> 6 {
                case 0: values.0 |= bit
                case 1: values.1 |= bit
                case 2: values.2 |= bit
                default: values.3 |= bit
                }
            }
            words = values
        }

        internal init(native: CNASwift_KeyboardState) {
            words = (
                native.pressed_key_words.0,
                native.pressed_key_words.1,
                native.pressed_key_words.2,
                native.pressed_key_words.3
            )
        }

        public subscript(_ key: Keys) -> KeyState {
            IsKeyDown(key) ? .Down : .Up
        }

        public func IsKeyDown(_ key: Keys) -> Bool {
            let raw = UInt32(bitPattern: key.rawValue)
            let bit = UInt64(1) << UInt64(raw & 63)
            switch raw >> 6 {
            case 0: return words.0 & bit != 0
            case 1: return words.1 & bit != 0
            case 2: return words.2 & bit != 0
            default: return words.3 & bit != 0
            }
        }

        public func IsKeyUp(_ key: Keys) -> Bool { !IsKeyDown(key) }

        public func GetPressedKeys() -> [Keys] {
            (0..<256).compactMap { raw in
                guard let key = Keys(rawValue: Int32(raw)), IsKeyDown(key) else { return nil }
                return key
            }
        }

        public func GetHashCode() -> Int32 {
            let folded = UInt32(truncatingIfNeeded: words.0) ^ UInt32(truncatingIfNeeded: words.0 >> 32) ^
                UInt32(truncatingIfNeeded: words.1) ^ UInt32(truncatingIfNeeded: words.1 >> 32) ^
                UInt32(truncatingIfNeeded: words.2) ^ UInt32(truncatingIfNeeded: words.2 >> 32) ^
                UInt32(truncatingIfNeeded: words.3) ^ UInt32(truncatingIfNeeded: words.3 >> 32)
            return Int32(bitPattern: folded)
        }

        public func Equals(_ obj: Any?) -> Bool {
            guard let other = obj as? KeyboardState else { return false }
            return self == other
        }

        public static func == (lhs: KeyboardState, rhs: KeyboardState) -> Bool {
            lhs.words.0 == rhs.words.0 && lhs.words.1 == rhs.words.1 &&
                lhs.words.2 == rhs.words.2 && lhs.words.3 == rhs.words.3
        }

        public static func != (lhs: KeyboardState, rhs: KeyboardState) -> Bool { !(lhs == rhs) }
    }

    public final class Keyboard {
        private init() {}

        public static func GetState() throws -> KeyboardState {
            let runtime = try RuntimeRegistry.current()
            try runtime.owner.validate("Keyboard.GetState")
            var state = CNASwift_KeyboardState()
            state.struct_size = UInt32(MemoryLayout<CNASwift_KeyboardState>.size)
            state.struct_version = 1
            try runtime.functions.check(
                runtime.functions.keyboardGetState(runtime.gameHandle, &state),
                operation: "cna_keyboard_get_state"
            )
            return KeyboardState(native: state)
        }

        public static func GetState(_ playerIndex: Microsoft.Xna.Framework.PlayerIndex) throws -> KeyboardState {
            let runtime = try RuntimeRegistry.current()
            try runtime.owner.validate("Keyboard.GetState")
            var state = CNASwift_KeyboardState()
            state.struct_size = UInt32(MemoryLayout<CNASwift_KeyboardState>.size)
            state.struct_version = 1
            try runtime.functions.check(
                runtime.functions.keyboardGetStateForPlayer(
                    runtime.gameHandle,
                    UInt32(bitPattern: playerIndex.rawValue),
                    &state
                ),
                operation: "cna_keyboard_get_state_for_player"
            )
            return KeyboardState(native: state)
        }
    }
}

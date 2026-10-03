// SPDX-License-Identifier: MIT

import Foundation

/// A PNG decoder written for these tests and owed to nothing else.
///
/// `Texture2D.SaveAsPng` cannot be verified by handing its output back to
/// `Texture2D.FromStream`: both ends would be CNA, and a symmetric defect —
/// a transposed dimension, a swapped channel, an ignored resize — would round
/// trip perfectly. The bytes have to be read by something that has never
/// heard of CNA, which is what this is: the PNG container, DEFLATE, and the
/// five scanline filters, from the format specification.
///
/// It is deliberately narrow. Eight bits per channel, colour types 2 and 6,
/// no interlacing: exactly what the encoder under test emits, and anything
/// else is an error rather than a silent approximation.
enum PortableNetworkGraphics {
    struct Image {
        let width: Int
        let height: Int
        let colorType: Int
        /// Row-major RGBA, four bytes per pixel, alpha 255 where the file has none.
        let pixels: [UInt8]

        func pixel(x: Int, y: Int) -> (r: UInt8, g: UInt8, b: UInt8, a: UInt8) {
            let base = (y * width + x) * 4
            return (pixels[base], pixels[base + 1], pixels[base + 2], pixels[base + 3])
        }
    }

    enum Failure: Error, CustomStringConvertible {
        case notPng
        case truncated
        case unsupported(String)
        case corrupt(String)

        var description: String {
            switch self {
            case .notPng: return "not a PNG signature"
            case .truncated: return "the byte stream ends inside a structure"
            case .unsupported(let what): return "unsupported: \(what)"
            case .corrupt(let what): return "corrupt: \(what)"
            }
        }
    }

    static let signature: [UInt8] = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]

    static func decode(_ bytes: [UInt8]) throws -> Image {
        guard bytes.count > 8, Array(bytes[0..<8]) == signature else { throw Failure.notPng }

        var position = 8
        var header: (width: Int, height: Int, depth: Int, color: Int, interlace: Int)?
        var compressed: [UInt8] = []

        func beInt(_ offset: Int) throws -> Int {
            guard offset + 4 <= bytes.count else { throw Failure.truncated }
            return (Int(bytes[offset]) << 24) | (Int(bytes[offset + 1]) << 16)
                | (Int(bytes[offset + 2]) << 8) | Int(bytes[offset + 3])
        }

        while position + 8 <= bytes.count {
            let length = try beInt(position)
            guard position + 12 + length <= bytes.count else { throw Failure.truncated }
            let kind = String(decoding: bytes[(position + 4)..<(position + 8)], as: UTF8.self)
            let body = Array(bytes[(position + 8)..<(position + 8 + length)])
            position += 12 + length
            switch kind {
            case "IHDR":
                guard body.count >= 13 else { throw Failure.truncated }
                header = (
                    width: (Int(body[0]) << 24) | (Int(body[1]) << 16) | (Int(body[2]) << 8) | Int(body[3]),
                    height: (Int(body[4]) << 24) | (Int(body[5]) << 16) | (Int(body[6]) << 8) | Int(body[7]),
                    depth: Int(body[8]), color: Int(body[9]), interlace: Int(body[12])
                )
            case "IDAT":
                compressed += body
            case "IEND":
                position = bytes.count
            default:
                break
            }
        }

        guard let header else { throw Failure.corrupt("no IHDR") }
        guard header.depth == 8 else { throw Failure.unsupported("bit depth \(header.depth)") }
        guard header.interlace == 0 else { throw Failure.unsupported("interlacing") }
        let channels: Int
        switch header.color {
        case 2: channels = 3
        case 6: channels = 4
        default: throw Failure.unsupported("colour type \(header.color)")
        }
        guard header.width > 0, header.height > 0 else { throw Failure.corrupt("zero extent") }

        // zlib: a two-byte header, the DEFLATE stream, a four-byte Adler-32.
        guard compressed.count > 6 else { throw Failure.truncated }
        guard compressed[0] & 0x0F == 8 else { throw Failure.unsupported("zlib method") }
        let raw = try inflate(Array(compressed[2...]))

        let stride = header.width * channels
        guard raw.count >= (stride + 1) * header.height else {
            throw Failure.corrupt("inflated \(raw.count) bytes for \((stride + 1) * header.height)")
        }

        var pixels = [UInt8](repeating: 255, count: header.width * header.height * 4)
        var previous = [UInt8](repeating: 0, count: stride)
        var offset = 0
        for y in 0..<header.height {
            let filter = raw[offset]
            offset += 1
            var line = Array(raw[offset..<(offset + stride)])
            offset += stride
            for index in 0..<stride {
                let left = index >= channels ? Int(line[index - channels]) : 0
                let up = Int(previous[index])
                let upLeft = index >= channels ? Int(previous[index - channels]) : 0
                let value = Int(line[index])
                switch filter {
                case 0: break
                case 1: line[index] = UInt8((value + left) & 0xFF)
                case 2: line[index] = UInt8((value + up) & 0xFF)
                case 3: line[index] = UInt8((value + (left + up) / 2) & 0xFF)
                case 4:
                    let estimate = left + up - upLeft
                    let dl = abs(estimate - left), du = abs(estimate - up), dul = abs(estimate - upLeft)
                    let predictor = (dl <= du && dl <= dul) ? left : (du <= dul ? up : upLeft)
                    line[index] = UInt8((value + predictor) & 0xFF)
                default: throw Failure.corrupt("filter \(filter)")
                }
            }
            for x in 0..<header.width {
                let source = x * channels
                let destination = (y * header.width + x) * 4
                pixels[destination] = line[source]
                pixels[destination + 1] = line[source + 1]
                pixels[destination + 2] = line[source + 2]
                pixels[destination + 3] = channels == 4 ? line[source + 3] : 255
            }
            previous = line
        }

        return Image(width: header.width, height: header.height,
                     colorType: header.color, pixels: pixels)
    }

    // MARK: - DEFLATE

    private struct BitReader {
        let bytes: [UInt8]
        var position = 0
        var bit = 0

        init(_ bytes: [UInt8]) { self.bytes = bytes }

        mutating func read() throws -> Int {
            guard position < bytes.count else { throw Failure.truncated }
            let value = (Int(bytes[position]) >> bit) & 1
            bit += 1
            if bit == 8 { bit = 0; position += 1 }
            return value
        }

        mutating func read(_ count: Int) throws -> Int {
            var value = 0
            for index in 0..<count { value |= try read() << index }
            return value
        }

        mutating func alignToByte() {
            if bit != 0 { bit = 0; position += 1 }
        }
    }

    /// A canonical Huffman table, decoded one bit at a time.
    private struct Huffman {
        // counts[length] and the symbols in canonical order.
        private var counts: [Int]
        private var symbols: [Int]

        init(lengths: [Int]) {
            var counts = [Int](repeating: 0, count: 16)
            for length in lengths where length > 0 { counts[length] += 1 }
            var offsets = [Int](repeating: 0, count: 16)
            var total = 0
            for length in 1..<16 {
                offsets[length] = total
                total += counts[length]
            }
            var symbols = [Int](repeating: 0, count: total)
            for (symbol, length) in lengths.enumerated() where length > 0 {
                symbols[offsets[length]] = symbol
                offsets[length] += 1
            }
            self.counts = counts
            self.symbols = symbols
        }

        func decode(_ reader: inout BitReader) throws -> Int {
            var code = 0, first = 0, index = 0
            for length in 1..<16 {
                code |= try reader.read()
                let count = counts[length]
                if code - first < count { return symbols[index + (code - first)] }
                index += count
                first = (first + count) << 1
                code <<= 1
            }
            throw Failure.corrupt("no Huffman code matches")
        }
    }

    private static let lengthBase = [3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                                     35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258]
    private static let lengthExtra = [0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                      3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0]
    private static let distanceBase = [1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                                       257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
                                       8193, 12289, 16385, 24577]
    private static let distanceExtra = [0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13]

    static func inflate(_ bytes: [UInt8]) throws -> [UInt8] {
        var reader = BitReader(bytes)
        var out: [UInt8] = []
        while true {
            let final = try reader.read()
            let kind = try reader.read(2)
            switch kind {
            case 0:
                reader.alignToByte()
                guard reader.position + 4 <= reader.bytes.count else { throw Failure.truncated }
                let length = Int(reader.bytes[reader.position]) | (Int(reader.bytes[reader.position + 1]) << 8)
                reader.position += 4
                guard reader.position + length <= reader.bytes.count else { throw Failure.truncated }
                out += reader.bytes[reader.position..<(reader.position + length)]
                reader.position += length
            case 1:
                var lengths = [Int](repeating: 8, count: 288)
                for symbol in 144..<256 { lengths[symbol] = 9 }
                for symbol in 256..<280 { lengths[symbol] = 7 }
                try inflateBlock(&reader, into: &out,
                                 literals: Huffman(lengths: lengths),
                                 distances: Huffman(lengths: [Int](repeating: 5, count: 30)))
            case 2:
                let literalCount = try reader.read(5) + 257
                let distanceCount = try reader.read(5) + 1
                let codeCount = try reader.read(4) + 4
                let order = [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
                var codeLengths = [Int](repeating: 0, count: 19)
                for index in 0..<codeCount { codeLengths[order[index]] = try reader.read(3) }
                let codeTable = Huffman(lengths: codeLengths)
                var lengths: [Int] = []
                while lengths.count < literalCount + distanceCount {
                    let symbol = try codeTable.decode(&reader)
                    switch symbol {
                    case 0..<16: lengths.append(symbol)
                    case 16:
                        guard let last = lengths.last else { throw Failure.corrupt("repeat with no previous length") }
                        lengths += [Int](repeating: last, count: try reader.read(2) + 3)
                    case 17: lengths += [Int](repeating: 0, count: try reader.read(3) + 3)
                    case 18: lengths += [Int](repeating: 0, count: try reader.read(7) + 11)
                    default: throw Failure.corrupt("code-length symbol \(symbol)")
                    }
                }
                try inflateBlock(
                    &reader, into: &out,
                    literals: Huffman(lengths: Array(lengths[0..<literalCount])),
                    distances: Huffman(lengths: Array(lengths[literalCount..<(literalCount + distanceCount)])))
            default:
                throw Failure.corrupt("reserved block type")
            }
            if final == 1 { return out }
        }
    }

    private static func inflateBlock(
        _ reader: inout BitReader, into out: inout [UInt8],
        literals: Huffman, distances: Huffman
    ) throws {
        while true {
            let symbol = try literals.decode(&reader)
            if symbol < 256 {
                out.append(UInt8(symbol))
            } else if symbol == 256 {
                return
            } else {
                let index = symbol - 257
                guard index < lengthBase.count else { throw Failure.corrupt("length symbol \(symbol)") }
                let length = lengthBase[index] + (try reader.read(lengthExtra[index]))
                let distanceSymbol = try distances.decode(&reader)
                guard distanceSymbol < distanceBase.count else {
                    throw Failure.corrupt("distance symbol \(distanceSymbol)")
                }
                let distance = distanceBase[distanceSymbol] + (try reader.read(distanceExtra[distanceSymbol]))
                guard distance <= out.count else { throw Failure.corrupt("distance past the window") }
                let start = out.count - distance
                for step in 0..<length { out.append(out[start + step]) }
            }
        }
    }
}

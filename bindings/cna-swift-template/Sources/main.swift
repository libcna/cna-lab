// SPDX-License-Identifier: MIT

import Foundation

func frameLimit() throws -> Int {
    let arguments = CommandLine.arguments
    guard let index = arguments.firstIndex(of: "--frames") else { return 600 }
    guard index + 1 < arguments.count,
          let value = Int(arguments[index + 1]),
          value > 0 else {
        throw HelloGameError.invalidFrameLimit(index + 1 < arguments.count ? arguments[index + 1] : "<missing>")
    }
    return value
}

do {
    let game = try HelloGame(frames: frameLimit())
    do {
        try game.Run()
        print(game.qualificationLine())
        try game.Dispose()
    } catch {
        try? game.Dispose()
        throw error
    }
} catch {
    FileHandle.standardError.write(Data("CNA Swift canary failed: \(error)\n".utf8))
    Foundation.exit(1)
}

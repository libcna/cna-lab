import XCTest
@testable import CNA

/// `SpriteBatch.Begin`'s two effect-carrying overloads.
///
/// The five that already stood forward into the seven-argument one; these are
/// that one and the six-argument overload above it.
final class Foundation76SpriteBatchEffectTests: XCTestCase {

    /// XNA's six-argument `Begin` is twenty-one bytes: five arguments, then
    /// `Matrix.Identity`, then a call to the seven-argument overload. **The
    /// identity is XNA's default**, so the two must be indistinguishable.
    func testTheSixArgumentOverloadIsTheSevenArgumentOneWithIdentity() throws {
        let game = try BatchEffectProbeGame()
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.acceptedWithoutMatrix, true,
                       "Begin(..., effect:) is accepted")
        XCTAssertEqual(game.acceptedWithIdentity, true,
                       "and so is the same call with Matrix.Identity spelled out")
    }

    /// A second `Begin` without an `End` is refused before anything else, and
    /// the effect overloads share that guard: XNA's first instruction is
    /// `ldfld inBeginEndPair`, ahead of any state work.
    func testASecondBeginIsRefusedThroughTheEffectOverloadToo() throws {
        let game = try BatchEffectProbeGame(nestBegin: true)
        try game.Run()
        if let failure = game.failure { throw failure }
        guard let error = game.nestedFailure else {
            return XCTFail("a nested Begin was accepted")
        }
        guard let refusal = error as? CNAInvalidOperationException else {
            return XCTFail("expected CNAInvalidOperationException, got \(error)")
        }
        XCTAssertEqual(refusal.Message, endMustBeCalledBeforeBeginMessage)
    }
}

private final class BatchEffectProbeGame: Microsoft.Xna.Framework.Game {
    let nestBegin: Bool
    var manager: Microsoft.Xna.Framework.GraphicsDeviceManager?
    var failure: Error?
    var nestedFailure: Error?
    var acceptedWithoutMatrix: Bool?
    var acceptedWithIdentity: Bool?

    init(nestBegin: Bool = false) throws {
        self.nestBegin = nestBegin
        try super.init()
        manager = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func LoadContent() throws {
        defer { try? Exit() }
        do {
            guard let device = try GraphicsDevice else { return }
            let batch = try Microsoft.Xna.Framework.Graphics.SpriteBatch(
                graphicsDevice: device)
            let effect = try Microsoft.Xna.Framework.Graphics.Effect.empty(
                graphicsDevice: device)

            try batch.Begin(.Deferred, blendState: nil, samplerState: nil,
                            depthStencilState: nil, rasterizerState: nil,
                            effect: effect)
            acceptedWithoutMatrix = true
            if nestBegin {
                do {
                    try batch.Begin(.Deferred, blendState: nil, samplerState: nil,
                                    depthStencilState: nil, rasterizerState: nil,
                                    effect: effect)
                } catch {
                    nestedFailure = error
                }
            }
            try batch.End()

            try batch.Begin(.Deferred, blendState: nil, samplerState: nil,
                            depthStencilState: nil, rasterizerState: nil,
                            effect: effect,
                            transformMatrix: .Identity)
            acceptedWithIdentity = true
            try batch.End()
        } catch {
            failure = error
        }
    }
}

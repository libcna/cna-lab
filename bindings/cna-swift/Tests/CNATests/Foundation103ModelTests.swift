// SPDX-License-Identifier: MIT

import XCTest
@testable import CNA

/// The Model family, projected at Foundation 103.
///
/// **These build their models out of parts rather than loading one.** XNA's
/// `Model` has no public constructor and arrives from
/// `ContentManager.Load<Model>`, which needs an `.xnb` this repository cannot
/// produce. `NEXT.md` recorded that as the family's blocker for several
/// Foundations, and a probe disproved it: `cna_model_create_default`,
/// `cna_model_bone_create` and the collection creates all answer
/// `CNA_RESULT_SUCCESS` with no game, no device and no asset. The content
/// pipeline is what a CONSUMER needs to obtain a model; it is not what the
/// twelve types need to be projected or exercised.
final class Foundation103ModelTests: XCTestCase {

    // MARK: - Bones

    func testABoneReportsTheNameAndIndexItWasMadeWith() throws {
        let game = try ModelProbeGame(probe: .boneIdentity)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.boneName, "root")
        XCTAssertEqual(game.boneIndex, 7)
    }

    func testTheTransformRoundTripsThroughTheRuntime() throws {
        let game = try ModelProbeGame(probe: .boneTransform)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.readBackTransform?.M11, 2)
        XCTAssertEqual(game.readBackTransform?.M42, 5)
        XCTAssertNil(game.transformPushFailure,
                     "a push that failed would be kept rather than thrown")
    }

    func testABoneWithoutAParentReportsNil() throws {
        let game = try ModelProbeGame(probe: .boneIdentity)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(game.rootHasNoParent,
                      "the root bone has no parent, which CNA reports through "
                      + "out_has_parent rather than a null handle")
    }

    // MARK: - Collections

    func testAnEmptyBoneCollectionCountsZero() throws {
        let game = try ModelProbeGame(probe: .emptyCollection)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.collectionCount, 0)
    }

    func testTryGetValueWritesThroughItsInoutParameter() throws {
        let game = try ModelProbeGame(probe: .emptyCollection)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertFalse(
            game.tryGetValueFound,
            "an absent name answers false and leaves the caller's slot alone, "
            + "which is the CLR shape: an out parameter, not an Optional")
    }

    func testTheEnumeratorOfAnEmptyCollectionNeverAdvances() throws {
        let game = try ModelProbeGame(probe: .emptyCollection)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertFalse(game.enumeratorMoved)
    }

    // MARK: - Model

    func testADefaultModelHasEmptyBonesAndMeshes() throws {
        let game = try ModelProbeGame(probe: .defaultModel)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.modelBoneCount, 0)
        XCTAssertEqual(game.modelMeshCount, 0)
    }

    func testTheTagIsHeldOnTheSwiftSide() throws {
        let game = try ModelProbeGame(probe: .defaultModel)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(
            game.tagReadBack as? String, "carried",
            "the CLR property is Object and the native tag is an opaque "
            + "uint64_t; the projection stores it here and the tag routes stay "
            + "unbound")
    }

    func testCopyingBoneTransformsWritesIntoTheCallersArray() throws {
        let game = try ModelProbeGame(probe: .defaultModel)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertTrue(
            game.copyLeftTheArrayAlone,
            "a model with no bones writes nothing, and the caller's array is "
            + "its own storage rather than a fresh one this method returned")
    }

    func testStartIndexAndVertexOffsetAreDifferentRoutes() throws {
        let game = try ModelProbeGame(probe: .describedPart)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        game.effectAfterSet = nil
        XCTAssertEqual(game.describedStartIndex, 3)
        XCTAssertEqual(game.describedVertexOffset, 4,
                       "two small integers that mean different things")
    }

    func testAnEffectSetOnAPartIsReadBack() throws {
        let game = try ModelProbeGame(probe: .describedPart)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertNotNil(game.effectAfterSet,
                        "a default part carries no effect, so reading one back "
                        + "proves the setter reached the runtime")
        // The getter returns a callback-scoped facade. Do not retain that
        // borrowed native identity into the parent game's explicit teardown.
        game.effectAfterSet = nil
    }

    func testDrawingAMeshWhosePartHasNoEffectRaisesXNAsMessage() throws {
        let game = try ModelProbeGame(probe: .effectlessMesh)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        let refusal = try XCTUnwrap(
            game.meshDrawFailure as? CNAInvalidOperationException)
        XCTAssertEqual(
            refusal.Message, "ModelMeshPart has a null Effect.",
            "the assembly's own wording, read out of "
            + "Microsoft.Xna.Framework.dll's table -- not the Graphics "
            + "assembly whose IL raises it, which carries no table at all")
    }

    // MARK: - Mesh parts

    func testADefaultMeshPartReadsItsCountsAsZero() throws {
        let game = try ModelProbeGame(probe: .meshPart)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertEqual(game.partNumVertices, 0)
        XCTAssertEqual(game.partPrimitiveCount, 0)
        XCTAssertEqual(game.partStartIndex, 0)
        XCTAssertEqual(game.partVertexOffset, 0)
    }

    func testADefaultMeshPartCarriesNoBuffersAndNoEffect() throws {
        let game = try ModelProbeGame(probe: .meshPart)
        defer { XCTAssertNoThrow(try game.Dispose()) }
        try game.Run()
        if let failure = game.failure { throw failure }
        XCTAssertNil(game.partVertexBuffer)
        XCTAssertNil(game.partIndexBuffer)
        XCTAssertNil(game.partEffect)
    }
}

private final class ModelProbeGame: Microsoft.Xna.Framework.Game {

    enum Probe { case boneIdentity, boneTransform, emptyCollection,
                      defaultModel, meshPart, describedPart, effectlessMesh }

    private let probe: Probe
    private var graphics: Microsoft.Xna.Framework.GraphicsDeviceManager?

    var failure: Error?
    var boneName: String?
    var boneIndex: Int32?
    var rootHasNoParent = false
    var readBackTransform: Microsoft.Xna.Framework.Matrix?
    var transformPushFailure: Error?
    var collectionCount: Int32?
    var tryGetValueFound = true
    var enumeratorMoved = true
    var modelBoneCount: Int32?
    var modelMeshCount: Int32?
    var tagReadBack: Any?
    var copyLeftTheArrayAlone = false
    var partNumVertices: Int32?
    var partPrimitiveCount: Int32?
    var partStartIndex: Int32?
    var partVertexOffset: Int32?
    var partVertexBuffer: Microsoft.Xna.Framework.Graphics.VertexBuffer?
    var partIndexBuffer: Microsoft.Xna.Framework.Graphics.IndexBuffer?
    var partEffect: Microsoft.Xna.Framework.Graphics.Effect?
    var describedStartIndex: Int32?
    var describedVertexOffset: Int32?
    var effectAfterSet: Microsoft.Xna.Framework.Graphics.Effect?
    var meshDrawFailure: Error?

    init(probe: Probe) throws {
        self.probe = probe
        try super.init()
        graphics = try Microsoft.Xna.Framework.GraphicsDeviceManager(game: self)
    }

    override func Update(_ gameTime: Microsoft.Xna.Framework.GameTime) throws {
        defer { try? Exit() }
        do {
            let runtime = try RuntimeRegistry.current()
            switch probe {
            case .boneIdentity:
                let bone = try makeBone(runtime, index: 7, name: "root")
                boneName = bone.Name
                boneIndex = bone.Index
                rootHasNoParent = bone.Parent == nil
            case .boneTransform:
                let bone = try makeBone(runtime, index: 0, name: "b")
                var wanted = Microsoft.Xna.Framework.Matrix.Identity
                wanted.M11 = 2
                wanted.M42 = 5
                bone.Transform = wanted
                readBackTransform = bone.Transform
                transformPushFailure = bone.lastPushFailure
            case .emptyCollection:
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.modelBoneCollectionCreate(&produced),
                    operation: "cna_model_bone_collection_create")
                let collection = Microsoft.Xna.Framework.Graphics
                    .ModelBoneCollection(handle: produced, runtime: runtime)
                collectionCount = collection.Count
                var slot = try makeBone(runtime, index: 0, name: "unused")
                tryGetValueFound = collection.TryGetValue("absent", value: &slot)
                var enumerator = collection.GetEnumerator()
                enumeratorMoved = enumerator.MoveNext()
                enumerator.Dispose()
            case .defaultModel:
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.modelCreateDefault(&produced),
                    operation: "cna_model_create_default")
                let model = Microsoft.Xna.Framework.Graphics.Model(
                    handle: produced, runtime: runtime, owned: true)
                modelBoneCount = model.Bones?.Count
                modelMeshCount = model.Meshes?.Count
                model.Tag = "carried"
                tagReadBack = model.Tag
                var destination = [Microsoft.Xna.Framework.Matrix](
                    repeating: .Identity, count: 3)
                try model.CopyBoneTransformsTo(&destination)
                copyLeftTheArrayAlone = destination.count == 3
            case .describedPart:
                // Distinct values on purpose. A part whose StartIndex and
                // VertexOffset are both zero cannot tell the two routes apart,
                // which is how `model-mesh-part-start-index-reads-the-offset`
                // survived its first run.
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.modelMeshPartCreate(
                        0, 0, 10, 2, 3, 4, &produced),
                    operation: "cna_model_mesh_part_create")
                let part = Microsoft.Xna.Framework.Graphics.ModelMeshPart(
                    handle: produced, runtime: runtime, owned: true)
                describedStartIndex = part.StartIndex
                describedVertexOffset = part.VertexOffset
                if let device = try? requireDevice() {
                    let effect = try Microsoft.Xna.Framework.Graphics
                        .BasicEffect(device: device)
                    part.Effect = effect
                    effectAfterSet = part.Effect
                }
            case .effectlessMesh:
                // A mesh whose one part carries no effect. XNA's Draw refuses
                // before it draws, and the message-coverage gate is what asked
                // for that: forwarding straight to cna_model_mesh_draw left a
                // message XNA raises neither reproduced nor recorded.
                var part: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.modelMeshPartCreateDefault(&part),
                    operation: "cna_model_mesh_part_create_default")
                let device = try requireDevice()
                var parts = [part]
                var mesh: UInt64 = 0
                try runtime.functions.check(
                    parts.withUnsafeMutableBufferPointer { buffer in
                        runtime.functions.modelMeshCreate(
                            try! device.validatedHandle("ModelMesh"),
                            buffer.baseAddress, 1, &mesh)
                    },
                    operation: "cna_model_mesh_create")
                let built = Microsoft.Xna.Framework.Graphics.ModelMesh(
                    handle: mesh, runtime: runtime, owned: true)
                do { try built.Draw() } catch { meshDrawFailure = error }
            case .meshPart:
                var produced: UInt64 = 0
                try runtime.functions.check(
                    runtime.functions.modelMeshPartCreateDefault(&produced),
                    operation: "cna_model_mesh_part_create_default")
                let part = Microsoft.Xna.Framework.Graphics.ModelMeshPart(
                    handle: produced, runtime: runtime, owned: true)
                partNumVertices = part.NumVertices
                partPrimitiveCount = part.PrimitiveCount
                partStartIndex = part.StartIndex
                partVertexOffset = part.VertexOffset
                partVertexBuffer = part.VertexBuffer
                partIndexBuffer = part.IndexBuffer
                partEffect = part.Effect
            }
        } catch {
            failure = error
        }
    }

    private func requireDevice() throws
        -> Microsoft.Xna.Framework.Graphics.GraphicsDevice {
        guard let device = graphics?.GraphicsDevice else {
            throw CNAError.callbackOutsideGameLifecycle
        }
        return device
    }

    private func makeBone(
        _ runtime: RuntimeState, index: Int32, name: String
    ) throws -> Microsoft.Xna.Framework.Graphics.ModelBone {
        var utf8 = Array(name.utf8)
        var produced: UInt64 = 0
        try runtime.functions.check(
            ModelSupport.withStringView(&utf8) { view in
                runtime.functions.modelBoneCreate(index, view, &produced)
            },
            operation: "cna_model_bone_create")
        return Microsoft.Xna.Framework.Graphics.ModelBone(
            handle: produced, runtime: runtime, owned: true)
    }
}

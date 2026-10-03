package org.openeggbert.cna.extensions.content;

import Microsoft.Xna.Framework.Curve;
import Microsoft.Xna.Framework.CurveKey;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/**
 * Dispatching a document to the loader that may read it.
 *
 * <p>What is being tested is not that a map holds a function. It is CNA's identity rule, which is
 * the reason this is not a {@code HashMap} in user code: a built-in type's number proves identity,
 * a custom type's number is a 31-bit hash that two unrelated games can collide on, and a
 * collision must be refused rather than silently loading one game's file with another's loader.
 * See {@code docs/cnb-loader-registry-decision.md}.
 */
@EnabledIfEnvironmentVariable(named = "CNA_NATIVE_LIBRARY", matches = ".+")
final class CnbLoaderTests {

    private static byte[] curveFile(String contentName) {
        Curve curve = new Curve();
        curve.getKeys().Add(new CurveKey(0.0f, 2.5f));
        curve.getKeys().Add(new CurveKey(1.0f, 7.5f));
        return CnbCurve.encode(curve, contentName);
    }

    @Test
    void aBuiltInTypeDispatchesOnItsNumberAlone() {
        CnbLoaders<Curve> loaders = new CnbLoaders<Curve>()
                .register(CnbAssetType.CURVE, CnbDocument::decodeCurve);
        assertTrue(loaders.isRegistered(CnbAssetType.CURVE));
        assertFalse(loaders.isRegistered(CnbAssetType.MODEL));

        try (CnbDocument document = CnbDocument.parse(
                     curveFile("curves/ease"), "ease.cnb", CnbReadLimits.standard())) {
            Curve loaded = loaders.load(document);
            assertEquals(2, loaded.getKeys().getCount());
            assertEquals(7.5f, loaded.Evaluate(1.0f));
        }

        // Nothing registered is an ordinary answer with its own identity, not a format failure.
        CnbLoaders<Curve> empty = new CnbLoaders<>();
        try (CnbDocument document = CnbDocument.parse(
                     curveFile(""), "ease.cnb", CnbReadLimits.standard())) {
            assertThrows(ContentNotSupportedException.class, () -> empty.load(document));
        }

        // A built-in type carries its identity in its number, so registering it with a name would
        // be claiming a check that does not apply.
        assertThrows(IllegalArgumentException.class, () -> new CnbLoaders<Curve>()
                .register(CnbAssetType.custom("Game.Thing"), CnbDocument::decodeCurve));
    }

    @Test
    void twoCustomTypesThatCollideAreKeptApart() {
        // A custom identifier is a 31-bit hash of the name, so the registry refuses a second
        // registration under a different name for the same identifier -- CNA's rule, kept,
        // because letting the second win would load one game's file with another's loader.
        CnbLoaders<String> loaders = new CnbLoaders<String>()
                .registerCustom("Game.Dialogue", document -> "dialogue");
        assertTrue(loaders.isRegistered(CnbAssetType.custom("Game.Dialogue")));

        // Registering the same name again is accepted and changes nothing, so two initialisation
        // paths registering the same type is not an error.
        loaders.registerCustom("Game.Dialogue", document -> "dialogue");

        CnbAssetType dialogue = CnbAssetType.custom("Game.Dialogue");
        CnbAssetType other = CnbAssetType.custom("Other.Thing");
        if (dialogue.Id() == other.Id()) {
            assertThrows(IllegalStateException.class,
                    () -> loaders.registerCustom("Other.Thing", document -> "other"));
        }
        assertTrue(dialogue.isCustom());
        assertFalse(CnbAssetType.CURVE.isCustom());
    }

    @Test
    void cnaAnswersWhichLoadersItHasOfItsOwn() {
        // The two built-ins that need nothing but their own codec are the two a process can have
        // without constructing a content manager. This is a query: CNA's loaders build C++
        // objects that no Java call could receive, which is the whole reason CnbLoaders exists.
        assertTrue(CnbLoaders.isRegisteredWithCna(CnbAssetType.CURVE),
                "Curve needs only its own codec, so CNA has it");
        assertEquals("Microsoft.Xna.Framework.Curve",
                CnbLoaders.getCnaRegisteredTypeName(CnbAssetType.CURVE));

        // A type whose loader constructs a runtime object is installed by a content manager, not
        // by the registry, so a process without one does not have it. That is the distinction
        // worth being able to ask about.
        assertFalse(CnbLoaders.isRegisteredWithCna(CnbAssetType.custom("Game.Dialogue")),
                "a game's own type is not something CNA knows");
        assertEquals("", CnbLoaders.getCnaRegisteredTypeName(CnbAssetType.custom("Game.Nope")));
    }
}

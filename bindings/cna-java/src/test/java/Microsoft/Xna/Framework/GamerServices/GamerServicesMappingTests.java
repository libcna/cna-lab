package Microsoft.Xna.Framework.GamerServices;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNotSame;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/** Managed behaviour of the GamerServices projection, with no native backend required. */
final class GamerServicesMappingTests {

    @Test
    void sequentialEnumsKeepTheirExactXnaNumbers() {
        assertEquals(0, AvatarBodyType.Female.ordinal());
        assertEquals(1, AvatarBodyType.Male.ordinal());
        assertEquals(0, GamerZone.Unknown.ordinal());
        assertEquals(4, GamerZone.Underground.ordinal());
        assertEquals(0, NotificationPosition.TopLeft.ordinal());
        assertEquals(8, NotificationPosition.BottomRight.ordinal());
        assertEquals(31, AvatarAnimationPreset.values().length);
        assertEquals(60, GamerPresenceMode.values().length);
    }

    @Test
    void nonSequentialAvatarBoneKeepsItsDeclaredNumbers() {
        // AvatarBone's numbers are not contiguous, so the projection carries the exact value
        // instead of relying on the Java ordinal.
        assertEquals(0, AvatarBone.Root.getValue());
        assertEquals(19, AvatarBone.Head.getValue());
        assertEquals(70, AvatarBone.FingerThumb3Right.getValue());
        assertEquals(55, AvatarBone.values().length);
    }

    @Test
    void avatarExpressionIsAMutableValueWithACopyConstructor() {
        AvatarExpression neutral = new AvatarExpression();
        assertEquals(AvatarEye.Neutral, neutral.getLeftEye());
        assertEquals(AvatarMouth.Neutral, neutral.getMouth());

        neutral.setMouth(AvatarMouth.Laughing);
        neutral.setLeftEyebrow(AvatarEyebrow.Raised);
        AvatarExpression copy = new AvatarExpression(neutral);
        assertEquals(AvatarMouth.Laughing, copy.getMouth());

        // A struct copy is independent: writing through one must not reach the other.
        copy.setMouth(AvatarMouth.Sad);
        assertEquals(AvatarMouth.Laughing, neutral.getMouth());
        assertNotSame(neutral, copy);
        assertThrows(NullPointerException.class, () -> copy.setMouth(null));
    }

    @Test
    void leaderboardIdentityCreateNamesTheKeyAndGameMode() {
        LeaderboardIdentity identity = LeaderboardIdentity.Create(LeaderboardKey.BestScoreRecent, 7);
        assertEquals("BestScoreRecent", identity.getKey());
        assertEquals(7, identity.getGameMode());

        LeaderboardIdentity defaulted = LeaderboardIdentity.Create(LeaderboardKey.BestTimeLifeTime);
        assertEquals(0, defaulted.getGameMode());

        LeaderboardIdentity zeroed = new LeaderboardIdentity();
        assertNull(zeroed.getKey());
        assertEquals(0, zeroed.getGameMode());

        LeaderboardIdentity copy = new LeaderboardIdentity(identity);
        copy.setGameMode(9);
        assertEquals(7, identity.getGameMode());
    }

    @Test
    void exceptionsKeepTheirXnaIdentityAndHierarchy() {
        assertTrue(new NetworkNotAvailableException() instanceof NetworkException);
        assertTrue(new GamerServicesNotAvailableException("x") instanceof RuntimeException);
        assertEquals("no live service",
                new GamerPrivilegeException("no live service").getMessage());
        RuntimeException cause = new IllegalStateException("cause");
        assertSame(cause, new GuideAlreadyVisibleException("guide", cause).getCause());
        // CLR serialization constructors have no Java equivalent and are omitted by rule, so
        // the three ordinary CLR constructors are the whole projected surface.
        assertEquals(3, GameUpdateRequiredException.class.getConstructors().length);
    }

    @Test
    void eventArgumentsCarryTheirGamerAndSessionFlag() {
        SignedInEventArgs signedIn = new SignedInEventArgs(null);
        assertNull(signedIn.getGamer());

        InviteAcceptedEventArgs invite = new InviteAcceptedEventArgs(null, true);
        assertTrue(invite.getIsCurrentSession());
        assertNull(invite.getGamer());

        SignedOutEventArgs signedOut = new SignedOutEventArgs(null);
        assertNull(signedOut.getGamer());
    }

    @Test
    void gamerServicesFacilitiesRefuseWhileTheDispatcherIsNotInitialized() {
        // XNA raises GamerServicesNotAvailableException both when the dispatcher was never
        // initialized and when the platform has no gamer services. Both reach the same
        // identity here, so a ported catch block behaves the same way.
        //
        // The dispatcher is process-wide state another test in this JVM may have started, so
        // the assertion is on the condition rather than on the order tests happen to run in.
        if (GamerServicesDispatcher.getIsInitialized()) {
            assertNotNull(Gamer.getSignedInGamers());
            return;
        }
        assertThrows(GamerServicesNotAvailableException.class, Gamer::getSignedInGamers);
        assertThrows(GamerServicesNotAvailableException.class,
                () -> Gamer.GetFromGamertag("someone"));
        assertThrows(GamerServicesNotAvailableException.class, AvatarDescription::CreateRandom);
    }

    @Test
    void avatarRendererDeclaresTheSkeletonSizeXnaDoes() {
        assertEquals(71, AvatarRenderer.BoneCount);
    }
}

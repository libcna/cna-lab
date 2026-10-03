package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.Color;
import Microsoft.Xna.Framework.Matrix;
import Microsoft.Xna.Framework.Quaternion;
import Microsoft.Xna.Framework.Vector2;
import Microsoft.Xna.Framework.Vector3;
import Microsoft.Xna.Framework.Vector4;
import Microsoft.Xna.Framework.GamerServices.NetworkException;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

/** Managed behaviour of the Net projection, with no native backend required. */
final class NetworkMappingTests {

    @Test
    void sendDataOptionsComposeTheWayTheFlagsEnumDoes() {
        // ReliableInOrder is the union of Reliable and InOrder, which is why this is a flags
        // enum and why Java projects it as a composable value rather than an enum constant.
        assertEquals(SendDataOptions.ReliableInOrder,
                SendDataOptions.Reliable.Or(SendDataOptions.InOrder));
        assertTrue(SendDataOptions.ReliableInOrder.Contains(SendDataOptions.Reliable));
        assertTrue(SendDataOptions.ReliableInOrder.Contains(SendDataOptions.InOrder));
        assertFalse(SendDataOptions.Reliable.Contains(SendDataOptions.InOrder));
        assertEquals(4, SendDataOptions.Chat.getValue());
        assertSame(SendDataOptions.Reliable, SendDataOptions.FromValue(1));
        assertEquals(SendDataOptions.None, SendDataOptions.FromValue(0));
    }

    @Test
    void sessionEnumsKeepTheirExactXnaNumbers() {
        assertEquals(0, NetworkSessionType.Local.ordinal());
        assertEquals(1, NetworkSessionType.SystemLink.ordinal());
        assertEquals(4, NetworkSessionType.LocalWithLeaderboards.ordinal());
        assertEquals(0, NetworkSessionEndReason.ClientSignedOut.ordinal());
        assertEquals(3, NetworkSessionEndReason.Disconnected.ordinal());
        assertEquals(2, NetworkSessionJoinError.SessionFull.ordinal());
        assertEquals(2, NetworkSessionState.Ended.ordinal());
    }

    @Test
    void joinExceptionCarriesItsReasonAndDerivesFromNetworkException() {
        NetworkSessionJoinException failure =
                new NetworkSessionJoinException("full", NetworkSessionJoinError.SessionFull);
        assertEquals(NetworkSessionJoinError.SessionFull, failure.getJoinError());
        assertTrue(failure instanceof NetworkException);
        failure.setJoinError(NetworkSessionJoinError.SessionNotJoinable);
        assertEquals(NetworkSessionJoinError.SessionNotJoinable, failure.getJoinError());
        // The serialization constructor is omitted by rule, leaving the four ordinary ones.
        assertEquals(4, NetworkSessionJoinException.class.getConstructors().length);
    }

    @Test
    void sessionLimitsAreTheOnesXnaDeclares() {
        assertEquals(31, NetworkSession.MaxSupportedGamers);
        assertEquals(100, NetworkSession.MaxPreviousGamers);
    }

    @Test
    void packetsRoundTripEveryValueTypeInXnasOwnLayout() {
        PacketWriter writer = new PacketWriter();
        writer.Write(new Vector2(1.0f, 2.0f));
        writer.Write(new Vector3(3.0f, 4.0f, 5.0f));
        writer.Write(new Vector4(6.0f, 7.0f, 8.0f, 9.0f));
        writer.Write(new Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
        writer.Write(Matrix.CreateTranslation(11.0f, 12.0f, 13.0f));
        writer.Write(new Color(10, 20, 30, 40));
        writer.Write(1.5f);
        writer.Write(2.5);
        writer.Write(1234567890);
        writer.Write("hello");

        PacketReader reader = new PacketReader();
        readerOf(reader, writer);

        assertEquals(new Vector2(1.0f, 2.0f), reader.ReadVector2());
        assertEquals(new Vector3(3.0f, 4.0f, 5.0f), reader.ReadVector3());
        assertEquals(new Vector4(6.0f, 7.0f, 8.0f, 9.0f), reader.ReadVector4());
        assertEquals(new Quaternion(0.1f, 0.2f, 0.3f, 0.4f), reader.ReadQuaternion());
        assertEquals(Matrix.CreateTranslation(11.0f, 12.0f, 13.0f), reader.ReadMatrix());
        assertEquals(new Color(10, 20, 30, 40), reader.ReadColor());
        assertEquals(1.5f, reader.ReadSingle());
        assertEquals(2.5, reader.ReadDouble());
        assertEquals(1234567890, reader.ReadInt32());
        assertEquals("hello", reader.ReadString());
    }

    @Test
    void packetPositionAndLengthMeanWhatTheyMeanInXna() {
        PacketWriter writer = new PacketWriter();
        writer.Write(1);
        writer.Write(2);
        assertEquals(8, writer.getLength());
        assertEquals(8, writer.getPosition());

        // Seeking back and writing overwrites in place rather than appending, which is what a
        // MemoryStream-backed BinaryWriter does.
        writer.setPosition(0);
        writer.Write(7);
        assertEquals(8, writer.getLength());
        assertEquals(4, writer.getPosition());

        PacketReader reader = new PacketReader();
        readerOf(reader, writer);
        assertEquals(8, reader.getLength());
        assertEquals(0, reader.getPosition());
        assertEquals(7, reader.ReadInt32());
        assertEquals(4, reader.getPosition());
        reader.setPosition(0);
        assertEquals(7, reader.ReadInt32());
        assertThrows(IndexOutOfBoundsException.class, () -> reader.setPosition(9));
        assertThrows(IndexOutOfBoundsException.class, () -> writer.setPosition(-1));
    }

    private static void readerOf(PacketReader reader, PacketWriter writer) {
        byte[] packet = PacketAccess.data(writer);
        PacketAccess.fill(reader, packet);
    }

    /** Reaches the package-private packet plumbing the network gamer normally drives. */
    private static final class PacketAccess {

        static byte[] data(PacketWriter writer) {
            return writer.data();
        }

        static void fill(PacketReader reader, byte[] packet) {
            reader.fill(packet, packet.length);
        }
    }
}

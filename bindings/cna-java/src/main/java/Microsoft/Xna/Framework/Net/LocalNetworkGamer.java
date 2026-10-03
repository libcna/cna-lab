package Microsoft.Xna.Framework.Net;

import Microsoft.Xna.Framework.GamerServices.SignedInGamer;
import org.openeggbert.cna.internal.GamerFactories;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeNetworkRoutes;

import java.util.Objects;

/**
 * A gamer signed in on this machine and joined to a session.
 *
 * <p>This is the only gamer a title can send from and receive on. Packets cross the boundary as
 * bytes: {@link PacketWriter} builds one in Java and {@code SendData} hands the finished bytes
 * to CNA, while {@code ReceiveData} fills a {@link PacketReader} from the bytes CNA hands back.
 */
public final class LocalNetworkGamer extends NetworkGamer {

    LocalNetworkGamer(long handle) {
        super(handle);
    }

    /**
     * XNA's voice control. CNA declares it a no-op, so it is accepted and does nothing rather
     * than pretending to route voice.
     */
    public void EnableSendVoice(NetworkGamer remoteGamer, boolean enable) {
        NativeGamerServices.check("LocalNetworkGamer.EnableSendVoice",
                NativeNetworkRoutes.localNetworkGamerEnableSendVoice(handle(),
                        Objects.requireNonNull(remoteGamer, "remoteGamer").handle(), enable));
    }

    public void SendData(PacketWriter data, SendDataOptions options, NetworkGamer recipient) {
        send(bytes(data), 0, bytes(data).length, options, recipient);
        data.reset();
    }

    public void SendData(PacketWriter data, SendDataOptions options) {
        byte[] packet = bytes(data);
        send(packet, 0, packet.length, options, null);
        data.reset();
    }

    public void SendData(int[] data, int offset, int count, SendDataOptions options,
            NetworkGamer recipient) {
        send(bytes(data), offset, count, options, recipient);
    }

    public void SendData(int[] data, int offset, int count, SendDataOptions options) {
        send(bytes(data), offset, count, options, null);
    }

    public void SendData(int[] data, SendDataOptions options, NetworkGamer recipient) {
        send(bytes(data), 0, data.length, options, recipient);
    }

    public void SendData(int[] data, SendDataOptions options) {
        send(bytes(data), 0, data.length, options, null);
    }

    /** XNA's party invitation. CNA declares it a no-op, and it is projected as one. */
    public void SendPartyInvites() {
        NativeGamerServices.check("LocalNetworkGamer.SendPartyInvites",
                NativeNetworkRoutes.localNetworkGamerSendPartyInvites(handle()));
    }

    /**
     * Reads the next queued packet into a {@link PacketReader}.
     *
     * <p>CLR pairs a result with an {@code out NetworkGamer}. Java has no {@code out}
     * parameter, so the pair becomes one {@link ReceiveResult}, the same rule
     * {@code GraphicsAdapter.QueryBackBufferFormat} follows.
     */
    public ReceiveResult ReceiveData(PacketReader data) {
        Objects.requireNonNull(data, "data");
        byte[] destination = new byte[MAXIMUM_PACKET_BYTES];
        ReceiveResult result = receive(destination, 0);
        if (result.getSucceeded()) {
            data.fill(destination, result.getBytesRead());
        }
        return result;
    }

    /** Reads the next queued packet into a caller-supplied buffer. */
    public ReceiveResult ReceiveData(int[] data) {
        return ReceiveData(data, 0);
    }

    /** Reads the next queued packet into a caller-supplied buffer at an offset. */
    public ReceiveResult ReceiveData(int[] data, int offset) {
        Objects.requireNonNull(data, "data");
        Objects.checkFromIndexSize(offset, 0, data.length);
        byte[] destination = new byte[Math.max(data.length - offset, 0)];
        ReceiveResult result = receive(destination, offset);
        for (int index = 0; index < result.getBytesRead(); index++) {
            data[offset + index] = destination[index] & 0xFF;
        }
        return result;
    }

    /**
     * Takes the next queued packet from CNA.
     *
     * <p>A read into the whole buffer takes CNA's whole-buffer route, as the send does.
     */
    private ReceiveResult receive(byte[] destination, int offset) {
        long[] senderHandle = new long[1];
        long[] received = new long[1];
        NativeGamerServices.check("LocalNetworkGamer.ReceiveData", offset == 0
                ? NativeNetworkRoutes.localNetworkGamerReceiveData(
                        handle(), destination, senderHandle, received)
                : NativeNetworkRoutes.localNetworkGamerReceiveDataAt(
                        handle(), destination, offset, senderHandle, received));
        boolean succeeded = received[0] != 0L || senderHandle[0] != 0L;
        return new ReceiveResult(succeeded, Math.toIntExact(received[0]),
                senderHandle[0] == 0L ? null : new NetworkGamer(senderHandle[0]));
    }

    /**
     * One received packet's outcome: whether a packet was available, how many bytes it
     * carried, and which gamer sent it.
     */
    public static final class ReceiveResult {

        private final boolean succeeded;
        private final int bytesRead;
        private final NetworkGamer sender;

        ReceiveResult(boolean succeeded, int bytesRead, NetworkGamer sender) {
            this.succeeded = succeeded;
            this.bytesRead = bytesRead;
            this.sender = sender;
        }

        public boolean getSucceeded() {
            return succeeded;
        }

        public int getBytesRead() {
            return bytesRead;
        }

        public NetworkGamer getSender() {
            return sender;
        }
    }

    public boolean getIsDataAvailable() {
        boolean[] value = new boolean[1];
        NativeGamerServices.check("LocalNetworkGamer.IsDataAvailable",
                NativeNetworkRoutes.localNetworkGamerGetIsDataAvailable(handle(), value));
        return value[0];
    }

    public SignedInGamer getSignedInGamer() {
        long[] gamer = new long[1];
        NativeGamerServices.check("LocalNetworkGamer.SignedInGamer",
                NativeNetworkRoutes.localNetworkGamerGetSignedInGamer(handle(), gamer));
        return gamer[0] == 0L ? null
                : (SignedInGamer) GamerFactories.createSignedInGamer(gamer[0]);
    }

    /** The largest packet XNA lets a title send, and therefore the receive buffer's size. */
    private static final int MAXIMUM_PACKET_BYTES = 1024;

    /**
     * Hands one packet to CNA.
     *
     * <p>A send of the whole array takes CNA's whole-buffer route and a windowed one takes the
     * windowed route, which is the same rule {@code VertexBuffer.SetData} follows: the narrower
     * route exists, and taking the wide one for every send would make CNA re-derive a window the
     * caller did not ask for.
     */
    private void send(byte[] data, int offset, int count, SendDataOptions options,
            NetworkGamer recipient) {
        Objects.requireNonNull(options, "options");
        boolean whole = offset == 0 && count == data.length;
        int result;
        if (recipient == null) {
            result = whole
                    ? NativeNetworkRoutes.localNetworkGamerSendData(
                            handle(), data, options.getValue())
                    : NativeNetworkRoutes.localNetworkGamerSendDataRange(
                            handle(), data, offset, count, options.getValue());
        } else {
            result = whole
                    ? NativeNetworkRoutes.localNetworkGamerSendDataTo(
                            handle(), data, options.getValue(), recipient.handle())
                    : NativeNetworkRoutes.localNetworkGamerSendDataRangeTo(
                            handle(), data, offset, count, options.getValue(),
                            recipient.handle());
        }
        NativeGamerServices.check("LocalNetworkGamer.SendData", result);
    }

    private static byte[] bytes(PacketWriter data) {
        return Objects.requireNonNull(data, "data").data();
    }

    /** The project maps CLR {@code Byte} to a range-checked Java {@code int}. */
    private static byte[] bytes(int[] data) {
        Objects.requireNonNull(data, "data");
        byte[] packet = new byte[data.length];
        for (int index = 0; index < data.length; index++) {
            int value = data[index];
            if (value < 0 || value > 255) {
                throw new IllegalArgumentException(
                        "data[" + index + "] is " + value + ", outside the byte range 0..255");
            }
            packet[index] = (byte) value;
        }
        return packet;
    }
}

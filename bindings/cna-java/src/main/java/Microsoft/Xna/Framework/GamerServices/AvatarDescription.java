package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.EventArgs;
import Microsoft.Xna.Framework.EventHandler;
import org.openeggbert.cna.internal.CompletedAsyncResult;
import org.openeggbert.cna.internal.NativeDeferredRelease;
import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;
import System.AsyncCallback;
import System.IAsyncResult;

import java.util.List;
import java.util.Objects;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The appearance data of one avatar.
 *
 * <p>A description is opaque bytes: XNA never documents their layout, and this projection does
 * not invent one. {@code IsValid} is what a title checks before handing a description to an
 * {@link AvatarRenderer}.
 */
public class AvatarDescription {

    private final List<EventHandler<EventArgs>> changed = new CopyOnWriteArrayList<>();
    private final long handle;

    /**
     * Adopts one description handle.
     *
     * <p>Every route that produces a description hands back an owned handle -- the byte
     * constructor, both random factories and the read from a gamer -- so the release is
     * registered here rather than at four call sites. XNA's type is not disposable, so it
     * happens once this object is unreachable, on the thread that created it, the next time that
     * thread pumps.
     */
    @SuppressWarnings("this-escape")
    AvatarDescription(long handle) {
        this.handle = handle;
        NativeDeferredRelease.onOwningThread(this, handle,
                NativeGamerServicesRoutes::avatarDescriptionDestroy,
                "cna_avatar_description_destroy");
    }

    /**
     * Builds a description from previously stored bytes.
     *
     * <p>XNA's parameter is a {@code byte[]}; the project maps CLR {@code Byte} to a
     * range-checked Java {@code int}, so the array is {@code int[]} with every element in
     * 0..255.
     */
    @SuppressWarnings("this-escape")
    public AvatarDescription(int[] data) {
        Objects.requireNonNull(data, "data");
        NativeGamerServices.requireAvailable("AvatarDescription");
        byte[] bytes = new byte[data.length];
        for (int index = 0; index < data.length; index++) {
            int value = data[index];
            if (value < 0 || value > 255) {
                throw new IllegalArgumentException(
                        "data[" + index + "] is " + value + ", outside the byte range 0..255");
            }
            bytes[index] = (byte) value;
        }
        long[] description = new long[1];
        NativeGamerServices.check("AvatarDescription",
                NativeGamerServicesRoutes.avatarDescriptionCreate(bytes, description));
        handle = description[0];
        NativeDeferredRelease.onOwningThread(this, handle,
                NativeGamerServicesRoutes::avatarDescriptionDestroy,
                "cna_avatar_description_destroy");
    }

    long handle() {
        return handle;
    }

    public final void addChangedListener(EventHandler<EventArgs> listener) {
        changed.add(Objects.requireNonNull(listener, "listener"));
    }

    public final void removeChangedListener(EventHandler<EventArgs> listener) {
        changed.remove(Objects.requireNonNull(listener, "listener"));
    }

    public static IAsyncResult BeginGetFromGamer(
            Gamer gamer, AsyncCallback callback, Object state) {
        return CompletedAsyncResult.begin(callback, state, () -> fromGamer(gamer));
    }

    public static AvatarDescription CreateRandom() {
        NativeGamerServices.requireAvailable("AvatarDescription.CreateRandom");
        long[] description = new long[1];
        NativeGamerServices.check("AvatarDescription.CreateRandom",
                NativeGamerServicesRoutes.avatarDescriptionCreateRandom(description));
        return new AvatarDescription(description[0]);
    }

    public static AvatarDescription CreateRandom(AvatarBodyType bodyType) {
        Objects.requireNonNull(bodyType, "bodyType");
        NativeGamerServices.requireAvailable("AvatarDescription.CreateRandom");
        long[] description = new long[1];
        NativeGamerServices.check("AvatarDescription.CreateRandom",
                NativeGamerServicesRoutes.avatarDescriptionCreateRandomForBodyType(
                        bodyType.ordinal(), description));
        return new AvatarDescription(description[0]);
    }

    public static AvatarDescription EndGetFromGamer(IAsyncResult result) {
        return CompletedAsyncResult.end(result, AvatarDescription.class);
    }

    public final AvatarBodyType getBodyType() {
        return AvatarBodyType.values()[(int) integers()[0]];
    }

    /** Returns the opaque description bytes, as range-checked {@code int} values. */
    public final int[] getDescription() {
        long[] values = integers();
        byte[] bytes = new byte[Math.toIntExact(values[1])];
        long[] written = new long[1];
        NativeGamerServices.check("AvatarDescription.Description",
                NativeGamerServicesRoutes.avatarDescriptionCopyDescription(
                        handle, bytes, written));
        int[] description = new int[Math.toIntExact(written[0])];
        for (int index = 0; index < description.length; index++) {
            description[index] = bytes[index] & 0xFF;
        }
        return description;
    }

    public final float getHeight() {
        float[] height = new float[1];
        NativeGamerServices.check("AvatarDescription.Height",
                NativeGamerServicesRoutes.avatarDescriptionGetInfo(
                        handle, new byte[7], new long[3], height));
        return height[0];
    }

    public final boolean getIsValid() {
        return integers()[2] != 0L;
    }

    private static AvatarDescription fromGamer(Gamer gamer) {
        Objects.requireNonNull(gamer, "gamer");
        long[] description = new long[1];
        NativeGamerServices.check("AvatarDescription.GetFromGamer",
                NativeGamerServicesRoutes.avatarDescriptionGetFromGamer(
                        gamer.handle(), description));
        return new AvatarDescription(description[0]);
    }

    private long[] integers() {
        long[] values = new long[3];
        NativeGamerServices.check("AvatarDescription",
                NativeGamerServicesRoutes.avatarDescriptionGetInfo(
                        handle, new byte[7], values, new float[1]));
        return values;
    }
}

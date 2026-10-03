package Microsoft.Xna.Framework.GamerServices;

import org.openeggbert.cna.internal.NativeGamerServices;
import org.openeggbert.cna.internal.generated.NativeGamerServicesRoutes;
import System.IO.Stream;

import java.io.ByteArrayInputStream;
import java.util.Locale;

/** The public profile of one gamer: score, zone, reputation, motto and region. */
public final class GamerProfile implements AutoCloseable {

    private final long handle;
    private boolean disposed;

    GamerProfile(long handle) {
        this.handle = handle;
    }

    public void Dispose() {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }
        NativeGamerServices.check("GamerProfile.Dispose",
                NativeGamerServicesRoutes.gamerProfileDestroy(handle));
    }

    @Override
    public void close() {
        Dispose();
    }

    /**
     * Returns the gamer's picture as a readable stream.
     *
     * <p>A gamer with no picture yields an empty stream rather than a failure, which is what
     * reading a zero-length picture does.
     */
    public Stream GetGamerPicture() {
        boolean[] hasPicture = new boolean[1];
        long[] bytes = new long[1];
        NativeGamerServices.check("GamerProfile.GetGamerPicture",
                NativeGamerServicesRoutes.gamerProfileGetPictureSize(handle, hasPicture, bytes));
        if (!hasPicture[0] || bytes[0] == 0L) {
            return new Stream(new ByteArrayInputStream(new byte[0]));
        }
        byte[] picture = new byte[Math.toIntExact(bytes[0])];
        NativeGamerServices.check("GamerProfile.GetGamerPicture",
                NativeGamerServicesRoutes.gamerProfileCopyPicture(handle, picture, bytes));
        return new Stream(new ByteArrayInputStream(picture, 0, Math.toIntExact(bytes[0])));
    }

    public int getGamerScore() {
        return (int) integers()[0];
    }

    public GamerZone getGamerZone() {
        return GamerZone.values()[(int) integers()[1]];
    }

    public boolean getIsDisposed() {
        return disposed || integers()[4] != 0L;
    }

    public String getMotto() {
        return NativeGamerServices.text("GamerProfile.Motto",
                out -> NativeGamerServicesRoutes.gamerProfileGetMottoSize(handle, out),
                (buffer, out) -> NativeGamerServicesRoutes.gamerProfileCopyMotto(
                        handle, buffer, out));
    }

    /**
     * Returns the gamer's region.
     *
     * <p>CLR {@code RegionInfo} projects to {@link Locale}, the Java type that carries a
     * country. An empty region name yields {@link Locale#ROOT} rather than a fabricated
     * country.
     */
    public Locale getRegion() {
        String name = NativeGamerServices.text("GamerProfile.Region",
                out -> NativeGamerServicesRoutes.gamerProfileGetRegionNameSize(handle, out),
                (buffer, out) -> NativeGamerServicesRoutes.gamerProfileCopyRegionName(
                        handle, buffer, out));
        return name.isEmpty() ? Locale.ROOT : new Locale("", name);
    }

    public float getReputation() {
        float[] values = new float[1];
        NativeGamerServices.check("GamerProfile.Reputation",
                NativeGamerServicesRoutes.gamerProfileGetInfo(handle, new byte[3], new long[5], values));
        return values[0];
    }

    public int getTitlesPlayed() {
        return (int) integers()[2];
    }

    public int getTotalAchievements() {
        return (int) integers()[3];
    }

    private long[] integers() {
        long[] values = new long[5];
        NativeGamerServices.check("GamerProfile",
                NativeGamerServicesRoutes.gamerProfileGetInfo(handle, new byte[3], values, new float[1]));
        return values;
    }
}

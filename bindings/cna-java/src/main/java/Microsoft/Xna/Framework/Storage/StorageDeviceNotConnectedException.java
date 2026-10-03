package Microsoft.Xna.Framework.Storage;

/** The selected storage device is no longer connected. */
public class StorageDeviceNotConnectedException extends RuntimeException {
    private static final long serialVersionUID = 1L;

    public StorageDeviceNotConnectedException() { super(); }
    public StorageDeviceNotConnectedException(String message) { super(message); }
    public StorageDeviceNotConnectedException(String message, RuntimeException innerException) {
        super(message, innerException);
    }
}

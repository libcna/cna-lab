package Microsoft.Xna.Framework.Storage;

import System.IO.FileAccess;
import System.IO.FileMode;
import System.IO.FileShare;
import System.IO.SeekOrigin;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

final class StorageManagedBehaviorTests {

    @Test
    void FileIdentitiesMatchTheClrContractsUsedByXnaStorage() {
        assertArrayEquals(new FileMode[] {
                FileMode.CreateNew, FileMode.Create, FileMode.Open,
                FileMode.OpenOrCreate, FileMode.Truncate, FileMode.Append
        }, FileMode.values());
        assertEquals(1, FileMode.CreateNew.getValue());
        assertEquals(6, FileMode.Append.getValue());
        assertEquals(1, FileAccess.Read.getValue());
        assertEquals(2, FileAccess.Write.getValue());
        assertEquals(3, FileAccess.ReadWrite.getValue());
        assertArrayEquals(new SeekOrigin[] {
                SeekOrigin.Begin, SeekOrigin.Current, SeekOrigin.End
        }, SeekOrigin.values());

        FileShare combined = FileShare.Read.Or(FileShare.Write).Or(FileShare.Delete);
        assertEquals(7, combined.getValue());
        assertTrue(combined.Contains(FileShare.ReadWrite));
        assertTrue(combined.Contains(FileShare.Delete));
        assertFalse(combined.Contains(FileShare.Inheritable));
        assertEquals(FileShare.ReadWrite, FileShare.FromValue(3));
        assertThrows(IllegalArgumentException.class, () -> FileShare.FromValue(8));
    }

    @Test
    void StorageExceptionPreservesMessageAndMappedInnerException() {
        RuntimeException inner = new RuntimeException("inner");
        StorageDeviceNotConnectedException empty =
                new StorageDeviceNotConnectedException();
        StorageDeviceNotConnectedException message =
                new StorageDeviceNotConnectedException("offline");
        StorageDeviceNotConnectedException nested =
                new StorageDeviceNotConnectedException("offline", inner);

        assertNull(empty.getMessage());
        assertEquals("offline", message.getMessage());
        assertEquals("offline", nested.getMessage());
        assertSame(inner, nested.getCause());
    }

    @Test
    void SelectorValidationPrecedesNativeGameLookupWhereXnaValidatesAtBegin() {
        assertThrows(NullPointerException.class,
                () -> StorageDevice.BeginShowSelector(null, null, null));
        assertThrows(IllegalArgumentException.class,
                () -> StorageDevice.BeginShowSelector(-1, 0, null, null));
    }
}

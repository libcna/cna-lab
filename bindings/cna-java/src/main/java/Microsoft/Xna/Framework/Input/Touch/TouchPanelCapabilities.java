package Microsoft.Xna.Framework.Input.Touch;

import java.util.Objects;

/** Value snapshot of current touch-device availability and capacity. */
public final class TouchPanelCapabilities {

    private boolean connected;
    private int maximumTouchCount;

    public TouchPanelCapabilities() {
    }

    public TouchPanelCapabilities(TouchPanelCapabilities value) {
        TouchPanelCapabilities source = Objects.requireNonNull(value, "value");
        connected = source.connected;
        maximumTouchCount = source.maximumTouchCount;
    }

    TouchPanelCapabilities(boolean connected, int maximumTouchCount) {
        this.connected = connected;
        this.maximumTouchCount = maximumTouchCount;
    }

    public boolean getIsConnected() {
        return connected;
    }

    protected void setIsConnected(boolean value) {
        connected = value;
    }

    public int getMaximumTouchCount() {
        return maximumTouchCount;
    }

    protected void setMaximumTouchCount(int value) {
        maximumTouchCount = value;
    }
}

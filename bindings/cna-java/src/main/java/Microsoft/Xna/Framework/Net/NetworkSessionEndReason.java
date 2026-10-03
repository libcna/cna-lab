package Microsoft.Xna.Framework.Net;

/** Identifies why a network session ended. */
public enum NetworkSessionEndReason {
    ClientSignedOut,
    HostEndedSession,
    RemovedByHost,
    Disconnected;
}

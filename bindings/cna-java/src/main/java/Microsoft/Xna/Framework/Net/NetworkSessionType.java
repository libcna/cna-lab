package Microsoft.Xna.Framework.Net;

/** Identifies the kind of network session to create or find. */
public enum NetworkSessionType {
    Local,
    SystemLink,
    PlayerMatch,
    Ranked,
    LocalWithLeaderboards;
}

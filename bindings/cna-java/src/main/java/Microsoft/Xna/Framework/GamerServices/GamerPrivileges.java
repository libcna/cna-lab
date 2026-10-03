package Microsoft.Xna.Framework.GamerServices;

/**
 * The privileges a signed-in gamer's account grants.
 *
 * <p>The values are a snapshot taken when the owning {@link SignedInGamer} is asked for them,
 * which is what XNA's own property returns: a live account change is observed by asking again.
 */
public final class GamerPrivileges {

    private final GamerPrivilegeSetting allowCommunication;
    private final GamerPrivilegeSetting allowProfileViewing;
    private final GamerPrivilegeSetting allowUserCreatedContent;
    private final boolean allowOnlineSessions;
    private final boolean allowPremiumContent;
    private final boolean allowPurchaseContent;
    private final boolean allowTradeContent;

    GamerPrivileges(long[] values) {
        allowCommunication = GamerPrivilegeSetting.values()[(int) values[0]];
        allowProfileViewing = GamerPrivilegeSetting.values()[(int) values[1]];
        allowUserCreatedContent = GamerPrivilegeSetting.values()[(int) values[2]];
        allowOnlineSessions = values[3] != 0L;
        allowPremiumContent = values[4] != 0L;
        allowPurchaseContent = values[5] != 0L;
        allowTradeContent = values[6] != 0L;
    }

    public GamerPrivilegeSetting getAllowCommunication() {
        return allowCommunication;
    }

    public boolean getAllowOnlineSessions() {
        return allowOnlineSessions;
    }

    public boolean getAllowPremiumContent() {
        return allowPremiumContent;
    }

    public GamerPrivilegeSetting getAllowProfileViewing() {
        return allowProfileViewing;
    }

    public boolean getAllowPurchaseContent() {
        return allowPurchaseContent;
    }

    public boolean getAllowTradeContent() {
        return allowTradeContent;
    }

    public GamerPrivilegeSetting getAllowUserCreatedContent() {
        return allowUserCreatedContent;
    }
}

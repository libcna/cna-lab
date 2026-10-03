package Microsoft.Xna.Framework;

import java.util.Objects;

/** Event payload for component collection insertion and removal. */
public class GameComponentCollectionEventArgs extends EventArgs {

    private final IGameComponent gameComponent;

    public GameComponentCollectionEventArgs(IGameComponent gameComponent) {
        this.gameComponent = Objects.requireNonNull(gameComponent, "gameComponent");
    }

    public final IGameComponent getGameComponent() {
        return gameComponent;
    }
}

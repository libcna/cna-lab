package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Game;
import Microsoft.Xna.Framework.GameComponent;
import Microsoft.Xna.Framework.GameTime;
import org.openeggbert.cna.internal.NativeBindings;

/**
 * XNA's game-component bridge to the process-wide GamerServices dispatcher.
 *
 * <p>This selected-profile type exposes only the component lifecycle. It does not project the
 * separate Gamer, Guide, Avatar, or networking APIs.</p>
 */
public class GamerServicesComponent extends GameComponent {

    public GamerServicesComponent(Game game) {
        super(game);
    }

    @Override
    public void Initialize() {
        NativeBindings.initializeGamerServices(getGame());
        super.Initialize();
    }

    @Override
    public void Update(GameTime gameTime) {
        NativeBindings.updateGamerServices(getGame());
        super.Update(gameTime);
    }
}

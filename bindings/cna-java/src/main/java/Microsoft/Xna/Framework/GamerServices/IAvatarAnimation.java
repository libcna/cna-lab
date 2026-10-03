package Microsoft.Xna.Framework.GamerServices;

import Microsoft.Xna.Framework.Matrix;

import java.time.Duration;
import java.util.List;

/**
 * An animation an {@link AvatarRenderer} can draw.
 *
 * <p>A title implements this to drive an avatar from its own animation data instead of from
 * one of XNA's built-in presets.
 */
public interface IAvatarAnimation {

    /** Advances the animation, wrapping to the start when {@code loop} is set. */
    void Update(Duration elapsedAnimationTime, boolean loop);

    /** Returns the bone transforms for the current position, in avatar skeleton order. */
    List<Matrix> getBoneTransforms();

    Duration getCurrentPosition();

    void setCurrentPosition(Duration value);

    /** Returns the facial expression for the current position. */
    AvatarExpression getExpression();

    Duration getLength();
}

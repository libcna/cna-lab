package Microsoft.Xna.Framework.GamerServices;

import java.util.Objects;

/**
 * Mutable value describing an avatar's facial expression.
 *
 * <p>XNA declares this as a struct, so a copy is taken wherever a value would be copied in
 * C#. The parameterless constructor produces the all-{@code Neutral} default a zeroed struct
 * has, and the copy constructor is the Java stand-in for struct assignment.
 */
public final class AvatarExpression {

    private AvatarEye leftEye = AvatarEye.Neutral;
    private AvatarEyebrow leftEyebrow = AvatarEyebrow.Neutral;
    private AvatarMouth mouth = AvatarMouth.Neutral;
    private AvatarEye rightEye = AvatarEye.Neutral;
    private AvatarEyebrow rightEyebrow = AvatarEyebrow.Neutral;

    public AvatarExpression() {
    }

    public AvatarExpression(AvatarExpression value) {
        AvatarExpression source = Objects.requireNonNull(value, "value");
        leftEye = source.leftEye;
        leftEyebrow = source.leftEyebrow;
        mouth = source.mouth;
        rightEye = source.rightEye;
        rightEyebrow = source.rightEyebrow;
    }

    public AvatarEye getLeftEye() {
        return leftEye;
    }

    public void setLeftEye(AvatarEye value) {
        leftEye = Objects.requireNonNull(value, "value");
    }

    public AvatarEyebrow getLeftEyebrow() {
        return leftEyebrow;
    }

    public void setLeftEyebrow(AvatarEyebrow value) {
        leftEyebrow = Objects.requireNonNull(value, "value");
    }

    public AvatarMouth getMouth() {
        return mouth;
    }

    public void setMouth(AvatarMouth value) {
        mouth = Objects.requireNonNull(value, "value");
    }

    public AvatarEye getRightEye() {
        return rightEye;
    }

    public void setRightEye(AvatarEye value) {
        rightEye = Objects.requireNonNull(value, "value");
    }

    public AvatarEyebrow getRightEyebrow() {
        return rightEyebrow;
    }

    public void setRightEyebrow(AvatarEyebrow value) {
        rightEyebrow = Objects.requireNonNull(value, "value");
    }
}

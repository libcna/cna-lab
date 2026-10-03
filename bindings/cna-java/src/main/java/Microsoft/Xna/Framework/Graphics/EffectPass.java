package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;

/** Parent-owned stable view of one Effect pass. */
public final class EffectPass {

    private final Effect owner;
    private EffectAnnotationCollection annotations;

    EffectPass(Effect owner, long nativeHandle) {
        this.owner = owner;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 7);
    }

    public final String getName() {
        requireAlive();
        return NativeBindings.getEffectString(this, 1);
    }

    public final EffectAnnotationCollection getAnnotations() {
        requireAlive();
        if (annotations == null) {
            annotations = new EffectAnnotationCollection(
                    owner, NativeBindings.getEffectMemberCollection(this, 5));
        }
        return annotations;
    }

    public final void Apply() {
        requireAlive();
        NativeBindings.applyEffectPass(this);
    }

    final void requireAlive() {
        owner.requireEffectAlive();
        NativeBindings.requireEffectMember(this);
    }
}

package Microsoft.Xna.Framework.Graphics;

import org.openeggbert.cna.internal.NativeBindings;

/** Parent-owned stable view of one Effect technique. */
public final class EffectTechnique {

    private final Effect owner;
    private final int index;
    private EffectPassCollection passes;
    private EffectAnnotationCollection annotations;

    EffectTechnique(Effect owner, int index, long nativeHandle) {
        this.owner = owner;
        this.index = index;
        NativeBindings.registerEffectMember(owner, this, nativeHandle, 6);
    }

    public final String getName() {
        requireAlive();
        return NativeBindings.getEffectString(this, 0);
    }

    public final EffectPassCollection getPasses() {
        requireAlive();
        if (passes == null) {
            passes = new EffectPassCollection(
                    owner, this, NativeBindings.getEffectMemberCollection(this, 3));
        }
        return passes;
    }

    public final EffectAnnotationCollection getAnnotations() {
        requireAlive();
        if (annotations == null) {
            annotations = new EffectAnnotationCollection(
                    owner, NativeBindings.getEffectMemberCollection(this, 4));
        }
        return annotations;
    }

    final Effect getOwner() {
        return owner;
    }

    final int getIndex() {
        return index;
    }

    final void requireAlive() {
        owner.requireEffectAlive();
        NativeBindings.requireEffectMember(this);
    }
}

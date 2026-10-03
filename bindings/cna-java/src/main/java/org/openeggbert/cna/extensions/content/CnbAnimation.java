package org.openeggbert.cna.extensions.content;

/**
 * One animation clip a compiled model carries.
 *
 * <p>This is the description a model carries: a name, a duration and how many bones the clip
 * animates. The poses themselves live in the clip, which
 * {@link CnbModelData#addAnimation(String, CnbClip, CnbClipTargetSpace)} puts there and
 * {@link CnbClip} holds.
 *
 * <p>Adding one used to be impossible from Java, because CNA takes a clip as a descriptor whose
 * fields are pointers to arrays of descriptors -- clip to tracks to keyframes -- and the JNI
 * generator refuses a shape whose lifetimes it cannot read off the declaration. It refuses it
 * still, and correctly; what changed is that the lifetimes turned out to be stated rather than
 * unknown, so the graph is built by hand for the duration of one call.
 *
 * @param Name the clip's name
 * @param DurationSeconds how long the clip runs
 * @param TrackCount how many bones the clip animates
 * @param TargetSpace which index space the tracks' bone indices live in
 */
public record CnbAnimation(
        String Name, double DurationSeconds, int TrackCount, CnbClipTargetSpace TargetSpace) {
}

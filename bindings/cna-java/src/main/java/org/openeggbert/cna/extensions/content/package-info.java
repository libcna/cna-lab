/**
 * CNA's own content objects, alongside the XNA content pipeline rather than instead of it.
 *
 * <p>A CNA extension. {@code ContentManager.Load} keeps producing the XNA types a game expects,
 * with the effect subclasses XNB names and XNA's contract exposes; nothing here changes that.
 * What is here is the same graph seen through CNA's runtime, which can draw it in one call,
 * compute its transforms, and answer the questions XNA's model has nowhere to put: cameras
 * authored in the scene, skins, and material variants.
 */
package org.openeggbert.cna.extensions.content;

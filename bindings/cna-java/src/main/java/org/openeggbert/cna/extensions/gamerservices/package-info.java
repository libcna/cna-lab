/**
 * The gamer-services capabilities CNA has and XNA 4.0 has no member for.
 *
 * <p>A CNA extension. XNA's {@code Guide} draws its own on-screen keyboard and message box,
 * because on an Xbox the system does; CNA has no system overlay, so it hands the request to the
 * application instead. That is a real difference in who is responsible, and it needs an API the
 * reference contract cannot express -- so it lives here rather than as members Microsoft never
 * shipped.
 */
package org.openeggbert.cna.extensions.gamerservices;

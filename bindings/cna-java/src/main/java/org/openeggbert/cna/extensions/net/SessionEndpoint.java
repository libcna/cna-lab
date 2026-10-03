package org.openeggbert.cna.extensions.net;

/**
 * Where a discovered session's host accepts connections.
 *
 * @param Host the address the session was found at, as the transport reported it
 * @param Port the port the host listens on
 */
public record SessionEndpoint(String Host, int Port) {
}

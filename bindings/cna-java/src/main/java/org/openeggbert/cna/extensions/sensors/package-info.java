/**
 * CNA's host motion sensors: the accelerometer, gyroscope, compass and fused motion.
 *
 * <p>A CNA extension. The pinned XNA 4.0 reference corpus this binding is measured against is the
 * desktop runtime, which has no sensor API at all; the sensor classes the original framework
 * shipped lived in {@code Microsoft.Devices.Sensors.dll}, a Windows Phone assembly that is not
 * part of that corpus. Nothing here belongs in {@code Microsoft.Xna.Framework}.
 *
 * <p>Every sensor is created even where the host has none: {@link
 * org.openeggbert.cna.extensions.sensors.Accelerometer#getIsSupported()} and the sensor's own
 * {@code getState()} say which case it is, and a reading that is a real measurement is told from
 * the zeroed default by {@code getIsDataValid()}. A desktop machine reporting no sensor is an
 * ordinary answer, not a failure.
 */
package org.openeggbert.cna.extensions.sensors;

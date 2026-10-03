/* SPDX-License-Identifier: MS-PL */
/*
 * Can a sensor be qualified on a machine with no sensors?
 *
 * A hundred and two sensor routes were unbound behind "host sensors are a CNA device extension;
 * XNA 4.0 exposes no sensor API on the desktop profile", which explains why they are not XNA and
 * says nothing about why they were absent. The assumption behind leaving them was the obvious
 * one: an accelerometer API needs an accelerometer. The camera family made exactly that
 * assumption a session earlier and it was wrong, so this asks rather than assumes.
 *
 * What it establishes before any Java is written:
 *
 *   1. what a sensor answers on a machine with none, and whether that is a refusal or a state;
 *   2. whether a test backend or a supported flag makes it reachable;
 *   3. whether `inject_synthetic_update_ext` produces a reading, and in which unit -- the header
 *      says the input is PLATFORM units and the reading is canonical, so 9.80665 m/s-squared
 *      should come back as 1 g, and that is arithmetic a test can assert;
 *   4. whether a subscription fires on an injection, on which thread, and whether it is
 *      synchronous with the call;
 *   5. whether `dispose` differs from `destroy` and what the handle does afterwards;
 *   6. whether the game is still destroyable at the end, which is what disqualified other
 *      families here.
 *
 * Every out-parameter is poisoned before the call. The dialogs probe in this directory nearly
 * filed five findings that were only its own argument-evaluation order, so every call below is a
 * statement of its own and its outputs are read on the next line.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "CNA/C/cna.h"
#include "CNA/C/sensors.h"

static int ran = 0;
static pthread_t main_thread;

static const char* name_of(const CNA_Result result)
{
    switch ((int)result) {
        case 0: return "SUCCESS";
        case 1: return "INVALID_ARGUMENT";
        case 2: return "INVALID_HANDLE";
        case 3: return "INVALID_STATE";
        case 6: return "NOT_SUPPORTED";
        case 12: return "INTERNAL";
        default: return "OTHER";
    }
}

#define PROBE_POISON ((CNA_Bool)0xAB)
#define POISON_FLOAT (-424242.0f)

static const char* flag_of(const CNA_Bool flag)
{
    if (flag == PROBE_POISON) return "UNTOUCHED";
    return flag ? "yes" : "no";
}

/* ---- what a subscription sees ---- */

static int reading_calls = 0;
static int reading_inside_inject = 0;
static int inside_inject = 0;
static pthread_t reading_thread;
static CNA_Vector3 last_acceleration;
static void* last_context = NULL;

static void on_reading(const CNA_AccelerometerReading* reading, void* context)
{
    reading_calls++;
    reading_thread = pthread_self();
    last_context = context;
    if (inside_inject) reading_inside_inject = 1;
    if (reading != NULL) last_acceleration = reading->acceleration;
}

static int event_calls = 0;

static void on_event(void* context)
{
    (void)context;
    event_calls++;
}

static void accelerometer(CNA_Handle game)
{
    printf("== accelerometer ==\n");

    CNA_Bool supported = PROBE_POISON;
    CNA_Result step = cna_accelerometer_get_is_supported(game, &supported);
    printf("  is_supported (host)     %s %s\n", name_of(step), flag_of(supported));

    CNA_AccelerometerHandle sensor = 0;
    step = cna_accelerometer_create(game, &sensor);
    printf("  create                  %s\n", name_of(step));
    if (step != CNA_RESULT_SUCCESS) {
        printf("\n");
        return;
    }

    /* What does a sensor with no hardware do when started? */
    step = cna_accelerometer_start(sensor);
    printf("  start (no backend)      %s\n", name_of(step));

    step = cna_accelerometer_set_supported_for_tests_ext(sensor, CNA_TRUE);
    printf("  set_supported_for_tests %s\n", name_of(step));
    supported = PROBE_POISON;
    step = cna_accelerometer_get_is_supported(game, &supported);
    printf("  is_supported (after)    %s %s\n", name_of(step), flag_of(supported));

    step = cna_accelerometer_start(sensor);
    printf("  start (supported)       %s\n", name_of(step));
    CNA_SensorState state = (CNA_SensorState)0xEEEEEEEEU;
    step = cna_accelerometer_get_state(sensor, &state);
    printf("  state                   %s value=%u\n", name_of(step), (unsigned)state);

    /* The subscription, before any injection, so the count below is the injection's. */
    CNA_SensorEventRegistrationHandle current = 0;
    step = cna_accelerometer_subscribe_current_value_changed(sensor, on_reading,
                                                             (void*)"accel", &current);
    printf("  subscribe current_value %s\n", name_of(step));
    CNA_SensorEventRegistrationHandle legacy = 0;
    step = cna_accelerometer_subscribe_reading_changed(sensor, NULL, NULL, &legacy);
    printf("  subscribe null callback %s\n", name_of(step));

    /* start refuses without the platform subsystem, so the canonical test path is the forced
       started flag plus the explicit dispatch helper -- which is what the seams are for. */
    step = cna_accelerometer_set_started_for_tests_ext(sensor, CNA_TRUE);
    printf("  set_started_for_tests   %s\n", name_of(step));
    state = (CNA_SensorState)0xEEEEEEEEU;
    step = cna_accelerometer_get_state(sensor, &state);
    printf("  state after forced      %s value=%u\n", name_of(step), (unsigned)state);
    CNA_Bool held = PROBE_POISON;
    step = cna_accelerometer_get_subsystem_held_for_tests_ext(sensor, &held);
    printf("  subsystem held          %s %s\n", name_of(step), flag_of(held));

    /* The arithmetic the header promises: platform units in, canonical units out. */
    reading_calls = 0;
    reading_inside_inject = 0;
    inside_inject = 1;
    step = cna_accelerometer_inject_synthetic_update_ext(sensor, 0.0f, 9.80665f, 0.0f);
    inside_inject = 0;
    printf("  inject 9.80665 m/s^2    %s calls=%d synchronous=%s\n",
           name_of(step), reading_calls, reading_inside_inject ? "yes" : "no");
    printf("  callback thread == main %s\n",
           reading_calls > 0 && pthread_equal(reading_thread, main_thread) ? "yes" : "no");
    printf("  callback context kept   %s\n",
           last_context != NULL && strcmp((const char*)last_context, "accel") == 0
                   ? "yes" : "no");
    printf("  callback acceleration   %.5f %.5f %.5f\n",
           (double)last_acceleration.x, (double)last_acceleration.y,
           (double)last_acceleration.z);

    CNA_AccelerometerReading reading;
    memset(&reading, 0, sizeof reading);
    reading.struct_size = (uint32_t)(sizeof reading);
    reading.struct_version = 1U;
    reading.acceleration.x = POISON_FLOAT;
    reading.acceleration.y = POISON_FLOAT;
    reading.acceleration.z = POISON_FLOAT;
    step = cna_accelerometer_get_current_value(sensor, &reading);
    printf("  current_value           %s %.5f %.5f %.5f%s\n", name_of(step),
           (double)reading.acceleration.x, (double)reading.acceleration.y,
           (double)reading.acceleration.z,
           reading.acceleration.y == POISON_FLOAT ? " (UNTOUCHED)" : "");
    CNA_Bool valid = PROBE_POISON;
    step = cna_accelerometer_get_is_data_valid(sensor, &valid);
    printf("  is_data_valid           %s %s\n", name_of(step), flag_of(valid));

    /* A second injection with different values, so a reading that never changes shows. */
    step = cna_accelerometer_inject_synthetic_update_ext(sensor, 19.6133f, 0.0f, 0.0f);
    printf("  inject 19.6133 on x     %s calls=%d\n", name_of(step), reading_calls);
    printf("  callback acceleration   %.5f %.5f %.5f\n",
           (double)last_acceleration.x, (double)last_acceleration.y,
           (double)last_acceleration.z);

    /* And the explicit dispatch helper, which names the sensors rather than relying on the
       started-instance registry. */
    step = cna_accelerometer_register_started_instance_for_tests_ext(sensor);
    printf("  register started        %s\n", name_of(step));
    reading_calls = 0;
    step = cna_accelerometer_inject_synthetic_update_ext(sensor, 0.0f, 9.80665f, 0.0f);
    printf("  inject after register   %s calls=%d\n", name_of(step), reading_calls);
    reading.acceleration.x = POISON_FLOAT;
    reading.acceleration.y = POISON_FLOAT;
    reading.acceleration.z = POISON_FLOAT;
    step = cna_accelerometer_get_current_value(sensor, &reading);
    printf("  current after register  %s %.5f %.5f %.5f\n", name_of(step),
           (double)reading.acceleration.x, (double)reading.acceleration.y,
           (double)reading.acceleration.z);

    reading_calls = 0;
    const CNA_AccelerometerHandle one[1] = {sensor};
    step = cna_accelerometer_dispatch_to_instances_for_tests_ext(game, one, 1U,
                                                                 0.0f, 0.0f, 19.6133f);
    printf("  dispatch_to_instances   %s calls=%d\n", name_of(step), reading_calls);
    printf("  dispatched acceleration %.5f %.5f %.5f\n",
           (double)last_acceleration.x, (double)last_acceleration.y,
           (double)last_acceleration.z);
    uint32_t exceptions = 0xEEEEEEEEU;
    step = cna_accelerometer_get_dispatch_exception_count_for_tests_ext(game, &exceptions);
    printf("  dispatch exceptions     %s count=%u\n", name_of(step), exceptions);

    /* Unsubscribing stops it, which is the whole reason the registration is a handle. */
    step = cna_sensor_unsubscribe_ext(current);
    printf("  unsubscribe             %s\n", name_of(step));
    const int before = reading_calls;
    (void)cna_accelerometer_inject_synthetic_update_ext(sensor, 1.0f, 2.0f, 3.0f);
    printf("  inject after unsub      calls fired=%d\n", reading_calls - before);
    step = cna_sensor_unsubscribe_ext(current);
    printf("  unsubscribe twice       %s\n", name_of(step));

    /* dispose against destroy. */
    step = cna_accelerometer_dispose(sensor);
    printf("  dispose                 %s\n", name_of(step));
    state = (CNA_SensorState)0xEEEEEEEEU;
    step = cna_accelerometer_get_state(sensor, &state);
    printf("  state after dispose     %s value=%u\n", name_of(step), (unsigned)state);
    step = cna_accelerometer_dispose(sensor);
    printf("  dispose twice           %s\n", name_of(step));
    step = cna_accelerometer_inject_synthetic_update_ext(sensor, 1.0f, 1.0f, 1.0f);
    printf("  inject after dispose    %s\n", name_of(step));
    step = cna_accelerometer_destroy(sensor);
    printf("  destroy after dispose   %s\n", name_of(step));
    step = cna_accelerometer_destroy(sensor);
    printf("  destroy twice           %s\n", name_of(step));
    printf("\n");
}

static void compass(CNA_Handle game)
{
    printf("== compass ==\n");
    CNA_CompassHandle sensor = 0;
    CNA_Result step = cna_compass_create(game, &sensor);
    printf("  create                  %s\n", name_of(step));
    if (step != CNA_RESULT_SUCCESS) {
        printf("\n");
        return;
    }
    CNA_Bool supported = PROBE_POISON;
    step = cna_compass_get_is_supported(game, &supported);
    printf("  is_supported (host)     %s %s\n", name_of(step), flag_of(supported));

    step = cna_compass_set_test_backend_ext(sensor, CNA_TRUE, CNA_TRUE);
    printf("  install test backend    %s\n", name_of(step));
    supported = PROBE_POISON;
    step = cna_compass_get_is_supported(game, &supported);
    printf("  is_supported (after)    %s %s\n", name_of(step), flag_of(supported));
    step = cna_compass_start(sensor);
    printf("  start                   %s\n", name_of(step));

    /* Installing while started is documented as INVALID_STATE. */
    step = cna_compass_set_test_backend_ext(sensor, CNA_TRUE, CNA_TRUE);
    printf("  install while started   %s\n", name_of(step));

    event_calls = 0;
    CNA_SensorEventRegistrationHandle calibrate = 0;
    step = cna_compass_subscribe_calibrate(sensor, on_event, NULL, &calibrate);
    printf("  subscribe calibrate     %s\n", name_of(step));
    step = cna_compass_inject_calibration_request_ext(sensor);
    printf("  inject calibration      %s calls=%d\n", name_of(step), event_calls);

    CNA_SensorEventRegistrationHandle changed = 0;
    step = cna_compass_subscribe_current_value_changed(sensor, NULL, NULL, &changed);
    printf("  subscribe null callback %s\n", name_of(step));

    CNA_CompassReading injected;
    memset(&injected, 0, sizeof injected);
    injected.struct_size = (uint32_t)(sizeof injected);
    injected.struct_version = 1U;
    injected.magnetic_heading = 42.5;
    injected.true_heading = 43.5;
    injected.heading_accuracy = 1.25;
    injected.magnetometer_reading.x = 1.0f;
    injected.magnetometer_reading.y = 2.0f;
    injected.magnetometer_reading.z = 3.0f;
    step = cna_compass_inject_synthetic_update_ext(sensor, &injected);
    printf("  inject reading          %s\n", name_of(step));
    CNA_CompassReading reading;
    memset(&reading, 0, sizeof reading);
    reading.struct_size = (uint32_t)(sizeof reading);
    reading.struct_version = 1U;
    reading.heading_accuracy = POISON_FLOAT;
    step = cna_compass_get_current_value(sensor, &reading);
    printf("  current_value           %s heading=%.5f accuracy=%.5f%s\n", name_of(step),
           reading.magnetic_heading, reading.heading_accuracy,
           reading.heading_accuracy == POISON_FLOAT ? " (UNTOUCHED)" : "");

    (void)cna_sensor_unsubscribe_ext(calibrate);
    (void)cna_compass_stop(sensor);
    step = cna_compass_set_test_backend_ext(sensor, CNA_FALSE, CNA_FALSE);
    printf("  remove test backend     %s\n", name_of(step));
    step = cna_compass_destroy(sensor);
    printf("  destroy                 %s\n", name_of(step));
    printf("\n");
}

static CNA_Result on_update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                            CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    if (ran) return CNA_RESULT_SUCCESS;
    ran = 1;
    accelerometer(game);
    compass(game);
    return CNA_RESULT_SUCCESS;
}

int main(void)
{
    main_thread = pthread_self();

    CNA_GameCallbacks callbacks;
    memset(&callbacks, 0, sizeof callbacks);
    callbacks.struct_size = (uint32_t)(sizeof callbacks);
    callbacks.struct_version = 1U;
    callbacks.update = on_update;

    CNA_GameCreateInfo info;
    memset(&info, 0, sizeof info);
    info.struct_size = (uint32_t)(sizeof info);
    info.struct_version = 1U;
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = 166667;
    info.window_title.data = "sensor injection";
    info.window_title.byte_length = 16U;
    info.callbacks = &callbacks;

    CNA_Handle game = CNA_INVALID_HANDLE;
    if (cna_game_create(&info, &game) != CNA_RESULT_SUCCESS) {
        printf("game create failed\n");
        return 1;
    }
    (void)cna_game_run_one_frame(game);
    printf("game destroy       %s\n", name_of(cna_game_destroy(game)));
    printf("PROBE %s\n", ran ? "OK" : "INCOMPLETE");
    return ran ? 0 : 1;
}

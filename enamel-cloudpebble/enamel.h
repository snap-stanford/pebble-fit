/**
 * This file was generated with Enamel : http://gregoiresage.github.io/enamel
 */

#ifndef ENAMEL_H
#define ENAMEL_H

#include <pebble.h>

// -----------------------------------------------------
// Getter for 'optin'
bool enamel_get_optin();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'vibrate'
bool enamel_get_vibrate();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'daily_start_time'
uint32_t enamel_get_daily_start_time();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'daily_end_time'
uint32_t enamel_get_daily_end_time();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'sleep_minutes'
typedef enum {
	SLEEP_MINUTES_1_HOUR = 60,
	SLEEP_MINUTES_30_MINUTES = 30,
	SLEEP_MINUTES_15_MINUTES = 15,
	SLEEP_MINUTES_10_MINUTES = 10,
	SLEEP_MINUTES_5_MINUTES = 5,
	SLEEP_MINUTES_3_MINUTES = 3,
	SLEEP_MINUTES_1_MINUTE = 1,
} SLEEP_MINUTESValue;
SLEEP_MINUTESValue enamel_get_sleep_minutes();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'step_threshold'
typedef enum {
	STEP_THRESHOLD_500 = 500,
	STEP_THRESHOLD_200 = 200,
	STEP_THRESHOLD_100 = 100,
	STEP_THRESHOLD_10 = 10,
} STEP_THRESHOLDValue;
STEP_THRESHOLDValue enamel_get_step_threshold();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'display_duration'
typedef enum {
	DISPLAY_DURATION_15 = 15,
	DISPLAY_DURATION_10 = 10,
	DISPLAY_DURATION_5 = 5,
} DISPLAY_DURATIONValue;
DISPLAY_DURATIONValue enamel_get_display_duration();
// -----------------------------------------------------

void enamel_init();

void enamel_deinit();

typedef void* EventHandle;
typedef void(EnamelSettingsReceivedHandler)(void* context);

EventHandle enamel_settings_received_subscribe(EnamelSettingsReceivedHandler *handler, void *context);
void enamel_settings_received_unsubscribe(EventHandle handle);

#endif
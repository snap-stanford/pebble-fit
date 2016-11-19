/**
 * This file was generated with Enamel : http://gregoiresage.github.io/enamel
 */

#ifndef ENAMEL_H
#define ENAMEL_H

#include <pebble.h>

// -----------------------------------------------------
// Getter for 'is_consent'
bool enamel_get_is_consent();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'consent_name'
const char* enamel_get_consent_name();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'consent_email'
const char* enamel_get_consent_email();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_age'
typedef enum {
	SURVEY_AGE_18_24 = 18,
	SURVEY_AGE_25_29 = 25,
	SURVEY_AGE_30_34 = 30,
	SURVEY_AGE_35_39 = 35,
	SURVEY_AGE_40_44 = 40,
	SURVEY_AGE_45_49 = 45,
	SURVEY_AGE_50_54 = 50,
	SURVEY_AGE_55_59 = 55,
	SURVEY_AGE_60_64 = 60,
	SURVEY_AGE_65_ = 65,
} SURVEY_AGEValue;
SURVEY_AGEValue enamel_get_survey_age();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_gender'
const char* enamel_get_survey_gender();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_height_unit'
const char* enamel_get_survey_height_unit();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_height'
const char* enamel_get_survey_height();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_weight_unit'
const char* enamel_get_survey_weight_unit();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_weight'
const char* enamel_get_survey_weight();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_race'
const char* enamel_get_survey_race();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'activate'
bool enamel_get_activate();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'vibrate'
typedef enum {
	VIBRATE_SINGLE_SHORT_PULSE = 1,
	VIBRATE_SINGLE_LONG_PULSE = 2,
	VIBRATE_DOUBLE_SHORT_PULSE = 3,
	VIBRATE_FIVE_SHORT_PULSE = 4,
	VIBRATE_OFF = 0,
} VIBRATEValue;
VIBRATEValue enamel_get_vibrate();
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
} SLEEP_MINUTESValue;
SLEEP_MINUTESValue enamel_get_sleep_minutes();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'sliding_window'
typedef enum {
	SLIDING_WINDOW_3_MINUTES = 3,
	SLIDING_WINDOW_5_MINUTES = 5,
	SLIDING_WINDOW_10_MINUTES = 10,
	SLIDING_WINDOW_15_MINUTES = 15,
} SLIDING_WINDOWValue;
SLIDING_WINDOWValue enamel_get_sliding_window();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'step_threshold'
typedef enum {
	STEP_THRESHOLD_10 = 10,
	STEP_THRESHOLD_200 = 200,
	STEP_THRESHOLD_100 = 100,
	STEP_THRESHOLD_300 = 300,
} STEP_THRESHOLDValue;
STEP_THRESHOLDValue enamel_get_step_threshold();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'display_duration'
typedef enum {
	DISPLAY_DURATION_10 = 10,
	DISPLAY_DURATION_15 = 15,
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
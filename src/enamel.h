/**
 * This file was generated with Enamel : http://gregoiresage.github.io/enamel
 */

#ifndef ENAMEL_H
#define ENAMEL_H

#include <pebble.h>

// -----------------------------------------------------
// Getter for 'config_update'
bool enamel_get_config_update();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'config_update_interval'
const char* enamel_get_config_update_interval();
// -----------------------------------------------------

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
// Getter for 'survey_age2'
#define SURVEY_AGE2_PRECISION 1
int32_t enamel_get_survey_age2();
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
#define SURVEY_HEIGHT_PRECISION 1
int32_t enamel_get_survey_height();
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
// Getter for 'break_freq'
typedef enum {
	BREAK_FREQ_EVERY_HOUR = 60,
} BREAK_FREQValue;
BREAK_FREQValue enamel_get_break_freq();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'break_len'
typedef enum {
	BREAK_LEN_5_MINUTE = 5,
} BREAK_LENValue;
BREAK_LENValue enamel_get_break_len();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'dynamic_wakeup'
bool enamel_get_dynamic_wakeup();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'sliding_window'
typedef enum {
	SLIDING_WINDOW_2_MINUTES = 2,
} SLIDING_WINDOWValue;
SLIDING_WINDOWValue enamel_get_sliding_window();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'step_threshold'
typedef enum {
	STEP_THRESHOLD_50 = 50,
	STEP_THRESHOLD_100 = 100,
	STEP_THRESHOLD_150 = 150,
	STEP_THRESHOLD_200 = 200,
	STEP_THRESHOLD_250 = 250,
	STEP_THRESHOLD_300 = 300,
	STEP_THRESHOLD_350 = 350,
} STEP_THRESHOLDValue;
STEP_THRESHOLDValue enamel_get_step_threshold();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'display_duration'
typedef enum {
	DISPLAY_DURATION_30 = 30,
	DISPLAY_DURATION_40 = 40,
	DISPLAY_DURATION_50 = 50,
	DISPLAY_DURATION_60 = 60,
} DISPLAY_DURATIONValue;
DISPLAY_DURATIONValue enamel_get_display_duration();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'total_hour'
const char* enamel_get_total_hour();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'group'
typedef enum {
	GROUP_PASSIVE_TRACKING = 0,
	GROUP_DAILY_MESSAGE = 1,
	GROUP_REALTIME_RANDOM = 2,
	GROUP_REALTIME_ACTION = 3,
	GROUP_REALTIME_OUTCOME = 4,
	GROUP_REALTIME_HEALTH = 5,
	GROUP_REALTIME_ADAPTIVE = 6,
} GROUPValue;
GROUPValue enamel_get_group();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'config_summary'
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'message_daily_summary'
const char* enamel_get_message_daily_summary();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'message_random_0'
const char* enamel_get_message_random_0();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'message_random_1'
const char* enamel_get_message_random_1();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'message_random_2'
const char* enamel_get_message_random_2();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'version'
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'watch_alert_text'
const char* enamel_get_watch_alert_text();
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'watch_pass_text'
const char* enamel_get_watch_pass_text();
// -----------------------------------------------------

void enamel_init();

void enamel_deinit();

typedef void* EventHandle;
typedef void(EnamelSettingsReceivedHandler)(void* context);

EventHandle enamel_settings_received_subscribe(EnamelSettingsReceivedHandler *handler, void *context);
void enamel_settings_received_unsubscribe(EventHandle handle);

#endif
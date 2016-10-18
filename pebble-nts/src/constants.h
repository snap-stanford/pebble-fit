#pragma once

// Number of data items to obtain from the HealthService API, ie: minutes
#define MAX_ENTRIES 60*12

// Number of minutes between uploads, and the number of items read from
// the HealthService minute data API.
#define INTERVAL_MINUTES 60

// Size of the incoming information for configuration, for example
#define INCOMING_DATA_SIZE 10

// Send squares test data instead of data from the HealthService minute
// minute data API
#define TEST false


/* +++++++++++++++ PERSISTANT STORAGE KEYS +++++++++++++++ */

// +++++++ RESERVED KEYS
// REMEMBER!!! keys 1-20 are reserved for the actigraphy keys

// +++++++ intra-app messaging


// +++++++ App State
static const int16_t HEALTH_EVENTS_LAST_UPLOAD_TIME_PERSIST_KEY = 10127;
static const int16_t ACTI_LAST_UPLOAD_TIME_PERSIST_KEY = 10128;
static const int16_t PINTERACT_KEY_COUNT_PERSIST_KEY = 10129;
static const int16_t CONFIG_WAKEUP_IDS_PERSIST_KEY = 10131;
static const int16_t ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY = 10132;
static const int16_t PINTERACT_STATE_PERSIST_KEY = 10133;


// +++++++ Configuration Data
static const int16_t CONFIG_GENERAL_PERSIST_KEY = 10180;
static const int16_t CONFIG_WAKEUP_PERSIST_KEY = 10181;
static const int32_t CUR_WP_VERSION_PERSIST_KEY = 10182;


// +++++++ Interaction Data

// +++++++ Continuous Data
// continuous daily metrics

// +++++++ Long Term Data

// +++++++ General Constants
// # DEFINED CONSTANTS
static const uint32_t NUM_SEC_IN_DAY = 24*60*60;
static const uint32_t NUM_SEC_IN_WEEK = 7*24*60*60;
static const int32_t CUR_WP_VERSION = 14;


// +++++++ Memory Constraints



/* +++++++++++++++ ENUMERATED TYPES +++++++++++++++ */


typedef struct{
  uint16_t height_cm;
  uint16_t weight_kg;
  bool pi11_active;
  bool pi14_active;
}__attribute__((__packed__)) ConfigGeneral;

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++ FOREGROUND APP CONSTANTS ONLY +++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


/* +++++++++++++++ VARIABLES +++++++++++++++ */

// +++++++ pinteract variables/parameters
static const uint16_t PINTERACT_LIST_PS_B_SIZE = PERSIST_DATA_MAX_LENGTH ;// assume it is uint16
static const uint16_t PINTERACT_PS_HEAD_B_SIZE = 6; // timestamp + counter
static const uint16_t PINTERACT_RES_BUF_COUNTER_B_SIZE = 2;
static const uint16_t PINTERACT_PS_B_COUNT_IND = 4; // byte counter

// +++++++ config wakeup variables/parameters
static const uint16_t NUM_CONFIG_WAKEUP = 4; // 8 -4 for the 1 hour, 6 hour, midnight, and +7 day fallback wakeups
static const uint16_t NUM_TOTAL_WAKEUP = 8; //

// +++++++ config general variables/parameters

// +++++++ daily summary variables/parameters
static const int16_t NUM_DAYS_DAILY_SUMMARY = 7; // we get a weeks work of data

static const int16_t CONFIG_WAKEUP_COOKIE = 1;

#define NUM_DAYS_HISTORY 8

/* +++++++++++++++ T +++++++++++++++ */
/* +++++++++++++++ T +++++++++++++++ */
/* +++++++++++++++ T +++++++++++++++ */



/* +++++++++++++++ VIBE PATTERNS +++++++++++++++ */

// this is an on-off pattern, special to remind the patient that this is unique
static const uint32_t pinteract_vibe_seg[] = {100,100,100,100,100,100,200};
static const VibePattern pinteract_vibe_pat = {
  .durations = pinteract_vibe_seg,
  .num_segments = ARRAY_LENGTH(pinteract_vibe_seg),
};

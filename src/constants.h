#pragma once

/* Number of data items of health history to be obtained in terms of minutes. */
#define MAX_ENTRIES       60

/* Maximum number of retries when scheduling a wakeup event. */
#define MAX_RETRY         10

/* Persistent Storage Keys. */
static const uint8_t WAKEUP_ID_PERSIST_KEY = 111;

/* Maximum number of wakeup events allowed for an App. */
//static uint8_t NUM_USED_WAKEUP   = 4;
//static uint8_t NUM_TOTAL_WAKEUP  = 8;
#define NUM_USED_WAKEUP   4
#define NUM_TOTAL_WAKEUP  8

#define SECONDS_PER_WEEK  (SECONDS_PER_DAY * 8)

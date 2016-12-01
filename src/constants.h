#pragma once

/* Number of data items of health history to be obtained in terms of minutes. */
#define MAX_ENTRIES       60

/* Maximum number of retries when scheduling a wakeup event. */
#define MAX_RETRY         10

/* Minimum number of minutes before the next wakeup event. */
#define MIN_SLEEP_MINUTES 10

/* Persistent Storage Keys. */
static const uint8_t WAKEUP_ID_PERSIST_KEY = 111;

/* Maximum number of wakeup events allowed for an App. */
//static uint8_t NUM_USED_WAKEUP   = 4;
//static uint8_t NUM_TOTAL_WAKEUP  = 8;
#define NUM_USED_WAKEUP   4
#define NUM_TOTAL_WAKEUP  8

/* Launch/delaunch reasons. */
#define OTHER_LAUNCH 0
#define USER_LAUNCH 1
#define PHONE_LAUNCH 2
#define WAKEUP_LAUNCH 3

#define OTHER_DELAUNCH 0
#define USER_DELAUNCH 1
#define TIMEOUT_DELAUNCH 2


#define SECONDS_PER_WEEK  (SECONDS_PER_DAY * 8)

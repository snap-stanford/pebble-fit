#pragma once

/* Hard limit on how long (in seconds) this app will remain on the foreground. */
#define MAX_DISPLAY_DURATION    (SECONDS_PER_MINUTE * 10)
/* Number of data items of health history to be obtained in terms of minutes. */
#define MAX_ENTRIES             60

/* Maximum number of retries when scheduling a wakeup event. */
#define MAX_RETRY               10

/* Minimum number of minutes before the next wakeup event. */
#define MIN_SLEEP_MINUTES       10

/* Maximum number of wakeup events allowed for an App. */
//static uint8_t NUM_USED_WAKEUP   = 4;
//static uint8_t NUM_TOTAL_WAKEUP  = 8;
#define NUM_USED_WAKEUP         4
#define NUM_TOTAL_WAKEUP        8

/* Launch/exit reasons. */
#define OTHER_LAUNCH            0
#define USER_LAUNCH             1
#define WAKEUP_LAUNCH           2
#define PHONE_LAUNCH            3

#define OTHER_EXIT              0
#define USER_EXIT               1
#define TIMEOUT_EXIT            2

#define SECONDS_PER_WEEK        (SECONDS_PER_DAY * 7)

/* Persistent Storage Keys. FIXME: supposed type is uint32_t. */
static const int      AppKeyArrayData               = 200; // FIXME: change to 4321? (diff from PERSIST_KEY_UPDATE_TIME)
static const uint32_t PERSIST_KEY_WAKEUP_ID         = 111;
static const uint32_t PERSIST_KEY_UPDATE_TIME       = 200;
static const uint32_t PERSIST_KEY_CONFIG_TIME       = 201;
static const uint32_t PERSIST_KEY_BREAK_COUNT       = 202;
static const uint32_t PERSIST_KEY_LAUNCHEXIT_COUNT  = 1000;
static const uint32_t PERSIST_KEY_LAUNCH_START      = 1001;
static const uint32_t PERSIST_KEY_LAUNCH_END        = 1128; // Keys 1001-1128 are used.


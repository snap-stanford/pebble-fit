#pragma once

#define SECONDS_PER_WEEK              (SECONDS_PER_DAY * 7)

/* Hard limit on how long (in seconds) this app will remain on the foreground. */
#define MAX_DISPLAY_DURATION          (SECONDS_PER_MINUTE * 10)
/* Number of data items of health history to be obtained in terms of minutes. */
#define MAX_ENTRIES                   60

/* Maximum number of retries when scheduling a wakeup event. */
#define MAX_RETRY                     10

/* Minimum number of minutes before the next wakeup event. */
#define MIN_SLEEP_MINUTES             10

/* Maximum number of wakeup events allowed for an App. */
#define NUM_USED_WAKEUP               6
#define NUM_TOTAL_WAKEUP              8

/* Launch/exit reasons. */
#define LAUNCH_OTHER                  0
#define LAUNCH_USER                   1
#define LAUNCH_PHONE                  2
#define LAUNCH_WAKEUP_PERIOD          3
#define LAUNCH_WAKEUP_NOTIFY          4
#define LAUNCH_WAKEUP_DAILY           5

#define EXIT_OTHER                    0
#define EXIT_USER                     1
#define EXIT_TIMEOUT                  2

/* The size of random message pool. */
#define RANDOM_MSG_POOL_SIZE          3

/* Different user groups (synchronized with PebbleKit JS). Already defined in enamel.h */
//#define GROUP_PASSIVE_TRACKING        0
//#define GROUP_DAILY_MESSAGE           1
//#define GROUP_REALTIME_RANDOM         2
//#define GROUP_REALTIME_ACTION         3
//#define GROUP_REALTIME_OUTCOME        4
//#define GROUP_REALTIME_HEALTH         5
//#define GROUP_REALTIME_ADAPTIVE       6

/* Persistent Storage Keys. FIXME: supposed type is uint32_t. */
static const int      AppKeyArrayData               = 200; // FIXME: change to 4321? (diff from PERSIST_KEY_UPLOAD_TIME)
static const uint32_t PERSIST_KEY_WAKEUP_ID         = 111;

static const uint32_t PERSIST_KEY_LAUNCHEXIT_COUNT  = 1000;
static const uint32_t PERSIST_KEY_LAUNCH_START      = 1001;
static const uint32_t PERSIST_KEY_LAUNCH_END        = 1128; // Keys 1001-1128 are used.

static const uint32_t PERSIST_KEY_UPLOAD_TIME       = 200;
static const uint32_t PERSIST_KEY_CONFIG_TIME       = 201;
static const uint32_t PERSIST_KEY_BREAK_COUNT       = 202;
static const uint32_t PERSIST_KEY_RANDOM_MSG_INDEX  = 203; // Random message index.


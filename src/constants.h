#pragma once

/* TODO: DEBUG flag to turn on more debugging message. */
#define DEBUG 0

#define SECONDS_PER_WEEK              (SECONDS_PER_DAY * 7)

/* Hard limit on how long (in seconds) this app will remain on the foreground. */
#define MAX_DISPLAY_DURATION          (SECONDS_PER_MINUTE * 4)
/* Number of data items of health history to be obtained in terms of minutes. */
#define MAX_ENTRIES                   60

/* Maximum number of retries when scheduling a wakeup event. */
#define MAX_RETRY                     10

/* Minimum number of minutes before the next wakeup event. */
#define MIN_SLEEP_MINUTES             10

/* Maximum number of wakeup events allowed for an App. */
#define NUM_USED_WAKEUP               6
#define NUM_TOTAL_WAKEUP              8

/* Launch reasons (3 bits - up to 7. Change store.c if this is modified). */
#define LAUNCH_OTHER                  0
#define LAUNCH_USER                   1
#define LAUNCH_PHONE                  2     // Also could the special fallback wakeup.
#define LAUNCH_WAKEUP_PERIOD          3
#define LAUNCH_WAKEUP_ALERT           4
#define LAUNCH_WAKEUP_DAILY           5

/* Exit reasons (2 bits - up to 3. Change store.c if this is modified). */
#define EXIT_OTHER                    0
#define EXIT_USER                     1
#define EXIT_TIMEOUT                  2

#define RANDOM_MSG_POOL_SIZE          12    // The size of random message pool
#define RANDOM_MSG_SIZE_MAX           256   // Maximum number of characters for random message

/* Communiation Keys.*/
static const int      AppKeyArrayData               = 200; // FIXME: change to 4321? (diff from PERSIST_KEY_UPLOAD_TIME)

/* Persistent Storage Keys. FIXME: supposed type is uint32_t. */
static const uint32_t PERSIST_KEY_WAKEUP_ID         = 111;

static const uint32_t PERSIST_KEY_LAUNCHEXIT_COUNT  = 1000;
static const uint32_t PERSIST_KEY_LAUNCH_START      = 1001;
static const uint32_t PERSIST_KEY_LAUNCH_END        = 1128; // Keys 1001-1128 are used.

static const uint32_t PERSIST_KEY_UPLOAD_TIME       = 200;
static const uint32_t PERSIST_KEY_CONFIG_TIME       = 201;
static const uint32_t PERSIST_KEY_CURR_SCORE        = 202;
static const uint32_t PERSIST_KEY_CURR_SCORE_TIME   = 203;
static const uint32_t PERSIST_KEY_RANDOM_MSG_INDEX  = 204; // Random message index.


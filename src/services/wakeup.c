#include "wakeup.h"

/* Set the wakeup to be X minutes from now. */
void wakeup_set() {
  time_t timestamp = time(NULL) + (5 * SECONDS_PER_MINUTE);
  const int cookie = 0;
  const bool notify_if_missed = true;
  wakeup_schedule(timestamp, cookie, notify_if_missed);
  APP_LOG(APP_LOG_LEVEL_INFO, "Set wakeup...");
}
#include "wakeup.h"

static void wakeup_handler(WakeupId id, int32_t reason) {
  vibes_double_pulse();
}


void wakeup_init_handle() {
  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Woken up by wakeup");
    WakeupId id = 0;
    int32_t reason = 0;
    if (wakeup_get_launch_event(&id, &reason)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "id: %d, reason %d:", (int) id, (int) reason);
      wakeup_handler(id, reason);
    }
  } else {
    time_t timestamp = time(NULL) + 10;
    const int cookie = 0;
    const bool notify_if_missed = true;
    wakeup_schedule(timestamp, cookie, notify_if_missed);
    APP_LOG(APP_LOG_LEVEL_INFO, "Set wakeup...");
  }
}
#include "tick.h"

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  main_window_update_time(tick_time);
}

void tick_subscribe() {
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
  
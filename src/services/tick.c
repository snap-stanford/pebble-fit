#include "tick.h"

void tick_step_progress_handle(struct tm *tick_time, TimeUnits units_changed) {
  main_window_breathe();
  APP_LOG(APP_LOG_LEVEL_INFO, "%d tick unit changed", units_changed);
}

/* handle second tick events. */
void tick_second_subscribe () {
  tick_timer_service_subscribe(SECOND_UNIT, tick_step_progress_handle);
}
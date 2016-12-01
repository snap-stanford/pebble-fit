#include "tick.h"

static int s_count;
static bool s_will_timeout;

/* Handle second tick events. */
static void prv_tick_step_progress_handle(struct tm *tick_time, TimeUnits units_changed) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "%d tick unit changed", units_changed);
  if (s_count >= SECONDS_PER_MINUTE * enamel_get_sliding_window()) {
    wakeup_window_breathe();
    s_count = 0; // FIXME: careful, must make sure display_duration < 60
  }
  if (s_will_timeout && s_count >= enamel_get_display_duration()) {
    // Exit after timeout via pop all windows of this App out
    delaunch_reason = TIMEOUT_DELAUNCH;
    window_stack_pop_all(false);
  }
  s_count += 1;
}

/* Subscribe to a tick timer service. */
void tick_second_subscribe(bool will_timeout) {
  s_count = 0;
  s_will_timeout = will_timeout;
  tick_timer_service_subscribe(SECOND_UNIT, prv_tick_step_progress_handle);
}

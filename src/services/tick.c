#include "tick.h"

static int s_count;
static bool s_will_timeout;

/* Handle second tick events. */
static void prv_tick_step_progress_handle(struct tm *tick_time, TimeUnits units_changed) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "%d tick unit changed", units_changed);
  //APP_LOG(APP_LOG_LEVEL_ERROR, "s_count=%d, s_will_timeout=%u, dis=%d",
  //  s_count, (unsigned) s_will_timeout, enamel_get_display_duration());

  if ((s_will_timeout && s_count >= enamel_get_display_duration()) || 
      s_count >= MAX_DISPLAY_DURATION) {
    // Exit after timeout by popping out all the windows of this app 
    e_exit_reason = EXIT_TIMEOUT;
    window_stack_pop_all(false);
  //} else if (s_count % (SECONDS_PER_MINUTE * enamel_get_sliding_window()) == 0) {
  //  wakeup_window_breathe();
  }
  s_count++;
}

/* Subscribe to a tick timer service. */
void tick_second_subscribe(bool will_timeout) {
  s_count = 1;
  s_will_timeout = will_timeout;
  tick_timer_service_subscribe(SECOND_UNIT, prv_tick_step_progress_handle);
}

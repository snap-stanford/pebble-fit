#include "tick.h"

static int s_count;
static bool s_will_timeout;

/* 
 * Handle second tick events. 
 */
static void prv_tick_step_progress_handle(struct tm *tick_time, TimeUnits units_changed) {
  if ((s_will_timeout && s_count >= enamel_get_display_duration()) || 
      s_count >= MAX_DISPLAY_DURATION) {
    // Exit after timeout by popping out all the windows of this app 
    e_exit_reason = EXIT_TIMEOUT;
    window_stack_pop_all(false);
  }
  s_count++;
}

/*
 * Reset the count to 1.
 */
void tick_reset_count() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Reset timeout count.");
  s_count = 1;
}

/**
 * Subscribe to a tick timer service (i.e. start timer). 
 * */
void tick_second_subscribe(bool will_timeout) {
  tick_reset_count();

  s_will_timeout = will_timeout;

  tick_timer_service_subscribe(SECOND_UNIT, prv_tick_step_progress_handle);
}

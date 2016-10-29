#include "tick.h"

static int s_count;
static bool s_optin;

void tick_step_progress_handle(struct tm *tick_time, TimeUnits units_changed) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "%d tick unit changed", units_changed);
  //main_window_breathe();
  if (s_optin && s_count >= enamel_get_display_duration()) {
		window_stack_pop_all(false); // exit after timeout 
  }
	s_count += 1;
}

/* Handle second tick events. */
void tick_second_subscribe(bool optin) {
  s_count = 0;
	s_optin = optin;
  tick_timer_service_subscribe(SECOND_UNIT, tick_step_progress_handle);
}

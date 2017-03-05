#include "health.h"

static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;

/* Average daily step count over whole day. */
static void get_step_goal() {
  const time_t start = time_start_of_today();
  const time_t end = start + SECONDS_PER_DAY;
  s_step_goal = (int) health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
}

/* Average daily step count upto that given time. */
static void get_step_average() {
  const time_t start = time_start_of_today();
  const time_t end = time(NULL);
  s_step_average = (int)health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
}

/* Sum today's steps. */
static void get_step_count() {
  s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
}

/* Get the step goal, count, and average, and update the main window with them. */
static void health_handler(HealthEventType event, void *context) {
  if(event == HealthEventSignificantUpdate) {
    get_step_goal();
  }
  if(event != HealthEventSleepUpdate) {
    get_step_count();
    get_step_average();
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps: %d %d %d", s_step_count, s_step_goal, s_step_average);
    main_window_update_steps(s_step_count, s_step_goal, s_step_average);
  }
}

/* Subscribe to health updates. Deprecated, moved into steps.c
*/
void health_subscribe() {
  HealthServiceAccessibilityMask result = 
    health_service_metric_accessible(
      HealthMetricStepCount, time_start_of_today(), time(NULL));
  if(result == HealthServiceAccessibilityMaskAvailable) {
    if(health_service_events_subscribe(health_handler, NULL)) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Subscribed to health!");
      return;
    }
  }
  APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available.");
}

  

#include <pebble.h>

#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/wakeup.h"

#include "windows/main_window.h"

static void deinit(void) {
  set_wakeup_in_x();
}

static void health_handler(HealthEventType event, void *context) {
  // Update step count
  HealthMetric metric = HealthMetricStepCount;
  HealthServiceAccessibilityMask result = 
    health_service_metric_accessible(metric, time_start_of_today(), time(NULL));
  int steps = 0;
  if(result == HealthServiceAccessibilityMaskAvailable) {
    steps = (int)health_service_sum_today(metric);
  }
  main_window_update_steps(steps);
}

static void init(void) {
  comm_init(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  main_window_push();
  get_wakeup_reason();
  if(!health_service_events_subscribe(health_handler, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
}


int main(void) {
	init();
	app_event_loop();
	deinit();
}

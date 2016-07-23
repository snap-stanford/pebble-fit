#include "health.h"

static void health_handler(HealthEventType event, void *context) {
  HealthMetric metric = HealthMetricStepCount;
  HealthServiceAccessibilityMask result = 
    health_service_metric_accessible(metric, time_start_of_today(), time(NULL));
  int steps = 0;
  if(result == HealthServiceAccessibilityMaskAvailable) {
    steps = (int)health_service_sum_today(metric);
  }
  main_window_update_steps(steps);
}


void health_subscribe() {
  if(!health_service_events_subscribe(health_handler, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Subscribed to health!");
  }
}

  
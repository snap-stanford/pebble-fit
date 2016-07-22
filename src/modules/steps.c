#include "steps.h"

static uint8_t s_data[MAX_ENTRIES];

int data_reload_steps(time_t * start, time_t * end) {
  // Clear old data
  for(uint8_t i = 0; i < MAX_ENTRIES; i++) {
    s_data[i] = 0;
  }

  // Check data is available
  int num_records = 0;
  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, *start, *end);
  if(result == HealthServiceAccessibilityMaskAvailable) {
    // Read the data
    HealthMinuteData minute_data[MAX_ENTRIES];
    num_records = health_service_get_minute_history(&minute_data[0], MAX_ENTRIES, start, end);

    // Store it
    for(int i = 0; i < num_records; i++) {
      s_data[i] = minute_data[i].steps;
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "No data available from %d to %d!", (int) *start, (int) *end);
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Got %d/%d new entries from the Health API from %d to %d", (int)num_records, MAX_ENTRIES, (int) *start, (int) *end);

  return (int)num_records;
}

uint8_t* data_get_steps_data() {
  return s_data;  
}
#include "data.h"

static int s_data[MAX_ENTRIES];
static time_t end;

int data_reload_steps() {
  // Clear old data
  for(int i = 0; i < MAX_ENTRIES; i++) {
    s_data[i] = 0;
  }

  if (end == 0) {
    end = time(NULL) - (15 * SECONDS_PER_MINUTE);
  }
  
  time_t start = end - (MAX_ENTRIES * SECONDS_PER_MINUTE);

  // Check data is available
  uint32_t num_records = 0;
  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, start, end);
  if(result == HealthServiceAccessibilityMaskAvailable) {
    // Read the data
    HealthMinuteData minute_data[MAX_ENTRIES];
    num_records = health_service_get_minute_history(&minute_data[0], MAX_ENTRIES, &start, &end);

    // Store it
    for(uint32_t i = 0; i < num_records; i++) {
      s_data[i] = (int)minute_data[i].steps;
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "No data available from %d to %d!", (int)start, (int)end);
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Got %d/%d new entries from the Health API from %lld to %lld", (int)num_records, MAX_ENTRIES, (long long) start, (long long) end);

  end = start;

  return (int)num_records;
}

int* data_get_steps_data() {
  return s_data;  
}
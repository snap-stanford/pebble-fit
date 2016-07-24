#include "steps.h"

#define MAX_ENTRIES 60
static uint8_t s_data[MAX_ENTRIES];
static int s_num_records;
static time_t s_start;

static void load_data(time_t * start, time_t * end) {
  // Clear old data
  s_num_records = 0;
  for(uint8_t i = 0; i < MAX_ENTRIES; i++) {
    s_data[i] = 0;
  }

  // Check data is available
  HealthServiceAccessibilityMask result = 
    health_service_metric_accessible(HealthMetricStepCount, *start, *end);

  if(result != HealthServiceAccessibilityMaskAvailable) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No steps data available from %d to %d!", (int) *start, (int) *end);
    return;
  }

  // Read the data
  HealthMinuteData minute_data[MAX_ENTRIES];
  
  // store new static variables
  s_num_records = health_service_get_minute_history(&minute_data[0], MAX_ENTRIES, start, end);
  s_start = *start;
  for(int i = 0; i < s_num_records; i++) {
    s_data[i] = minute_data[i].steps;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Got %d/%d new entries for steps data from %d to %d", (int)s_num_records, MAX_ENTRIES, (int) *start, (int) *end);
}

static void data_write(DictionaryIterator * out) {
  //write the data
  dict_write_data(out, AppKeyStepsData, s_data, sizeof(uint8_t) * s_num_records);
  dict_write_int(out, AppKeyDate, &s_start, sizeof(int), true);
}

static void send_to_phone(time_t start, time_t end) {
  load_data(&start, &end);

  if (s_num_records == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No new steps data to send.");
    return;
  }

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}

void send_latest_steps_to_phone() {
  time_t end = time(NULL) - (15 * SECONDS_PER_MINUTE);
  time_t start = end - (MAX_ENTRIES * SECONDS_PER_MINUTE);
  send_to_phone(start, end);
}
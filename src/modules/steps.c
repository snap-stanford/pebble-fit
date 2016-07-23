#include "steps.h"

#define MAX_ENTRIES 60
static uint8_t s_data[MAX_ENTRIES];
static time_t end;

static int data_reload_steps(time_t * start, time_t * end) {
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

    APP_LOG(APP_LOG_LEVEL_INFO, "Got %d/%d new entries from the Health API from %d to %d", (int)num_records, MAX_ENTRIES, (int) *start, (int) *end);

  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "No data available from %d to %d!", (int) *start, (int) *end);
  }

  return (int) num_records;
}

static void steps_sent_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Steps Sent from Watch!");
}

static void server_received_steps_handler(DictionaryIterator *iter, void *context) {
  if(dict_find(iter, AppKeyServerReceived)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps Received by Server!");
  }
  CommCallback *cb = (CommCallback *) context;
  cb();
}

static void send_steps_to_phone(time_t start, time_t end) {
  int num_records = data_reload_steps(&start, &end);

  if (num_records == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No new data");
    return;
  }

  DictionaryIterator *out;

  if(app_message_outbox_begin(&out) == APP_MSG_OK) {
    app_message_register_outbox_sent(steps_sent_handler);
    app_message_register_inbox_received(server_received_steps_handler);
  
    dict_write_data(out, AppKeyStepsData, s_data, sizeof(uint8_t) * num_records);
    dict_write_int(out, AppKeyStepsEndDate, &start, sizeof(int), true);

    if(app_message_outbox_send() != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error starting send of message.");
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
  }
}

void send_latest_steps_to_phone() {
  if (end == 0) {
    end = time(NULL) - (15 * SECONDS_PER_MINUTE);
  }

  time_t start = end - (MAX_ENTRIES * SECONDS_PER_MINUTE);
  send_steps_to_phone(start, end);
}
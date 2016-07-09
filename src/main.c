#include <pebble.h>

#include "windows/main_window.h"

static uint32_t get_available_records(HealthMinuteData *array, time_t query_start, time_t query_end, uint32_t max_records) {
  time_t next_start = query_start;
  time_t next_end = query_end;
  uint32_t num_records_found = 0;

  // Find more records until no more are returned
  while (num_records_found < max_records) {
    int ask_num_records = max_records - num_records_found;
    uint32_t ret_val = health_service_get_minute_history(&array[num_records_found], 
      ask_num_records, &next_start, &next_end);
    if (ret_val == 0) {
      // a 0 return value means no more data is available
      return num_records_found;
    }
    num_records_found += ret_val;
    next_start = next_end;
    next_end = query_end;
  } 

  return num_records_found;
}

static void print_steps_from_to(time_t query_start, time_t query_end) {
  uint32_t max_records = (query_end - query_start) / SECONDS_PER_MINUTE;
  HealthMinuteData *data = 
  (HealthMinuteData*)malloc(max_records * sizeof(HealthMinuteData));
  // Populate the array
  max_records = get_available_records(data, query_start, query_end, max_records);

  // Print the results
  for(uint32_t i = 0; i < max_records; i++) {
    APP_LOG(APP_LOG_LEVEL_INFO, "%lld\n", (long long) query_start + SECONDS_PER_MINUTE*i);
    if(!data[i].is_invalid) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Record %d contains %d steps.", (int)i, 
        (int)data[i].steps);
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Record %d was not valid.", (int)i);
    }
  }

  free(data);
}

static void print_steps_in_batch_to_now() {
  time_t query_end = time(NULL) - (15 * SECONDS_PER_MINUTE);
  while (true) {
    time_t query_start = query_end - SECONDS_PER_DAY;
    print_steps_from_to(query_start, query_end);
    query_end = query_start;
  }
}


static void init(void) {
  main_window_push();
  print_steps_in_batch_to_now();
}

static void deinit(void) {}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

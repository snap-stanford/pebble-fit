#include "steps.h"

static uint8_t s_step_records[MAX_ENTRIES];
static int s_num_records;
static int s_steps;
static time_t s_start, s_end;
static const int AppKeyArrayData = 200;

static int s_debug_entry_count;

/* Set static array to zeros. */
static void clear_old_data() {
  s_num_records = 0;
  for(int i = 0; i < MAX_ENTRIES; i++) {
    s_step_records[i] = 0;
  }
}

/* Load health service data into static array. */
static void load_data(time_t * start, time_t * end) {
  clear_old_data();

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
  s_debug_entry_count = 0;
  for(int i = 0; i < enamel_get_sleep_minutes(); i++) {
    if (minute_data[i].is_invalid) {
      APP_LOG(APP_LOG_LEVEL_INFO, "(Data invalid) Entry %d = %d", (int)i, (int)s_step_records[i]);
    } else {
      s_step_records[i] = minute_data[i].steps;
      
      if (s_step_records[i] > 0) {
        s_debug_entry_count = 0;
      } else {
        s_debug_entry_count++;
      }

      APP_LOG(APP_LOG_LEVEL_INFO, "Entry %d = %d", (int)i, (int)s_step_records[i]);
    }
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Got %d/%d new entries for steps data from %d to %d", (int)s_num_records, MAX_ENTRIES, (int) *start, (int) *end);



  // Make a timestamp for now
  time_t db_end = time(NULL);
  
  // Make a timestamp for the last hour's worth of data
  time_t db_start = db_end - SECONDS_PER_HOUR;
  
  // Check data is available
  HealthServiceAccessibilityMask res = 
      health_service_metric_accessible(HealthMetricStepCount, db_start, db_end);
  if(res & HealthServiceAccessibilityMaskAvailable) {
    // Data is available! Read it
    HealthValue steps = health_service_sum(HealthMetricStepCount, db_start, db_end);
    //HealthValue steps = health_service_sum(HealthMetricStepCount, *start, *end);
  
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps in the last hour (%d-%d): %d", (int)db_start, (int)db_end, (int)steps);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No data available!");
  }

}

/* Write steps array data to dict. */
static void data_write(DictionaryIterator * out) {
  //write the data
  int true_value = 1;
  dict_write_int(out, AppKeyStepsData, &true_value, sizeof(int), true);

  dict_write_int(out, AppKeyDate, &s_start, sizeof(int), true);
  dict_write_int(out, AppKeyArrayLength, &s_num_records, sizeof(int), true);
  dict_write_int(out, AppKeyArrayStart, &AppKeyArrayData, sizeof(int), true);
  for (int i = 0; i < s_num_records; i++) {
    dict_write_int(out, AppKeyArrayData + i, &s_step_records[i], sizeof(int), true);
  }
}

/* Send steps in time frame. */
void steps_send_in_between(time_t start, time_t end) {
  load_data(&start, &end);

  if (s_num_records == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No new steps data to send.");
    return;
  }

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}

/* Send the steps from before 15 minutes back. */
void steps_send_latest() {
  // start from 15 minutes back (real time is not accurate)
  time_t now = time(NULL) - (15 * SECONDS_PER_MINUTE);
  time_t start = now - (MAX_ENTRIES * SECONDS_PER_MINUTE);
  steps_send_in_between(start, now);
}

/* Return the number of steps in the last sleep period. */
int steps_get_latest() {
  // TODO: Change s_end to a timestamp slightly in the future, otherwise, Pebble health
  // service will round it down to the boundary of one minute, likely will lose the latest
  // step count (less accurate especially when sleep duration is small)
  s_end = time(NULL);
  s_start = s_end - ((int)enamel_get_sleep_minutes() * SECONDS_PER_MINUTE);

  load_data(&s_start, &s_end);

  s_steps = 0;
  for(int i = 0; i < (int)enamel_get_sleep_minutes(); i++) {
    s_steps += s_step_records[i];
  }
  return s_steps;
}

/* Update the wakeup_window with the number of steps in the last sleep period. */
void steps_update_wakeup_window_steps() { 
  steps_get_latest(); // TODO: optimize

  char start_buf[12]; char end_buf[12];
  strftime(start_buf, sizeof(start_buf), "%H:%M:%S", localtime(&s_start));
  strftime(end_buf, sizeof(end_buf), "%H:%M:%S", localtime(&s_end));
  APP_LOG(APP_LOG_LEVEL_INFO, "Got %d step count from %s to %s", s_steps, start_buf, end_buf);

  wakeup_window_update_steps(s_steps, start_buf, end_buf, s_debug_entry_count);
}

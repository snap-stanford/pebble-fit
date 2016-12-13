#include "steps.h"

static uint8_t s_step_records[MAX_ENTRIES];
static int s_num_records;
static int s_step;
static time_t s_start, s_end;
static int s_inactive_mins;
static char s_start_buf[12];
static char s_end_buf[12];
static bool s_is_loaded = false;        // Set this to true to skip prv_load_data()
static bool s_is_update = false;

/* Load health service data for the last hour into a static array. */
static void prv_load_data(time_t *start, time_t *end) {
	if (!s_is_loaded) {
  	// Set the static array to zeros
  	s_num_records = 0;
  	for(int i = 0; i < MAX_ENTRIES; i++) {
  	  s_step_records[i] = 0;
  	}

  	// Check data is available
  	HealthServiceAccessibilityMask result = 
  	  health_service_metric_accessible(HealthMetricStepCount, *start, *end);
  	if (result != HealthServiceAccessibilityMaskAvailable) {
  	  APP_LOG(APP_LOG_LEVEL_ERROR, "No steps data available from %d to %d!", (int) *start, (int) *end);
  	}

  	// Read the data and store into the static array
    // FIXME: due to some unknown bugs, health_service_get_minute_history() can only fetch
    // a limited number of elements (less than 100). Be carefull, since it will crash 
    // the watch firmware and force the reboot of the watch.
  	HealthMinuteData minute_data[MAX_ENTRIES];
  	s_num_records = health_service_get_minute_history(&minute_data[0], MAX_ENTRIES, start, end);
  	s_start = *start;
  	s_end = *end;

  	for (int i = 0; i < s_num_records; i++) {
  	  if (minute_data[i].is_invalid) {
  	    // No valid data recorded; steps = -1. Treat it as 0 step count.
  	    s_step_records[i] = 0;
  	    //APP_LOG(APP_LOG_LEVEL_INFO, "Invalid data %d = %d", (int)i, (int)minute_data[i].steps);
  	  } else {
  	    s_step_records[i] = minute_data[i].steps;
				if (s_step_records[i] > 0) {
  	  		APP_LOG(APP_LOG_LEVEL_INFO, "s_step_records %d = %d", (int)i, (int)s_step_records[i]);
        }
  	  }
  	  //APP_LOG(APP_LOG_LEVEL_INFO, "s_step_records %d = %d", (int)i, (int)s_step_records[i]);
  	}
		s_is_loaded = true;
	}
}

/* Update steps count. */
void steps_update() {
  //if (!s_is_update) { // FIXME: wanted to avoid multiple updates in the same session (i.e. within 1 minute so that step count will not change at all)
    s_step = 0;
    s_end = time(NULL);
    s_start = s_end - SECONDS_PER_MINUTE * MAX_ENTRIES; 

    // Read recorded step count data from the Pebble Health service.
		s_is_loaded = false; // Force to load new data from Health service.
    prv_load_data(&s_start, &s_end);

    // Calculate the total inactive minutes since the last active window. Up to 
    // 60 regardless of the sleep_minutes value.
    // FIXME: make sure 60 can be divisible by sleep_minutes
    s_inactive_mins = 0;
    int sliding_window_sum;
    for(int i = 0; i < MAX_ENTRIES; i += enamel_get_sliding_window()) {
      sliding_window_sum = 0;
      for (int j = i; j < i+enamel_get_sliding_window(); j++) {
        sliding_window_sum += s_step_records[j];
      }
      if (sliding_window_sum >= enamel_get_step_threshold()) {
        s_inactive_mins = 0;
      } else {
        s_inactive_mins += enamel_get_sliding_window();
      }

      // Count the total step count in the last sleeping interval.
      if (i >= MAX_ENTRIES - enamel_get_sleep_minutes()) {
        s_step += sliding_window_sum;
        //APP_LOG(APP_LOG_LEVEL_INFO, "s_steps=%d; sliding_window_sum=%d", s_step, sliding_window_sum);
      }
    }

    // Convert to human readable time for the display purpose.
    //APP_LOG(APP_LOG_LEVEL_ERROR, "enamel_get_sleep_minutes=%d", enamel_get_sleep_minutes());
    s_start = s_end - ((int)enamel_get_sleep_minutes() * SECONDS_PER_MINUTE);
    strftime(s_start_buf, sizeof(s_start_buf), "%H:%M", localtime(&s_start));
    strftime(s_end_buf, sizeof(s_end_buf), "%H:%M", localtime(&s_end));
    APP_LOG(APP_LOG_LEVEL_INFO, "%d steps from %s to %s", s_step, s_start_buf, s_end_buf);

    //s_is_update = true;
  //}
}

/* Send updated info to wakeup_window for displaying on the watch. 
 * This assume steps_update() was called recently.
 */
void steps_wakeup_window_update() { 
  wakeup_window_update(s_step, s_start_buf, s_end_buf, s_inactive_mins);
}

/* Whether we should alert the user or not. 
 * Yes if the user was not active in any sliding window during last sleeping interval.
 */
bool steps_whether_alert() {
	APP_LOG(APP_LOG_LEVEL_INFO, "%d/%d", s_inactive_mins, enamel_get_sleep_minutes());
  if (s_inactive_mins >= enamel_get_sleep_minutes()) {
    return true;
  } else {
    return false;
  }
}

/* Return the inactive minutes. */
int steps_get_inactive_minutes() {
  return s_inactive_mins;
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
    //dict_write_int(out, AppKeyArrayData + i, &s_step_records[i], sizeof(int), true);
		// FIXME: s_step_records is the type of uint8_t[]
    dict_write_uint8(out, AppKeyArrayData + i, s_step_records[i]);
  }
}

/* Send steps in the time frame of 60 minutes. */
void steps_send_in_between(time_t start, time_t end, bool force) {
  if (force) {
    // Force prv_load_data to load load from Pebble Health.
    s_is_loaded = false;
  }
  prv_load_data(&start, &end);

  if (s_num_records == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No new steps data to send.");
    return;
  }

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}

/* Send the latest steps data in the last hour.
 * FIXME: this could be integrated into store_resend_steps(). 
 */
void steps_send_latest(time_t curr_time) {
  time_t start = curr_time - (MAX_ENTRIES * SECONDS_PER_MINUTE);
  steps_send_in_between(start, curr_time, false);

  store_write_update_time(curr_time);
}



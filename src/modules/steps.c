#include "steps.h"

static uint8_t s_step_records[MAX_ENTRIES];
static int s_num_records;
static int s_steps;
static time_t s_start, s_end;
static int s_inactive_mins;
static char s_start_buf[12];
static char s_end_buf[12];
static bool s_is_loaded = false;        // Set this to true to skip prv_load_data()
static bool s_is_update = false;
static bool s_pass = false;

/* Load health service data for the last hour into a static array. */
static void prv_load_data(time_t *start, time_t *end) {
	s_steps = 0;
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
      APP_LOG(APP_LOG_LEVEL_ERROR, "No steps data available from %u to %u!", (unsigned) *start, (unsigned) *end);
    }

    // Read the data and store into the static array
    // FIXME: due to some unknown bugs, health_service_get_minute_history() can only fetch
    // a limited number of elements (less than 100). Be carefull, since it will crash 
    // the watch firmware and force the reboot of the watch.
    HealthMinuteData minute_data[MAX_ENTRIES];
    s_num_records = health_service_get_minute_history(&minute_data[0], MAX_ENTRIES, start, end);

    for (int i = 0; i < s_num_records; i++) {
      if (minute_data[i].is_invalid) {
        // No valid data recorded, so minute_date[i] = -1. Treat it as 0 step count.
        s_step_records[i] = 0;
        //APP_LOG(APP_LOG_LEVEL_INFO, "Invalid data %d = %d", (int)i, (int)minute_data[i].steps);
      } else {
        s_step_records[i] = minute_data[i].steps;
        s_steps += s_step_records[i];
        //if (s_step_records[i] > 0) {
        //  APP_LOG(APP_LOG_LEVEL_INFO, "s_step_records %d = %d", (int)i, (int)s_step_records[i]);
        //}
      }
    }
    s_is_loaded = true;
  }
}

/* TODO: debugging function to report the nonsed periods. */
void prv_report_steps(int i) {
  for (int j = i-enamel_get_break_len()-enamel_get_sliding_window()+1; j<=i; j++) {
    APP_LOG(APP_LOG_LEVEL_INFO, "prv_report_steps: j = %d, steps = %d", j, s_step_records[j]);
  }
}
/* 
 * Update steps count. 
 */
void steps_update() {
  int i, break_freq, break_len, sliding_window;
  int nonsed_period = 0;
  s_pass = false;
  //if (!s_is_update) { // FIXME: wanted to avoid multiple updates in the same session (i.e. within 1 minute so that step count will not change at all)
    s_end = time(NULL);
    s_start = s_end - SECONDS_PER_MINUTE * MAX_ENTRIES; 

    // Read recorded step count data from the Pebble Health service.
    s_is_loaded = false; // Force to load new data from Health service.
    APP_LOG(APP_LOG_LEVEL_ERROR, "before prv_load_data");
    prv_load_data(&s_start, &s_end);

    s_inactive_mins = 0;
    
    // Force break frequency <= MAX_ENTRIES, and break length <= break frequency.
    break_freq = enamel_get_break_freq() > MAX_ENTRIES? MAX_ENTRIES : enamel_get_break_freq();
    break_len = enamel_get_break_len() > break_freq ?  break_freq : enamel_get_break_len();
    sliding_window = enamel_get_sliding_window();
      
    // Check whether the condition is met. Update s_pass which is the indicator.
    for (i = 0; i < break_len + sliding_window; i++) {
      if (s_step_records[i] >= enamel_get_step_threshold()) {
        nonsed_period += 1;
      }
    }
    if (nonsed_period >= break_len) {
      s_pass = true;
    } else {
      int prv_i = 0;
      for ( ; i < MAX_ENTRIES; i++, prv_i++) {
        if (s_step_records[i] >= enamel_get_step_threshold()) {
          nonsed_period += 1;
        }
        if (i - prv_i > enamel_get_break_len() + 2) {
          if (s_step_records[prv_i] >= enamel_get_step_threshold()) {
            nonsed_period -= 1;
          }
        }
        if (nonsed_period >= break_len) {
          s_pass = true;
          break;
        }
      }
    }

    // Convert to human readable time for the display purpose.
    //APP_LOG(APP_LOG_LEVEL_ERROR, "enamel_get_sleep_minutes=%d", enamel_get_sleep_minutes());
    if (s_pass) {
      prv_report_steps(i);
      store_increment_break_count();

      s_end = s_start + i;
      s_start = s_end - break_len - sliding_window + 1;
      strftime(s_start_buf, sizeof(s_start_buf), "%H:%M", localtime(&s_start));
      strftime(s_end_buf, sizeof(s_end_buf), "%H:%M", localtime(&s_end));
      APP_LOG(APP_LOG_LEVEL_INFO, "Non-sedentary period from %s to %s", s_start_buf, s_end_buf);
    } else {
      strftime(s_start_buf, sizeof(s_start_buf), "%H:%M", localtime(&s_start));
      strftime(s_end_buf, sizeof(s_end_buf), "%H:%M", localtime(&s_end));
      APP_LOG(APP_LOG_LEVEL_INFO, "%d steps from %s to %s", s_steps, s_start_buf, s_end_buf);
    }
}

/**
 * Send updated info to wakeup_window for displaying on the watch. 
 * This assume steps_update() was called recently.
 */
void steps_wakeup_window_update() { 
  wakeup_window_update(s_steps, s_start_buf, s_end_buf, s_inactive_mins);
}

/** 
 * Return true if the user pass the condition check (i.e. non-sedentary in the period).
 */
bool steps_get_pass() {
  return s_pass;
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

/**
 * Send the latest steps data in the last hour.
 * FIXME: this could be integrated into store_resend_steps(). 
 */
void steps_send_latest(time_t curr_time) {
  time_t start = curr_time - (MAX_ENTRIES * SECONDS_PER_MINUTE);
  steps_send_in_between(start, curr_time, false);

  store_write_update_time(curr_time);
}

/**
 * Fetch the priolr week data.
 * TODO: use this function at the time of installation.
 */
void steps_get_prior_week() {
  int step;
  time_t end, s, e;
  char buf[12];
  end = time(NULL);
  s = end - 7 * SECONDS_PER_DAY;
  while (s < end) {
    s_is_loaded = false;
    step = 0;
    e = s + SECONDS_PER_HOUR;
    prv_load_data(&s, &e);
    for (int i = 0; i < MAX_ENTRIES; i++) {
      step += s_step_records[i]; 
    }
    strftime(buf, sizeof(buf), "%d:%H:%M", localtime(&s));
    APP_LOG(APP_LOG_LEVEL_INFO, "Hisotrical hourly = %s, steps = %d", buf, step);
    s += SECONDS_PER_HOUR;
  }
}

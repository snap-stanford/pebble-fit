#include "steps.h"

#define STEP_BATCH_MAXIMUM_SIZE 60
static uint8_t s_step_batch_records[STEP_BATCH_MAXIMUM_SIZE];
static char s_step_batch_string[STEP_BATCH_MAXIMUM_SIZE * 4]; // Number up to 3 digits + ','
static int s_step_batch_size;

static uint8_t s_step_records[MAX_ENTRIES];
static int s_num_records;
static int s_steps;
static time_t s_start, s_end;

static char s_start_buf[12];
static char s_end_buf[12];
static bool s_pass = false;

// Deprecated. TODO: clean up
//static int s_inactive_mins;
static bool s_is_loaded = false;        // Set this to true to skip prv_load_data()
static bool s_is_update = false;

/* Load health service data between start time and end time into a static array. */
static void prv_load_data(time_t *start, time_t *end) {
  s_steps = 0;

  // Initialize the static array to zeros
  s_num_records = 0;
  for(int i = 0; i < MAX_ENTRIES; i++) {
    s_step_records[i] = 0;
  }

  // Check data is available
  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, *start, *end);
  if (result != HealthServiceAccessibilityMaskAvailable) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No steps data available from %u to %u!", (unsigned) *start, (unsigned) *end);
  }

  // Read the data and store into the static array
  // FIXME: due to some unknown bugs, health_service_get_minute_history() can only fetch
  // a limited number of elements (less than 100). This is why we set MAX_ENTRIES = 60.
  // Otherwise it will crash the watch and force the reboot of the watch.
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
}

/* TODO: debugging function to report the nonsed periods. */
void prv_report_steps(int i) {
  //for (int j = i-enamel_get_break_len()-enamel_get_sliding_window()+1; j<=i; j++) {
  for (int j = i-enamel_get_break_len()+1; j<=i; j++) {
    APP_LOG(APP_LOG_LEVEL_INFO, "prv_report_steps: j = %d, steps = %d", j, s_step_records[j]);
  }
}

/** 
 * Return true if the user pass the condition check (i.e. non-sedentary in the period).
 */
bool steps_get_pass() {
  return s_pass;
}

/** 
 * Get the latest steps count for the latest period, and then check for pass/fail.
 */
void steps_update() {
  int left, right, start_index, break_freq, break_len, sliding_window, step_threshold;
  
  //if (!s_is_update) { // FIXME: wanted to avoid multiple updates in the same session (i.e. within 1 minute so that step count will not change at all)
    s_pass = false;

    // Read recorded step count data from the Pebble Health service.
    s_is_loaded = false; // Force to load new data from Health service.
    //s_end = time(NULL);
    s_end = e_launch_time;
    s_start = s_end - SECONDS_PER_MINUTE * MAX_ENTRIES; 
    //APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: steps_update - before prv_load_data.");
    prv_load_data(&s_start, &s_end);

    //s_inactive_mins = 0;
    // Force break frequency <= MAX_ENTRIES, and break length <= break frequency.
    break_freq = enamel_get_break_freq() > MAX_ENTRIES? MAX_ENTRIES : enamel_get_break_freq();
    break_len = enamel_get_break_len() > break_freq ?  break_freq : enamel_get_break_len();
    sliding_window = enamel_get_sliding_window();
    step_threshold = enamel_get_step_threshold();
      
    //APP_LOG(APP_LOG_LEVEL_ERROR, "bl=%d, bf=%d, sw=%d, threshold=%d", 
    //  break_len, break_freq, sliding_window, step_threshold);
    
    // Check whether the goal is met. Use s_pass as the indicator.
    start_index = e_launch_reason == LAUNCH_WAKEUP_ALERT ? 
     //MAX_ENTRIES-enamel_get_break_freq()+2*break_len : MAX_ENTRIES-enamel_get_break_freq();
     s_num_records-enamel_get_break_freq()+2*break_len : 
     s_num_records-enamel_get_break_freq();
    //APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: s_num_records = %d", s_num_records);
    //APP_LOG(APP_LOG_LEVEL_ERROR, "DEBUG: step_threshold = %d", step_threshold);
    //APP_LOG(APP_LOG_LEVEL_ERROR, "DEBUG: start_index = %d", start_index);
    if (start_index < 0) start_index = 0;
    
    // 1. This is the approach for checking whether step_count > threshold in at least 
    // "break_len" minutes in the last period of "break_freq" minutes.
    int step_count = 0;
    for (left = right = start_index; right < start_index + break_len; right++) {
      step_count += s_step_records[right];
    }
    if (step_count >= step_threshold) {
      s_pass = true;
    } else {
      for ( ; right < s_num_records; right++, left++) {
        step_count += s_step_records[right] - s_step_records[left];
        //APP_LOG(APP_LOG_LEVEL_ERROR, "step_count = %d", step_count);
        if (step_count >= step_threshold) {
          s_pass = true;
          break;
        }
      }
    }

    // 2. This is the approach for checking whether step_count > threshold in at least
    // "break_len" minutes within "break_len+sliding_window" minutes in the last period of
    // "break_freq" minutes. 
    /*
    int nonsed_period = 0;
    // Use the first loop is to initialize.
    for (right = start_index; right < break_len + sliding_window; right++) {
      if (s_step_records[right] >= step_threshold) {
        nonsed_period += 1;
      }
    }
    if (nonsed_period >= break_len) {
      s_pass = true;
    } else {
      left = 0;
      // Count the non-sedentary periods and move both left and right ends.
      for ( ; right < MAX_ENTRIES; right++, left++) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "left=%d,right=%d,stepl=%d stepr=%d,nonsed_period=%d", 
          left, right, s_step_records[left],s_step_records[right], nonsed_period);
        if (s_step_records[right] >= step_threshold) {
          nonsed_period += 1;
        }
        if (//right - left > enamel_get_break_len() + sliding_window &&
            s_step_records[left] >= step_threshold) {
          nonsed_period -= 1;
        }

        // Once we know the goal is met, not need to continue computation.
        if (nonsed_period >= break_len) {
          s_pass = true;
          break;
        }
      }
    }
    */

    // Convert to human readable time for the display purpose.
    //APP_LOG(APP_LOG_LEVEL_ERROR, "enamel_get_sleep_minutes=%d", enamel_get_sleep_minutes());
    if (s_pass) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "pass");
      prv_report_steps(right);
      
      // Only increment score if this is a period-wakeup.
      // TODO: We might only want to check steps at period-wakeup in final version.
      // FIXME: could optimiza by moving to the front of this function to save time.
      APP_LOG(APP_LOG_LEVEL_ERROR, "%d", e_launch_reason);
      if (e_launch_reason == LAUNCH_WAKEUP_PERIOD) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "ABSZ store_increment_curr_score");
        store_increment_curr_score();
      }

      s_end = s_start + right * SECONDS_PER_MINUTE;
      s_start = s_end - (break_len + sliding_window - 1) * SECONDS_PER_MINUTE;
      strftime(s_start_buf, sizeof(s_start_buf), "%H:%M", localtime(&s_start));
      strftime(s_end_buf, sizeof(s_end_buf), "%H:%M", localtime(&s_end));
      APP_LOG(APP_LOG_LEVEL_ERROR, "Non-sedentary period from %s to %s", s_start_buf, s_end_buf);
    } else {
      strftime(s_start_buf, sizeof(s_start_buf), "%H:%M", localtime(&s_start));
      strftime(s_end_buf, sizeof(s_end_buf), "%H:%M", localtime(&s_end));
      APP_LOG(APP_LOG_LEVEL_ERROR, "%d steps from %s to %s", s_steps, s_start_buf, s_end_buf);
    }
}

/* Write steps array data to dict. */
static void prv_data_write(DictionaryIterator * out) {
  int true_value = 1;

  dict_write_int(out, AppKeyStepsData, &true_value, sizeof(int), true);

  dict_write_int(out, AppKeyDate, &s_start, sizeof(int), true);

  dict_write_cstring(out, AppKeyStringData, s_step_batch_string);

  //dict_write_int(out, AppKeyArrayLength, &s_num_records, sizeof(int), true);
  //dict_write_int(out, AppKeyArrayStart, &AppKeyArrayData, sizeof(int), true);
  //for (int i = 0; i < s_num_records; i++) {
  //  dict_write_uint8(out, AppKeyArrayData + i, s_step_records[i]);
  //}
}

/**
 * Upload steps data since the last upload time until the current launch time.
 * The data of the last upload time needs to be uploaded as well (inclusive).
 *
 * Return true if it has uploaded all the neccessary data. Otherwise return false (this
 * function will be called again to once receive ACK from the server).
 */
bool steps_upload_steps() {
  int i, fill_index = 0;
  time_t t_start, t_end, t_final;
  time_t t_last_upload = store_read_upload_time();
  time_t break_freq_seconds = enamel_get_break_freq() * SECONDS_PER_MINUTE;

  s_step_batch_size = 0;

  if (t_last_upload == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: No upload time existed.");

    // TODO: only send 2 hours history. (later on 7 days)
    t_last_upload = time_start_of_today() - 2 * SECONDS_PER_HOUR;
  } else if (t_last_upload > e_launch_time -  1 * SECONDS_PER_MINUTE) {
    // We arbitrarily stop uploading data at 1 minute before the current launch time.
    return true;
  }
  t_last_upload = t_last_upload / SECONDS_PER_MINUTE * SECONDS_PER_MINUTE; // Rounding.

  // DEBUG
  char buf[32];
  strftime(buf, sizeof(buf), "%d %H:%M", localtime(&t_last_upload));
  APP_LOG(APP_LOG_LEVEL_ERROR, "t_last_upload=%u, %s",  (unsigned)t_last_upload, buf);
  // DEBUG

  // Load the data. Exclude the last min. Up to the maximum batch size.
  t_start = s_start = t_last_upload;
  t_final = e_launch_time - SECONDS_PER_MINUTE <= 
            t_start + STEP_BATCH_MAXIMUM_SIZE * SECONDS_PER_MINUTE?
              e_launch_time - SECONDS_PER_MINUTE : 
              t_start + STEP_BATCH_MAXIMUM_SIZE * SECONDS_PER_MINUTE;
  while (t_start < t_final) {
    // Read data into a temporary array with limited size (MAX_ENTRIES). 
    t_end = t_start + MAX_ENTRIES * SECONDS_PER_MINUTE;
    // DEBUG
    char buf[12], bufe[12];
    strftime(buf, sizeof(buf), "%d/%H:%M", localtime(&t_start));
    strftime(bufe, sizeof(bufe), "%d/%H:%M", localtime(&t_end));
    APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: loading data between %s-%s", buf, bufe);
    // DEBUG
    prv_load_data(&t_start, &t_end);

    // Move data into an accumalative string/char array.
    for (i = 0; i < MAX_ENTRIES; i++) {
      // TODO: load the actual data.
      //s_prior_step_records[fill_index] = s_step_records[i];
      
      int step = i;
      if (step > 99) {
        s_step_batch_string[fill_index++] = step / 100 + '0';
        step %= 100;
        s_step_batch_string[fill_index++] = step / 10 + '0';
        s_step_batch_string[fill_index++] = step % 10 + '0';
      } else if (step > 9) {
        s_step_batch_string[fill_index++] = step / 10 + '0';
        s_step_batch_string[fill_index++] = step % 10 + '0';
      } else {
        s_step_batch_string[fill_index++] = step + '0';
      }
      s_step_batch_string[fill_index++] = ',';
    }

    t_start = t_end + 1; // Prepare for the next load_data() function
  }
  s_step_batch_string[fill_index-1] = '\0'; // Remove the tailing ','
  s_step_batch_size = fill_index-1; 

  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: Done loading history data, fill_index=%d, strlen=%d", 
    fill_index, strlen(s_step_batch_string));
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: t_end = %u, diff = %u", (unsigned)t_end,
      (unsigned)(t_end-t_last_upload));
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: data = %s", s_step_batch_string);

  comm_send_data(prv_data_write, comm_sent_handler, comm_server_received_handler);
  
  // Update the last upload time stored persistently.
  t_last_upload += s_step_batch_size * SECONDS_PER_MINUTE; 
  //t_last_upload = e_launch_time - 2 * SECONDS_PER_HOUR; // TODO: delete
  persist_write_data(PERSIST_KEY_UPLOAD_TIME, &t_last_upload, sizeof(time_t));

  return false;
}


/* Send steps in the time frame of 60 minutes. */
void steps_send_in_between(time_t t_start, time_t end, bool force) {
  //if (force) {
  //  // Force prv_load_data to load load from Pebble Health.
  //  s_is_loaded = false;
  //}
  s_start = t_start;
  prv_load_data(&t_start, &end);

  if (s_num_records == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No new steps data to send.");
    return;
  }

  comm_send_data(prv_data_write, comm_sent_handler, comm_server_received_handler);
}

/**
 * Send the latest steps data in the last hour.
 * FIXME: this could be integrated into store_resend_steps(). 
 */
void steps_send_latest() {
  s_start = e_launch_time - (MAX_ENTRIES * SECONDS_PER_MINUTE);

  steps_send_in_between(s_start, e_launch_time, false);

  store_write_upload_time(e_launch_time);
}


// Functions for sending the step data in the last week (supposed to be used when user first
// install the app). 
#define STEP_PRIOR_BATCH_SIZE 180
static uint8_t s_prior_step_records[STEP_PRIOR_BATCH_SIZE];
static char s_prior_step_records_string[STEP_PRIOR_BATCH_SIZE*4];
static int s_prior_num_records;
static time_t s_prior_start;
static void prv_prior_data_write(DictionaryIterator * out) {
  //write the data
  int true_value = 1;
  dict_write_int(out, AppKeyStepsData, &true_value, sizeof(int), true);

  dict_write_int(out, AppKeyDate, &s_prior_start, sizeof(int), true);

  dict_write_cstring(out, AppKeyStringData, s_prior_step_records_string);

  //APP_LOG(APP_LOG_LEVEL_ERROR, "dict_size = %u", (unsigned) dict_size(out));
  //for (int i = 0; i < s_prior_num_records; i++) {
  //  dict_write_uint8(out, AppKeyArrayData + i, s_prior_step_records[i]);
  //}
  //APP_LOG(APP_LOG_LEVEL_ERROR, "dict_size = %u", (unsigned) dict_size(out));
}

/**
 * Fetch the step data in the last week and then upload to the server. 
 * TODO: optimize to reduce upload time.
 */
void steps_upload_prior_week() {
  // Use a larger buffer to send the historical data.
  events_app_message_request_outbox_size(4096);
  events_app_message_open(); // Call pebble-events app_message_open function
  
  int i, fill_index = 0, curr = 0;
  time_t t_start, t_end, t_final;

  s_prior_num_records = 0;
  //s = e_launch_time - 7 * SECONDS_PER_DAY;
  // TODO: For now only send limited data.
  t_final = e_launch_time - MAX_ENTRIES * SECONDS_PER_MINUTE; // not include last period.
  t_start = s_prior_start = t_final - STEP_PRIOR_BATCH_SIZE * SECONDS_PER_MINUTE + 1; 
  while (t_start < t_final) {
    // Read data into a temporary array with limited size. Due to the fact we cannot load
    // more than a certain amount of data in a given call.
    s_is_loaded = false;
    t_end = t_start + MAX_ENTRIES * SECONDS_PER_MINUTE;

    // DEBUG
    char buf[12], bufe[12];
    strftime(buf, sizeof(buf), "%d/%H:%M", localtime(&t_start));
    strftime(bufe, sizeof(bufe), "%d/%H:%M", localtime(&t_end));
    APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: loading history data at %s-%s", buf, bufe);
    // DEBUG
    //prv_load_data(&t_start, &t_end);

    // Move data into an accumalative array.
    for (i = 0; i < MAX_ENTRIES; i++, curr++) {
      //s_prior_step_records[curr] = i; // TODO: load the actual data.
      
      int step = 100+i;
      if (step > 99) {
        s_prior_step_records_string[fill_index++] = step / 100 + '0';
        step %= 100;
        s_prior_step_records_string[fill_index++] = step / 10 + '0';
        s_prior_step_records_string[fill_index++] = step % 10 + '0';
      } else if (step > 9) {
        s_prior_step_records_string[fill_index++] = step / 10 + '0';
        s_prior_step_records_string[fill_index++] = step % 10 + '0';
      } else {
        s_prior_step_records_string[fill_index++] = step + '0';
      }
      s_prior_step_records_string[fill_index++] = ',';
    }

    s_prior_num_records += MAX_ENTRIES;
    t_start = t_end + 1;
  }
  s_prior_step_records_string[fill_index-1] = '\0'; // Remove the tailing ','

  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: Done loading history data, fill_index=%d", fill_index);
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: data = %s", s_prior_step_records_string);

  comm_send_data(prv_prior_data_write, comm_sent_handler, comm_server_received_handler);
}



/**
 * DEBUG
 * Send updated info to wakeup_window for displaying on the watch. 
 * This assume steps_update() was called recently.
 */
void steps_wakeup_window_update() { 
  //wakeup_window_update(s_steps, s_start_buf, s_end_buf, s_inactive_mins);
  wakeup_window_update(s_steps, s_start_buf, s_end_buf, 0);
}


/* Return the inactive minutes. */
//int steps_get_inactive_minutes() {
//  return s_inactive_mins;
//}

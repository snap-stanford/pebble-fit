#include "wakeup.h"

/** 
 * Get the wakeup ID from persistent storage at a given index. 
 */
static WakeupId prv_read_wakeup_id_pm(uint8_t wakeup_i) {
  WakeupId wakeup_id = -1;
  if (wakeup_i < NUM_USED_WAKEUP) {
    WakeupId wakeup_array[NUM_USED_WAKEUP];
    persist_read_data(PERSIST_KEY_WAKEUP_ID, wakeup_array, sizeof(wakeup_array));
    wakeup_id = wakeup_array[wakeup_i];
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "The given wakeup_i %d is out of range", (int)wakeup_i);
  }
  return wakeup_id;
}

/** 
 * Set the wakeup ID of a given index in persistent storage. 
 */
void prv_write_wakeup_id_pm(uint8_t wakeup_i, WakeupId wakeup_id) {
  if (wakeup_i < NUM_USED_WAKEUP) {
    WakeupId wakeup_array[NUM_USED_WAKEUP];
    persist_read_data(PERSIST_KEY_WAKEUP_ID, wakeup_array, sizeof(wakeup_array));
    wakeup_array[wakeup_i] = wakeup_id;
    persist_write_data(PERSIST_KEY_WAKEUP_ID, wakeup_array, sizeof(wakeup_array));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "The given wakeup_i %d is out of range", (int)wakeup_i);
  }
}

/** 
 * Reschedule a wakeup event given its persistent storage index and next timestamp. 
 */
static WakeupId prv_reschedule_wakeup_event(uint8_t wakeup_i, time_t wakeup_time) {
  WakeupId wakeup_id = -1;
  uint8_t try = 0;

  if ((wakeup_i < NUM_TOTAL_WAKEUP) && (wakeup_time > time(NULL))) {
    wakeup_id = prv_read_wakeup_id_pm(wakeup_i);
    //APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling wakeup_id %d", (int)wakeup_id);
    wakeup_cancel(wakeup_id);
    
    // Automatically retry for a fixed number of iterations (increment by minutes)
    do {
      wakeup_id = wakeup_schedule(wakeup_time + try * SECONDS_PER_MINUTE, 
        (int32_t)wakeup_i, false);
      try++;
    } while (wakeup_id < 0 && try <= MAX_RETRY); 

    if (wakeup_id < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, 
        "Failed to reschedule wakeup_i %d for wakeup_time %d with wakeup_id %d",
        (int)wakeup_i, (int)wakeup_time, (int)wakeup_id);
    } else {
      // Debug message
      char buf[16];
      wakeup_time += --try * SECONDS_PER_MINUTE;
      strftime(buf, sizeof(buf), "%d,%H:%M:%S", localtime(&wakeup_time));
      APP_LOG(APP_LOG_LEVEL_INFO, "Schedule wakeup NO.%d on %s with wakeup_id %d",
        (int) wakeup_i, buf, (int)wakeup_id);
    }
    prv_write_wakeup_id_pm(wakeup_i, wakeup_id);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "The given wakeup_i %d or wakeup_time %d is invalid", 
      (int)wakeup_i, (int)wakeup_time);
  }

  return wakeup_id;
}

/* Schedule the next wakeup event and reschedule fallback wakeup events.
 * Try to set fallback wakeups also be within the start and end time
 * There are in total 4 wakeup events being scheduled.
 *  Cookie / Index         | Description
 *  0                      | Fallback wakeup: 1 days later 
 *  1                      | Fallback wakeup: 2 days later 
 *  2                      | Fallback wakeup: 3 days later 
 *  3/LAUNCH_WAKEUP_PERIOD | Next periodic wakeup: rounded to the break_freq minutes. 
 *  4/LAUNCH_WAKEUP_NOTIFY | Notification wakeup: 2 * break_len before the next periodic wakeup. 
 *  5/LAUNCH_WAKEUP_DAILY  | Daily wakeup: at the end time of the day. 
 */
//void wakeup_schedule_events(int inactive_mins) {
void wakeup_schedule_events() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
  wakeup_cancel_all();
  
  time_t t_notify, t_wakeup;
  time_t break_freq_seconds = (time_t)enamel_get_break_freq() * SECONDS_PER_MINUTE;
  time_t break_len_seconds = (time_t)enamel_get_break_len() * SECONDS_PER_MINUTE;

  // The first periodic wakeup event should be one period after the start time.
  // If end time is smaller than start time, treat it as overnight exercise...
  // Another alternative is to prevent this kind of input on Clay settings.
  time_t t_sod = time_start_of_today();
  time_t t_start = t_sod + enamel_get_daily_start_time() + break_freq_seconds;
  time_t t_end = t_sod + enamel_get_daily_end_time();
  if (t_end <= t_start) { t_end += SECONDS_PER_DAY; }

  // Debug messages
  char curr_buf[12], start_buf[12], end_buf[12];
  strftime(curr_buf, sizeof(curr_buf),"%H:%M:%S", localtime(&e_launch_time));
  strftime(start_buf, sizeof(start_buf),"%H:%M:%S", localtime(&t_start));
  strftime(end_buf, sizeof(end_buf), "%H:%M:%S", localtime(&t_end));
  APP_LOG(APP_LOG_LEVEL_INFO,"group=%s, curr=%s, start=%s, end=%s", 
    enamel_get_group(), curr_buf, start_buf, end_buf);

  if (strncmp(enamel_get_group(), "real_time", strlen("real_time")) == 0) {
    if (enamel_get_dynamic_wakeup() == true) { // TODO: dynamic wakeup is deprecated.
      APP_LOG(APP_LOG_LEVEL_ERROR, "Dynamic wakeup is deprecated!");
      //if (enamel_get_break_freq() - inactive_mins <= MIN_SLEEP_MINUTES) {
      //  t_wakeup = e_launch_time + MIN_SLEEP_MINUTES * SECONDS_PER_MINUTE;
      //} else {
      //  t_wakeup = e_launch_time + (enamel_get_break_freq()-inactive_mins) * SECONDS_PER_MINUTE;
      //}
    } 

    // Compute the next period-wakeup (i.e. break_freq minutes later). Rounding up the result. 
    // Note: use the current timestamp instead of the launch timestamp, since the launch 
    // timestamp might be serveral minutes ago if the app wakes up while the app is on (i.e. 
    // manually launched by the user).
    t_wakeup = (time(NULL)+break_freq_seconds) / break_freq_seconds * break_freq_seconds;

    // Boundary conditions checking
    if (t_wakeup < t_start) {
      t_wakeup = t_start;
    } else if (t_wakeup > t_end) {
      t_wakeup = t_start + SECONDS_PER_DAY;
    }

    // Schedule periodic wakeup
    prv_reschedule_wakeup_event(LAUNCH_WAKEUP_PERIOD, t_wakeup);
    
    // Schedule notification wakeup
    t_notify = t_wakeup - 2 * break_len_seconds;
    prv_reschedule_wakeup_event(LAUNCH_WAKEUP_NOTIFY, t_notify);
  } else if (strncmp(enamel_get_group(), "daily_message", strlen("daily_message")) == 1) {
    // Schedule end-of-day wakeup only
    prv_reschedule_wakeup_event(LAUNCH_WAKEUP_DAILY, t_end);
  } // else the group must be "passive_tracking, so no normal wakeup scheduled.

  // Schedule the fallback wakeup events
  prv_reschedule_wakeup_event(0, t_start + SECONDS_PER_DAY);
  prv_reschedule_wakeup_event(1, t_start + 2 * SECONDS_PER_DAY);
  prv_reschedule_wakeup_event(2, t_start + 3 * SECONDS_PER_DAY);
}

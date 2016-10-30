#include "wakeup.h"

/* Set the wakeup to be X minutes from now. */
//static void prv_wakeup_set_minutes_from_now(double min) {
//  time_t timestamp = time(NULL) + (min * SECONDS_PER_MINUTE);
//  const int cookie = 0;
//  const bool notify_if_missed = false;
//  wakeup_schedule(timestamp, cookie, notify_if_missed);
//  APP_LOG(APP_LOG_LEVEL_INFO, "Set wakeup ~%d minutes from now", (int) min);
//}

/* Cancel a specific scheduled wakeup event. */
static void prv_cancel_event(){
  APP_LOG(APP_LOG_LEVEL_INFO, "in prv_cancel_event");
  uint32_t key = WAKEUP_ID_PERSIST_KEY;
  if (persist_exists(key)) {
    WakeupId wakeup_id = persist_read_int(key);
    time_t wakeup_timestamp = 0;
    if (wakeup_query(wakeup_id, &wakeup_timestamp)) {
      //APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling previously scheduled wakeup event at %s, ");
      wakeup_cancel(wakeup_id);
    }
  }
}

/* Get the timestamp of a specific wakeup event. */
//static time_t prv_event_timestamp() {
//  uint32_t key = WAKEUP_ID_PERSIST_KEY;
//  if (persist_exists(key)) {
//    WakeupId wakeup_id = persist_read_int(key);
//    time_t wakeup_timestamp = 0;
//    if (wakeup_query(wakeup_id, &wakeup_timestamp)) {
//      return wakeup_timestamp;
//    }
//  }
//  return -1;
//}

/* Get the wakeup ID from persistent storage at a given index. */
static WakeupId prv_read_wakeup_id_pm(uint8_t wakeup_i) {
  WakeupId wakeup_id = -1;
  if (wakeup_i < NUM_USED_WAKEUP) {
    WakeupId wakeup_array[NUM_USED_WAKEUP];
    persist_read_data(WAKEUP_ID_PERSIST_KEY, wakeup_array, sizeof(wakeup_array));
    wakeup_id = wakeup_array[wakeup_i];
  } else {
  	APP_LOG(APP_LOG_LEVEL_ERROR, "The given wakeup_i %d is out of range", (int)wakeup_i);
	}
  return wakeup_id;
}

/* Set the wakeup ID of a given index in persistent storage. */
void prv_write_wakeup_id_pm(uint8_t wakeup_i, WakeupId wakeup_id) {
  if (wakeup_i < NUM_USED_WAKEUP) {
    WakeupId wakeup_array[NUM_USED_WAKEUP];
    persist_read_data(WAKEUP_ID_PERSIST_KEY, wakeup_array, sizeof(wakeup_array));
    wakeup_array[wakeup_i] = wakeup_id;
    persist_write_data(WAKEUP_ID_PERSIST_KEY, wakeup_array, sizeof(wakeup_array));
  } else {
  	APP_LOG(APP_LOG_LEVEL_ERROR, "The given wakeup_i %d is out of range", (int)wakeup_i);
	}
}

/* Reschedule a wakeup event given its persistent storage index and next timestamp. */
WakeupId prv_reschedule_wakeup_event(uint8_t wakeup_i, time_t wakeup_time_t) {
  WakeupId wakeup_id = -1;
  uint8_t try = 0;

  if ((wakeup_i < NUM_TOTAL_WAKEUP) && (wakeup_time_t > time(NULL))) {
    wakeup_id = prv_read_wakeup_id_pm(wakeup_i);
    wakeup_cancel(wakeup_id);
	  while (wakeup_id < 0 && try <= MAX_RETRY) {
    	wakeup_id = wakeup_schedule(wakeup_time_t, (int32_t)wakeup_i, false);
		}
		if (wakeup_id < 0) {
    	APP_LOG(APP_LOG_LEVEL_ERROR, 
				"Failed to reschedule wakeup_i %d for wakeup_time_t %d with wakeup_id %d",
        (int)wakeup_i, (int)wakeup_time_t, (int)wakeup_id);
		}
    prv_write_wakeup_id_pm(wakeup_i, wakeup_id);
  } else {
  	APP_LOG(APP_LOG_LEVEL_ERROR, "The given wakeup_i %d is out of range", (int)wakeup_i);
	}

  APP_LOG(APP_LOG_LEVEL_INFO, 
		"reschedule_wakeup_event(): wakeup_i %d for time_t %d with wakeup_id %d",
    (int) wakeup_i,(int) wakeup_time_t, (int)wakeup_id);

  return wakeup_id;
}


/* Schedule the next wakeup event and reschedule fallback wakeup events.
 *  Index		| Description
 * 	0				| depends on the value set by Clay configuration from the phone
 * 	1				| in 12 hours from now, 
 *  2				| in 1 hour before the start time of tomorrow 
 * 	3				| in 7 days @ 12:01 AM
 */
void schedule_wakeup_events(bool force) {
	time_t t_curr  = time(NULL);
	time_t t_sod   = time_start_of_today();
  time_t t_start = t_sod + enamel_get_daily_start_time();
  time_t t_end   = t_sod + enamel_get_daily_end_time() ;

  //force = force || t_event < prv_event_timestamp(WAKEUP_ID_PERSIST_KEY); // TODO
  if(force){
    //prv_cancel_event();
  	time_t t_event = t_curr + enamel_get_sleep_minutes() * SECONDS_PER_MINUTE;
  	if (t_end <= t_start) { t_end += SECONDS_PER_DAY; }
  	if (t_event < t_start) {
  	  t_event = t_start;
  	} else if (t_event > t_end) {
  	  t_event = t_start + SECONDS_PER_DAY;
  	}
		prv_reschedule_wakeup_event(1, t_curr + 12 * SECONDS_PER_HOUR);
  }

	prv_reschedule_wakeup_event(1, t_curr + 12 * SECONDS_PER_HOUR);
	prv_reschedule_wakeup_event(2, t_start - 1 * SECONDS_PER_HOUR);
	prv_reschedule_wakeup_event(3, time_start_of_today() + SECONDS_PER_WEEK + 60);
}

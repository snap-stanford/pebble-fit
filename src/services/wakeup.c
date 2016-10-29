#include "wakeup.h"
#include "enamel.h"

#define WKUP_ID   66

/* Set the wakeup to be X minutes from now. */
//static void prv_wakeup_set_minutes_from_now(double min) {
//  time_t timestamp = time(NULL) + (min * SECONDS_PER_MINUTE);
//  const int cookie = 0;
//  const bool notify_if_missed = false;
//  wakeup_schedule(timestamp, cookie, notify_if_missed);
//  APP_LOG(APP_LOG_LEVEL_INFO, "Set wakeup ~%d minutes from now", (int) min);
//}

static void prv_cancel_event(){
  APP_LOG(APP_LOG_LEVEL_INFO, "in prv_cancel_event");
  uint32_t key = WKUP_ID;
  if (persist_exists(key)) {
    WakeupId wakeup_id = persist_read_int(key);
    time_t wakeup_timestamp = 0;
    if (wakeup_query(wakeup_id, &wakeup_timestamp)) {
      //APP_LOG(APP_LOG_LEVEL_INFO, "Cancelling previously scheduled wakeup event at %s, ");
      wakeup_cancel(wakeup_id);
    }
  }
}

static time_t prv_event_timestamp() {
  uint32_t key = WKUP_ID;
  if (persist_exists(key)) {
    WakeupId wakeup_id = persist_read_int(key);
    time_t wakeup_timestamp = 0;
    if (wakeup_query(wakeup_id, &wakeup_timestamp)) {
      return wakeup_timestamp;
    }
  }
  return -1;
}

void schedule_event(bool force){
  WakeupId id = -1;
  uint8_t try = 0;

  time_t t_event = time(NULL) + enamel_get_sleep_minutes() * SECONDS_PER_MINUTE;
	time_t t_sod   = time_start_of_today();
  time_t t_start = t_sod + enamel_get_daily_start_time();
  time_t t_end   = t_sod + enamel_get_daily_end_time() ;
  
  if (t_end <= t_start) { t_end += SECONDS_PER_DAY; }
  if (t_event < t_start) {
    t_event = t_start;
  } else if (t_event > t_end) {
    t_event = t_start + SECONDS_PER_DAY;
  }

  force = force || t_event < prv_event_timestamp(WKUP_ID);
  if(force){
    prv_cancel_event();
    while(id < 0 && try < 5){
      APP_LOG(APP_LOG_LEVEL_INFO, "try to schedule at timestamp=%d", (int)t_event);
      id = wakeup_schedule(t_event + SECONDS_PER_MINUTE * try, WKUP_ID, true);
      try++;
    }
    if (id >= 0){
      persist_write_int(WKUP_ID, id);
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Fail to schedule a wakeup at time=%d with status=%d", (int)t_event, (int)id);
    }
  }
}

#include "store.h"

//static int s_data[MAX_ENTRIES];
static int s_num_records;
static int latest_key;

static uint8_t s_launchexit_count;
static uint32_t s_launchexit_data[3];

/* 
 * The format of launch-exit record stored persistently (compact to save space).
 * Total size: 4 bytes
 * 28 bits                  | 2 bits   | 2 bits  
 * exit (secs after launch) | l reason | e reason
 */
#define LAUNCHEXIT_DATA_SIZE    12 
#define LAUNCHEXIT_COUNT_SIZE   1
#define COMPACT(es, lr, er)     ((es&0x0FFFFFFF)<<4 | (lr&0x3)<<2 | (er&0x3))
#define GET_SECOND(x)           ((x>>4)  & 0x0FFFFFFF)
#define GET_LAUNCH_REASON(x)    ((x>>2)  & 0x0003)
#define GET_EXIT_REASON(x)      (x       & 0x0003)

/**
 * Write into the persistent storage the latest timestamp at which we update configuration.
 */
void store_write_config_time(time_t time) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Store configuration time = %u", (unsigned) time);
  persist_write_data(PERSIST_KEY_CONFIG_TIME, &time, sizeof(time_t));
}

/**
 * Return true if we need to send new configuration request to the server, otherwise false.
 * Note: Since currently we will reset the daily step status (store_read_break_count()), we 
 *   must avoid sending config_request if the user has accomplished any daily break goal.
 *   Make sure there is a non-Period Wakup scheduled daily before any Period Wakeup.
 */
bool store_resend_config_request(time_t t_curr) {
  if (!persist_exists(PERSIST_KEY_CONFIG_TIME)) {
    return true;
  }

  time_t last_config_time;

  persist_read_data(PERSIST_KEY_CONFIG_TIME, &last_config_time, sizeof(time_t));

  if (store_read_break_count() == 0 && 
			t_curr-last_config_time > atoi(enamel_get_config_update_interval()) * SECONDS_PER_DAY) {
    return true;
  } else {
    return false;
  }
}

/**
 * Write the latest timestamp at which we upload steps data.
 */
void store_write_upload_time(time_t time) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Store update time = %u", (unsigned) time);
  persist_write_data(PERSIST_KEY_UPLOAD_TIME, &time, sizeof(time_t));
}

/**
 * Return the timestamp of the last time we upload steps data to the server.
 */
time_t store_read_upload_time() {
  time_t res;

  persist_read_data(PERSIST_KEY_UPLOAD_TIME, &res, sizeof(time_t));

  return res;
}

/**
 * Write launch and exit events into the persistent storage 
 */
void store_write_launchexit_event(time_t t_launch, time_t t_exit, uint8_t lr, uint8_t er) {
  time_t t_diff = t_exit - t_launch; 

  // Convert message ID to ascii code (assuming each ID is 4 bytes)
  const char *msg_id = launch_get_random_message_id();
  uint32_t msg_id_ascii = 0;
  for (int i = 0; i < 4; i++) {
    msg_id_ascii <<= 8;
    msg_id_ascii |= (int)msg_id[i] & 0xFF;
  }
  
  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 1);
  } else {
    s_launchexit_count = 0;
  }

  // TODO: should implement as circular buffer instead of linear array.
  if (s_launchexit_count > PERSIST_KEY_LAUNCH_END - PERSIST_KEY_LAUNCH_START + 1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "s_launchexit_count less than zero = %u", s_launchexit_count);
    s_launchexit_count = 0;
  }
  s_launchexit_data[0] = t_launch;
  s_launchexit_data[1] = COMPACT(t_diff, lr, er);
  s_launchexit_data[2] = msg_id_ascii;
  s_launchexit_count++;

  // Write data first. Can tolerate losing data to avoid uploading wrong data to the server.
  persist_write_data(PERSIST_KEY_LAUNCH_START+s_launchexit_count-1, 
    &s_launchexit_data, LAUNCHEXIT_DATA_SIZE);
  persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);

  // DEBUG
  APP_LOG(APP_LOG_LEVEL_INFO, "Write launch and exit events to persistent storage" \
      ". new records count=%d.", s_launchexit_count);
  APP_LOG(APP_LOG_LEVEL_INFO, "t_launch=%u, t_exit=%u", (unsigned int)t_launch, (unsigned int)t_exit);
  APP_LOG(APP_LOG_LEVEL_INFO, "msg_id=%s, msg_id_ascii=%08x, t_diff=%u, lr=%d, er=%d", 
    msg_id, (unsigned int)msg_id_ascii, (unsigned int)t_diff, lr, er);
}

// Deprecated.
// 11 bits                 | 17 bits                  | 2 bits   | 2 bits  
// launch (mins since SoD) | exit (secs after launch) | l reason | e reason
//#define COMPACT(lm, es, lr, er) (lm<<21 | (es&0x1ffff)<<4 | (lr&0x3)<<2 | (er&0x3))
/*
void deprecated_store_write_launchexit_event(time_t t_launch, time_t t_exit, uint8_t lr, uint8_t er) {
  time_t minutes, seconds;

  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 1);
  }

  if (s_launchexit_count > PERSIST_KEY_LAUNCH_END - PERSIST_KEY_LAUNCH_START + 1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "s_launchexit_count less than zero = %u", s_launchexit_count);
    s_launchexit_count = 0;
  }

  // Find the number of minutes since SoD (rounded to the nearest minute)
  // Always round down for consistency.
  //minutes = (t_launch - time_start_of_today() + SECONDS_PER_MINUTE/2) / SECONDS_PER_MINUTE;
  minutes = (t_launch - time_start_of_today()) / SECONDS_PER_MINUTE;
  seconds = t_exit - t_launch;
  s_launchexit_data = COMPACT(minutes, seconds, lr, er);
  s_launchexit_count++;

  // DEBUG
  APP_LOG(APP_LOG_LEVEL_INFO, "Write launch and exit events to persistent storage" \
      ". new records count=%d.", s_launchexit_count);
  APP_LOG(APP_LOG_LEVEL_INFO, "t_launch=%u, t_exit=%u", (unsigned int)t_launch, (unsigned int)t_exit);
  APP_LOG(APP_LOG_LEVEL_INFO, "min=%u, seconds=%u, lr=%d, er=%d, data=%08x", (unsigned int)minutes, (unsigned int)seconds, lr, er, (unsigned int)s_launchexit_data);
  
  // Write data first. Can tolerate losing data to avoid uploading wrong data to the server.
  persist_write_data(PERSIST_KEY_LAUNCH_START+s_launchexit_count-1, &s_launchexit_data, LAUNCHEXIT_DATA_SIZE);
  persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);
}
*/

/**
 * Return whether we finish resending launch/exit event (only when we do not send any data 
 * to the server. 
 */
bool store_resend_launchexit_event() {
  // FIXME: consider using a single key and sequential storage location
  uint32_t key, msg_id_ascii;
  time_t t_launch, t_exit;
  uint8_t lr, er;
  char msg_id[5];

  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);

    if (s_launchexit_count <= 0) {
      return true;
    }

    // Read each record and try to resend to the server; decrement count.
    // FIXME: instead of sending all records to the server, only send one at a time.
    // When ACK comes back, this function will be called again.
    key = PERSIST_KEY_LAUNCH_START + s_launchexit_count - 1;
    if (persist_exists(key) && 
        key >= PERSIST_KEY_LAUNCH_START && 
        key <= PERSIST_KEY_LAUNCH_END) {
      persist_read_data(key, &s_launchexit_data, LAUNCHEXIT_DATA_SIZE);

      t_launch = s_launchexit_data[0];

      t_exit = s_launchexit_data[1];
      lr = GET_LAUNCH_REASON(t_exit);
      er = GET_EXIT_REASON(t_exit);
      t_exit = t_launch + GET_SECOND(t_exit);

      // Convert message ID from ascii code back to char (assuming each ID is 4 bytes)
      msg_id_ascii = s_launchexit_data[2];
      for (int i = 3; i >= 0; i--) {
        msg_id[i] = (char)msg_id_ascii;
        msg_id_ascii >>= 8;
      }
      msg_id[4] = '\0';

      launch_resend(t_launch, t_exit, msg_id, lr, er);

      persist_delete(key);
      s_launchexit_count--;

      // DEBUG
      APP_LOG(APP_LOG_LEVEL_INFO, "Resend launch and exit events to the server" \
          ". new records count=%d.", s_launchexit_count);
      APP_LOG(APP_LOG_LEVEL_INFO, "t_launch=%u, t_exit=%u, msg_id_ascii=%04x, lr=%d, er=%d", 
        (unsigned int)t_launch, (unsigned int)t_exit, (unsigned int)msg_id_ascii,
        (int)lr, (int)er);
    }

    persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);
    return false;
  } else {
    return true;
  }
}

/**
 * Return whether we finish resending steps data (only when we do not send any data 
 * to the server. Later in steps_send_latest()  we might resend some data that we are 
 * sending in here.
 */
bool store_resend_steps(time_t t_curr) {
  time_t t_last_upload; 
  time_t interval_seconds = enamel_get_break_freq() * SECONDS_PER_MINUTE;
  //t_last_upload = (t_last_upload < time_start_of_today());

  if (persist_exists(PERSIST_KEY_UPLOAD_TIME)) {
    persist_read_data(PERSIST_KEY_UPLOAD_TIME, &t_last_upload, sizeof(time_t));
    if (t_last_upload  < time_start_of_today() - 2 * SECONDS_PER_DAY) {
      // If last upload time is ealier than 2 days ago, only upload starting at 2 days ago
      t_last_upload = time_start_of_today() - 2 * SECONDS_PER_DAY;
    } else if (t_last_upload >= t_curr - interval_seconds) {
      // Data in the lastest 60 minutes will be sent by the normal data upload routine.
      return true;
    }

    steps_send_in_between(t_last_upload, t_last_upload + interval_seconds, true);

    t_last_upload += interval_seconds;
    persist_write_data(PERSIST_KEY_UPLOAD_TIME, &t_last_upload, sizeof(time_t));
    //return t_last_upload < t_curr - interval_seconds;
    return false;
  } else {
    return true;
  }
}

/**
 * Reset the break count to 0. This should be performed in the first wakeup daily.
 */
void store_reset_break_count() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "RESET break count");
  persist_write_int(PERSIST_KEY_BREAK_COUNT, 0); 
}

/**
 * Increment the break count by 1. If it has not existed, set it to 1.
 * Also record the current time associated with this break count increment.
 */
void store_increment_break_count() {
  if (!persist_exists(PERSIST_KEY_BREAK_COUNT)) {
    persist_write_int(PERSIST_KEY_BREAK_COUNT, 1); 
  } else {
    persist_write_int(PERSIST_KEY_BREAK_COUNT, persist_read_int(PERSIST_KEY_BREAK_COUNT)+1); 
  }
  time_t t_curr = time(NULL);
  persist_write_data(PERSIST_KEY_BREAK_COUNT_TIME, &t_curr, sizeof(time_t));
}

/**
 * Return the timestamp associated with the last break count increment.
 */
time_t store_read_break_count_time() {
  time_t t_last;
  persist_read_data(PERSIST_KEY_BREAK_COUNT_TIME, &t_last, sizeof(time_t));
  return t_last;
}

/**
 * Return the value stored with the key PERSIST_KEY_BREAK_COUNT.
 */
int store_read_break_count() {
  return persist_read_int(PERSIST_KEY_BREAK_COUNT);
}

/**
 * Return the next random message from the message pool.
 */
const char* store_read_random_message() {
  if (!persist_exists(PERSIST_KEY_RANDOM_MSG_INDEX)) {
    persist_write_int(PERSIST_KEY_RANDOM_MSG_INDEX, 1);
    return enamel_get_message_random_0();
  } else {
    int index = persist_read_int(PERSIST_KEY_RANDOM_MSG_INDEX);
    index = index >= RANDOM_MSG_POOL_SIZE - 1? 0 : index + 1;
    persist_write_int(PERSIST_KEY_RANDOM_MSG_INDEX, index);

    switch (index) {
      case 1: return enamel_get_message_random_1(); break;
      case 2: return enamel_get_message_random_2(); break;
      case 3: return enamel_get_message_random_3(); break;
      case 4: return enamel_get_message_random_4(); break;
      case 5: return enamel_get_message_random_5(); break;
      case 6: return enamel_get_message_random_6(); break;
      case 7: return enamel_get_message_random_7(); break;
      case 8: return enamel_get_message_random_8(); break;
      case 9: return enamel_get_message_random_9(); break;
      default: return enamel_get_message_random_0();
    }
  }
}

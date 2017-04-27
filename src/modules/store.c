#include "store.h"

static int s_possible_score = -1;

static uint8_t s_launchexit_count;
static uint32_t s_launchexit_data[3];

/* 
 * The format of launch-exit record stored persistently (compact to save space).
 * Total size: 4 bytes
 * 19 bits                  | 8 bits      | 3 bits   | 2 bits  
 * exit (secs after launch) | break count | l reason | e reason
 */
#define LAUNCHEXIT_DATA_SIZE    12 
#define LAUNCHEXIT_COUNT_SIZE   1
#define COMPACT(es, br, lr, er) ((es&0x0007FFFF)<<13 | (br&0xFF)<<5 | (lr&0x7)<<2 | (er&0x3))
#define GET_SECOND(x)           ((x>>13) & 0x0007FFFF)
#define GET_SCORE(x)            ((x>>5)  & 0x000000FF)
#define GET_LAUNCH_REASON(x)    ((x>>2)  & 0x00000007)
#define GET_EXIT_REASON(x)      (x       & 0x00000003)

/**
 * Write into the persistent storage the latest timestamp at which we update configuration.
 */
void store_write_config_time(time_t time) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Store configuration time = %u", (unsigned) time);
  persist_write_data(PERSIST_KEY_CONFIG_TIME, &time, sizeof(time_t));
}

/**
 * Return true if we need to send new configuration request to the server, otherwise false.
 * Note: Since currently we will reset the daily step status (store_read_curr_score()), we 
 *   must avoid sending config_request if the user has accomplished any daily break goal.
 *   Make sure there is a non-Period Wakup scheduled daily before any Period Wakeup.
 */
bool store_resend_config_request(time_t t_curr) {
  //return true; // TODO: forced to get the latest config for the new version.

  if (!persist_exists(PERSIST_KEY_CONFIG_TIME)) {
    return true;
  }

  time_t t_last_config_time;

  persist_read_data(PERSIST_KEY_CONFIG_TIME, &t_last_config_time, sizeof(time_t));
  #if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "t_config_time=%u", (unsigned) t_last_config_time);
  #endif

  // TODO
  if (store_read_curr_score() == 0 && 
      t_curr-t_last_config_time > atoi(enamel_get_config_update_interval()) * SECONDS_PER_DAY) {
    return true;
  } else {
    return false;
  }
}

/**
 * Write the latest timestamp at which we upload steps data.
 */
void store_write_upload_time(time_t time) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Store update time = %u", (unsigned) time);
  persist_write_data(PERSIST_KEY_UPLOAD_TIME, &time, sizeof(time_t));
}

/**
 * Return the timestamp of the last time we upload steps data to the server.
 * Return 0 if there is no upload-time stored (i.e. never uploaded steps data yet).
 */
time_t store_read_upload_time() {
  time_t t_last_upload; 

  if (!persist_exists(PERSIST_KEY_UPLOAD_TIME)) {
    return 0;
  } else {
    persist_read_data(PERSIST_KEY_UPLOAD_TIME, &t_last_upload, sizeof(time_t));
    return t_last_upload;
  }
}

/**
 * Write launch and exit events into the persistent storage 
 */
void store_write_launchexit_event(time_t t_launch, time_t t_exit, uint8_t lr, uint8_t er) {
  time_t t_diff = t_exit - t_launch; 

  int br = store_read_curr_score();

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
  s_launchexit_data[1] = COMPACT(t_diff, br, lr, er);
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
  APP_LOG(APP_LOG_LEVEL_INFO, "msg_id=%s, msg_id_ascii=%08x, t_diff=%u, br=%d lr=%d, er=%d", 
    msg_id, (unsigned int)msg_id_ascii, (unsigned int)t_diff, br, lr, er);
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
  APP_LOG(APP_LOG_LEVEL_ERROR, "in store_resend_launchexit_event");
  // FIXME: consider using a single key and sequential storage location
  uint32_t key, msg_id_ascii;
  time_t t_launch, t_exit;
  uint8_t br, lr, er;
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
      br = GET_SCORE(t_exit);
      lr = GET_LAUNCH_REASON(t_exit);
      er = GET_EXIT_REASON(t_exit);
      t_exit = t_launch + GET_SECOND(t_exit);

      // Convert message ID from ascii code back to char (assuming each ID is 4 bytes)
      msg_id_ascii = s_launchexit_data[2];
      // DEBUG
      APP_LOG(APP_LOG_LEVEL_INFO, "Resend launch and exit events to the server" \
          ". new records count=%d.", s_launchexit_count);
      APP_LOG(APP_LOG_LEVEL_INFO, "t_launch=%u, t_exit=%u, msg_id_ascii=%04x, br=%d, lr=%d, er=%d"
        , (unsigned int)t_launch, (unsigned int)t_exit, (unsigned int)msg_id_ascii,
        (int)br, (int)lr, (int)er); // DEBUG
      for (int i = 3; i >= 0; i--) {
        msg_id[i] = (char)msg_id_ascii;
        msg_id_ascii >>= 8;
      }
      msg_id[4] = '\0';

      launch_resend(t_launch, t_exit, msg_id, br, lr, er);

      persist_delete(key);
      s_launchexit_count--;

      persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 
        LAUNCHEXIT_COUNT_SIZE);
      return false;
    } else {
      s_launchexit_count--;
      persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 
        LAUNCHEXIT_COUNT_SIZE);
      return true;
    }
  } else {
    return true;
  }
}

/**
 * Return whether we finish resending steps data (only when we do not send any data 
 * to the server. Later in steps_send_latest()  we might resend some data that we are 
 * sending in here.
 */
bool store_resend_steps() {
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: in store_resend_steps");
  time_t t_last_upload; 
  time_t interval_seconds = enamel_get_break_freq() * SECONDS_PER_MINUTE;
  //t_last_upload = (t_last_upload < time_start_of_today());

  if (!persist_exists(PERSIST_KEY_UPLOAD_TIME)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: ABSZ");
    t_last_upload = time_start_of_today() - 2 * SECONDS_PER_HOUR; // TODO: only send 2 hours history.
  } else {
    persist_read_data(PERSIST_KEY_UPLOAD_TIME, &t_last_upload, sizeof(time_t));
    if (t_last_upload >= e_launch_time - interval_seconds) {
      // Data in the lastest 60 minutes will be sent by the normal data upload routine.
      return true;
    }
  }

  // DEBUG
  char buf[32];
  strftime(buf, sizeof(buf), "%d %H:%M", localtime(&t_last_upload));
  APP_LOG(APP_LOG_LEVEL_ERROR, "t_last_upload=%u, %s",  (unsigned)t_last_upload, buf);
  //DEBUG
  /*
  if (t_last_upload  < time_start_of_today() - 2 * SECONDS_PER_DAY) {
    // If last upload time is ealier than 2 days ago, only upload starting at 2 days ago
    t_last_upload = time_start_of_today() - 2 * SECONDS_PER_DAY;
  } else if (t_last_upload >= e_launch_time - interval_seconds) {
    // Data in the lastest 60 minutes will be sent by the normal data upload routine.
    return true;
  }
  */

  steps_send_in_between(t_last_upload, t_last_upload + interval_seconds, true);

  t_last_upload += interval_seconds;
  persist_write_data(PERSIST_KEY_UPLOAD_TIME, &t_last_upload, sizeof(time_t));

  //return t_last_upload < t_curr - interval_seconds;
  return false;
}

/**
 * Reset the break count to 0. This should be performed in the first wakeup daily.
 */
void store_reset_curr_score() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "RESET break count");
  time_t t_last = time(NULL);
  persist_write_int(PERSIST_KEY_CURR_SCORE, 0); 
  persist_write_data(PERSIST_KEY_CURR_SCORE_TIME, &t_last, sizeof(time_t));
}

/**
 * Increment the break count by 1. If it has not existed, set it to 1.
 * Also record the current time associated with this break count increment.
 */
void store_increment_curr_score() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "enter store_increment_curr_score()");
  if (!persist_exists(PERSIST_KEY_CURR_SCORE)) {
    persist_write_int(PERSIST_KEY_CURR_SCORE, 1); 
  } else {
    persist_write_int(PERSIST_KEY_CURR_SCORE, persist_read_int(PERSIST_KEY_CURR_SCORE)+1); 
  }
  time_t t_curr = time(NULL);
  persist_write_data(PERSIST_KEY_CURR_SCORE_TIME, &t_curr, sizeof(time_t));
}

/**
 * Return the timestamp associated with the last break count increment.
 */
time_t store_read_curr_score_time() {
  time_t t_last;
  persist_read_data(PERSIST_KEY_CURR_SCORE_TIME, &t_last, sizeof(time_t));
  return t_last;
}

/**
 * Return the value stored with the key PERSIST_KEY_CURR_SCORE.
 */
int store_read_curr_score() {
  return persist_read_int(PERSIST_KEY_CURR_SCORE);
}

void store_set_possible_score() {
  // Calculate the current position of time in a day in seconds.
  time_t break_freq_seconds = enamel_get_break_freq() * SECONDS_PER_MINUTE;
  int diff = e_launch_time - time_start_of_today() - enamel_get_daily_start_time();

  if (diff <= 0) {
    s_possible_score = 0;
  } else {
    // +/- 1 difference.
    //s_possible_score = (diff + break_freq_seconds - 1) / break_freq_seconds;
    s_possible_score = diff / break_freq_seconds;

    // Capped at the maximum score.
    if (s_possible_score > enamel_get_total_break()) {
      s_possible_score = enamel_get_total_break();
    }
  }
}

int store_read_possible_score() {
  return s_possible_score;
}

/**
 * Compare the current score to a reference score.
 * The reference score used depends on the mode:
 *   1 - personal average
 *   2 - personal best
 *   3 - all average
 * Return an integer value = current_score - reference.
 */
int store_compare_ref_score(int mode) {
  if (mode == 2) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "curr_score=%d, ref_score=%d", (int)store_read_curr_score(), (int)enamel_get_score_p_best());
    return store_read_curr_score() - enamel_get_score_p_best();
  } else if (mode == 1 || mode == 3) {
    int i, start, end, count, ref_count, ref_score = 0;
    const char *buf;

    // Parse the CSV format string to get the reference score.
    if (mode == 1) {
      buf = enamel_get_score_p_average();
      ref_count = enamel_get_score_p_count();
    } else { // mode == 3
      buf = enamel_get_score_a_average();
      ref_count = enamel_get_score_a_count();
    }

    // Calculate the correct index for the reference scores.
    //printf("rf=%d, p=%d, tb=%d\n", ref_count, s_possible_score, (int)enamel_get_total_break());
    ref_count = (int)round((float)ref_count * s_possible_score / enamel_get_total_break());
    for (start = 0, end = 0, count = 0; buf[end] != '\0'; end++) {
      if (buf[end] == ',') {
        count++;

        if (count >= ref_count) {
          break;
        }

        start = end + 1;
      }
    }

    // Eliminate trailing '\0' or ','
    if (buf[end] == '\0') {
      end--;
    }
    while (buf[end] == ',') {
      end--;
    }
    //printf("start=%d, end=%d, count=%d\n", start, end, count);

    // Extract the integer portion of the number.
    float integer = 0, decimal = 0, multiplier = 10;
    for (i = start; i <= end; i++) {
      #if DEBUG
        APP_LOG(APP_LOG_LEVEL_INFO, "buf[%d] = %c", i, buf[i]);
      #endif
      if (buf[i] == '.') {
        break;
      }
      integer = integer * 10 + buf[i] - '0';
    }
    count = 10;

    // Extract the decimal portion of the number.
    for (++i ; i <= end; i++) {
      #if DEBUG
        APP_LOG(APP_LOG_LEVEL_INFO, "buf[%d] = %c", i, buf[i]);
      #endif
      decimal += (buf[i] - '0') / multiplier;
      multiplier *= 10;
    }
    //APP_LOG(APP_LOG_LEVEL_ERROR, "res = %d.%d\n", (int)integer, (int)(decimal*100));

    // Formula: reference_score = reference_percentage * possible_score.
    // 0 score (before the 1st wakeup) gives ref_score = 0;
    ref_score = (int)round((integer + decimal) * s_possible_score / 100);

    #if DEBUG
      APP_LOG(APP_LOG_LEVEL_ERROR, "buf = %s", buf);
      APP_LOG(APP_LOG_LEVEL_ERROR, "s_possible_score=%d, curr_score=%d, ref_score=%d",
        s_possible_score, store_read_curr_score(), ref_score);
    #endif
    return store_read_curr_score() - ref_score;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown mode value %d", mode);
  }
  
  return 0;
}

/**
 * Return the next random message from the message pool.
 */
const char* store_read_random_message() {
  if (!persist_exists(PERSIST_KEY_RANDOM_MSG_INDEX)) {
    persist_write_int(PERSIST_KEY_RANDOM_MSG_INDEX, 1);
    return enamel_get_random_message_0();
  } else {
    int index = persist_read_int(PERSIST_KEY_RANDOM_MSG_INDEX);
    index = index >= RANDOM_MSG_POOL_SIZE - 1? 0 : index + 1;
    persist_write_int(PERSIST_KEY_RANDOM_MSG_INDEX, index);

    APP_LOG(APP_LOG_LEVEL_ERROR, "index=%d", index);
    switch (index) {
      case 1:  return enamel_get_random_message_1();    break;
      case 2:  return enamel_get_random_message_2();    break;
      case 3:  return enamel_get_random_message_3();    break;
      case 4:  return enamel_get_random_message_4();    break;
      case 5:  return enamel_get_random_message_5();    break;
      case 6:  return enamel_get_random_message_6();    break;
      case 7:  return enamel_get_random_message_7();    break;
      case 8:  return enamel_get_random_message_8();    break;
      case 9:  return enamel_get_random_message_9();    break;
      case 10: return enamel_get_random_message_10();   break;
      case 11: return enamel_get_random_message_11();   break;
      default: return enamel_get_random_message_0();
    }
  }
}

/**
 * Delete all persistent storage value. This should be called when user inactivate the app.
 */
void store_delete_all() {
  persist_delete(PERSIST_KEY_LAUNCHEXIT_COUNT);
  persist_delete(PERSIST_KEY_CONFIG_TIME);
  persist_delete(PERSIST_KEY_UPLOAD_TIME);
  persist_delete(PERSIST_KEY_CURR_SCORE);
  persist_delete(PERSIST_KEY_CURR_SCORE_TIME);
  persist_delete(PERSIST_KEY_RANDOM_MSG_INDEX);
}

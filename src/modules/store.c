#include "store.h"


static int s_possible_score = -1;

static uint8_t s_launchexit_count;
static uint32_t s_launchexit_data[3];

/* 
 * The format of launch-exit record stored persistently (compact to save space).
 * Total size: 4 bytes
 * 8 bits     | 11 bits                  | 8 bits      | 3 bits   | 2 bits
 * score diff | exit (secs after launch) | break count | l reason | e reason
 */
#define LAUNCHEXIT_DATA_SIZE    12 
#define LAUNCHEXIT_COUNT_SIZE   1
#define COMPACT(sd, es, br, lr, er) \
        ((sd&0xF)<<24 | (es&0x7FF)<<13 | (br&0xFF)<<5 | (lr&0x7)<<2 | (er&0x3))
#define GET_SCORE_DIFF(x)       ((x>>24) & 0x0000000F)
#define GET_SECOND(x)           ((x>>13) & 0x000007FF)
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
  #if DEBUG
    //return true; // TODO: forced to get the latest config for the new version.
  #endif

  if (!persist_exists(PERSIST_KEY_CONFIG_TIME)) {
    return true;
  }

  time_t t_last_config_time;

  persist_read_data(PERSIST_KEY_CONFIG_TIME, &t_last_config_time, sizeof(time_t));
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_ERROR, "t_config_time=%u", (unsigned) t_last_config_time);
  #endif

  // Force to request new config from the server every day.
  time_t t_user_start = time_start_of_today() + enamel_get_daily_start_time();
  //if (store_read_curr_score() == 0 && t_last_config_time < t_user_start) {
  if (t_last_config_time < t_user_start) {
      //t_curr-t_last_config_time > atoi(enamel_get_config_update_interval()) * SECONDS_PER_DAY) {
    return true;
  } else {
    return false;
  }
}

/**
 * Deprecated.
 * Write the latest timestamp at which we upload steps data.
 */
void store_write_upload_time(time_t time) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Store new upload time = %u", (unsigned) time);

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
  // Get the difference time between launch_time and exit_time. Note that this number
  // will be clipped due to the limited storage bits allocated for it.
  time_t t_diff = t_exit - t_launch; 

  // Get the current break score.
  int br = store_read_curr_score();

  // Get the score difference.
  uint8_t sd = launch_get_score_diff();

  // Get the random message ID and convert it to ASCII (assuming each ID is 4 bytes)
  const char *msg_id = launch_get_random_message_id();
  uint32_t msg_id_ascii = 0;
  for (int i = 0; i < 4; i++) {
    msg_id_ascii <<= 8;
    msg_id_ascii |= (int)msg_id[i] & 0xFF;
  }
  
  // Read the current number of launch-exit events stored.
  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 1);
  } else {
    s_launchexit_count = 0;
  }

  // TODO: should implement as circular buffer instead of linear array with head and tail pointers.
  if (s_launchexit_count > PERSIST_KEY_LAUNCH_END - PERSIST_KEY_LAUNCH_START + 1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "s_launchexit_count less than zero = %u", s_launchexit_count);
    s_launchexit_count = 0;
  }
  s_launchexit_data[0] = t_launch;
  s_launchexit_data[1] = COMPACT(sd, t_diff, br, lr, er);
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
  APP_LOG(APP_LOG_LEVEL_INFO, "msg_id=%s, msgid_ascii=%08x, t_diff=%u, sd=%d, br=%d lr=%d, er=%d",
    msg_id, (unsigned int)msg_id_ascii, (unsigned int)t_diff, sd, br, lr, er);
}

/**
 * Return whether we finish resending launch/exit event (only when we do not send any data 
 * to the server. 
 */
bool store_resend_launchexit_event() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "in store_resend_launchexit_event");
  // FIXME: consider using a single key and sequential storage location
  uint32_t key, msg_id_ascii;
  time_t t_launch, t_exit;
  uint8_t sd, br, lr, er;
  char msg_id[5];

  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);
    // Decrement count if this function is called previously and data is sent to server.
    if (e_waiting_launchexit_ack) {
      key = PERSIST_KEY_LAUNCH_START + s_launchexit_count - 1;
      persist_delete(key);

      s_launchexit_count--;
      key--;

      persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 
        LAUNCHEXIT_COUNT_SIZE);
      
      e_waiting_launchexit_ack = false;
    } else {
      key = PERSIST_KEY_LAUNCH_START + s_launchexit_count - 1;
    }

    if (s_launchexit_count <= 0) {
      return true;
    }

    // Read launch-exit record and try to resend to the server
    if (persist_exists(key) && 
        key >= PERSIST_KEY_LAUNCH_START && 
        key <= PERSIST_KEY_LAUNCH_END) {
      persist_read_data(key, &s_launchexit_data, LAUNCHEXIT_DATA_SIZE);

      t_launch = s_launchexit_data[0];

      // Decompose stored information.
      t_exit = s_launchexit_data[1];
      sd = GET_SCORE_DIFF(t_exit);
      br = GET_SCORE(t_exit);
      lr = GET_LAUNCH_REASON(t_exit);
      er = GET_EXIT_REASON(t_exit);
      t_exit = t_launch + GET_SECOND(t_exit);

      // Convert message ID from ascii code back to char (assuming each ID is 4 bytes)
      msg_id_ascii = s_launchexit_data[2];
      #if DEBUG
        APP_LOG(APP_LOG_LEVEL_INFO, "Resend launch and exit events to the server" \
          ". new records count=%d.", s_launchexit_count);
        APP_LOG(APP_LOG_LEVEL_INFO, "t_launch=%u, t_exit=%u, msg_id_ascii=%04x, " \
          "sd=%d, br=%d, lr=%d, er=%d",
          (unsigned int)t_launch, (unsigned int)t_exit, (unsigned int)msg_id_ascii, (int)sd,
          (int)br, (int)lr, (int)er); // DEBUG
      #endif
      for (int i = 3; i >= 0; i--) {
        msg_id[i] = (char)msg_id_ascii;
        msg_id_ascii >>= 8;
      }
      msg_id[4] = '\0';

      launch_resend(t_launch, t_exit, msg_id, sd, br, lr, er);

      e_waiting_launchexit_ack = true;
      //persist_delete(key);
      //s_launchexit_count--;

      //persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 
      //  LAUNCHEXIT_COUNT_SIZE);
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
  APP_LOG(APP_LOG_LEVEL_INFO, "in store_set_possible_score");
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
  if (s_possible_score < 0) {
    // initalize the score if not yet done
    store_set_possible_score();
  }
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
    ref_count = (int)round((double)ref_count * s_possible_score / enamel_get_total_break());
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
    double integer = 0, decimal = 0, multiplier = 10;
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
      //#if DEBUG
      //  APP_LOG(APP_LOG_LEVEL_INFO, "buf[%d] = %c", i, buf[i]);
      //#endif
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
  int index = 0;
  if (strcmp(enamel_get_group(), "real_time_adaptive") == 0) {
    // real_time_adaptive -- get message with highest weight
    int weight = -1;
    int weights[RANDOM_MSG_POOL_SIZE];
    persist_read_data(PERSIST_KEY_RANDOM_MSG_WEIGHTS, weights, sizeof(int)*RANDOM_MSG_POOL_SIZE);

    for (int i=0; i < RANDOM_MSG_POOL_SIZE; i++) {
      if (weights[i] > weight) {
        weight = weights[i];
        index = i;
      }
    }
    persist_write_string(PERSIST_KEY_WEIGHTS_DATA, "");
    // if PERSIST_KEY_WEIGHTS_DATA exists
    // then we are in adaptive AND used a message in the latest alert
  } else {
    // real_time_random|simple -- iterate through random messages
    if (!persist_exists(PERSIST_KEY_RANDOM_MSG_INDEX)) {
      persist_write_int(PERSIST_KEY_RANDOM_MSG_INDEX, 1);
      return enamel_get_random_message_0();
    } else {
      int index = persist_read_int(PERSIST_KEY_RANDOM_MSG_INDEX);
      index = index >= RANDOM_MSG_POOL_SIZE - 1? 0 : index + 1;
      APP_LOG(APP_LOG_LEVEL_ERROR, "index=%d", index);
    }
    persist_delete(PERSIST_KEY_WEIGHTS_DATA); 
  }
  persist_write_int(PERSIST_KEY_RANDOM_MSG_INDEX, index);
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


/**
  * Updates the weight of the message that was previously displayed.
  * Called during LAUNCH_WAKEUP_PERIOD.
  * 
  * Follows alg: new_weight = old_weight + alpha * (outcome - old_weight)
  * outcome :=  factor if pass, else 0
  * alpha := learning rate
  */
void store_weight_update(bool pass) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "enter store_weight_update()");
  // check we are in real_time_adaptive and a message was just used
  if (persist_exists(PERSIST_KEY_WEIGHTS_DATA) && strcmp(enamel_get_group(), "real_time_adaptive") == 0) {
    int index = persist_read_int(PERSIST_KEY_RANDOM_MSG_INDEX);
    int factor = atoi(enamel_get_weight_factor_param());
    int alpha = atoi(enamel_get_weight_update_param());
    int outcome = pass ? factor : 0;

    int weights[RANDOM_MSG_POOL_SIZE];
    persist_read_data(PERSIST_KEY_RANDOM_MSG_WEIGHTS, weights, sizeof(int)*RANDOM_MSG_POOL_SIZE);
    int weight = weights[index];

    APP_LOG(APP_LOG_LEVEL_ERROR, "OLD: index=%d,weight=%d, pass=%d", index, weight, (int) pass);

    //update algorithm. Float division done last to minimize float rounding errors. 
    weight = weight + (int) floor(alpha * (outcome - weight) / ((double) factor));
    weights[index] = weight;
    persist_write_data(PERSIST_KEY_RANDOM_MSG_WEIGHTS, weights, sizeof(int)*RANDOM_MSG_POOL_SIZE);
    APP_LOG(APP_LOG_LEVEL_ERROR, "NEW: weight=%d", weight);
  }
}

/**
  * Perists the random_message_id into persistent storage IF we just used
  * a message and are in real_time_adaptive 
  * So PERSIST_KEY_WEIGHTS_DATA is "" (see store_read_random_message)
  * 
  * It is resaved so reparsing the message is unnecessary and 
  *  because s_msg_id is overwritten to pass/fail during the period wakeup.
  */
void store_weight_write_msgid(){
  if (persist_exists(PERSIST_KEY_WEIGHTS_DATA)) {
    const char * msg_id = launch_get_random_message_id();
    persist_write_string(PERSIST_KEY_WEIGHTS_DATA, msg_id);
  }
}

/**
  * Returns true if we have a weight update to send. False otherwise.
  * If true, fills the msg_id_buf with the corresponding msg_id to send an update on
  * 
  * Called when sending a launch event. 
  */
bool store_weight_read_then_delete_msgid(char *msg_id_buf) {
  // PERSIST_KEY_WEIGHTS_DATA does not exist if we are not in real_time_adaptive 
  //  or if we did not use a message during the latest alert period.
  if (!persist_exists(PERSIST_KEY_WEIGHTS_DATA)) {
    return false;
  }

  if (msg_id_buf != NULL) {
    persist_read_string(PERSIST_KEY_WEIGHTS_DATA, msg_id_buf, 6);
  }

  // so the same weight update is not sent multiple times during launch events
  persist_delete(PERSIST_KEY_WEIGHTS_DATA); 
  return true;
}

/**
  * Returns the int weight value of the most recent weight update done in store_weight_update.
  * 
  * Called when sending a launch event. 
  */
int store_weight_get_recent_update() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "enter store_weight_get_recent_update()");
  if (strcmp(enamel_get_group(), "real_time_adaptive") == 0) {
    int index = persist_read_int(PERSIST_KEY_RANDOM_MSG_INDEX);

    int weights[RANDOM_MSG_POOL_SIZE];
    persist_read_data(PERSIST_KEY_RANDOM_MSG_WEIGHTS, weights, sizeof(int)*RANDOM_MSG_POOL_SIZE);
    APP_LOG(APP_LOG_LEVEL_ERROR, "index=%d,weight=%d", index, weights[index]);
    return weights[index];
  }
  return -1;
}


/**
  * Parse and store weights from settings update in persistent storage
  * weights are sent as integers multipled by a given factor, seperated by '|'
  */
void store_weights_set() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "enter store_weights_set()");
  if (strcmp(enamel_get_group(), "real_time_adaptive") == 0) {
  
    const char* weights_string_const = enamel_get_random_message_weights();
    const char* weight = weights_string_const;

    int weights_values[RANDOM_MSG_POOL_SIZE]; 
    int index = 0;
    int i=0;
    
    while (weights_string_const[i] != '\0') {
      weights_values[index] = atoi(weight);
      index++;
      while (isdigit((int) weights_string_const[i])) i++;
      if (weights_string_const[i] == '|') {
        i++;
        weight = weights_string_const + i;
      }
    }
    persist_write_data(PERSIST_KEY_RANDOM_MSG_WEIGHTS, weights_values, sizeof(int)*RANDOM_MSG_POOL_SIZE);
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
  persist_delete(PERSIST_KEY_RANDOM_MSG_WEIGHTS);
  persist_delete(PERSIST_KEY_WEIGHTS_DATA);
}

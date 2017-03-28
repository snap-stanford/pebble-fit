#include "launch.h"

// Global variables for the current launch.
time_t e_launch_time;
int e_launch_reason;
int e_exit_reason;

static Window *s_dialog_window = NULL; 
static Window *s_wakeup_window = NULL;

static int s_config_request;
static char *s_random_message;
char s_random_message_buf[RANDOM_MSG_SIZE_MAX];

// For resend functions.
static time_t s_t_launch, s_t_exit, s_curr_time;
static uint8_t s_br, s_lr, s_er;
static const char *s_msg_id;

/* Add launch reason and date to out dict. */
// FIXME: could combine both launch and exit packets, since they have very similar format.
static void prv_launch_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyDate, &e_launch_time, sizeof(int), true);
  dict_write_int(out, AppKeyLaunchReason, &e_launch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyConfigRequest, &s_config_request, sizeof(int), true);
  dict_write_uint8(out, AppKeyBreakCount, s_br);
  dict_write_cstring(out, AppKeyMessageID, s_msg_id);
}
/* Add exit reason and date to out dict. */
static void prv_exit_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyExitReason, &e_exit_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_t_exit, sizeof(int), true);
}

/* Add both launch and exit reasons and dates to out dict. */
static void prv_launch_exit_data_write(DictionaryIterator * out) {
  s_curr_time = time(NULL);
  dict_write_int(out, AppKeyDate, &s_curr_time, sizeof(int), true);
  dict_write_int(out, AppKeyLaunchTime, &s_t_launch, sizeof(int), true);
  dict_write_int(out, AppKeyExitTime, &s_t_exit, sizeof(int), true);
  dict_write_uint8(out, AppKeyBreakCount, s_br);
  dict_write_uint8(out, AppKeyLaunchReason, s_lr);
  dict_write_uint8(out, AppKeyExitReason, s_er);
  dict_write_cstring(out, AppKeyMessageID, s_msg_id);
}

/**
 * Upload the current launch event. Might associate a request for the new configuration.
 */
void launch_send_launch_notification() {
  s_config_request = store_resend_config_request(e_launch_time)? 1 : 0;
	s_br = store_read_curr_score();

  comm_send_data(prv_launch_data_write, comm_sent_handler, comm_server_received_handler);
}

/* Upload the current exit event to phone. */
void launch_send_exit_notification(time_t time) {
  s_t_exit = time;
  comm_send_data(prv_exit_data_write, comm_sent_handler, NULL);
}

/**
 * Resend the previous launch and exit events.
 */
void launch_resend(time_t t_launch, time_t t_exit, char *msg_id, uint8_t br, uint8_t lr, uint8_t er) {
  // The current launch record has been uploaded and the current exit reason has 
  // not yet been collected, so it is safe to modify these two varaibles here.
  s_t_launch = t_launch;
  s_t_exit = t_exit;
  s_msg_id = msg_id;
  s_br = br;
  s_lr = lr;
  s_er = er;

  comm_send_data(prv_launch_exit_data_write, comm_sent_handler, comm_server_received_handler);

  //if (is_launch) {
  //  comm_send_data(prv_launch_data_write, comm_sent_handler, comm_server_received_handler);
  //} else {
  //  comm_send_data(prv_exit_data_write, comm_sent_handler, NULL);
  //}
}

/**
 * If this is a notify wakeup, read a random message from the persistent storage into memory.
 * Otherwise, set message ID to be either "pass" or "fail".
 */
void launch_set_random_message() {
  //char random_message[RANDOM_MSG_SIZE_MAX]; // FIXME: change to global static variable.

  const char *msg_ptr = store_read_random_message();

  //snprintf(random_message, sizeof(random_message), store_read_random_message()); // FIXME: root cause?
  //APP_LOG(APP_LOG_LEVEL_ERROR, "%s!", random_message);
  //printf(random_message);
  //printf(store_read_random_message());
  
  if (msg_ptr[0] != 'o') { // Deal with action and health messages.
    char *c;  // TODO: might could just using msg_ptr.

    snprintf(s_random_message_buf, sizeof(s_random_message_buf), msg_ptr);
    for (s_msg_id = c = s_random_message_buf; *c != ':' && *c != '\0'; c++) {}
    c[4] = '\0';
    snprintf(s_random_message_buf, sizeof(s_random_message_buf), msg_ptr+5);

    s_random_message = c;
  } else { // Deal with the outcome messages specially.
    int i, start, end, message_index, score_diff, size = 0;
    //const char *c;
    char mode = msg_ptr[1];

    for (s_msg_id = msg_ptr, i = 0; msg_ptr[i] != ':'; i++, size++) {}
    if (size != 4) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "store_read_random_message provides wrong size %d.", size);
      return;
    }
  
    // Parse the random message ID, which contains 4 bytes/characters:
    // Action messages: amxx, where xx represents digits 0-9.
    //
    // Health messages: hmxx, where xx represents digits 0-9.
    //
    // Outcome messages: oyxx, where xx represents digits 0-9, and y represents:
    //                   u - compare to user self's average score, without number.
    //                   v - compare to user self's average score, with a number.
    //                   w - compare to user self's best score, with a number.
    //                   a - compare to the average score of all users, without number.
    //                   b - compare to the average score of all users, with a number.
  
    // Compare with the reference score.
    switch (mode) {
      case 'u':
      case 'v':
        score_diff = store_compare_ref_score(1);
        break;
      case 'w':
        score_diff = store_compare_ref_score(2);
        break;
      case 'a':
      case 'b':
        score_diff = store_compare_ref_score(3);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown outcome message type %d.", s_msg_id[1]);
        return;
    }
  
    // Fetch the proper message.
    if (score_diff > 0) {
      for (i = 0; msg_ptr[i] != '|'; i++) {}
      start = 0;
      end = i;
    } else if (score_diff < 0) {
      for (i = 0; msg_ptr[i] != '|'; i++) {}
      for (start = ++i; msg_ptr[i] != '|'; i++) {}
      end = i;
    } else { // score_diff == 0
      for (i = 0; msg_ptr[i] != '|'; i++) {}
      for (++i; msg_ptr[i] != '|'; i++) {}
      for (start = ++i; msg_ptr[i] != '\0'; i++) {}
      end = i;
    }
    APP_LOG(APP_LOG_LEVEL_ERROR, "msgid=%s, score_diff=%d", s_msg_id, score_diff);
    APP_LOG(APP_LOG_LEVEL_ERROR, "start=%d, end=%d", start, end);
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s!", msg_ptr);
  
    if (mode == 'u' || mode == 'a') {
      //snprintf(random_message, end - start+1, msg_ptr+start);
      snprintf(s_random_message_buf, end - start+1, msg_ptr+start);
    } else {
      // Substitute the number in the message.
      //APP_LOG(APP_LOG_LEVEL_ERROR, "Substitute the number in the message.");
      if (score_diff < 0) {
        score_diff = -1 * score_diff;
      }
      //snprintf(random_message, end - start, msg_ptr+start, score_diff);
      snprintf(s_random_message_buf, end - start, msg_ptr+start, score_diff);

    }
    s_random_message = s_random_message_buf;
  }
} 

/**
 * Return the content of the random message previouly read from the persistent memory. 
 */
const char * launch_get_random_message() {
  return s_random_message;
}

/**
 * Return the ID of the random message previouly read from the persistent memory. 
 */
const char * launch_get_random_message_id() {
  if (s_msg_id) {
    return s_msg_id;
  } else {
    return "nana";
  }
}

/* This function is called at scheduled wakeup event.
 * If the activity goal is met or the current activity is Sleep/RestfulSleep, it does nothing.
 * Otherwise, it will alert users by vibration and popping up alert window on the watch.
 */
static void prv_wakeup_vibrate(bool force) {
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: in prv_wakeup_vibrate()");
  HealthActivityMask activity = health_service_peek_current_activities();

  if (activity != HealthActivitySleep && activity != HealthActivityRestfulSleep &&
      (force || !steps_get_pass())) {
    switch (enamel_get_vibrate()) {
      case 1: vibes_short_pulse();      break;
      case 2: vibes_long_pulse();       break;
      case 3: vibes_double_pulse();     break;
      case 4: {
        // Customized vibration pattern.
        static const uint32_t const five_pulse[] = 
          {200, 100, 200, 100, 200, 100, 200, 100, 200}; // Five pulses
        VibePattern pat = {
          .durations = five_pulse,
          .num_segments = ARRAY_LENGTH(five_pulse),
        };
        vibes_enqueue_custom_pattern(pat);
        break;
      }
      default: break;
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Step goal is met. Silent wakeup.");
  }
}

/**
 * Handle wakeup events.
 */
void wakeup_handler(WakeupId wakeup_id, int32_t wakeup_cookie) {
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: wakeup=%d cookie=%d", (int)wakeup_id, (int)wakeup_cookie);
  
  steps_update();
  
  // wakeup_cookie is the index associated to the wakeup event. It is also the wakeup type.
  if (wakeup_cookie >= LAUNCH_WAKEUP_PERIOD) {
    e_launch_reason = wakeup_cookie;
    prv_wakeup_vibrate(false); // TODO: consider moving into swtich statement.

    // This could happen if we receive wakeup event while the app has been on the foreground.
    if (s_wakeup_window) {
      window_stack_remove(s_wakeup_window, false);
    }

    switch (wakeup_cookie) {
      case LAUNCH_WAKEUP_ALERT:
        // Get a random message from the persistent storage. This must happen before
        // wakeup_window_push() and the first call to init_callback. 
        APP_LOG(APP_LOG_LEVEL_ERROR, "Make sure this happens before init_callback()!");
        if (!steps_get_pass()) { // Only push the window if step goal is not met.
          launch_set_random_message();
          s_wakeup_window = wakeup_window_push();
        } else {
          e_exit_reason = EXIT_TIMEOUT; // TODO: or using a new coding for silent-wakeup?
        }
        break;
      case LAUNCH_WAKEUP_PERIOD:
      case LAUNCH_WAKEUP_DAILY:
        // TODO: For now, even for period-wakeup and goal is met, we still push window.
        s_wakeup_window = wakeup_window_push();
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "\nShould NOT reach here!\n");
    }
  } else {
    e_launch_reason = -1 * wakeup_cookie; // To distinguish from other normal launch types.
    APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
  }
  
  // Start timer
  tick_second_subscribe(true);

  // Always re-schedule wakeup events
  wakeup_schedule_events();

  //TODO: should call init_callback() to upload data, but how should we prevent interupting
  // the current data-upload process.
  //if (e_server_ready) {
  //  init_callback();
  //}
}

/** 
 * Handle launch events. 
 * Push a window depends on whether this App is activated or not.
 * Return the newly created window.
 */
void launch_handler(bool activate) {
  if (activate) {
    bool will_timeout = false;
    int lr = launch_reason();

    // Reset the score to 0 at the first launch of the day.
    //   Previosuly: reset break count to 0 if it is the first launch in the day (since we will 
    //   re-calculate the steps upon the first wakeup event, it is safe to reset multiple times)
    //   time_t t_start = time_start_of_today() + enamel_get_daily_start_time();
         //if (e_launch_time < t_start + SECONDS_PER_HOUR + 5 * SECONDS_PER_MINUTE) { 
    if (store_read_curr_score_time() < 
        time_start_of_today() + (time_t)enamel_get_daily_start_time()) { 
      store_reset_curr_score();
    }

    // TODO: Calculate steps only at the scheduled wakeup event? What if user accomplish goal and manually check it before the scheduled wakeup?
    // This is redundant and for debug only, later on we will only update steps at wakeup launch, and we won't change curr_score other than notification/period launch.
    steps_update(); 

    // Set the message ID to be pass/fail. This will be overwritten by the true random
    // message ID if this is a LAUNCH_WAKEUP_ALERT event.
    if (steps_get_pass()) {
      s_msg_id = "pass";
    } else {
      s_msg_id = "fail";
    }
    
    if (lr != APP_LAUNCH_WAKEUP) {
      switch (lr) {
        case APP_LAUNCH_USER: // When launched via the launch menu on the watch.
          e_launch_reason = LAUNCH_USER;
          break;
        case APP_LAUNCH_PHONE: // When open the App's settings page or after installation 
          e_launch_reason = LAUNCH_PHONE;
          break;
        case APP_LAUNCH_WAKEUP: 
          APP_LOG(APP_LOG_LEVEL_ERROR, "Should not enter here!");
          break;
        default: 
          e_launch_reason = LAUNCH_OTHER;
      }
      // Display the wakeup window.
      s_wakeup_window = wakeup_window_push();

      // Always re-schedule wakeup events.
      wakeup_schedule_events();
    } else { // i.e. case APP_LAUNCH_WAKEUP: // When launched due to wakeup event.
      WakeupId wakeup_id;
      int32_t wakeup_cookie;

      // Call the wakeup handler.
      wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
      wakeup_handler(wakeup_id, wakeup_cookie);

      // Only the wakeup launch will timeout.
      will_timeout = true;
    }

    // Start timer.
    tick_second_subscribe(will_timeout);
  } else {
    s_dialog_window = dialog_window_push();
    dialog_text_layer_update_proc(
      "You must provide consent and activate this app from the 'Settings' page on your phone.");
  }
}

/**
 * Received configuration update from PebbleKit JS. 
 * Enamel will automatically persist the new configuration. 
 * Here we only update the GUI window on the watch to reflect the new settings right away,
 * and store the current timestamp to the persistent storage.
 */
void update_config(void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "in update_config. %d", enamel_get_activate());
  APP_LOG(APP_LOG_LEVEL_INFO, "%s, %d, %d", enamel_get_watch_alert_text(), enamel_get_is_consent(), enamel_get_break_freq());
  // FIXME: this seems to cause the scroll window not properly response to the up/down buttons.
  
  // Assuming only two states/windows (activated/non-activated)
  if (enamel_get_activate()) {
    //TODO: double check whether this is redundant?
    if (s_wakeup_window == NULL) {
      launch_handler(true); // Change from dialog_window to wakeup_window.
    } else {
      wakeup_schedule_events();

      // Update the current content of wakeup_window.
      wakeup_window_breathe(); 
    }

    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_dialog_window, false);
    s_dialog_window = NULL;
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got inactivated. Cancelling all wakeup events.");
    wakeup_cancel_all();

    if (s_wakeup_window != NULL) {
      launch_handler(false); // Change from wakeup_window to dialog_window.
    }

    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_wakeup_window, false);
    s_wakeup_window = NULL;
  }

  // Update the configuration update time to be the current launch time.
  store_write_config_time(e_launch_time);

  // Reset the current progress to eliminate any inconsistency after config change.
  APP_LOG(APP_LOG_LEVEL_ERROR, "IDSJFODSIJFOISDJGOIDSJGPOISDJG");
  store_reset_curr_score();

  // Force it to timeout.
  tick_second_subscribe(true);
}

/* Received message from the Pebble phone app (i.e. PebbleKit JS). 
 * Once connection is up (i.e. received the first message from the phone app), we start
 * performing the following actions in order:
 * 1. Send the launch info of the current launch.
 * 2. Try to resend launch info in the history.
 * 3. Try to resend steps data in the history.
 * 4. Send the steps data of the current launch (since we only keep track of the timestamp of
 *    the last uploaded steps data, we want to send the oldest steps data first).
 *
 * Will send the exit info of the current launch in deinit().
 */
void init_callback(DictionaryIterator *iter, void *context) {
  if (!enamel_get_activate()) return; // Will not response to PebbleKit JS if inactivated.

  static int init_stage = 0; // Track data uploading progress.

  if(dict_find(iter, AppKeyJSReady)) {
    // If is possible to receive multiple ready message if the Pebble app on phone is re-
    // launched. Reset the stage variable to prevent going further in the data-upload process.
    e_js_ready = true;
    //init_stage = 0; // FIXME: somehow this will enter multiple time?
    APP_LOG(APP_LOG_LEVEL_INFO, "Connected to JS!");
  } else if (!dict_find(iter, AppKeyServerReceived)) {
    // If this message is NOT coming from the server, it is a Clay setting message, which 
    // has already been handled by Enamel (simply return here).
    return;
  } else {
    e_server_ready = true;
  }

  bool is_finished = false;

  APP_LOG(APP_LOG_LEVEL_INFO, "Init stage %d", init_stage);

  // Reset the timer so that app will not timeout and exit while data is transferring (assume
  // the round trip time is less than the timeout limit).
  tick_reset_count();
  
  //Tuple* tuple = dict_find(iter, MESSAGE_KEY_config_update_by_server);
  //if (tuple) {
  //  APP_LOG(APP_LOG_LEVEL_ERROR, "config_update_by_server=%d", (int)tuple->value->int32);
  //} else {
  //  APP_LOG(APP_LOG_LEVEL_ERROR, "Not contain MESSAGE_KEY_config_update_by_server");
  //}

  switch (init_stage) {
    case 0:
      // First message from phone. Connection between watch and phone is established.
      e_js_ready = true;
      launch_send_launch_notification();
      init_stage++;
      break;
    case 1:
      // Connection between phone and server is established.
      is_finished = store_resend_launchexit_event();
      if (is_finished) {
        init_stage++; 

        // Since no data is sent and no packet expected to arrive, we call this function 
        // again to move to the next stage.
        init_callback(iter, context);
      }
      break;
    case 2: 
      is_finished = store_resend_steps();
      if (is_finished) {
        init_stage++;
        init_callback(iter, context); // Same reason as in init_stage 1.
      }
      break;
    case 3: 
      steps_send_latest(e_launch_time);
      init_stage++;
      break;
    default:
      // Now resend the stored data that we were not able to send previously.
      APP_LOG(APP_LOG_LEVEL_ERROR, "Should reach here only once!");
  }
}



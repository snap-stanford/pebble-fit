#include "launch.h"

// Global variables for the current launch.
time_t e_launch_time;
time_t e_step_upload_time = 0;
int e_launch_reason;
int e_exit_reason;
bool e_waiting_launchexit_ack;
bool e_force_to_save;

// Display windows.
static Window *s_dialog_window = NULL;
static Window *s_wakeup_window = NULL;

// Communication.
static int s_init_stage = 0;  // Track data uploading progress.

static int s_config_request;
static char *s_random_message = "";
char s_random_message_buf[RANDOM_MSG_SIZE_MAX];

// For functions that upload data to the server.
static time_t s_t_launch, s_t_exit, s_curr_time;
static uint8_t s_br, s_lr, s_er;
static const char *s_msg_id;
static uint8_t s_score_diff = 255; // Assume total break is less than 256.

/* Add launch reason and date to out dict. */
// FIXME: could combine both launch and exit packets, since they have very similar format.
static void prv_launch_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyDate, &e_launch_time, sizeof(int), true);
  dict_write_int(out, AppKeyLaunchReason, &e_launch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyConfigRequest, &s_config_request, sizeof(int), true);
  dict_write_uint8(out, AppKeyScoreDiff, s_score_diff);
  dict_write_uint8(out, AppKeyBreakCount, s_br);
  dict_write_cstring(out, AppKeyMessageID, s_msg_id);

  dict_write_cstring(out, MESSAGE_KEY_time_zone, enamel_get_time_zone());

  int start_time = enamel_get_daily_start_time();
  dict_write_int(out, MESSAGE_KEY_daily_start_time, &start_time, sizeof(int), true);

  int end_time = enamel_get_daily_end_time();
  dict_write_int(out, MESSAGE_KEY_daily_end_time, &end_time, sizeof(int), true);
  dict_write_uint8(out, MESSAGE_KEY_break_freq, enamel_get_break_freq());
  dict_write_uint8(out, MESSAGE_KEY_break_len, enamel_get_break_len());
  dict_write_uint8(out, MESSAGE_KEY_step_threshold, enamel_get_step_threshold());
  dict_write_cstring(out, MESSAGE_KEY_group, enamel_get_group());
  int vibrate = enamel_get_vibrate();
  dict_write_int(out, MESSAGE_KEY_vibrate, &vibrate, sizeof(int), true);
  int display_duration = enamel_get_display_duration();
  dict_write_int(out, MESSAGE_KEY_display_duration, &display_duration, sizeof(int), true);
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
  dict_write_uint8(out, AppKeyScoreDiff, s_score_diff);
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
void launch_resend(time_t t_launch, time_t t_exit, char *msg_id, uint8_t sd,
                   uint8_t br, uint8_t lr, uint8_t er) {
  // The current launch record has been uploaded and the current exit reason has
  // not yet been collected, so it is safe to modify these two varaibles here.
  s_t_launch = t_launch;
  s_t_exit = t_exit;
  s_msg_id = msg_id;
  s_score_diff = sd;
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
  int i, msgSize;

  const char *msg_ptr = store_read_random_message();
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s!", msg_ptr);
  #endif

  snprintf(s_random_message_buf, sizeof(s_random_message_buf), msg_ptr);
  s_msg_id = s_random_message_buf;

  // Seperate the message ID from the message content.
  for (msgSize = 0; msg_ptr[msgSize] != ':'; msgSize++) {}
  s_random_message_buf[msgSize] = '\0';
  if (msgSize != 4) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "msgID should be 4 characters.");
  }

  if (msg_ptr[0] != 'o') { // Deal with action and health messages.
    s_random_message = s_random_message_buf + msgSize + 1;
  } else { // Deal with the outcome messages specially.
    int start, end, score_diff;
    char mode = s_msg_id[1];

    // Parse the random message ID, which contains 4 bytes/characters:
    // Action messages: am**, where ** represents digits 0-9.
    //
    // Health messages: hm**, where ** represents digits 0-9.
    //
    // Outcome messages: oy**, where ** represents digits 0-9, and y represents:
    //                   u - compare to user self's average score, without number.
    //                   v - compare to user self's average score, with a number.
    //                   w - compare to user self's best score, with a number.
    //                   a - compare to the average score of all users, without number.
    //                   b - compare to the average score of all users, with a number.
    //   To be able to distinguish between one of three outcomes of the outcome messages,
    //   we will change the first letter from 'o' to one of the following:
    //                   x - current score > reference score.
    //                   y - current score < reference score.
    //                   z - current score = reference score.

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

    // Fetch the proper message. Loop starts at index 5 assuming msgID is 4-char long and
    // ':' is used as delimiter betwee msgID and the actual message content.
    // Add special encoding at the end of msgID (a/b/c) to indicate the message selection.
    if (score_diff > 0) {
      start = msgSize + 1;
      for (i = start; msg_ptr[i] != '|'; i++) {}
      end = i;
      s_random_message_buf[3] = 'a';
    } else if (score_diff < 0) {
      for (i = 5; msg_ptr[i] != '|'; i++) {}
      for (start = ++i; msg_ptr[i] != '|'; i++) {}
      end = i;
      s_random_message_buf[3] = 'b';
    } else { // score_diff == 0 (achiving the same score)
      for (i = 5; msg_ptr[i] != '|'; i++) {}
      for (++i; msg_ptr[i] != '|'; i++) {}
      for (start = ++i; msg_ptr[i] != '\0'; i++) {}
      end = i;
      s_random_message_buf[3] = 'c';
    }

    // Separate it from the remaining string.
    s_random_message_buf[end] = '\0';
    #if DEBUG
      APP_LOG(APP_LOG_LEVEL_ERROR, "msgid=%s, score_diff=%d", s_msg_id, score_diff);
      APP_LOG(APP_LOG_LEVEL_ERROR, "start=%d, end=%d", start, end);
    #endif
    if (score_diff < 0) {
      score_diff = -1 * score_diff;
    }
    s_score_diff = (uint8_t)(score_diff & 0xF);

    if (mode == 'u' || mode == 'a') {
      s_random_message = s_random_message_buf + start;
    } else {
      // Substitute the number in the message.
      s_random_message = s_random_message_buf + msgSize + 1;
      snprintf(s_random_message, end - start + score_diff / 10, msg_ptr + start, score_diff);
    }
  }
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_ERROR, "Score diff: %u", (unsigned int)s_score_diff);
    APP_LOG(APP_LOG_LEVEL_ERROR, "Random msgID: %s", s_msg_id);
    APP_LOG(APP_LOG_LEVEL_ERROR, "Random msg: %s", s_random_message);
  #endif
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

/**
 * Return the score different (only applicable for certain outcome messages).
 */
uint8_t launch_get_score_diff() {
  return s_score_diff;
}

/* This function is called at scheduled wakeup event.
 * If the activity goal is met or the current activity is Sleep/RestfulSleep, it does nothing.
 * Otherwise, it will alert users by vibration and popping up alert window on the watch.
 */
static void prv_wakeup_vibrate(bool force) {
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
void launch_wakeup_handler(WakeupId wakeup_id, int32_t wakeup_cookie) {
#if DEBUG
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: wakeup=%d cookie=%d", (int)wakeup_id, (int)wakeup_cookie);
#endif

  // Always re-schedule wakeup events
  wakeup_schedule_events();

  // wakeup_cookie is the index associated to the wakeup event. It is also the wakeup type for
  // the normal wakeup events.
  if (wakeup_cookie >= LAUNCH_WAKEUP_PERIOD && wakeup_cookie <= LAUNCH_WAKEUP_SILENT) {
    e_launch_reason = wakeup_cookie;

    // Re-init communication and upload data. This is not called for the standard wakeup,
    // but only when a wakeup event happens during the app is on.
    if (e_js_ready) {
      //APP_LOG(APP_LOG_LEVEL_ERROR, "should before init_callback");
      launch_send_launch_notification();
      s_init_stage = 1;
    }

    // Calculate the current period steps info, and then set the message ID to be pass/fail.
    // This will be overwritten by the true random message ID if this is a LAUNCH_WAKEUP_ALERT.
    steps_update();
    if (steps_get_pass()) {
      s_msg_id = "pass";
    } else {
      s_msg_id = "fail";
    }

    // This could happen if we receive wakeup event while the app has been on the foreground.
    if (s_wakeup_window) {
      window_stack_remove(s_wakeup_window, false);
    }

    // Calculate the maximum possible score (used for outcome and summary message).
    store_set_possible_score();

    switch (e_launch_reason) {
      case LAUNCH_WAKEUP_ALERT:
        // Get a random message from the persistent storage. This must happen before
        // wakeup_window_push() and the first call to init_callback.
        //APP_LOG(APP_LOG_LEVEL_ERROR, "Make sure this happens before init_callback()!");
        if (!steps_get_pass()) { // Only push the window if step goal is not met.
          launch_set_random_message();
          s_wakeup_window = wakeup_window_push();
        } else {
          e_exit_reason = EXIT_TIMEOUT; // FIXME: or using a new coding for silent-wakeup?
        }
        prv_wakeup_vibrate(false);
        break;
      case LAUNCH_WAKEUP_PERIOD:
        // For now, even for period-wakeup and goal is met, we still push window.
        prv_wakeup_vibrate(true);
        s_wakeup_window = wakeup_window_push();
        break;
      case LAUNCH_WAKEUP_SILENT:
        APP_LOG(APP_LOG_LEVEL_INFO, "Silent wakeup.\n");
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "\nShould NOT reach here!\n");
    }

    // FIXME: Vibrate after the window is displayed.
    // Force every wakeup to vibrate to get attention?
    //prv_wakeup_vibrate(false);
    //prv_wakeup_vibrate(true);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
    e_launch_reason = LAUNCH_WAKEUP_FALLBACK;
  }

  // Start timer
  tick_second_subscribe(true);

  // FIXME: should call init_callback() to upload data? but how should we prevent interupting
  // the current data-upload process. Or just store persistently and resend later.
  //if (e_server_ready) {
  //  init_callback();
  //}
}

/**
 * Wrapper for launch_wakeup_handler.
 * Force to save the current launch event to persistent storage.
 */
void launch_wakeup_handler_wrapper(WakeupId wakeup_id, int32_t wakeup_cookie) {
  e_force_to_save = true;

  launch_wakeup_handler(wakeup_id, wakeup_cookie);
}

/**
 * Handle launch events.
 * Push a window depends on whether this App is activated or not.
 * Return the newly created window.
 */
void launch_handler(bool activate) {
    //int ref_count = 4;
    //int s_possible_score  = 4;
    //ref_count = (int)round((float)ref_count * s_possible_score / enamel_get_total_break());
    //printf("aaa:%d", ref_count);

  if (activate) {
    bool will_timeout = false;

    // Reset the score to 0 at the first launch of the day.
    //   Previosuly: reset break count to 0 if it is the first launch in the day (since we will
    //   re-calculate the steps upon the first wakeup event, it is safe to reset multiple times)
    //   time_t t_start = time_start_of_today() + enamel_get_daily_start_time();
         //if (e_launch_time < t_start + SECONDS_PER_HOUR + 5 * SECONDS_PER_MINUTE) {
    if (store_read_curr_score_time() <
        time_start_of_today() + (time_t)enamel_get_daily_start_time()) {
      store_reset_curr_score();
    }

    int lr = launch_reason();
    if (lr != APP_LAUNCH_WAKEUP) {
      steps_update();
      if (steps_get_pass()) {
        s_msg_id = "pass";
      } else {
        s_msg_id = "fail";
      }

      // Calculate the maximum possible score (used for outcome and summary message).
      store_set_possible_score();

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

      // Call the wakeup handler with fetched wakeup ID and cookie.
      // Note that common tasks (e.g. steps_update, store_set_possible_score, etc.) are
      // moved into launch_wakeup_handler since they are needed when wakeup happens during
      // the app is on.
      wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
      launch_wakeup_handler(wakeup_id, wakeup_cookie);

      // Only the wakeup launch will timeout.
      will_timeout = true;
    }

    // Start timer.
    tick_second_subscribe(will_timeout);
  } else {
    s_dialog_window = dialog_window_push();
    //dialog_text_layer_update_proc(
    //  "You must provide consent and activate this app from the 'Settings' page on your phone.");
    // Use different formats for watches with different shape.
    #if defined(PBL_ROUND)
      dialog_text_layer_update_proc("To join the study, open the MyMobilize app on your phone (found in your Pebble app library), and click                    on settings to complete the Eligibility and Consent process.");
    #else
      dialog_text_layer_update_proc("To join the study, open the MyMobilize app on your phone (found in your Pebble app library), and click ______________                                       on settings to complete the Eligibility and Consent process.");
    #endif

    prv_wakeup_vibrate(true);

    // Still buzz user for providing consent otherwise user might just forget.
    wakeup_schedule_events();
  }
}

/**
 * Received configuration update from PebbleKit JS.
 * Enamel will automatically persist the new configuration.
 * Here we only update the GUI window on the watch to reflect the new settings right away,
 * and store the current timestamp to the persistent storage.
 */
void update_config(void *context) {
  // FIXME: this seems to cause the scroll window not properly response to the up/down buttons.
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: update_config(): activate=%d", enamel_get_activate());
    APP_LOG(APP_LOG_LEVEL_ERROR, "DEBUG: first_config=%d",  enamel_get_first_config());
  #endif

  // Assuming only two states/windows (activated/non-activated)
  if (enamel_get_activate()) {
    // Calculate the maximum possible score (used for outcome and summary message).
    store_set_possible_score();

    if (s_wakeup_window == NULL) {
      if (enamel_get_first_config() != 1)
        APP_LOG(APP_LOG_LEVEL_ERROR, "first_config must be 1");

      // Also upload the historical data up to 7 days before.
      //store_write_upload_time(e_launch_time - 7 * SECONDS_PER_DAY);

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
    APP_LOG(APP_LOG_LEVEL_INFO, "INFO: Got inactivated. Cancelling all wakeup events.");
    wakeup_cancel_all();

    store_delete_all();

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
  // TODO: temporary turn off this feature.
  //store_reset_curr_score();
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
  APP_LOG(APP_LOG_LEVEL_INFO, "This is init_callback(), s_init_stage=%d!", s_init_stage); 

  if (dict_find(iter, AppKeyJSReady)) {
    // If is possible to receive multiple ready message if the Pebble app on phone is re-
    // launched. Reset the stage variable to prevent going further in the data-upload process.
    e_js_ready = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Connected to JS!");
  }

  if (dict_find(iter, AppKeyServerReceived)) {
    e_server_ready = true;

    #if DEBUG
      APP_LOG(APP_LOG_LEVEL_ERROR, "DEBUG: AppKeyServerReceived received!!!!!!!");
    #endif

    // Update the last upload time covered by the previous data upload.
    //if (e_step_upload_time > store_read_upload_time()) {
    //  store_write_upload_time(e_step_upload_time);
    //}
  } else if (s_init_stage >= 1) {
    // If this message is NOT coming from the server, it is initiated from the phone alone,
    // so we should not continue uploading more data.
    #if DEBUG
      APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: AppKeyServerReceived is missing");
    #endif
    return;
  } else if (e_launch_reason == LAUNCH_WAKEUP_ALERT && steps_get_pass()) {
    // Do not communicate with the phone if we do not display any message on the screen,
    // Since there is no enough time for the communication to complete.
    #if DEBUG
      APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: ALERT wakeup and pass.");
    #endif
    return;
  }

  Tuple* tuple = dict_find(iter, MESSAGE_KEY_step_upload_time);
  if (tuple) {
    store_write_upload_time(tuple->value->int32);
  }

  if (!enamel_get_activate()) return; // Will not response to PebbleKit JS if inactivated.

  bool is_finished = false;

  // Reset the timer so that app will not timeout and exit while data is transferring (assume
  // the round trip time is less than the timeout limit).
  tick_reset_count();

  //Tuple* tuple = dict_find(iter, MESSAGE_KEY_config_update_by_server);
  //if (tuple) {
  //  APP_LOG(APP_LOG_LEVEL_ERROR, "config_update_by_server=%d", (int)tuple->value->int32);
  //} else {
  //  APP_LOG(APP_LOG_LEVEL_ERROR, "Not contain MESSAGE_KEY_config_update_by_server");
  //}

  switch (s_init_stage) {
    case 0:
      // First message from phone. Connection between watch and phone is established.
      e_js_ready = true;
      launch_send_launch_notification();
      s_init_stage++;
      break;
    case 1:
      // Connection between phone and server is established.
      is_finished = store_resend_launchexit_event();
      if (is_finished) {
        s_init_stage++;

        // Since no data is sent and no packet expected to arrive, we call this function
        // again to move to the next stage.
        init_callback(iter, context);
      }
      break;
    case 2:
      //is_finished = store_resend_steps();
      is_finished = steps_upload_steps();
      if (is_finished) {
        s_init_stage++;
        init_callback(iter, context); // Same reason as in s_init_stage 1.
      }
      break;
    default:
      // Now resend the stored data that we were not able to send previously.
      // TODO: we might enter here is new message sent from phone arrives.
      APP_LOG(APP_LOG_LEVEL_ERROR, "Should reach here only once!");
  }
}



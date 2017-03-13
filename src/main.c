#include <pebble.h>

#include "enamel.h"
#include <pebble-events/pebble-events.h>

#include "constants.h"
#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/launch.h"
#include "modules/store.h"
#include "services/health.h"
#include "services/tick.h"
#include "services/wakeup.h"
#include "windows/main_window.h"
#include "windows/wakeup_window.h"
#include "windows/dialog_window.h"

static EventHandle s_normal_msg_handler, s_enamel_msg_handler;
static time_t s_exit_time;
static Window *s_dialog_window = NULL; 
static Window *s_wakeup_window = NULL;

//static void prv_launch_write(DictionaryIterator * out);
static void prv_update_config(void *context);
static void prv_init_callback(DictionaryIterator *iter, void *context);
static void prv_launch_handler(bool activate);


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
static void prv_init_callback(DictionaryIterator *iter, void *context) {
  if (!enamel_get_activate()) return; // Will not response to PebbleKit JS if inactivated.

  static int init_stage = 0;

  bool is_finished = false;

  APP_LOG(APP_LOG_LEVEL_INFO, "Init stage %d", init_stage);

  if (!e_server_ready && (dict_find(iter, MESSAGE_KEY_config_update) || 
      dict_find(iter, AppKeyServerReceived))) {
    e_server_ready = true;
  }

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
        prv_init_callback(iter, context);
      }
      break;
    case 2: 
      is_finished = store_resend_steps();
      if (is_finished) {
        init_stage++;
        prv_init_callback(iter, context); // Same reason as in init_stage 1.
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

/**
 * Received configuration update from PebbleKit JS. 
 * Enamel will automatically persist the new configuration. 
 * Here we only update the GUI window on the watch to reflect the new settings right away,
 * and store the current timestamp to the persistent storage.
 */
static void prv_update_config(void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "in prv_update_config. %d", enamel_get_activate());
  APP_LOG(APP_LOG_LEVEL_INFO, "%s, %d, %d", enamel_get_watch_alert_text(), enamel_get_is_consent(), enamel_get_break_freq());
  // TODO 2/24: this seems to cause the scroll window not properly response to the up/down buttons.
  
  if (enamel_get_activate()) {
    //TODO: double check whether this is redundant?
    wakeup_schedule_events();
    if (s_wakeup_window == NULL) {
      prv_launch_handler(true); // FIXME: this will cause infinite recursive calls.  // Change from dialog_window to wakeup_window.
    } else {
      // Update the current content of wakeup_window.
      wakeup_window_breathe(); 
    }

    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_dialog_window, false);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Got inactivated. Cancelling all wakeup events!");
    wakeup_cancel_all();
    if (s_wakeup_window != NULL) {
      prv_launch_handler(false); // Change from wakeup_window to dialog_window.
    }

    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_wakeup_window, false);
  }

  store_write_config_time(e_launch_time);

  store_reset_break_count();

  tick_second_subscribe(true); // Will timeout
}

/* This function is called at scheduled wakeup event.
 * If the activity goal is met or the current activity is Sleep/RestfulSleep, it does nothing.
 * Otherwise, it will alert users by vibration and popping up alert window on the watch.
 */
static void prv_wakeup_vibrate(bool force) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "in prv_wakeup_vibrate()");
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
static void wakeup_handler(WakeupId wakeup_id, int32_t wakeup_cookie) {
  APP_LOG(APP_LOG_LEVEL_INFO, "wakeup %d , cookie %d", (int)wakeup_id, (int)wakeup_cookie);
  
  // wakeup_cookie is the index associated to the wakeup event.
  e_launch_reason = wakeup_cookie;
  
  if (wakeup_cookie >= LAUNCH_WAKEUP_PERIOD) {
    prv_wakeup_vibrate(false);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
  }
  
  // This could happen if we receive wakeup event while the app has been on the foreground.
  if (s_wakeup_window) {
    window_stack_remove(s_wakeup_window, false);
  }

  // TODO: if this is a notification wakeup and goal is met, should we still push window?
  if (e_launch_reason != LAUNCH_WAKEUP_NOTIFY || !steps_get_pass()) {
    s_wakeup_window = wakeup_window_push();
  }

  // Start timer
  tick_second_subscribe(true);

  // Always re-schedule wakeup events (do not put return in the above code)
  wakeup_schedule_events();
}

/** 
 * Handle launch events. 
 * Push a window depends on whether this App is activated or not.
 */
static void prv_launch_handler(bool activate) {
  if (activate) {
    WakeupId wakeup_id;
    bool will_timeout = false;
    // FIXME: subscribe to wakeup event to update steps even App is in the foreground.

    // Reset break count to 0 if it is the first launch in the day (since we will re-calculate
    // the steps upon the first wakeup event, it is safe to reset multiple times)
    time_t start_time = time_start_of_today() + enamel_get_daily_start_time();
    if (e_launch_time < start_time + SECONDS_PER_HOUR + 5 * SECONDS_PER_MINUTE) { 
      store_reset_break_count();
    }

    // TODO: Calculate steps only at the scheduled wakeup event? What if user accomplish goal and manually check it before the scheduled wakeup?
    steps_update(); 

    // Get a random message from the persistent storage. This must happen before
    // wakeup_window_push() and the first call to prv_init_callback. 
    // TODO: no every group needs this. Only wakeup launch needs this.
    launch_set_random_message();

    switch (launch_reason()) {
      case APP_LAUNCH_USER: // When launched via the launch menu on the watch.
        e_launch_reason = LAUNCH_USER;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();

        s_wakeup_window = wakeup_window_push();
        break;
      case APP_LAUNCH_WAKEUP: // When launched due to wakeup event.
        will_timeout = true;

        int32_t wakeup_cookie;
        wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
        wakeup_handler(wakeup_id, wakeup_cookie);        
/*
        APP_LOG(APP_LOG_LEVEL_INFO, "wakeup %d , cookie %d", (int)wakeup_id, (int)wakeup_cookie);

        // wakeup_cookie is the index associated to the wakeup event.
        e_launch_reason = wakeup_cookie;
        
        if (wakeup_cookie <= LAUNCH_WAKEUP_DAILY) {
          prv_wakeup_vibrate(false);
        } else {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
        }

        // TODO: if this is a notification wakeup and goal is met, should we still push window?
        s_wakeup_window =  wakeup_window_push();
*/
        break;
      case APP_LAUNCH_PHONE: // When open the App's settings page or after installation 
        e_launch_reason = LAUNCH_PHONE;

        s_wakeup_window = wakeup_window_push();
        break;
      default: 
        e_launch_reason = LAUNCH_OTHER;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();
        s_wakeup_window = wakeup_window_push();
    }

    if (launch_reason() != APP_LAUNCH_WAKEUP) {
      // Start timer
      tick_second_subscribe(will_timeout);

      // Always re-schedule wakeup events (do not put return in the above code)
      wakeup_schedule_events();
    }
  } else {
    s_dialog_window = dialog_window_push();
    dialog_text_layer_update_proc("You must activate this app from the 'Settings' page on your phone.");
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "pebble-fit launch_reason = %d", e_launch_reason);
}

static void init(void) {
  e_launch_time = time(NULL); // FIXME: rounded to minute?

  //health_subscribe();
  enamel_init();
  
  //app_message_register_inbox_received(e_js_ready_handler);
  //app_message_set_context(callback);
  
  events_app_message_request_outbox_size(APP_MESSAGE_OUTBOX_SIZE_MINIMUM);

  s_normal_msg_handler = events_app_message_register_inbox_received(prv_init_callback, NULL);
  s_enamel_msg_handler = enamel_settings_received_subscribe(prv_update_config, NULL);

  events_app_message_open(); // Call pebble-events app_message_open function

  // subscribe to wakeup service to get wakeup events while app is running
  wakeup_service_subscribe(wakeup_handler);

  // TODO: Modify the default value of activate in config.json
  prv_launch_handler(enamel_get_activate());
}

static void deinit(void) {
  // FIXME: if app remains active, steps data keep sending to the server?
  s_exit_time = time(NULL);
  if (enamel_get_activate()) {
    if (e_server_ready) {
      // Send the exit record (the launch record has already been uploaded).
      launch_send_exit_notification(s_exit_time);
    } else {
      // Store launch-exit record if the connection could not be established right now.
      store_write_launchexit_event(e_launch_time, s_exit_time, e_launch_reason, e_exit_reason);
    }
  }

  // Deinit Enamel to unregister App Message handlers and save settings
  enamel_settings_received_unsubscribe(s_normal_msg_handler);
  enamel_settings_received_unsubscribe(s_enamel_msg_handler);
  enamel_deinit();

  // App will exit to default watchface
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

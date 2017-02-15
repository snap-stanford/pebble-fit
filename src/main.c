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
static time_t s_launch_time;
static time_t s_exit_time;
static Window *s_dialog_window = NULL; 
static Window *s_wakeup_window = NULL;
static bool s_enamel_on = false;

//static void prv_launch_write(DictionaryIterator * out);
static void prv_update_config(void *context);
static void prv_init_callback(DictionaryIterator *iter, void *context);
static void prv_wakeup_vibrate();
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
  switch (init_stage) {
    case 0:
      // Connection between watch and phone is established (may or may not contain AppKeyJSReady).
      js_ready = true;
      launch_send_launch_notification(s_launch_time);
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
      steps_send_latest();
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
  
  if (enamel_get_activate()) {
    //TODO: double check whether this is redundant?
    schedule_wakeup_events(steps_get_inactive_minutes(), s_launch_time);
    if (s_wakeup_window == NULL) {
      prv_launch_handler(true); // FIXME: this will cause infinite recursive calls.  // Change from dialog_window to wakeup_window.
    } else {
      wakeup_window_breathe(); // Update the current content of wakeup_window.
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Got inactivated. Cancelling all wakeup events!");
    wakeup_cancel_all();
    if (s_wakeup_window != NULL) {
      prv_launch_handler(false); // Change from wakeup_window to dialog_window.
    }
  }

  store_write_config_time(time(NULL));

  store_reset_break_count();

  tick_second_subscribe(true); // Will timeout
}

/* This function is called at scheduled wakeup event.
 * If the activity goal is met or the current activity is Sleep/RestfulSleep, it does nothing.
 * Otherwise, it will alert users by vibration and popping up alert window on the watch.
 */
static void prv_wakeup_vibrate() {
  HealthActivityMask activity = health_service_peek_current_activities();

  if (activity != HealthActivitySleep && activity != HealthActivityRestfulSleep &&
      !steps_get_pass()) {
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

/* Push a window depends on whether this App is activated or not. */ 
static void prv_launch_handler(bool activate) {
  APP_LOG(APP_LOG_LEVEL_INFO, "pebble-fit launch_reason = %d", (int)launch_reason());
  // Testing-begin
  HealthActivityMask activity = health_service_peek_current_activities();
  switch(activity) {
    case HealthActivityNone:
      APP_LOG(APP_LOG_LEVEL_ERROR, "No activity."); break;
    case HealthActivitySleep: 
    case HealthActivityRestfulSleep: 
      APP_LOG(APP_LOG_LEVEL_ERROR, "Sleeping activity."); break;
    case HealthActivityWalk:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Walking activity."); break;
    case HealthActivityRun:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Running activity."); break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown activity."); break;
  }
  // Testing-end
  if (activate) {
    WakeupId wakeup_id;
    int32_t wakeup_cookie;
    bool will_timeout = false;
    // FIXME: subscribe to wakeup event to update steps even App is in the foreground.

    // Reset break count to 0 if it is the first launch in the day (since we will re-calculate
    // the steps upon the first wakeup event, it is safe to reset multiple times)
    time_t start_time = time_start_of_today() + enamel_get_daily_start_time();
    if (s_launch_time < start_time + SECONDS_PER_HOUR + 5 * SECONDS_PER_MINUTE) { 
      store_reset_break_count();
    }

    switch (launch_reason()) {
      case APP_LAUNCH_USER: // When launched via the launch menu on the watch.
        e_launch_reason = USER_LAUNCH;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();

        s_wakeup_window = wakeup_window_push(false);
        break;
      case APP_LAUNCH_WAKEUP: // When launched due to wakeup event.
        e_launch_reason = WAKEUP_LAUNCH;
        will_timeout = true;

	// TODO: Calculate steps only at the scheduled wakeup event? What if user accomplish goal and manually check it before the scheduled wakeup?
        steps_update(); 


        wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
        APP_LOG(APP_LOG_LEVEL_INFO, "wakeup %d , cookie %d", (int)wakeup_id, (int)wakeup_cookie);
        if (wakeup_cookie == 0) {
          prv_wakeup_vibrate();
        } else {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
        }

        s_wakeup_window =  wakeup_window_push(true);
        break;
      case APP_LAUNCH_PHONE: // When open the App's settings page or after installation 
        e_launch_reason = PHONE_LAUNCH;

        s_wakeup_window = wakeup_window_push(false);
        break;
      default: 
        e_launch_reason = OTHER_LAUNCH;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();
        s_wakeup_window = wakeup_window_push(false);
    }
    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_dialog_window, false);

    // Start timer
    tick_second_subscribe(will_timeout);

    // Always re-schedule wakeup events (do not put return in the above code)
    schedule_wakeup_events(steps_get_inactive_minutes(), s_launch_time);
  } else {
    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_wakeup_window, false);
    s_dialog_window = dialog_window_push();
  }
}

static void init(void) {
  s_launch_time = time(NULL); // FIXME: rounded to minute?

  //health_subscribe();
  enamel_init();
  
  //app_message_register_inbox_received(js_ready_handler);
  //app_message_set_context(callback);
  
  events_app_message_request_outbox_size(APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
	//s_normal_msg_handler = events_app_message_register_inbox_received(prv_test, prv_init_callback);
	s_normal_msg_handler = events_app_message_register_inbox_received(prv_init_callback, NULL);
  s_enamel_msg_handler = enamel_settings_received_subscribe(prv_update_config, NULL);
  events_app_message_open(); // Call pebble-events app_message_open function

  // TODO: Modify the default value of activate in config.json
  prv_launch_handler(enamel_get_activate());
}

static void deinit(void) {
  // FIXME: if app remains active, steps data keep sending to the server.
  s_exit_time = time(NULL);
  if (js_ready) {
    // Send the exit record (the launch record has already been uploaded).
    launch_send_exit_notification(s_exit_time);
  } else {
    // Store launch-exit record if the connection could not be established right now.
    store_write_launchexit_event(s_launch_time, s_exit_time, e_launch_reason, e_exit_reason);
  }
  
  //store_write_launchexit_event(s_launch_time, s_exit_time, e_launch_reason, e_exit_reason);

  // Deinit Enamel to unregister App Message handlers and save settings
  if (s_enamel_on) {
    enamel_settings_received_unsubscribe(s_normal_msg_handler);
    enamel_settings_received_unsubscribe(s_enamel_msg_handler);
  }
  enamel_deinit();

  // App will exit to default watchface
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

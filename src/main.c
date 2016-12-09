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

static EventHandle s_enamel_handler;
static Window *s_dialog_window, *s_wakeup_window;
static time_t s_launch_time;
static time_t s_exit_time;
static bool s_enamel_on = false;

//static void prv_launch_write(DictionaryIterator * out);
static void prv_update_config(void *context);
static void prv_init_callback();
static void prv_wakeup_alert();
static void prv_launch_handler(bool activate);


/* Received message from the Pebble phone app. */
static void prv_init_callback() {
  static int init_stage = 1; // FIXME.
  APP_LOG(APP_LOG_LEVEL_INFO, "Init stage %d", init_stage);
  switch (init_stage) {
    case 0:
      js_ready = true;
      // Firstly, send the launch event.
      //comm_send_data(prv_launch_write, comm_sent_handler, comm_server_received_handler);
      launch_send_on_notification(s_launch_time);
      break;
    case 1:
      // And then send the steps data.
      steps_send_latest();
      break;
    default:
      // Now resend the stored data that we were not able to send previously.
      store_send_launch_exit_event();
  }
  init_stage++;
}

/* Received configuration update from the Pebble phone app. */
static void prv_update_config(void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "in prv_update_config");
  if (enamel_get_activate()) {
    schedule_wakeup_events(steps_get_inactive_minutes());
    //prv_launch_handler(true); // FIXME: this will cause infinite recursive calls.

    // FIXME: workaround for communication channel conflict between Enamel and data uploading
    //enamel_settings_received_unsubscribe(s_enamel_handler);
    //enamel_deinit();
    //comm_init(prv_init_callback); 
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events!");
    wakeup_cancel_all();
    prv_launch_handler(false);
  }
  tick_second_subscribe(true); // Will timeout
}

static void prv_wakeup_alert() {
  HealthActivityMask activity = health_service_peek_current_activities();
  switch(activity) {
    case HealthActivityNone:
      APP_LOG(APP_LOG_LEVEL_INFO, "No activity.");
      break;
    case HealthActivitySleep: 
    case HealthActivityRestfulSleep: 
      APP_LOG(APP_LOG_LEVEL_INFO, "Sleeping activity.");
      break;
    case HealthActivityWalk:
      APP_LOG(APP_LOG_LEVEL_INFO, "Walking activity.");
      break;
    case HealthActivityRun:
      APP_LOG(APP_LOG_LEVEL_INFO, "Running activity.");
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_INFO, "Unknown activity.");
      break;
  }
  if (activity != HealthActivitySleep && activity != HealthActivityRestfulSleep &&
      steps_whether_alert()) {
    APP_LOG(APP_LOG_LEVEL_INFO, "enamel_get_vibrate()=%d", enamel_get_vibrate());
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
    //window_stack_remove(s_dialog_window, false);
    s_wakeup_window =  wakeup_window_push();
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Step goal is met. Silent wakeup.");
  }
}

/* Push a window depends on whether this App is activated or not. */ 
static void prv_launch_handler(bool activate) {
  s_launch_time = time(NULL); // FIXME: use end_time from steps?
  APP_LOG(APP_LOG_LEVEL_INFO, "pebble-fit launch_reason = %d", (int)launch_reason());
  if (activate) {
    WakeupId wakeup_id;
    int32_t wakeup_cookie;
    bool will_timeout = false;
    // FIXME: subscribe to wakeup event to update steps even App is in the foreground.

    steps_update(); // Essential for steps_whether_alert in prv_wakeup_alert.
    switch (launch_reason()) {
      case APP_LAUNCH_USER: // When launched via the launch menu on the watch.
        e_launch_reason = USER_LAUNCH;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();
        s_wakeup_window = wakeup_window_push();
        break;
      case APP_LAUNCH_WAKEUP: // When launched due to wakeup event.
        e_launch_reason = WAKEUP_LAUNCH;
        will_timeout = true;
        wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
        APP_LOG(APP_LOG_LEVEL_INFO, "wakeup %d , cookie %d", (int)wakeup_id, (int)wakeup_cookie);
        if (wakeup_cookie == 0) {
          //steps_wakeup_window_update();
          prv_wakeup_alert();
        } else {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
        }

        // Initialize communication to the phone for uploading data to the server.
        // FIXME: if not initialize our own, could use the communication set up by Enamel, but
        // seems that outbox size is not large enough. Since this conflicts with channels used
        // by Enamel, the current workaround will upload data only at wakeup event.
        //comm_init(prv_init_callback); 

        break;
      case APP_LAUNCH_PHONE: // When open the App's settings page or after installation 
        e_launch_reason = PHONE_LAUNCH;
        //window_stack_remove(s_dialog_window, false);
        //steps_wakeup_window_update();
        prv_wakeup_alert();
        s_wakeup_window = wakeup_window_push();
        break;
      default: 
        e_launch_reason = OTHER_LAUNCH;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();
        //window_stack_remove(s_dialog_window, false);
        //steps_wakeup_window_update();
        s_wakeup_window = wakeup_window_push();
        //main_window_push();
    }
    // Send the launch reason to the server.
    //launch_send_on_notification();

    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_dialog_window, false);

    // Start timer
    tick_second_subscribe(will_timeout);

    // Always re-schedule wakeup events (do not put return in the above code)
    schedule_wakeup_events(steps_get_inactive_minutes());
  } else {
    // Prevent seeing other windows when presseing the "back" button.
    window_stack_remove(s_wakeup_window, false);
    s_dialog_window = dialog_window_push();
  }
}

static void init(void) {
  //health_subscribe();
  enamel_init();
  switch (launch_reason()) {
    case APP_LAUNCH_PHONE: 
      // Initialize Enamel to register App Message handlers and restores settings
      s_enamel_handler = enamel_settings_received_subscribe(prv_update_config, NULL);
      events_app_message_open(); // Call pebble-events app_message_open function
      s_enamel_on = true;
      break;
    default: 
      // Using normal communication channel.
      comm_init(prv_init_callback);
  }

  //prv_launch_handler(enamel_get_activate()); // FIXME: fatal error causing watch to restart
  prv_launch_handler(true);
}

static void deinit(void) {
  s_exit_time = time(NULL);
  //if (js_ready) {
  //  // Send the exit record (the launch record has already been uploaded).
  //  launch_send_off_notification(s_exit_time);
  //} else {
  //  // Store launch-exit record if the connection could not be established right now.
  //  store_write_launch_exit_event(s_launch_time, s_exit_time, e_launch_reason, e_exit_reason);
  //}
  store_write_launch_exit_event(s_launch_time, s_exit_time, e_launch_reason, e_exit_reason);

  // Deinit Enamel to unregister App Message handlers and save settings
  if (s_enamel_on) {
    enamel_settings_received_unsubscribe(s_enamel_handler);
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

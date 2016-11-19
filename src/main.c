#include <pebble.h>

#include "enamel.h"
#include <pebble-events/pebble-events.h>

#include "constants.h"
#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/launch.h"
#include "services/health.h"
#include "services/tick.h"
#include "services/wakeup.h"
#include "windows/main_window.h"
#include "windows/wakeup_window.h"
#include "windows/dialog_window.h"

static EventHandle s_enamel_handler;
static Window *s_dialog_window, *s_wakeup_window;

// TODO: upload data to server

/* Push a window depends on whether this App is activated or not. */ 
static void prv_launch_handler(bool activate) {
  APP_LOG(APP_LOG_LEVEL_INFO, "pebble-fit launch_reason = %d", (int)launch_reason());
  if (activate) {
    WakeupId wakeup_id;
    int32_t wakeup_cookie;
    // FIXME: subscribe to wakeup event to update steps even App is in the foreground.

    steps_update();
    switch (launch_reason()) {
      case APP_LAUNCH_WAKEUP: // When launched due to wakeup event.
        wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
        APP_LOG(APP_LOG_LEVEL_INFO, "wakeup %d , cookie %d", (int)wakeup_id, (int)wakeup_cookie);
        if (wakeup_cookie == 0) {
          //steps_wakeup_window_update();
          if (steps_whether_alert()) {
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
            window_stack_remove(s_dialog_window, false);
            s_wakeup_window =  wakeup_window_push();
            tick_second_subscribe(true); // Will timeout
          } else {
            APP_LOG(APP_LOG_LEVEL_INFO, "Step goal is met. Silent wakeup.");
          }
        } else {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
        }
        break;
      case APP_LAUNCH_PHONE: // When open the App's settings page or after installation 
        //steps_wakeup_window_update();
        window_stack_remove(s_dialog_window, false);
        s_wakeup_window = wakeup_window_push();
        break;
      default: // When launched via the launch menu on the watch.
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();
        window_stack_remove(s_dialog_window, false);
        //steps_wakeup_window_update();
        s_wakeup_window = wakeup_window_push();
        //tick_second_subscribe(true);
        //main_window_push();
    }
    // Always re-schedule wakeup events (do not put return in the above code)
    schedule_wakeup_events(steps_get_inactive_minutes());
  } else {
    window_stack_remove(s_wakeup_window, false);
    s_dialog_window = dialog_window_push();
  }
}

static void prv_init_callback() {
  static int init_stage = 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "Init stage %d", init_stage);
  /*
  switch (init_stage) {
    case 0:
      steps_send_latest();
      break;
    case 1:
      launch_send_on_notification();
      break;
  }
  init_stage++;
  */
}

/* Received configuration update from the phone. */
static void prv_update_config(void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "in prv_update_config");
  if (enamel_get_activate()) {
    prv_launch_handler(true);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events!");
    wakeup_cancel_all();
    prv_launch_handler(false);
  }
  tick_second_subscribe(true); // Will timeout
}

static void init(void) {
  // Initialize Enamel to register App Message handlers and restores settings
  enamel_init();

  s_enamel_handler = enamel_settings_received_subscribe(prv_update_config, NULL);

  //health_subscribe();
  //comm_init(prv_init_callback);

  // call pebble-events app_message_open function
  events_app_message_open(); 

  prv_launch_handler(enamel_get_activate());
}

static void deinit(void) {
  //launch_send_off_notification();

  // Deinit Enamel to unregister App Message handlers and save settings
  enamel_settings_received_unsubscribe(s_enamel_handler);
  enamel_deinit();

  // App will exit to default watchface
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

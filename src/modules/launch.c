#include "launch.h"

static Window *s_dialog_window, *s_wakeup_window;
static time_t s_launch_time, s_delaunch_time;
static int s_launch_reason;
int delaunch_reason;

/* Add reason and date to out dict. */
static void launch_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyLaunchReason, &s_launch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_delaunch_time, sizeof(int), true);
}

/* Send launch event to phone. */
void launch_send_on_notification() {
  s_delaunch_time = time(NULL);
  switch (launch_reason()) {
    case APP_LAUNCH_USER:
    case APP_LAUNCH_QUICK_LAUNCH:
      s_launch_reason = USER_LAUNCH;
      break;
    case APP_LAUNCH_WAKEUP:
      s_launch_reason = WAKEUP_LAUNCH;
      break;
    case APP_LAUNCH_PHONE:
      s_launch_reason = PHONE_LAUNCH;
      break;
    default:
      s_launch_reason = OTHER_LAUNCH;
  }

  comm_send_data(launch_write, comm_sent_handler, comm_server_received_handler);
}

/* Main launch handler. */ 
/*
void launch_handler(bool activate) {
  APP_LOG(APP_LOG_LEVEL_INFO, "pebble-fit launch_reason = %d", (int)launch_reason());
  if (activate) {
    WakeupId wakeup_id;
    int32_t wakeup_cookie;
    bool will_timeout = false;
    // FIXME: subscribe to wakeup event to update steps even App is in the foreground.

    //steps_update();
    switch (launch_reason()) {
      case APP_LAUNCH_WAKEUP: // When launched due to wakeup event.
        s_launch_reason = WAKEUP_LAUNCH;
        will_timeout = true;
        wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
        APP_LOG(APP_LOG_LEVEL_INFO, "wakeup %d , cookie %d", (int)wakeup_id, (int)wakeup_cookie);
        if (wakeup_cookie == 0) {
          //steps_wakeup_window_update();

          // Alert
          if (steps_whether_alert()) {
            APP_LOG(APP_LOG_LEVEL_INFO, "enamel_get_vibrate()=%d", enamel_get_vibrate());
            switch (enamel_get_vibrate()) {
              // FIXME: use more descriptive cases
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
          } else {
            APP_LOG(APP_LOG_LEVEL_INFO, "Step goal is met. Silent wakeup.");
          }
        } else {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Fallback wakeup! cookie=%d", (int)wakeup_cookie);
        }
        break;
      case APP_LAUNCH_PHONE: // When open the App's settings page or after installation 
        s_launch_reason = PHONE_LAUNCH;
        //steps_wakeup_window_update();
        window_stack_remove(s_dialog_window, false);
        s_wakeup_window = wakeup_window_push();
        break;
      default: // When launched via the launch menu on the watch.
        s_launch_reason = OTHER_LAUNCH;
        APP_LOG(APP_LOG_LEVEL_ERROR, "Cancelling all wakeup events! Must be rescheduled.");
        wakeup_cancel_all();
        window_stack_remove(s_dialog_window, false);
        //steps_wakeup_window_update();
        s_wakeup_window = wakeup_window_push();
        //main_window_push();
    }

    // Start timer.
    tick_second_subscribe(will_timeout);

    // Always re-schedule wakeup events (do not put return in the above code).
    schedule_wakeup_events(steps_get_inactive_minutes());
  } else {
    window_stack_remove(s_wakeup_window, false);
    s_dialog_window = dialog_window_push();
  }

  // Send launch reason to the server.
  comm_send_data(launch_write, comm_sent_handler, comm_server_received_handler);
}
*/

/* Add reason (placeholder) and date to out dict. */
static void delaunch_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyDelaunchReason, &delaunch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_delaunch_time, sizeof(int), true);
}

/* Send delaunched event to phone. */
void launch_send_off_notification() {
  s_delaunch_time = time(NULL);
  comm_send_data(delaunch_write, comm_sent_handler, NULL);
}

#include <pebble.h>

#include "enamel.h"
#include <pebble-events/pebble-events.h>

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

static void prv_window_push(bool optin) {
 switch (launch_reason()) {
    case APP_LAUNCH_WAKEUP:
      // Must be optind
      APP_LOG(APP_LOG_LEVEL_INFO, "Wake up!!!!!!!!!!!!!!!!!!!!!!!");
      steps_update_wakeup_window_steps();
      wakeup_window_push();
      tick_second_subscribe(true);
      break;
    default:
      wakeup_cancel_all();
      if (optin) {
        steps_update_wakeup_window_steps();
        wakeup_window_push();
        tick_second_subscribe(true);
      } else {
        dialog_window_push();
      }
      //main_window_push();
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

static void prv_update_config(void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "in prv_update_config");
  if (enamel_get_optin()) {
    schedule_event(true);
    prv_window_push(true);
  } else {
    prv_window_push(false);
  }
}

static void init(void) {
  // Initialize Enamel to register App Message handlers and restores settings
  enamel_init();

  s_enamel_handler = enamel_settings_received_subscribe(prv_update_config, NULL);

  //health_subscribe();
  //comm_init(prv_init_callback);

  // call pebble-events app_message_open function
  events_app_message_open(); 

  prv_update_config(NULL);
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

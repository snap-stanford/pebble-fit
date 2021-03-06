#include <pebble.h>

#include "enamel.h"
#include <pebble-events/pebble-events.h>

#include "constants.h"
#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/launch.h"
#include "modules/store.h"
#include "services/tick.h"
#include "services/wakeup.h"

static EventHandle s_normal_msg_handler, s_enamel_msg_handler;
static time_t s_exit_time;

static void init(void) {
  e_launch_time = time(NULL); // FIXME: rounded to minute?

  e_js_ready = false;

  e_server_ready = false;

  e_waiting_launchexit_ack = false;

  e_force_to_save = false;

  enamel_init();
  
  //app_message_register_inbox_received(e_js_ready_handler);
  //app_message_set_context(callback);
  
#if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox sizes: %d, %u", APP_MESSAGE_OUTBOX_SIZE_MINIMUM, (unsigned)app_message_outbox_size_maximum());
#endif
  // Use a larger buffer to be able to send steps data in batch.
  //events_app_message_request_outbox_size(APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  events_app_message_request_outbox_size(4000);

  s_normal_msg_handler = events_app_message_register_inbox_received(init_callback, NULL);
  s_enamel_msg_handler = enamel_settings_received_subscribe(update_config, NULL);

  events_app_message_open(); // Call pebble-events app_message_open function

  // subscribe to wakeup service to get wakeup events while app is running
  wakeup_service_subscribe(launch_wakeup_handler_wrapper);

  // Handle this launch event.
  launch_handler(enamel_get_activate());
}

static void deinit(void) {
  s_exit_time = time(NULL);
  if (enamel_get_activate()) {
    if (e_server_ready && !e_force_to_save) {
      // Send the exit record (the launch record has already been uploaded).
      // TODO: there is some chance (might be rare) the connection is down suddenly.
      // Maybe we should always store this info for safe (server receive duplicate packets.
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

#include <pebble.h>

#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/wakeup.h"

#include "services/health.h"
#include "services/connection.h"

#include "windows/main_window.h"

static void deinit(void) {
  wakeup_set();
}

static void js_ready_callback() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Connected to js");
  send_latest_steps_to_phone();
}

static void init(void) {
  comm_init(js_ready_callback);
  main_window_push();
  if (wakeup_caused_launch()) {
    main_window_remove();
  } else {
    health_subscribe();
    connection_subscribe();
  }
}


int main(void) {
	init();
	app_event_loop();
	deinit();
}

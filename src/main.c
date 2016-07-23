#include <pebble.h>

#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/wakeup.h"

#include "services/health.h"
#include "services/connection.h"

#include "windows/main_window.h"

static int init_stage;

static void deinit(void) {
  wakeup_set();
}

static void init_callback() {
  if (!init_stage) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Connected to js");
    init_stage = 0;
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Init stage %d", init_stage);
  switch (init_stage) {
    case 0:
      send_latest_steps_to_phone();
      break;
  }
  init_stage++;
}

static void init(void) {
  comm_init(init_callback);
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

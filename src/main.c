#include <pebble.h>

#include "modules/comm.h"
#include "modules/steps.h"
#include "modules/launch.h"

#include "services/health.h"
#include "services/wakeup.h"

#include "windows/main_window.h"

static void init_callback() {
  static int init_stage = 0;
  APP_LOG(APP_LOG_LEVEL_INFO, "Init stage %d", init_stage);
  switch (init_stage) {
    case 0:
      send_latest_steps();
      break;
    case 1:
      send_launch_notification();
      break;
  }
  init_stage++;
}

static void init(void) {
  health_subscribe();
  main_window_push();
  // comm_init(init_callback);
}

static void deinit(void) {
  wakeup_set();
  send_delaunch_notification();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

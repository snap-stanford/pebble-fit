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
      steps_send_latest();
      break;
    case 1:
      launch_send_on_notification();
      break;
  }
  init_stage++;
}

static void init(void) {
  health_subscribe();
  main_window_push();
  comm_init(init_callback);
}

static void deinit(void) {
  wakeup_set();
  launch_send_off_notification();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

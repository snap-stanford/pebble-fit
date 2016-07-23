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

static void init(void) {
  comm_init(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
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

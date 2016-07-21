#include <pebble.h>

#include "modules/comm.h"
#include "modules/steps.h"

#include "windows/main_window.h"

static void init(void) {
  comm_init(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  main_window_push();
  upload_event();
}

static void deinit(void) {}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

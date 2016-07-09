#include <pebble.h>

#include "windows/main_window.h"
#include "modules/health.h"

static void init(void) {
  main_window_push();
  print_steps_in_batch_to_now();
}

static void deinit(void) {}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

#pragma once

#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "../enamel.h"
#include "../modules/steps.h"
#include "../modules/launch.h"
#include "../modules/store.h" // TODO: for debugging purpose only

/* Create the window and push to the window stack. */
Window * wakeup_window_push(bool is_wakeup_launch);

/* Pop window from the window stack. */
void wakeup_window_remove();

void wakeup_window_update(int steps, char *start, char *end, int entries);

void wakeup_window_breathe();

#pragma once

#include <pebble.h>
#include "enamel.h"
#include <pebble-events/pebble-events.h>
//#include "../modules/steps.h"

/* Create the window and push to the window stack. */
void wakeup_window_push();

/* Pop window from the window stack. */
void wakeup_window_remove();

void wakeup_window_update_steps(int steps, char *start, char *end, int entries);

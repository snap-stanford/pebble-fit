#pragma once

#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "../enamel.h"
#include "../modules/steps.h"
#include "../modules/launch.h"
#include "../modules/store.h" // TODO: for debugging purpose only

Window * wakeup_window_push();

void wakeup_window_remove();

void wakeup_window_update(int steps, char *start, char *end, int entries);

void wakeup_window_breathe();

#pragma once

#include <pebble.h>
#include "comm.h"
#include "../windows/wakeup_window.h"

/* Send steps in time frame */
void steps_send_in_between(time_t start, time_t end);

/* Send the latest steps to phone. */
void steps_send_latest();

/* Return the latest step count */
int steps_get_latest_step();

void steps_update_wakeup_window_steps();

#pragma once

#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "comm.h"
#include "../enamel.h"
#include "../constants.h"
#include "../windows/wakeup_window.h"

/* Send steps in time frame */
void steps_send_in_between(time_t start, time_t end);

/* Send updated info to wakeup_window for displaying on the watch. */
void steps_wakeup_window_update();

/* Send the steps from before 15 minutes back to the phone. */
void steps_send_latest();

/* Whether we should alert the user or not. */
bool steps_whether_alert();

/* Update steps count. */
void steps_update();

/* Return the inactive minutes. */
int steps_get_inactive_minutes();

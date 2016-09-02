#pragma once

#include <pebble.h>
#include "comm.h"

/* Send steps in time frame */
void send_steps_in_between(time_t start, time_t end);

/* Send the latest steps to phone. */
void send_latest_steps();
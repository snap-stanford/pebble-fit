#pragma once

#include <pebble.h>
#include "comm.h"

/* Send steps in time frame */
void steps_send_in_between(time_t start, time_t end);

/* Send the latest steps to phone. */
void steps_send_latest();
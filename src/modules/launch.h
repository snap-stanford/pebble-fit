#pragma once

#include <pebble.h>
#include "comm.h"

/* Send launch event to phone. */
void send_launch_notification();

/* Send delaunched event to phone. */
void send_delaunch_notification();
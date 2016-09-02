#pragma once

#include <pebble.h>
#include "comm.h"

/* Send launch event to phone. */
void launch_send_on_notification();

/* Send delaunched event to phone. */
void launch_send_off_notification();
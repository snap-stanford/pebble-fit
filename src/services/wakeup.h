#pragma once

#include <pebble.h>
#include "../enamel.h"
#include "../constants.h"

/* Schedule wakeup event */
void schedule_wakeup_events(int inactive_mins);

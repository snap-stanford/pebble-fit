#pragma once

#include <pebble.h>
#include "../enamel.h"
#include "../constants.h"

void schedule_wakeup_events(int inactive_mins, time_t t_curr);

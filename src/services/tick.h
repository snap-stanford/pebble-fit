#pragma once

#include <pebble.h>
#include "../enamel.h"
#include "../windows/wakeup_window.h"

/* Subscribe to a tick timer service. */
void tick_second_subscribe(bool activate);

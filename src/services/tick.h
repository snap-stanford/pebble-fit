#pragma once

#include <pebble.h>
#include "../constants.h"
#include "../enamel.h"
#include "../modules/launch.h"
#include "../windows/wakeup_window.h"

void tick_reset_count();
void tick_second_subscribe(bool activate);

#pragma once

#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "../constants.h"
#include "../enamel.h"
#include "comm.h"
#include "store.h"
#include "../windows/wakeup_window.h"

void steps_send_in_between(time_t start, time_t end, bool force);

void steps_wakeup_window_update();

bool steps_upload_steps();

void steps_send_latest();

bool steps_get_pass();

void steps_update();

void steps_upload_prior_week();

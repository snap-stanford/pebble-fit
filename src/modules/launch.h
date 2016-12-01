#pragma once

#include <pebble.h>
#include "../constants.h"
#include "comm.h"
#include "windows/main_window.h"
#include "windows/wakeup_window.h"
#include "windows/dialog_window.h"
#include "modules/steps.h"
#include "services/wakeup.h"

extern int delaunch_reason;

/* Send launch event to phone. */
void launch_send_on_notification();

/* Main launch handler. */
void launch_handler(bool activate);

/* Send delaunched event to phone. */
void launch_send_off_notification();

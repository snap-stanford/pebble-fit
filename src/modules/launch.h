#pragma once

#include <pebble.h>
#include "../constants.h"
#include "comm.h"
#include "steps.h"
#include "../services/wakeup.h"

extern time_t e_launch_time;
extern int e_launch_reason;
extern int e_exit_reason;

/* Send launch events to the phone app. */
void launch_send_launch_notification();

/* Send exited event to the phone app. */
void launch_send_exit_notification();

/* Send testing message to the phone app. */
void launch_send_test();

/* TODO .*/
//void launch_resend(time_t time, int reason, bool is_launch);
void launch_resend(time_t launch_time, time_t exit_time, int launch_reason, int exit_reason);

void launch_set_random_message();
const char* launch_get_random_message();

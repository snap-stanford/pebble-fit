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

void launch_resend(time_t t_launch, time_t t_exit, char *msg_id, uint8_t lr, uint8_t er);

void launch_set_random_message();
const char* launch_get_random_message();
const char* launch_get_random_message_id();

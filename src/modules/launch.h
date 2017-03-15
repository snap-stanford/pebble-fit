#pragma once

#include <pebble.h>
#include "../enamel.h"
#include "../constants.h"
#include "../windows/wakeup_window.h"
#include "../windows/dialog_window.h"
#include "../services/wakeup.h"
#include "../services/tick.h"
#include "comm.h"
#include "steps.h"

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

void launch_set_random_message(bool is_notify_wakeup);

const char * launch_get_random_message();
const char * launch_get_random_message_id();

void wakeup_handler(WakeupId wakeup_id, int32_t wakeup_cookie);

void launch_handler(bool activate);

void update_config(void *context);

void init_callback(DictionaryIterator *iter, void *context);

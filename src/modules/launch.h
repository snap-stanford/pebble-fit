#pragma once

#include <pebble.h>
#include "../enamel.h"
#include "../constants.h"
#include "../windows/wakeup_window.h"
#include "../windows/dialog_window.h"
#include "../services/wakeup.h"
#include "../services/tick.h"
#include "store.h"
#include "comm.h"
#include "steps.h"

extern time_t e_launch_time;

extern time_t e_step_upload_time;

extern int e_launch_reason;

extern int e_exit_reason;

extern bool e_waiting_launchexit_ack;

extern bool e_force_to_save;

/* Send launch events to the phone app. */
void launch_send_launch_notification();

/* Send exited event to the phone app. */
void launch_send_exit_notification();

/* Send testing message to the phone app. */
void launch_send_test();

void launch_resend(time_t t_launch, time_t t_exit, char *msg_id, uint8_t sd, uint8_t br, uint8_t lr, uint8_t er);

void launch_set_random_message();

const char * launch_get_random_message();
const char * launch_get_random_message_id();
uint8_t launch_get_score_diff();

void launch_wakeup_handler(WakeupId wakeup_id, int32_t wakeup_cookie);

void launch_wakeup_handler_wrapper(WakeupId wakeup_id, int32_t wakeup_cookie);

void launch_handler(bool activate);

void update_config(void *context);

void init_callback(DictionaryIterator *iter, void *context);

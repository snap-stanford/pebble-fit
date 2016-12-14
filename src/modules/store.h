#pragma once

#include <pebble.h>
#include "../constants.h"
#include "comm.h"
#include "launch.h"


void store_write_launchexit_event(time_t launch_time, time_t exit_time, uint8_t lr, uint8_t er);
bool store_resend_launchexit_event();

void store_write_update_time(time_t time);
bool store_resend_steps();

/* Load persist data associated with key, and send it. */
void store_transfer_data(int key);

/* Append new element into persistant array. */
void store_add_key_data(int key, int element_to_add);

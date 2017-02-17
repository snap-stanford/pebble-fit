#pragma once

#include <pebble.h>
#include "../constants.h"
#include "comm.h"
#include "launch.h"


void store_write_config_time(time_t time);
bool store_resend_config_request(time_t curr_time);

void store_write_launchexit_event(time_t launch_time, time_t exit_time, uint8_t lr, uint8_t er);
bool store_resend_launchexit_event();

void store_write_upload_time(time_t time);
time_t store_read_upload_time();
bool store_resend_steps();

void store_reset_break_count();
void store_increment_break_count();
time_t store_read_break_count_time();
int store_read_break_count();

const char* store_read_random_message();

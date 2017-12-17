#pragma once

#include <pebble.h>
#include <math.h>
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

void store_set_possible_score();
int store_read_possible_score();

void store_reset_curr_score();
void store_increment_curr_score();
time_t store_read_curr_score_time();
int store_read_curr_score();
int store_compare_ref_score(int mode);

const char* store_read_random_message();

void store_weight_update(bool pass);
int store_weight_get_recent_update();
void store_weights_set();

void store_delete_all();

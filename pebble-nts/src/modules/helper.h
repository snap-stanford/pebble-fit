#pragma once
#include <pebble.h>

#include "../constants.h"
#include "../config_all/config_func.h"
#include "comm.h"
#include "../pinteract/pinteract.h"
#include "../pinteract/pinteract_structs.h"
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++ STORAGE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void init_persistent_storage();

void reset_pinteract_persistent_storage();

void reset_config_wakeup_persistent_storage();

void reset_pinteract_states();

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ FORE APP MASTER TICK FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



void fore_app_master_tick_timer_handler(struct tm *tick_time, TimeUnits units_changed);


void fore_app_master_tick_timer_service_clock_subscribe(TimeUnits tick_units, TickHandler handler);

void fore_app_master_tick_timer_service_clock_unsubscribe(TimeUnits tick_units);


void fore_app_master_tick_timer_service_aux_subscribe(TimeUnits tick_units, TickHandler handler);

void fore_app_master_tick_timer_service_aux_unsubscribe(TimeUnits tick_units);


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ CONVIENCE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



ConfigGeneral get_config_general();

PinteractStates get_pinteract_state_all();

void set_pinteract_state(int16_t pi_i, void* state ,int16_t index);

void pinteract_state_roll_over_days_entry(time_t time_entry);

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MAJOR OPERATIONS FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void wakeup_main_response_handler(WakeupId wakeup_id, int32_t wakeup_cookie);

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MATHEMATICS FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int32_t rand_lw_up_bounds(int32_t lw_bnd, int32_t up_bnd);

int16_t pow_int(int16_t x, int16_t y);

uint32_t isqrt(uint32_t x);

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIME FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void write_time_to_array_head(time_t ts, uint8_t * buf);

time_t  read_time_from_array_head(uint8_t * buf);


int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s);

int32_t today_ms(int32_t * out_today_ms);

time_t today_s_to_time_t(int32_t today_s);


int32_t time_t_to_today_s(time_t t);

uint16_t today_s_to_today_m(int32_t today_s);

int32_t today_m_to_today_s(uint16_t today_m);


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ SORTING FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

// >> directly compares values *a and *b
// static int cmpfunc_ascend(const void *a, const void *b);
//
// static int cmpfunc_descend(const void *a, const void *b);
//
// // >> compares the values of an array at sortpt, where, a & b are indicies
// // into that array. Used with an auxillary array ind_d to modify d_ind such that
// // ind_d contains the sorted indicies of the sortpt array
// static int cmpfunc_ascend_index(const void *a, const void *b);
//
// static int cmpfunc_descend_index(const void *a, const void *b);
// // >> takes a
// static void maxminval(int16_t d[], int16_t p, int16_t r, int16_t *max, int16_t *min, int (*compar)(const void*,const void*) );
//
// // >> the swap function for qsort
// static void swapf(int16_t *d, int16_t i, int16_t j);
// // >> basic implementation of
// static void qsortf(int16_t* d, int16_t p, int16_t r, int (*compar)(const void*,const void*));
// // >> sorts the input array d and puts its sorted indicies into ind_d
// static void sort_order_descend(int16_t *d, int16_t *ind_d, int16_t dlen);
//
// static void sort_order_ascend(int16_t *d, int16_t *ind_d, int16_t dlen);

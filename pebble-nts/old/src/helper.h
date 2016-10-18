#pragma once
#include <pebble.h>
#include "constants.h"
#include "transmit.h"
#include "config_all/config_func.h"
#include "pinteract_all/pinteract_func.h"
#include "display_all/display_func.h"


// Constants

static const uint32_t x1000_KG_TO_LBS = 2204;
static const uint32_t x1000_CM_TO_IN = 394;
//
// /* +++++++ GLOBAL VARIABLES +++++*/
// /* FORE APP MASTER TICK VARIABLES */
// static TickHandler tick_timer_clock_minute_handler;
// static TickHandler tick_timer_clock_second_handler;
// static TickHandler tick_timer_aux_minute_handler;
// static TickHandler tick_timer_aux_second_handler;


/* +++++++++++++++ STORAGE FUNCTIONS +++++++++++++++ */

void init_persistent_storage();

void reset_pinteract_persistent_storage();

void reset_daily_acti_persistent_storage();

void reset_config_general_persistent_storage();

void reset_config_wakeup_persistent_storage();

void reset_state_persistent_storage();

void config_wakeup_schedule();

/* +++++++++++++++ FORE APP MASTER TICK FUNCTIONS +++++++++++++++ */

void worker_start_fore_app_reason_exec();

void fore_app_master_tick_timer_handler(struct tm *tick_time, TimeUnits units_changed);

void fore_app_master_tick_timer_service_clock_subscribe(TimeUnits tick_units, TickHandler handler);

void fore_app_master_tick_timer_service_clock_unsubscribe(TimeUnits tick_units);

void fore_app_master_tick_timer_service_aux_subscribe(TimeUnits tick_units, TickHandler handler);

void fore_app_master_tick_timer_service_aux_unsubscribe(TimeUnits tick_units);



/* +++++++++++++++ CONVIENENCE FUNCTIONS +++++++++++++++ */

struct config_general get_config_general();

struct pinteract_state get_pinteract_state();

struct daily_acti get_daily_acti();


void set_pinteract_state(int16_t pi_i, int16_t state);


/* +++++++++++++++ TIMER FUNCTIONS +++++++++++++++ */

void write_time_to_array_head(uint32_t ts, uint8_t * buf);

time_t read_time_from_array_head(uint8_t * buf);

int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s);

int32_t today_ms(int32_t * out_today_ms);

time_t today_s_to_time_t(int32_t today_s);

int32_t time_t_to_today_s(time_t t);

uint16_t today_s_to_today_m(int32_t today_s);

int32_t today_m_to_today_s(uint16_t today_m);

/* +++++++++++++++ MAJOR OPERATIONS FUNCTIONS +++++++++++++++ */

void wakeup_main_response_handler(WakeupId wakeup_id, int32_t cookie);

/* +++++++++++++++ MISC FUNCTIONS +++++++++++++++ */

int32_t rand_lw_up_bounds(int32_t lw_bnd, int32_t up_bnd);

uint16_t kg_to_lbs(uint16_t kg);

uint16_t lbs_to_kg(uint16_t kg);

uint16_t cm_to_in(uint16_t cm);

void cm_to_ft_in_apart(uint16_t cm, uint16_t* ft, uint16_t* in_side);

uint16_t ft_in_apart_to_cm(uint16_t ft, uint16_t in_side);

// take a maximum number of slots, a current slot, a center point, and a radius,
// and return a x,y point that is RELATIVE to it on the circle

GPoint get_point_on_circle_at_ang(uint16_t n_ang, uint16_t cur_ang, GPoint center, uint16_t radius );

GPoint get_point_on_circle_at_ang_CW_noon(uint16_t n_ang, uint16_t cur_ang, GPoint center, uint16_t radius );

// GPoint get_point_on_circle_at_ang_CCW_noon(uint16_t n_ang, uint16_t cur_ang, GPoint center, uint16_t radius );

int16_t pow_int(int16_t x, int16_t y);

uint32_t isqrt(uint32_t x);

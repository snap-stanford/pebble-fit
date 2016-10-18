#pragma once
#include <pebble_worker.h>
#include "constants_worker.h"


int16_t pow_int(int16_t x, int16_t y);


uint32_t isqrt(uint32_t x);

int32_t integral_abs(int16_t *d, int16_t srti, int16_t endi);

int32_t integral_l2(int16_t *d, int16_t srti, int16_t endi);

uint8_t get_angle_i(int16_t x, int16_t y, uint8_t n_ang );

// actigraphy functions

uint8_t orient_encode(int16_t *mean_ary, uint8_t n_ang);

void fft_mag(int16_t *d, int16_t dlenpwr);


// time functions

void write_time_to_array_head(uint32_t ts, uint8_t * buf);

time_t  read_time_from_array_head(uint8_t * buf);

int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s);

int32_t today_ms(int32_t * out_today_ms);

time_t today_s_to_time_t(int32_t today_s);

int32_t time_t_to_today_s(time_t t);

uint16_t today_s_to_today_m(int32_t today_s);

int32_t today_m_to_today_s(uint16_t today_m);

// convienence functions

struct config_general get_config_general();

struct pinteract_state get_pinteract_state();

struct daily_acti get_daily_acti();

int16_t pow_int(int16_t x, int16_t y);

uint32_t isqrt(uint32_t x);

// int16_t pow_int(int16_t x, int16_t y);
//
//
// uint32_t isqrt(uint32_t x);
//
// int32_t integral_abs(int16_t *d, int16_t srti, int16_t endi);
//
// uint8_t get_angle_i(int16_t x, int16_t y, uint8_t n_ang );
//
// // actigraphy functions
//
// uint8_t orient_encode(int16_t *mean_ary, uint8_t n_ang);
//
// void fft_mag(int16_t *d, int16_t dlenpwr);
//
//
// // time functions
//
// void write_time_to_array_head(uint8_t * buf);
//
// time_t  read_time_from_array_head(uint8_t * buf);
//
// int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s);
//
// int32_t today_ms(int32_t * out_today_ms);
//
// time_t today_s_to_time_t(int32_t today_s);
//
// int32_t time_t_to_today_s(time_t t);
//
// uint16_t today_s_to_today_m(int32_t today_s);
//
// int32_t today_m_to_today_s(uint16_t today_m);
//
// // convienence functions
//
// struct config_general get_config_general();

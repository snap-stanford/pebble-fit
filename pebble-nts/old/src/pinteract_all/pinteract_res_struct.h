#pragma once
#include <pebble.h>

#include "constants.h"
#include "helper.h"
/* “patient interactions”, ie: including w/ &w/o associated structs,
will be referred to by “pinteractN” generally, and associated
response structs will be referred to as “pinteractN_res". We will
*/

// unix timestamp, in seconds : unix_ts_s => when interaction finished

// CHECKLIST : NEW STRUCT
// 1. CHANGE "I" OF
//     - pinteract_I_res at struct def
//     - pinteract_code = I;
//     - The initializer declaration
//         - struct pinteract_I_res
//         - init_pinteract_I_res
// 3.

//
struct pinteract_code_unix_timestamp{
  // METADATA
  uint16_t pinteract_code;
  uint32_t unix_ts_s; // unix timestamp

}__attribute__((__packed__));


struct pinteract_priv_scrn_res {
  // CHANGE LOG
  //   created 2015-5-21
  // SUMMARY
  //   mood rating.
  // METADATA
  time_t time_t_srt_priv_scrn; // unix timestamp when the survey ends
  time_t time_t_srt_pi; // unix timestamp when the survey ends
  uint16_t pinteract_code;
} __attribute__((__packed__));


// TO BE DEPRECATED
struct pinteract_Nitem_survey_res {
  // CHANGE LOG
  //   created 2015-5-21
  // SUMMARY
  //   mood rating.
  // data...
  uint8_t survey_res;
  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;

} __attribute__((__packed__));


struct pinteract_11_res {
  // CHANGE LOG
  //   created 2015-5-21
  // SUMMARY
  //   mood rating.
  // data...
  uint8_t current_state_rating;
  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;

} __attribute__((__packed__));




struct pinteract_12_res {
  // CHANGE LOG
  //   created 2015-5-21
  // SUMMARY
  //   sleep rating

  // data ....
  uint8_t sleep_quality; //mood rating, today relative to yesterday
  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;

}__attribute__((__packed__));
// INITIALIZER
struct pinteract_12_res init_pinteract_12_res();


struct pinteract_13_res {
  // CHANGE LOG
  // SUMMARY
  // data ....
  uint8_t well_rating; //mood rating, today relative to yesterday
  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;
}__attribute__((__packed__)) ;
// INITIALIZER
struct pinteract_13_res init_pinteract_13_res();


// this is for the smaller bipolar sleep diary
struct pinteract_14_res {
  // CHANGE LOG
  // SUMMARY
  // data ....
  uint16_t sleep_duration_min;
  uint8_t sleep_quality;

  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;
}__attribute__((__packed__)) ;


// this is for the balsalmer sleep diary
struct pinteract_15_res {
  // CHANGE LOG
  // SUMMARY
  // data ....
  uint16_t sleep_duration_min;
  uint16_t falling_asleep_min;
  uint16_t awake_overnight_min;
  uint8_t sleep_quality;

  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;
}__attribute__((__packed__)) ;


// this is for the balsalmer sleep diary
struct pinteract_16_res {
  // CHANGE LOG
  // SUMMARY
  // data ....
  uint16_t bedtime_min;
  uint16_t wakeup_min;
  uint8_t sleep_quality;

  // METADATA
  time_t time_t_end; // unix timestamp when the survey ends
  uint16_t pinteract_code;
}__attribute__((__packed__)) ;




// struct pinteract4_res {
//   // CHANGE LOG
//   // SUMMARY
//   // METADATA
//   uint16_t pinteract_code;
//   uint32_t unix_ts_s; // unix timestamp

//   // data ....
//   uint8_t sleep_rate; //mood rating, today relative to yesterday

// } __attribute__((__packed__)) ;
// INITIALIZER
// struct pinteract_4_res init_pinteract_4_res();

// struct pinteract5_res {
//   // CHANGE LOG
//   // SUMMARY
//   // METADATA
//   uint16_t pinteract_code;
//   uint32_t unix_ts_s; // unix timestamp

//   // data ....
//   uint8_t sleep_rate; //mood rating, today relative to yesterday

// } __attribute__((__packed__)) ;
// INITIALIZER
// struct pinteract_5_res init_pinteract_5_res();

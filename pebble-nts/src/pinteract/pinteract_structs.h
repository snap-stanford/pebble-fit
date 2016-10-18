
#pragma once
// This file is to contain the struct specifications for each datatype

typedef struct{
  uint16_t pinteract_code; // NOTE: must set this to 11 on initialization
  uint16_t data_size; // NOTE: must set this to sizeof(Pinteract11Data) when initialize
  time_t time_srt_priv_scrn; // when the privacy screen first prompts the user
  time_t time_srt_pi; // when the user enters into the pinteract
  time_t time_end_pi; // when the user ends the pinteract
  uint8_t mood; // the response to the pinteract
}__attribute__((__packed__)) Pinteract11Data;

typedef struct{
  uint16_t pinteract_code; // NOTE: must set this to 11 on initialization
  uint16_t data_size; // NOTE: must set this to sizeof(Pinteract11Data) when initialize
  time_t time_srt_priv_scrn; // when the privacy screen first prompts the user
  time_t time_srt_pi; // when the user enters into the pinteract
  time_t time_end_pi; // when the user ends the pinteract
  uint16_t sleep_duration_min; // the response to the pinteract
  uint8_t sleep_quality; // the response to the pinteract
}__attribute__((__packed__)) Pinteract14Data;


typedef struct{
  uint16_t pinteract_code; // NOTE: must set this to 11 on initialization
  uint16_t data_size; // NOTE: must set this to sizeof(Pinteract11Data) when initialize
  time_t time_srt_priv_scrn; // when the privacy screen first prompts the user
  time_t time_srt_pi; // when the user enters into the pinteract
  time_t time_end_pi; // when the user ends the pinteract
  uint16_t sleep_srt_min; // the response to the pinteract
  uint16_t sleep_end_min; // the response to the pinteract
}__attribute__((__packed__)) Pinteract15Data;



typedef struct{
  int8_t mood_index;
}__attribute__((__packed__)) Pinteract11State;

typedef struct{
  int16_t sleep_duration_min;
  int8_t sleep_quality_index;
}__attribute__((__packed__)) Pinteract14State;

typedef struct{
  Pinteract11State pi_11[NUM_DAYS_HISTORY]; // can make this a uint8_t array for last 10 days
  Pinteract14State pi_14[NUM_DAYS_HISTORY]; // minutes<<3, quality in lower 3 bits
  time_t time_last_entry;
}__attribute__((__packed__)) PinteractStates;

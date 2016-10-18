#pragma once
#include <pebble.h>
#include "constants.h"
// #include "config_schedule.h"
#include "pinteract_res_struct.h"


// declare the structs used by all pinteract functions
struct pinteract_func_ll_el; // the linked list structure for pinteract callback functions

// define the pinteract function pointers as types

typedef uint16_t (*PinteractFunc)(struct pinteract_func_ll_el* pif_ll_el);

typedef struct survey_opt_return (*SurveySelectFunc)(bool exec);

typedef struct enter_num_opt_return (*EnterNumFunc)(int32_t *res);


struct pif_survey_params{
  uint8_t nitems;
  uint8_t init_selection;
  uint8_t survey_type;
  uint8_t row_height;

  bool title_flag;
  bool row_subtitle_flag;

  char title[20]; // can fit 13 lower case letters
  char up_text[20];
  char down_text[20];

  SurveySelectFunc opt0fptr;
  SurveySelectFunc opt1fptr;
  SurveySelectFunc opt2fptr;
  SurveySelectFunc opt3fptr;
  SurveySelectFunc opt4fptr;
  SurveySelectFunc opt5fptr;
  SurveySelectFunc opt6fptr;
  SurveySelectFunc opt7fptr;
  SurveySelectFunc opt8fptr;
  SurveySelectFunc opt9fptr;

};

struct survey_opt_return{
  char title[20];
  char subtitle[20];
};

struct pif_enter_num_params{
  char row_titles[2][20];
  uint8_t n_rows;
  uint8_t n_cols_r0;
  uint8_t n_cols_r1;
  EnterNumFunc opt_fptr_ary[4]; // 4, for 2 rows, 2 col

};

struct enter_num_opt_return{
  char label[8];
  int32_t num_max;
  int32_t num_min;
  int32_t num_init;
  int32_t num_delta;
  uint8_t prev_opt_i;
  uint8_t next_opt_i;
};

// // define the structs used by all pinteract functions
// struct pinteract_func_ll_el{
//   PinteractFunc pif;
//   uint8_t *buf;
//   struct pif_context *ctx;
//   struct pinteract_func_ll_el *next;
// };

// define the structs used by all pinteract functions
struct pinteract_func_ll_el{
  PinteractFunc pif;
  uint8_t *buf;
  void *ctx; // better make sure you know what you're doing
  struct pinteract_func_ll_el *next;
};

// hepler functions
uint16_t write_res_buf_byte_count(uint8_t* res_buf, uint16_t srt_ind, uint16_t byte_count);

uint16_t read_res_buf_byte_count(uint8_t* res_buf, uint16_t byte_cnt_ind);

uint16_t write_data_to_res_buf(uint8_t* res_buf, uint8_t* data, uint16_t data_dl);

uint16_t pinteract_write_res_buf_to_pesistant_storage(struct pinteract_func_ll_el* pif_ll_el );

// non-chained pinteracts

void pinteract_demo();


// direct interactions

uint16_t pinteract_Nitem_survey(struct pif_survey_params* pif_srvy_prm );

// uint16_t pinteract_Nitem_survey_July2(struct pinteract_func_ll_el* pif_ll_el );

// uint16_t pinteract_Nitem_survey_June28(struct pinteract_func_ll_el* pif_ll_el );

uint16_t pinteract_enter_num(struct pif_enter_num_params* pif_enter_num_prm );



// pinteracts define behaviour

uint16_t pinteract_alert(struct pinteract_func_ll_el* pif_ll_el);

uint16_t pinteract_priv_scrn(struct pinteract_func_ll_el* pif_ll_el );

uint16_t pinteract_11(struct pinteract_func_ll_el* pif_ll_el );

uint16_t pinteract_12(struct pinteract_func_ll_el* pif_ll_el );

uint16_t pinteract_13(struct pinteract_func_ll_el* pif_ll_el );

uint16_t pinteract_14(struct pinteract_func_ll_el* pif_ll_el );

uint16_t pinteract_15(struct pinteract_func_ll_el* pif_ll_el );




// interaction driver

// interaction driver
//  This is the only function that REALLY needs to be public. It is very small
// just very simple functionality at this point that might expand if needed
void pinteract_driver(uint16_t pinteract_code );


// CONVIENENCE
// the switch function that takes the desired pinteract
// code and returns with a PinteractCodeFunc pointer to that function
// this could be a public function, because other places might like such
// a use for debugging or quick and dirty displays
PinteractFunc pinteract_code_to_func_ptr(int16_t pinteract_code);

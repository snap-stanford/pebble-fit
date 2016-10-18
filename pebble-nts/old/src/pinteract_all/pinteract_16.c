#include "pinteract_func.h"


static struct pinteract_func_ll_el* pif_ll_el_lcl;

static struct pinteract_16_res pi_s_res = {
  .pinteract_code = 16
};

static void finish_this_interact(){
  // DO NOTHING
  // write the data to the buffer
  uint8_t *res_buf = pif_ll_el_lcl->buf;
  pi_s_res.time_t_end = time(NULL);
  pi_s_res.pinteract_code = 14;

  write_data_to_res_buf(res_buf, (uint8_t*) &pi_s_res, sizeof(pi_s_res));
  // call the next function
  (pif_ll_el_lcl->pif)(pif_ll_el_lcl->next);
}



static struct survey_opt_return opt0(bool exec){
  if(exec){
    pi_s_res.sleep_quality = 5;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "5",
  };
  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
    pi_s_res.sleep_quality = 4;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "4",
  };
  return out;
}

static struct survey_opt_return opt2(bool exec){
  if(exec){
    pi_s_res.sleep_quality = 3;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "3",
  };
  return out;
}

static struct survey_opt_return opt3(bool exec){
  if(exec){
    pi_s_res.sleep_quality = 2;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "2",
  };
  return out;
}

static struct survey_opt_return opt4(bool exec){
  if(exec){
    pi_s_res.sleep_quality = 1;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "1",
  };
  return out;
}

/* <<<<<<<<<< BEDTIME >>>>>>>>>*/

// create the struct that will define the question and answers
// of the survery we are about to call
static struct pif_survey_params survey_params = {
  .nitems = 5,
  .init_selection = 2,
  .survey_type = 1,
  .title_flag = true,
  .row_subtitle_flag = false,
  .title = "Sleep \n Quality?",
  .up_text = "Very Good  ",
  .down_text = "Very Poor  ",
  .opt0fptr = opt0,
  .opt1fptr = opt1,
  .opt2fptr = opt2,
  .opt3fptr = opt3,
  .opt4fptr = opt4
};

static struct enter_num_opt_return enter_num_opt0_wakeup(int32_t *res){
  struct enter_num_opt_return result = {
    .num_max = 24,
    .num_min = 0,
    .num_init = 8,
    .num_delta = 1,
    .prev_opt_i = 0,
    .next_opt_i = 1,
    .label = "hrs"
  };
  return result;
}

static struct enter_num_opt_return enter_num_opt1_wakeup(int32_t *res){
  if(res != NULL){
    pi_s_res.wakeup_min = res[0]*60 + res[1];
    pinteract_Nitem_survey(&survey_params);
  }

  struct enter_num_opt_return result = {
    .num_max = 60,
    .num_min = 0,
    .num_init = 0,
    .num_delta = 10,
    .prev_opt_i = 0,
    .next_opt_i = 4,
    .label = "min"
  };
  return result;
}


static struct pif_enter_num_params enter_num_params_wakeup = {
  .n_rows = 1,
  .n_cols_r0 = 2
};


/* <<<<<<<<<< BEDTIME >>>>>>>>>*/


static struct enter_num_opt_return enter_num_opt0_bedtime(int32_t *res){
  struct enter_num_opt_return result = {
    .num_max = 24,
    .num_min = 0,
    .num_init = 8,
    .num_delta = 1,
    .prev_opt_i = 0,
    .next_opt_i = 1,
    .label = "hrs"
  };
  return result;
}

static struct enter_num_opt_return enter_num_opt1_bedtime(int32_t *res){
  if(res != NULL){
    pi_s_res.bedtime_min = res[0]*60 + res[1];
    strcpy(enter_num_params_wakeup.row_titles[0], "Wakeup");
    strcpy(enter_num_params_wakeup.row_titles[1], "_");
    enter_num_params_wakeup.opt_fptr_ary[0] = enter_num_opt0_wakeup;
    enter_num_params_wakeup.opt_fptr_ary[1] = enter_num_opt1_wakeup;
    enter_num_params_wakeup.opt_fptr_ary[2] = enter_num_opt1_wakeup;
    enter_num_params_wakeup.opt_fptr_ary[3] = enter_num_opt1_wakeup;

    // THIS is where we determine which survey function we are going into
    pinteract_enter_num(&enter_num_params_wakeup);
  }

  struct enter_num_opt_return result = {
    .num_max = 60,
    .num_min = 0,
    .num_init = 0,
    .num_delta = 10,
    .prev_opt_i = 0,
    .next_opt_i = 4,
    .label = "min"
  };
  return result;
}


static struct pif_enter_num_params enter_num_params_bedtime = {
  .n_rows = 1,
  .n_cols_r0 = 2
};


uint16_t pinteract_16(struct pinteract_func_ll_el* pif_ll_el ){
  pif_ll_el_lcl = pif_ll_el;

  strcpy(enter_num_params_bedtime.row_titles[0], "Bedtime");
  strcpy(enter_num_params_bedtime.row_titles[1], "_");
  enter_num_params_bedtime.opt_fptr_ary[0] = enter_num_opt0_bedtime;
  enter_num_params_bedtime.opt_fptr_ary[1] = enter_num_opt1_bedtime;
  enter_num_params_bedtime.opt_fptr_ary[2] = enter_num_opt1_bedtime;
  enter_num_params_bedtime.opt_fptr_ary[3] = enter_num_opt1_bedtime;

  // THIS is where we determine which survey function we are going into
  pinteract_enter_num(&enter_num_params_bedtime);

  return 16;
}

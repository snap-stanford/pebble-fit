#include "pinteract_func.h"


// create the struct that will define the question and answers
// of the survery we are about to call



static struct pinteract_func_ll_el* pif_ll_el_lcl;
static struct pinteract_13_res pi_s_res = {
  .pinteract_code = 13
};


static void finish_this_interact(){

  // write the mood rating to the buffer
  uint8_t * res_buf = pif_ll_el_lcl->buf;
  pi_s_res.time_t_end = time(NULL);

  write_data_to_res_buf(res_buf, (uint8_t*) &pi_s_res, sizeof(pi_s_res));

  // call the next function
  (pif_ll_el_lcl->pif)(pif_ll_el_lcl->next);

}


static struct survey_opt_return opt0(bool exec){
  if(exec){
    pi_s_res.well_rating = 0;
    finish_this_interact();

  }
  struct survey_opt_return out = {
    .title = "Very Well",
  };
  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
    pi_s_res.well_rating = 1;
    finish_this_interact();

  }
  struct survey_opt_return out = {
    .title = "Well",
  };
  return out;
}

static struct survey_opt_return opt2(bool exec){
  if(exec){
    pi_s_res.well_rating = 2;
    finish_this_interact();

  }
  struct survey_opt_return out = {
    .title = "Less Well",
  };
  return out;
}


// create the struct that will define the question and answers
// of the survery we are about to call

static struct pif_survey_params survey_params = {
  .nitems = 3,
  .init_selection = 1,
  .survey_type = 0,
  .title_flag = true,
  .row_subtitle_flag = false,
  .title = "You feel?",
  .opt0fptr = opt0,
  .opt1fptr = opt1,
  .opt2fptr = opt2,
};


uint16_t pinteract_13(struct pinteract_func_ll_el* pif_ll_el ){
  pif_ll_el_lcl = pif_ll_el;

  // THIS is where we determine which survey function we are going into
  pinteract_Nitem_survey(&survey_params);


  return 13;
}


// static void draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context){

//   switch(cell_index->row){
//     case 0 :
//       menu_cell_basic_draw(ctx, cell_layer, "Well",NULL,NULL );
//       break;
//     case 1 :
//       menu_cell_basic_draw(ctx, cell_layer, "Normal",NULL,NULL );
//       break;
//     case 2 :
//       menu_cell_basic_draw(ctx, cell_layer, "Not Well",NULL,NULL );
//       break;
//   }

// }

// static struct pif_context_survey survey_params = {
//   .pinteract_code = 13,
//   .nitems = 3,
//   .init_selection = 1,
//   .survey_type = 0,
//   .question = "You feel?",
//   .fptr = draw_row_cb
// };

// static struct pinteract_func_ll_el survey_struct;


// uint16_t pinteract_13(struct pinteract_func_ll_el* pif_ll_el ){
//   // add the new struct intor the daisy chain
//   survey_struct.pif = pif_ll_el->pif;
//   survey_struct.buf = pif_ll_el->buf;
//   survey_struct.ctx = &survey_params;
//   survey_struct.next = pif_ll_el->next;

//   // THIS is where we determine which survey function we are going into
//   pinteract_Nitem_survey(&survey_struct);


//   return 13;
// }

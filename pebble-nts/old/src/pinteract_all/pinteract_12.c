#include "pinteract_func.h"



static struct pinteract_func_ll_el* pif_ll_el_lcl;
static struct pinteract_12_res pi_s_res = {
  .pinteract_code = 12
};



static void finish_this_interact(){

  // write the mood rating to the buffer
  uint8_t * res_buf = pif_ll_el_lcl->buf;
  pi_s_res.time_t_end = time(NULL);
  pi_s_res.pinteract_code = 12;

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


uint16_t pinteract_12(struct pinteract_func_ll_el* pif_ll_el ){
  pif_ll_el_lcl = pif_ll_el;

  // THIS is where we determine which survey function we are going into
  pinteract_Nitem_survey(&survey_params);


  return 12;
}


// create the struct that will define the question and answers
// of the survery we are about to call


// static void draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context){

//   switch(cell_index->row){
//     case 0 :
//       menu_cell_title_draw(ctx, cell_layer, "5");
//       break;
//     case 1 :
//       menu_cell_title_draw(ctx, cell_layer, "4");
//       break;
//     case 2 :
//       menu_cell_title_draw(ctx, cell_layer, "3");
//       break;
//     case 3 :
//       menu_cell_title_draw(ctx, cell_layer,"2");
//       break;
//     case 4 :
//       menu_cell_title_draw(ctx, cell_layer, "1");
//       break;
//   }
// }

// static struct pif_context_survey survey_params = {
//   .pinteract_code = 12,
//   .nitems = 5,
//   .init_selection = 2,
//   .survey_type = 1,
//   .question = "Sleep \nwas? ",
//   .up_option = "RESTFUL > ",
//   .down_option = "RESTLESS > ",
//   .fptr = draw_row_cb
// };

// static struct pinteract_func_ll_el survey_struct;


// uint16_t pinteract_12(struct pinteract_func_ll_el* pif_ll_el ){
//   // add the new struct intor the daisy chain
//   survey_struct.pif = pif_ll_el->pif;
//   survey_struct.buf = pif_ll_el->buf;
//   survey_struct.ctx = &survey_params;
//   survey_struct.next = pif_ll_el->next;

//   // THIS is where we determine which survey function we are going into
//   pinteract_Nitem_survey(&survey_struct);


//   return 12;
// }

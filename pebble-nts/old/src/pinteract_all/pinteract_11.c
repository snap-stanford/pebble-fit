#include "pinteract_func.h"



static struct pinteract_func_ll_el* pif_ll_el_lcl;
static struct pinteract_11_res pi_s_res = {
    .pinteract_code = 11
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
    set_pinteract_state(11, 0);
    pi_s_res.current_state_rating = 5;
    finish_this_interact();

  }
  struct survey_opt_return out = {
    .title = "Very High",
  };

  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
    set_pinteract_state(11, 1);
    pi_s_res.current_state_rating = 4;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "High",
  };
  return out;
}

static struct survey_opt_return opt2(bool exec){
  if(exec){
    set_pinteract_state(11, 2);
    pi_s_res.current_state_rating = 3;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "Normal",
  };
  return out;
}

static struct survey_opt_return opt3(bool exec){
  if(exec){
    set_pinteract_state(11, 3);
    pi_s_res.current_state_rating = 2;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "Low",
  };
  return out;
}

static struct survey_opt_return opt4(bool exec){
  if(exec){
    set_pinteract_state(11, 4);
    pi_s_res.current_state_rating = 1;
    finish_this_interact();
  }
  struct survey_opt_return out = {
    .title = "Very Low",
  };
  return out;
}


// create the struct that will define the question and answers
// of the survery we are about to call

static struct pif_survey_params survey_params = {
  .nitems = 5,
  .survey_type = 0,
  .title_flag = true,
  .row_subtitle_flag = false,
  .title = "Mood Today?",
  .opt0fptr = opt0,
  .opt1fptr = opt1,
  .opt2fptr = opt2,
  .opt3fptr = opt3,
  .opt4fptr = opt4
};


uint16_t pinteract_11(struct pinteract_func_ll_el* pif_ll_el ){
  pif_ll_el_lcl = pif_ll_el;

  survey_params.init_selection = (get_pinteract_state().pi_11[1]>=0) ?
    get_pinteract_state().pi_11[1] : 2;

  // THIS is where we determine which survey function we are going into
  pinteract_Nitem_survey(&survey_params);

  return 11;
}



// // create the struct that will define the question and answers
// // of the survery we are about to call
// // go ahead an initalize these structures to const, because IF we
// // want to write to persistant storage (which we can NOT do if we
// // change the .next ) member when we are assigning values to the
// // sequence start element, we have to always terminate it like this.

// // NOTE!! the callback MUST be defined BEFORE assigning it to the
// // function pointer in the pif_context struct
// static void draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context){

//   switch(cell_index->row){
//     case 0 :
//       menu_cell_basic_draw(ctx, cell_layer, "Very High",NULL,NULL );
//       break;
//     case 1 :
//       menu_cell_basic_draw(ctx, cell_layer, "High",NULL,NULL );
//       break;
//     case 2 :
//       menu_cell_basic_draw(ctx, cell_layer, "Normal",NULL,NULL );
//       break;
//     case 3 :
//       menu_cell_basic_draw(ctx, cell_layer,"Low",NULL,NULL );
//       break;
//     case 4 :
//       menu_cell_basic_draw(ctx, cell_layer, "Very Low",NULL,NULL );
//       break;
//   }
// }

// static struct pif_context_survey survey_params = {
//   .pinteract_code = 11,
//   .nitems = 5,
//   .init_selection = 2,
//   .survey_type = 0,
//   .question = "Current State?",
//   .fptr = draw_row_cb
// };

// static struct pinteract_func_ll_el survey_struct;


// uint16_t pinteract_11(struct pinteract_func_ll_el* pif_ll_el ){
//   // add the new struct intor the daisy chain
//   survey_struct.pif = pif_ll_el->pif;
//   survey_struct.buf = pif_ll_el->buf;
//   survey_struct.ctx = &survey_params;
//   survey_struct.next = pif_ll_el->next;

//   // THIS is where we determine which survey function we are going into
//   pinteract_Nitem_survey(&survey_struct);


//   return 11;
// }

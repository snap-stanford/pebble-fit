// NOTE!!!! This file is for testing interactions and functionality ONLY
// It is NEVER meant to be included in the actual build

#include "demo_screens.h"


static struct survey_opt_return opt0(bool exec){
  if(exec){
    pinteract_driver(11);

  }
  struct survey_opt_return out = {
    .title = "Mood Today?",
  };
  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
    pinteract_driver(14);

  }
  struct survey_opt_return out = {
    .title = "Sleep quality",
  };
  return out;
}


static struct survey_opt_return opt2(bool exec){
  if(exec){

    display_main_dash();
  }
  struct survey_opt_return out ={
    .title = "Display Dash",
  };
  return out;
}

static struct survey_opt_return opt3(bool exec){
  if(exec){
    init_transmit_to_phone(TR_PUSH_ALL_DATA_TO_SERVER);

  }
  struct survey_opt_return out = {
    .title = "Transmit Server",
  };
  return out;
}

static struct survey_opt_return opt4(bool exec){
  if(exec){
    display_reminder(RR_MEMORY_LOW);

  }
  struct survey_opt_return out = {
    .title = "Memory Low",
  };
  return out;
}





// create the struct that will define the question and answers
// of the survery we are about to call

static struct pif_survey_params survey_params = {
  .nitems = 5,
  .init_selection = 0,
  .survey_type = 0,
  .title_flag= true,
  .row_subtitle_flag = false,
  .title = "Demo",
  .opt0fptr = opt0,
  .opt1fptr = opt1,
  .opt2fptr = opt2,
  .opt3fptr = opt3,
  .opt4fptr = opt4
};


void demo_screens_open(){
  // THIS is where we determine which survey function we are going into
  pinteract_Nitem_survey(&survey_params);


}

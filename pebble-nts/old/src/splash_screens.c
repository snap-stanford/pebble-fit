// NOTE!!!! This file is for testing interactions and functionality ONLY
// It is NEVER meant to be included in the actual build

#include "splash_screens.h"


static struct survey_opt_return opt0(bool exec){
  if(exec){
    pinteract_driver(11);

  }
  struct survey_opt_return out = {
    .title = "Current State?",
  };
  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
//     pinteract_driver(12);
    display_main_dash();

  }
//   return "Sleep Quality?";
  struct survey_opt_return out ={
    .title = "Display Dash",
  };
  return out;
}

static struct survey_opt_return opt2(bool exec){
  if(exec){
//     pinteract_driver(13);
    pinteract_driver(14);

  }
//   return "You feel?";
  struct survey_opt_return out = {
    .title = "Sleep quality",
  };
  return out;
}

static struct survey_opt_return opt3(bool exec){
  if(exec){
    //     init_transmit_all_to_server();

//     init_transmit_all_to_server();
//     init_transmit();
    init_transmit_to_phone(TR_PUSH_ALL_DATA_TO_SERVER);

  }
  struct survey_opt_return out = {
    .title = "Transmit Server",
  };
  return out;
}

static struct survey_opt_return opt4(bool exec){
  if(exec){
//     init_transmit_all_to_server();
//     reset_pinteract_persistent_storage();
        // display_BT_connect_help();
        // display_reminder(RR_MEMORY_LOW);
    persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,1);

  }
  struct survey_opt_return out = {
    .title = "wakeup config 1",
  };
  return out;
}

static struct survey_opt_return opt5(bool exec){
  if(exec){
    init_transmit_to_phone(TR_PUSH_ALL_DATA_TO_PHONE);
  }
  struct survey_opt_return out =  {
    .title = "Transmit Phone",
  };
  return out;
}

static struct survey_opt_return opt6(bool exec){
  if(exec){
//     init_transmit_all_to_server();
    init_persistent_storage();
  }
  struct survey_opt_return out =  {
    .title = "Reset ALL Ps",
  };
  return out;
}

static struct survey_opt_return opt7(bool exec){
  if(exec){
//     init_transmit_all_to_server();
    config_settings_menu();
  }
  struct survey_opt_return out =  {
    .title = "Settings Menu",
  };
  return out;
}

static struct survey_opt_return opt8(bool exec){
  if(exec){
//     init_transmit_all_to_server();
    splash_screens_open();
  }
  struct survey_opt_return out =  {
    .title = "Splash screen",
  };
  return out;
}




// create the struct that will define the question and answers
// of the survery we are about to call

static struct pif_survey_params survey_params = {
  .nitems = 9,
  .init_selection = 1,
  .survey_type = 0,
  .title_flag= true,
  .row_subtitle_flag = false,
  .title = "P Interacts",
  .opt0fptr = opt0,
  .opt1fptr = opt1,
  .opt2fptr = opt2,
  .opt3fptr = opt3,
  .opt4fptr = opt4,
  .opt5fptr = opt5,
  .opt6fptr = opt6,
  .opt7fptr = opt7,
  .opt8fptr = opt8,
};


void splash_screens_open(){
  // THIS is where we determine which survey function we are going into
  pinteract_Nitem_survey(&survey_params);


}

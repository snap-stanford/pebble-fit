
// #include "demo_screens.h"
#include "display_func.h"


static struct survey_opt_return opt0(bool exec){
  if(exec){
    config_settings_menu();

  }
  struct survey_opt_return out = {
    .title = "Configuration",
  };
  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
    display_survey_list();

  }
  struct survey_opt_return out = {
    .title = "Surveys",
  };
  return out;
}




// create the struct that will define the question and answers
// of the survery we are about to call

static struct pif_survey_params survey_params = {
  .nitems = 2,
  .init_selection = 0,
  .survey_type = 0,
  .title_flag= true,
  .row_subtitle_flag = false,
  .title = "Settings",
  .opt0fptr = opt0,
  .opt1fptr = opt1
};


void display_detail_dash(){
  // THIS is where we determine which survey function we are going into
  pinteract_Nitem_survey(&survey_params);


}

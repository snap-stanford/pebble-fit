
#include "config_func.h"

// this shows what we can change and the current values
// as title and subtext in a simple menulayer

// we need a settings menu before we push because we
// need to be able to adjust BOTH the weight/height and the step
// goals, which are fundamental to the patient interaction and
// getting them to use it. So, Ketter needs to be able to use it
// as well.

// so, the design for this setting menu is simple. We don't even
// really need a title, And conceivably we could even just use the
// current survery, just tweaked to allow for a few more parameters
// I mean, we have already defined the callbacks, so we only really
// need to adjust
// -> title, yes/no, if no, dont push and reduce title text to nothing
// -> use a struct to pass around the title and subtitle text in the
// optN return values. If dont wan want subtitle, then just pass a
// NULL in the value for the subtitle

static struct survey_opt_return opt0(bool exec){
  if(exec){

    config_personal_params();
  }

  struct survey_opt_return out = {
    .title = "Height & Weight",
  };
  struct config_general cg = get_config_general();
  uint16_t ft, in_side;
  cm_to_ft_in_apart(cg.pheight_cm, &ft, &in_side);

  snprintf(out.subtitle,sizeof(out.subtitle),"%d ft% d in : %d lbs",
    ft, in_side, (kg_to_lbs(cg.pweight_kg)/5 + 1)*5);

  return out;
}

static struct survey_opt_return opt1(bool exec){
  if(exec){
    config_personal_goals();
  }

   struct survey_opt_return out = {
    .title = "Steps Goal",
  };

  struct config_general cg = get_config_general();
  snprintf(out.subtitle,sizeof(out.subtitle),"%d pts", (int) cg.pts_goal);
  return out;
}



static struct pif_survey_params survey_params = {
  .nitems = 2,
  .init_selection = 2,
  .survey_type = 0,
  .row_height = 40,
  .title_flag = true,
  .row_subtitle_flag = true,
  .title = "Settings",
  .opt0fptr = opt0,
  .opt1fptr = opt1
};


// one row for setting goals, one row for setting weight/height

void config_settings_menu(){
  pinteract_Nitem_survey(&survey_params);

}

#include "config_func.h"

// this allows the user to enter their daily step count goal


static struct enter_num_opt_return enter_num_opt0(int32_t *res){
  if(res != NULL){
    struct config_general cg = get_config_general();
    cg.pts_goal = res[0];
    persist_write_data(CONFIG_GENERAL_PERSIST_KEY,&cg,sizeof(cg));
  }

  struct config_general cg = get_config_general();

  struct enter_num_opt_return result = {
    .num_max = 50000,  // 4hz for 24 hours
    .num_min = 1000,
    .num_init = cg.pts_goal,
    .num_delta = 500,
    .prev_opt_i = 0,
    .next_opt_i = 4,
    .label = "steps"
  };
  return result;
}

static struct pif_enter_num_params enter_num_params = {
  .n_rows = 1,
  .n_cols_r0 = 1
};

void config_personal_goals(){
  strcpy(enter_num_params.row_titles[0], "Daily Goal");
  strcpy(enter_num_params.row_titles[1], "Steps");
  enter_num_params.opt_fptr_ary[0] = enter_num_opt0;
  enter_num_params.opt_fptr_ary[1] = enter_num_opt0;
  enter_num_params.opt_fptr_ary[2] = enter_num_opt0;
  enter_num_params.opt_fptr_ary[3] = enter_num_opt0;

  pinteract_enter_num(&enter_num_params);
}

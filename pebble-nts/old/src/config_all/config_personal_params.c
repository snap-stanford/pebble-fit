#include "config_func.h"

// this file allows the user to enter their height and weight for
// better calculation of kcal


static struct enter_num_opt_return enter_num_opt0(int32_t *res){
  struct config_general cg = get_config_general();
  uint16_t ft, in_side;
  cm_to_ft_in_apart(cg.pheight_cm, &ft, &in_side);

  struct enter_num_opt_return result = {
    .num_max = 8,
    .num_min = 1,
    .num_init = ft,
    .num_delta = 1,
    .prev_opt_i = 0,
    .next_opt_i = 1,
    .label = "ft"
  };
  return result;
}

static struct enter_num_opt_return enter_num_opt1(int32_t *res){
  struct config_general cg = get_config_general();
  uint16_t ft, in_side;
  cm_to_ft_in_apart(cg.pheight_cm, &ft, &in_side);

  struct enter_num_opt_return result = {
    .num_max = 11,
    .num_min = 0,
    .num_init = in_side,
    .num_delta = 1,
    .prev_opt_i = 0,
    .next_opt_i = 2,
    .label = "in"
  };
  return result;
}

static struct enter_num_opt_return enter_num_opt2(int32_t *res){

  if(res != NULL){
    struct config_general cg = get_config_general();
    cg.pheight_cm = ft_in_apart_to_cm(res[0],res[1]);
    cg.pweight_kg = lbs_to_kg(res[2]);
    persist_write_data(CONFIG_GENERAL_PERSIST_KEY,&cg,sizeof(cg));
  }

  struct config_general cg = get_config_general();


  struct enter_num_opt_return result = {
    .num_max = 550,
    .num_min = 50,
    .num_init = (kg_to_lbs(cg.pweight_kg)/5 + 1)*5,
    .num_delta = 5,
    .prev_opt_i = 1,
    .next_opt_i = 4,
    .label = "lbs"
  };
  return result;
}


static struct pif_enter_num_params enter_num_params = {
  .n_rows = 2,
  .n_cols_r0 = 2,
  .n_cols_r1 = 1
};

void config_personal_params(){
  strcpy(enter_num_params.row_titles[0], "Enter Height");
  strcpy(enter_num_params.row_titles[1], "Enter Weight");
  enter_num_params.opt_fptr_ary[0] = enter_num_opt0;
  enter_num_params.opt_fptr_ary[1] = enter_num_opt1;
  enter_num_params.opt_fptr_ary[2] = enter_num_opt2;
  enter_num_params.opt_fptr_ary[3] = enter_num_opt2;

  pinteract_enter_num(&enter_num_params);
}

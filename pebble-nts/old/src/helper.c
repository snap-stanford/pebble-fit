#include "helper.h"

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++ STORAGE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void init_persistent_storage(){
  // struct config_general cg = get_config_general();

  // WE reset IF
  // 1. the app has not been installed before, and the config keys does not exist yet.
  // 2. the pk_version is less than a given
  // NOTE,
  // THIS IS FOR FIRST TIME SETUP
  // if(!persist_exists(CONFIG_GENERAL_PERSIST_KEY)){
    // at the start of each of these, we delete all the persist keys associated
    reset_pinteract_persistent_storage();
    reset_daily_acti_persistent_storage();

    reset_config_general_persistent_storage(); // note, pk_version == CUR_PK_VERSION here
    reset_config_wakeup_persistent_storage();
    reset_state_persistent_storage();
  // }else{
    // DO VERSIONING HERE
    // SINCE this is the first time we are doing this, we reset ALL KEYS
    // struct config_general cg = get_config_general();
    // cg.pk_version = CUR_PK_VERSION;
    // persist_write_data(CONFIG_GENERAL_PERSIST_KEY, &cg, sizeof(cg));
  // }
}

void reset_pinteract_persistent_storage(){
  if(persist_exists(PIRPS_B1_PERSIST_KEY)){
    persist_delete(PIRPS_B1_PERSIST_KEY);
  }

  uint8_t ps_buf[MAX_PINTERACT_PS_B_SIZE];
  for(int16_t i = 0; i < MAX_PINTERACT_PS_B_SIZE; i++){ ps_buf[i] = 0; }

  write_time_to_array_head(time(NULL),ps_buf);
  write_res_buf_byte_count(ps_buf, PINTERACT_PS_B_COUNT_IND,
                           PINTERACT_PS_HEAD_B_SIZE);
  persist_write_data(PIRPS_B1_PERSIST_KEY, ps_buf,sizeof(ps_buf));
  // logging
  // uint8_t *tmp = ps_buf;
  // APP_LOG(APP_LOG_LEVEL_ERROR, "reset pinteract ps: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
  //         tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],
  //         tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15],tmp[16],tmp[17]);
}


void reset_daily_acti_persistent_storage(){
  if(persist_exists(DAILY_SUMMARY_WEEKS_PERSIST_KEY)){
    persist_delete(DAILY_SUMMARY_WEEKS_PERSIST_KEY);
  }
  if(persist_exists(DAILY_ACTI_PERSIST_KEY)){
    persist_delete(DAILY_ACTI_PERSIST_KEY);
  }
  // reset the contiuous actigraphy counter, the rest okay
  persist_write_int(I_BLK_PERSIST_KEY, 0);
}


void reset_config_general_persistent_storage(){
  if(persist_exists(CONFIG_GENERAL_PERSIST_KEY)){
    persist_delete(CONFIG_GENERAL_PERSIST_KEY);
  }

  //   CONFIG_GENERAL_PERSIST_KEY
  struct config_general init_cg = {0};
  init_cg.pheight_cm = 175; // for setting up 175cm standard
  init_cg.pweight_kg = 70;
  // NOTE: for temporary solution, hard code the limits as being the lower And
  // upper standard deviations of the stepping score. also, evaluate this
  // on the L-1/2 norm as well.

  // FOR NON-COSINE WINDOW
  init_cg.stepc_fft_thres0 = 22; //@L1,+-1hz:16  @  35, 38 (42) 24percent*100 of energy in 0.3-4hz band
  init_cg.stepc_fft_thres1 = 18; //@L1,+-1hz:16  @  35, 38 (42) 24percent*100 of energy in 0.3-4hz band
  init_cg.stepc_fft_thres2 = 22; //@L1,+-1hz:16  @  35, 38 (42) 24percent*100 of energy in 0.3-4hz band


  init_cg.stepc_vmc_thres0 = 170; //50// 170 @ 185 vmc scaling, 130 @ 140 vmc scaling;
  init_cg.stepc_vmc_thres1 = 1000; // 170 @ 185 vmc scaling, 130 @ 140 vmc scaling;
  init_cg.stepc_vmc_thres2 = 2000; // 170 @ 185 vmc scaling, 130 @ 140 vmc scaling;

  init_cg.wear_class_thres = 290*12*10; // have this set for the 10 minute resolution
  init_cg.pts_goal = 10000; // assume kcal for now

  persist_write_data(CONFIG_GENERAL_PERSIST_KEY,&init_cg, sizeof(init_cg));
  // TEMPORARY
  struct pinteract_state pis = get_pinteract_state();
  struct daily_acti da = get_daily_acti();

  for(int16_t i = 0; i<(NUM_DAYS_HISTORY);i++){
    pis.pi_11[i] = -1; //i%5; // -1
    pis.pi_140[i] = -1; // 360 + (i*30); // -1
    pis.pi_141[i] = -1; // i%5; // -1
    da.steps[i] = 0; // 90 + (i*10);
    da.kcal[i] = 0; //31 + (i*4)%5;
  }
  persist_write_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
  persist_write_data(DAILY_ACTI_PERSIST_KEY, &da, sizeof(da));
}


void reset_config_wakeup_persistent_storage(){
  if(persist_exists(CONFIG_WAKEUP_IDS_PERSIST_KEY)){
    persist_delete(CONFIG_WAKEUP_IDS_PERSIST_KEY);
  }

  // write the current day of the month
  time_t cur_time = time(NULL);
  struct tm *cur_time_tm =  localtime(&cur_time);
  persist_write_int(CUR_MDAY_PERSIST_KEY,cur_time_tm->tm_mday);

  struct wakeup_config cs_ary[2];

  // SUPER ROUGH, but good enough for user testing
  cs_ary[0].pinteract_code = 14;
  cs_ary[0].srt = 540;
  cs_ary[0].end = 600;
  cs_ary[1].pinteract_code = 11;
  cs_ary[1].srt = 1080;
  cs_ary[1].end = 1140;

  reset_config_wakeup_schedule();
  write_to_config_wakeup_persistant_storage(cs_ary, 2);
  // call config_wakeup_schedule
  config_wakeup_schedule();

  // DEBUGGING
  // WakeupId wakeup_id = read_wakeup_id_at_config_wakeup_index(3);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "config wakeup i=0, wakeup_id %d ", (int) wakeup_id);
}

void reset_state_persistent_storage(){
  // remove old keys
  if(persist_exists(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY)){
    persist_delete(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY);
  }
  if(persist_exists(WORKER_START_FORE_APP_REASON_PERSIST_KEY)){
    persist_delete(WORKER_START_FORE_APP_REASON_PERSIST_KEY);
  }
  if(persist_exists(DAILY_STEPC_PERSIST_KEY)){
    persist_delete(DAILY_STEPC_PERSIST_KEY);
  }
  if(persist_exists(DAILY_x1000_KCAL_PERSIST_KEY)){
    persist_delete(DAILY_x1000_KCAL_PERSIST_KEY);
  }

  // delete all the actigraphy keys
  // for(int16_t i = 0 ; i < 12; i++ ){
  //   if(persist_exists(i)){ persist_delete(i); }
  // }

  // initialize old keys
  persist_write_int(WORKER_START_FORE_APP_REASON_PERSIST_KEY,0);
  persist_write_int(DAILY_STEPC_PERSIST_KEY,0);
  persist_write_int(DAILY_x1000_KCAL_PERSIST_KEY,0);

  set_pinteract_state(11, -1);
  set_pinteract_state(12, -1);
  set_pinteract_state(13, -1);
  set_pinteract_state(140,-1);
  set_pinteract_state(141, -1);
  set_pinteract_state(15, -1);

  // // set the init_alg flag to false
  // struct acticlass_learn_alg_state aclas = {0};
  // persist_read_data(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY, &aclas, sizeof(aclas));
  // aclas.init_alg = false;
  // persist_write_data(ACTICLASS_LEARN_ALG_STATE_PERSIST_KEY, &aclas, sizeof(aclas));
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ FORE APP MASTER TICK FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* +++++++ GLOBAL VARIABLES +++++*/
/* FORE APP MASTER TICK VARIABLES */
static TickHandler tick_timer_clock_minute_handler;
static TickHandler tick_timer_clock_second_handler;
static TickHandler tick_timer_aux_minute_handler;
static TickHandler tick_timer_aux_second_handler;


void worker_start_fore_app_reason_exec(){
  /* >>> EXECUTE REASONS FOR WAKING FOREGROUND APP FROM WORKER <<<< */
  switch (persist_read_int(WORKER_START_FORE_APP_REASON_PERSIST_KEY)) {
    case WFAWR_DO_NOTHING:
      break; // do nothing
    case WFAWR_PUSH_ALL_DATA_TO_SERVER :
      // when only trying to push data to server
      if(heap_bytes_free()< N_B_TRANSMIT_CODE){
        window_stack_pop_all(false);
      }
      init_transmit_to_phone(TR_PUSH_ALL_DATA_TO_SERVER);
      break;
    case WFAWR_PUSH_ALL_DATA_TO_PHONE :
      // when want to push to phone; But, can wait if it is for a pinteract
      if(heap_bytes_free()< N_B_TRANSMIT_CODE){
        window_stack_pop_all(false);
      }
      init_transmit_to_phone(TR_PUSH_ALL_DATA_TO_PHONE);
      break;
    case WFAWR_MEMORY_LOW_REMINDER :
      // when want to remind the patient to connect the phone cause memory is low
      if(heap_bytes_free()< N_B_REMINDER_CODE){
        window_stack_pop_all(false);
      }
      display_reminder(RR_MEMORY_LOW);
      break;
    case WFAWR_WEAR_REMINDER :
      // vibrate to remind the person to remind
      vibes_enqueue_custom_pattern(pinteract_vibe_pat);
    default :
      break;
  }
  // reset the wakeup fore app reason key
  persist_write_int(WORKER_START_FORE_APP_REASON_PERSIST_KEY,WFAWR_DO_NOTHING);
}

static void app_state_change_trigger_minute(){
  worker_start_fore_app_reason_exec(); // only executes if non-0 reason to wakeup
}

static void app_state_change_trigger_second(){

}



void fore_app_master_tick_timer_handler(struct tm *tick_time, TimeUnits units_changed){
  // guards to prevent functions from being called more than once in a given
  // second/minute
  static int8_t cur_sec = -1;
  static int8_t cur_min = -1;

  // execute each second, if it is a new second
  if( tick_time->tm_sec != cur_sec){
    cur_sec = tick_time->tm_sec; // update the current second
    // execute the assigned function, IF it is not null
    if(tick_timer_clock_second_handler != NULL){
      tick_timer_clock_second_handler(tick_time, units_changed);
    }
    if(tick_timer_aux_second_handler != NULL){
      tick_timer_aux_second_handler(tick_time, units_changed);
    }
    // execute based on what has changed in the app state
    app_state_change_trigger_second();
  }


  // execute at the top of the minute, if it is a new minute
  if((tick_time->tm_sec == 0) && (tick_time->tm_min != cur_min )){
    cur_min =  tick_time->tm_min; //update the current minute
    // execute the assigned function, IF it is not null
    if(tick_timer_clock_minute_handler != NULL){
      tick_timer_clock_minute_handler(tick_time, units_changed);
    }
    if(tick_timer_aux_minute_handler != NULL){
      tick_timer_aux_minute_handler(tick_time, units_changed);
    }
    // execute based on what has changed in the app state
    app_state_change_trigger_minute();
  }

}


void fore_app_master_tick_timer_service_clock_subscribe(TimeUnits tick_units, TickHandler handler){
  // depending on the time_unit, we determine if the handlers have been assigned or not
  // and if they are free, we assign the new handler
  if(tick_units == SECOND_UNIT){
    if(tick_timer_clock_second_handler == NULL){
      tick_timer_clock_second_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E1");
    }
  }else if(tick_units == MINUTE_UNIT){
    if(tick_timer_clock_minute_handler == NULL){
      tick_timer_clock_minute_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E2");
    }
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E3");
  }
}

void fore_app_master_tick_timer_service_clock_unsubscribe(TimeUnits tick_units){
  if(tick_units == SECOND_UNIT){
    tick_timer_clock_second_handler = NULL;
  }else if(tick_units == MINUTE_UNIT){
    tick_timer_clock_minute_handler = NULL;
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E4");
  }
}


void fore_app_master_tick_timer_service_aux_subscribe(TimeUnits tick_units, TickHandler handler){
  // depending on the time_unit, we determine if the handlers have been assigned or not
  // and if they are free, we assign the new handler
  if(tick_units == SECOND_UNIT){
    if(tick_timer_aux_second_handler == NULL){
      tick_timer_aux_second_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E5");
    }
  }else if(tick_units == MINUTE_UNIT){
    if(tick_timer_aux_minute_handler == NULL){
      tick_timer_aux_minute_handler = handler;
    }else{
      APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E6");
    }
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E7");
  }
}

void fore_app_master_tick_timer_service_aux_unsubscribe(TimeUnits tick_units){
  if(tick_units == SECOND_UNIT){
    tick_timer_aux_second_handler = NULL;
  }else if(tick_units == MINUTE_UNIT){
    tick_timer_aux_minute_handler = NULL;
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "FA_TTS:E8");
  }
}



/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ CONVIENCE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



struct config_general get_config_general(){
  struct config_general cg;
  persist_read_data(CONFIG_GENERAL_PERSIST_KEY,&cg,sizeof(cg));
  return cg;
}

struct pinteract_state get_pinteract_state(){
  struct pinteract_state pis;
  persist_read_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
  return pis;
}

struct daily_acti get_daily_acti(){
  struct daily_acti da;
  persist_read_data(DAILY_ACTI_PERSIST_KEY, &da, sizeof(da));
  return da;
}

void set_pinteract_state(int16_t pi_i, int16_t state){
  struct pinteract_state pis = get_pinteract_state();
  switch(pi_i){
    case 11 :
      pis.pi_11[0] = state;
      break;
    case 12 :
      pis.pi_12 = state;
      break;
    case 13 :
      pis.pi_13 = state;
      break;
    case 140 :
      pis.pi_140[0] = state;
      break;
    case 141 :
      pis.pi_141[0] = state;
      break;
    case 15 :
      pis.pi_15 = state;
      break;
    default :
      break;
  }
  persist_write_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
}





//
// struct pinteract_state get_activityclass_params(){
//   struct activityclass_params acps;
//   persist_read_data(ACTIVITYCLASS_PARAMS_PERSIST_KEY, &pis, sizeof(pis));
//   return pis;
// }
//
// void set_activityclass_mean_std(enum ActivityClass ac_i, int16_t state){
//   struct pinteract_state pis = get_pinteract_state();
//   switch(pi_i){
//     case NO_ACTIVITY_CLASS :
//       pis.pi_11 = state;
//       break;
//     case SLOW_WALK :
//       pis.pi_12 = state;
//       break;
//     case FAST_WALK :
//       pis.pi_13 = state;
//       break;
//     case RUN :
//       pis.pi_14 = state;
//       break;
//     case FAST_RUN :
//       pis.pi_15 = state;
//       break;
//     default :
//       break;
//   }
//   persist_write_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
// }

// need a programatic way to re-write post/prior effectively




/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIMER FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


void write_time_to_array_head(uint32_t ts, uint8_t * buf){
  /* put timestamp into 1st 4 bytes of array, assumw little endian encoding*/
  for(int16_t i=0; i < 4; i ++ ){ buf[i] = (uint8_t) (ts >> (8*i) ); }
}

time_t  read_time_from_array_head(uint8_t * buf){
  /* get time from array head */
  return (time_t)( buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24) );
}



int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s){
  // time_t * today_srt_t -> the unix timestamp of the start of today
  // int32_t today_s -> the # of seconds that have elapsed today
  // return : # of seconds that have elapsed today

  time_t cur_time = time(NULL);
  struct tm *tt = localtime(&cur_time);
  int32_t today_s = (int32_t)( (tt->tm_sec) + (tt->tm_min)*60 + (tt->tm_hour)*(60*60) );

  if( out_today_s != NULL){
    *out_today_s = today_s;
  }
  if(today_srt_time_t != NULL){
    *today_srt_time_t = (time_t) (cur_time - today_s);
  }

  return today_s;
}

int32_t today_ms(int32_t * out_today_ms){
  *out_today_ms = today_srt_time_t_today_s(NULL,NULL)*1000 + time_ms(NULL,NULL);
  return *out_today_ms;
}


time_t today_s_to_time_t(int32_t today_s){
  time_t today_srt_time_t;
  today_srt_time_t_today_s(&today_srt_time_t, NULL);
  return (int32_t)(today_srt_time_t + today_s);
}


int32_t time_t_to_today_s(time_t t){
  time_t today_srt_time_t;
  today_srt_time_t_today_s(&today_srt_time_t, NULL);
  return (int32_t)(t - today_srt_time_t);
}


uint16_t today_s_to_today_m(int32_t today_s){
  return today_s/60;
}

int32_t today_m_to_today_s(uint16_t today_m){
  return today_m*60;
}



/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MAJOR OPERATIONS FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void wakeup_main_response_handler(WakeupId wakeup_id, int32_t wakeup_cookie){
  // get cookie that this wakeup correponds to. At this point
  // we assume that it directly correponds to pinteract_code of the
  // pinteract that we want to happen now, but latter we might use an
  // encoding scheme to have different kinds of wakeup

  // reschedule all events in the day.
  config_wakeup_schedule();

  // APP_LOG(APP_LOG_LEVEL_ERROR, "wakeup_id %d  wakeup_cookie %d",
  //     (int) wakeup_id, (int) wakeup_cookie);

  // IF the wakeup_cookie is < NUM_CONFIG_WAKEUP and non-negative, then we know
  // it refers to a config wakeup schedule index
  if(( wakeup_cookie < NUM_CONFIG_WAKEUP) && ( wakeup_cookie >= 0) ) {
    vibes_enqueue_custom_pattern(pinteract_vibe_pat);
    uint16_t config_wakeup_i = (uint16_t)wakeup_cookie; // for clarity
    persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,config_wakeup_i);
    uint16_t pinteract_code = pinteract_code_from_config_wakeup_index(config_wakeup_i); //

    // we assume that pinteract is the most important thing is to have the
    // pinteract, so we close all the windows in favor of opening the pinteract
    window_stack_pop_all(false);
    pinteract_driver(pinteract_code);


    // APP_LOG(APP_LOG_LEVEL_ERROR, "pinteract_code : %d", (int) pinteract_code );
    // psleep(100); // a slight delay to let any concurrent transmissions finish
      // their business.
    // if(heap_bytes_free()<1500){
      // pinteract_driver(pinteract_code);
    // }else{
    //   time_t wakeup_time_t = time(NULL) + 60*(up_delay_mins); // minutes;
    //   reschedule_config_wakeup_index(
    //     persist_read_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY), wakeup_time_t);
    //   persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,-1);
    //   window_stack_pop_all(false);
    // }

  }
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MISC FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


int32_t rand_lw_up_bounds(int32_t lw_bnd, int32_t up_bnd){
  srand(time(NULL)); // seed a new random generator, comment out for debugging
  return (rand()%(up_bnd - lw_bnd + 1)) ;
}


// conversion function

uint16_t kg_to_lbs(uint16_t kg){
  return (uint16_t) ( (kg * x1000_KG_TO_LBS)/1000);
}

uint16_t lbs_to_kg(uint16_t lbs){
  return (uint16_t) ((lbs * 1000)/ x1000_KG_TO_LBS );
}

uint16_t cm_to_in(uint16_t cm){
  // first convert to in, round up
  return (uint16_t) (((cm * x1000_CM_TO_IN )/1000) + 1);
}

void cm_to_ft_in_apart(uint16_t cm, uint16_t* ft, uint16_t* in_side){
  uint16_t in_full = cm_to_in(cm);
  *ft = in_full/12;
  *in_side = in_full - ((*ft) * 12);
}

uint16_t ft_in_apart_to_cm(uint16_t ft, uint16_t in_side){
  return (uint16_t) ( ((ft*12 + in_side)*1000)/x1000_CM_TO_IN);
}


// take a maximum number of slots, a current slot, a center point, and a radius,
// and return a x,y point that is RELATIVE to it on the circle

GPoint get_point_on_circle_at_ang(uint16_t n_ang, uint16_t cur_ang, GPoint center, uint16_t radius ){
  // get the proportion of the cur_ang to n_ang in terms of the 2^16 angle
  int32_t A = (cur_ang*TRIG_MAX_ANGLE)/n_ang ;
  int32_t x_ofs= (radius*cos_lookup(A)) /TRIG_MAX_ANGLE;
  int32_t y_ofs= (radius*sin_lookup(A)) /TRIG_MAX_ANGLE;

  return GPoint(center.x + x_ofs, center.y + y_ofs );
}

GPoint get_point_on_circle_at_ang_CW_noon(uint16_t n_ang, uint16_t cur_ang, GPoint center, uint16_t radius ){
    // get the proportion of the cur_ang to n_ang in terms of the 2^16 angle
  int32_t A = (cur_ang*TRIG_MAX_ANGLE)/n_ang;
  int32_t x_ofs= (radius*sin_lookup(A)) /TRIG_MAX_ANGLE;
  int32_t y_ofs= (radius*cos_lookup(A)) /TRIG_MAX_ANGLE;

  return GPoint(center.x + x_ofs, center.y - y_ofs );
}


// TEST : PASSED
int16_t pow_int(int16_t x, int16_t y){
  // Returns x^y if y>0 and x,y are integers
  int16_t r = 1; // result
  for(int16_t i =1; i<= abs(y) ; i++ ){ r = x*r; }
  return r;
}


// TEST : PASSED
/* Take square roots */
uint32_t isqrt(uint32_t x){
  uint32_t op, res, one;

  op = x;
  res = 0;

  /* "one" starts at the highest power of four <= than the argument. */
  one = 1 << 30;  /* second-to-top bit set */
  while (one > op) one >>= 2;

  while (one != 0) {
    if (op >= res + one) {
      op -= res + one;
      res += one << 1;  // <-- faster than 2 * one
    }
    res >>= 1;
    one >>= 2;
  }
  return res;
}




// void reset_config_general_persistent_storage(){
//
//   //   CONFIG_GENERAL_PERSIST_KEY
//   struct config_general init_cg = {0};
//   init_cg.pheight_cm = 175; // for setting up 175cm standard
//   init_cg.pweight_kg = 70;
//   init_cg.stepc_fft_thres0 = 20; // 24percent*100 of energy in 0.3-4hz band
//   init_cg.stepc_fft_thres1 = 20; // 17
//   init_cg.stepc_fft_thres2 = 18; // 17
//   init_cg.stepc_fft_thres3 = 14; // 17
//   init_cg.stepc_vma_thres0 = 1250;
//   init_cg.stepc_vmc_thres0 = 225;
//   init_cg.stepc_vmc_thres1 = 510;
//   init_cg.stepc_vmc_thres2 = 806;
//   init_cg.stepc_vmc_thres3 = 833;
//   init_cg.wear_class_thres = 290*12*10; // have this set for the 10 minute resolution
//   init_cg.pts_goal = 3000; // assume steps for now
//   init_cg.stepc_goal = 10000; // assume steps for now
//
//   init_cg.wear_class_thres = 290*12*10; // have this set for the 10 minute resolution
//   init_cg.pts_goal = 10000; // assume steps for now
//
//   persist_write_data(CONFIG_GENERAL_PERSIST_KEY,
//                      &init_cg, sizeof(init_cg));
// }

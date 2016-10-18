#include "config_func.h"
// a function that follows the time_ms example and will write it to two
// variables, a timestamp for start of day and number of seconds since
// start of the day, which is also directly returned


// find the next task in the configuration sheet
//  note, we still have to iterate over all of it because of potential
//  errors in the encoding and because we will have to reset it. Also,
//  the encoding might change during the day
//  Or, hell, just go one by one, whatever.
// NOTE, we use the size of the configuration file to determine


static void zero_out_wakeup_config_ary(struct wakeup_config *cs_ary_ps){
  for(int16_t i = 0; i < NUM_CONFIG_WAKEUP; i++){
    cs_ary_ps[i].pinteract_code = 0;
    cs_ary_ps[i].srt = 0;
    cs_ary_ps[i].end = 0;
  }
}

void write_to_config_wakeup_persistant_storage(
  struct wakeup_config * cs_ary_in, int16_t n_cs_ary_el_in ){
  // we know that the maximum numbe of wake wakeup_configs is 6, so we
  // initialize the arrary to be written to all zeros, as pinteract = 0 is our NULL
  // to NOT schedule. BUT, we are trying to make this an easy setup, ie, we
  // want to ensure that we ALWAYS initialize to zero even if we only want to define
  // a few of the config. so, we pass in wakeup_config arry, cs_ary_in, that
  // has c_cs_el_in elements, which MAY be less than 6.

  struct wakeup_config cs_ary_ps[NUM_CONFIG_WAKEUP];
  // // initialize all elements of the cs_ary_ps to have 0
  // for(int16_t i = 0; i < NUM_CONFIG_WAKEUP; i++){
  //   cs_ary_ps[i].pinteract_code = 0;
  //   cs_ary_ps[i].srt = 0;
  //   cs_ary_ps[i].end = 0;
  // }
  zero_out_wakeup_config_ary(cs_ary_ps);
  memcpy(cs_ary_ps, cs_ary_in, n_cs_ary_el_in*sizeof(struct wakeup_config));
  persist_write_data(CONFIG_WAKEUP_PERSIST_KEY,cs_ary_ps, sizeof(cs_ary_ps));
}


struct wakeup_config read_config_wakeup_index_persistant_storage( int16_t config_wakeup_i ){
  // we know that the maximum numbe of wake wakeup_configs is 6, so we
  // initialize the arrary to be written to all zeros, as pinteract = 0 is our NULL
  // to NOT schedule. BUT, we are trying to make this an easy setup, ie, we
  // want to ensure that we ALWAYS initialize to zero even if we only want to define
  // a few of the config. so, we pass in wakeup_config arry, cs_ary_in, that
  // has c_cs_el_in elements, which MAY be less than 6.
  // guard against improper inputs
  struct wakeup_config cs_ary_ps[NUM_CONFIG_WAKEUP];
  // set all elements to zero
  // for(int16_t i =0; i < NUM_CONFIG_WAKEUP; i++){
  //   cs_ary_ps[i].pinteract_code = 0;
  //   cs_ary_ps[i].srt = 0;
  //   cs_ary_ps[i].end = 0;
  // }
  zero_out_wakeup_config_ary(cs_ary_ps);
  // guard against improper inputs
  if((config_wakeup_i >= 0) && (config_wakeup_i < NUM_CONFIG_WAKEUP)){
    persist_read_data(CONFIG_WAKEUP_PERSIST_KEY, cs_ary_ps, sizeof(cs_ary_ps));
  }
  return cs_ary_ps[config_wakeup_i];
}



int16_t pinteract_code_from_config_wakeup_index(int16_t config_wakeup_i){
  return read_config_wakeup_index_persistant_storage(config_wakeup_i).pinteract_code;
}




void reset_config_wakeup_schedule(){
  // cancel all the current wakeups
  wakeup_cancel_all();
  // reset the boolean "is_scheduled" persistent storage array
  WakeupId cwi_ary_ps[NUM_TOTAL_WAKEUP]; // 8, cause that is number of possible wakeup

  // set all values of cwi_ary_ps to 0
  for(uint16_t i = 0; i < NUM_TOTAL_WAKEUP; i++){
    cwi_ary_ps[i] = 0;
  }

  persist_write_data(CONFIG_WAKEUP_IDS_PERSIST_KEY, cwi_ary_ps, sizeof(cwi_ary_ps));
}



WakeupId read_wakeup_id_at_config_wakeup_index(int16_t config_wakeup_i){
  WakeupId cw_wakeup_id = -1; // signal that reschedule was unsuccessful
  // guard against improper inputs
  if((config_wakeup_i >= 0) && (config_wakeup_i < NUM_CONFIG_WAKEUP)){
    WakeupId cwi_ary_ps[NUM_TOTAL_WAKEUP];
    persist_read_data(CONFIG_WAKEUP_IDS_PERSIST_KEY,cwi_ary_ps,sizeof(cwi_ary_ps));
    cw_wakeup_id = cwi_ary_ps[config_wakeup_i];
  }
  return cw_wakeup_id;
}



void write_wakeup_id_at_config_wakeup_index(int16_t config_wakeup_i, WakeupId wakeup_id){
  // guard against improper inputs
  if((config_wakeup_i >= 0) && (config_wakeup_i < NUM_CONFIG_WAKEUP)){
    WakeupId cwi_ary_ps[NUM_TOTAL_WAKEUP];
    persist_read_data(CONFIG_WAKEUP_IDS_PERSIST_KEY,cwi_ary_ps,sizeof(cwi_ary_ps));
    cwi_ary_ps[config_wakeup_i] =  wakeup_id;
    persist_write_data(CONFIG_WAKEUP_IDS_PERSIST_KEY,cwi_ary_ps,sizeof(cwi_ary_ps));
  }
}



WakeupId reschedule_config_wakeup_index(int16_t config_wakeup_i, time_t wakeup_time_t){

  WakeupId cw_wakeup_id = -1; // signal that reschedule was unsuccessful
  // guard against improper inputs
  if((config_wakeup_i >= 0) && (config_wakeup_i < NUM_CONFIG_WAKEUP)
    && (wakeup_time_t > time(NULL))){
      cw_wakeup_id = read_wakeup_id_at_config_wakeup_index(config_wakeup_i);
      wakeup_cancel(cw_wakeup_id);
      cw_wakeup_id = wakeup_schedule(wakeup_time_t, (int32_t) config_wakeup_i, false);
      write_wakeup_id_at_config_wakeup_index(config_wakeup_i, cw_wakeup_id);
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "dealing with config_wakeup_i %d for time_t %d with wakeup_id %d",
          (int) config_wakeup_i,(int) wakeup_time_t, (int)cw_wakeup_id);

  return cw_wakeup_id;
}


void config_wakeup_schedule(){
  // we have a persist buffer that consists of EXACTLY 6

  struct wakeup_config cs_ary[NUM_CONFIG_WAKEUP];
  persist_read_data(CONFIG_WAKEUP_PERSIST_KEY, cs_ary, sizeof(cs_ary));

  // need to
  // 1. schedule the days pinteracts.
  // 2. schedule the tomorrow @ 7am to sc
  // 3. reschedule the +7 day fallback (enough time to return from a trip, etc.)

  // 1.
  // iterate through all the elements of the cs_ary and schedule at their times
  time_t today_srt_time_t, wakeup_time_t;
  int32_t today_s;
  WakeupId cw_wakeup_id;
  today_srt_time_t_today_s(&today_srt_time_t, &today_s);
  // IF the wakeups havent been scheduled yet to day, THEN we schedule
  for(int16_t i = 0; i < NUM_CONFIG_WAKEUP; i++){
    // only schedule valid pinteracts
    if( cs_ary[i].pinteract_code > 0 ){
      wakeup_time_t = today_srt_time_t
        + today_m_to_today_s(cs_ary[i].srt)
        + rand_lw_up_bounds(
        today_m_to_today_s(cs_ary[i].srt),
        today_m_to_today_s(cs_ary[i].end));

      // ONLY reschedule IF the current i's wakeup has expired (ie, it doesn't
      // have a wakeup event scheduled later today) AND the current time is NOT
      // AFTER the start time of the period which the current i's wakeup COULD
      // occur in. This second condition prevents re-scheduling within the period
      // once the i's wakeup has been called, the wakeup query expires, and we
      // call to reschedule it.

      APP_LOG(APP_LOG_LEVEL_ERROR, "config wakeup index %d",i); // for debuggin only

      if(!wakeup_query(read_wakeup_id_at_config_wakeup_index(i),NULL)
            && (today_s < today_m_to_today_s(cs_ary[i].srt) )){

        cw_wakeup_id = reschedule_config_wakeup_index(i, wakeup_time_t);
        APP_LOG(APP_LOG_LEVEL_ERROR, "dealing with index %d for time_t %d with wakeup_id %d",
                i,(int) wakeup_time_t, (int)cw_wakeup_id);
      }
    }
  }

  // REMEMEBER, every time the worker launches the app, we reset the
  // config. SO, we don't have to do it at midnight. So, the worker will
  // launch the scheduling config in the early morning, AT LEAST by 2am
  // with each data block being 2 hours @ 1 min intervals
  // These wakeups are just backup incase the worker dies.

  // NOTE For midnight and 7day fallbacks, we can just cancel and rewrite

  int32_t sec_1am = 60;
  // 2.
  // wakeup tomorrow @ 7:00am, for possible time zone changes
  if(!wakeup_query(read_wakeup_id_at_config_wakeup_index(6),NULL)){
    reschedule_config_wakeup_index(6, today_srt_time_t + NUM_SEC_IN_DAY + sec_1am);
  }
  //3.
  // fallback wakeup, +7 days from now @ 7:00 am, for possible timezone changes
  // just always cancel and then rewrite
  reschedule_config_wakeup_index(7, today_srt_time_t + NUM_SEC_IN_WEEK + sec_1am);


  //   APP_LOG(APP_LOG_LEVEL_ERROR,"n_wakeup_config : %d",n_wakeup_config);
  //   APP_LOG(APP_LOG_LEVEL_ERROR,"sizeof(cs_ary) : %d",sizeof(cs_ary));
  //   APP_LOG(APP_LOG_LEVEL_ERROR,"sizeof(cs_ary) : %d",sizeof(cs_ary[0]));

}




// void config_wakeup_schedule(){

//   // get config structure and read into
//   int16_t n_wakeup_config = persist_get_size(CONFIG_PERSIST_KEY)/sizeof(struct wakeup_config);
//   // declare the wakeup_config array
//   struct wakeup_config cs_ary[n_wakeup_config];
//   // read data into the wakeup_config array
//   persist_read_data(CONFIG_PERSIST_KEY, cs_ary, sizeof(cs_ary));

// //   APP_LOG(APP_LOG_LEVEL_ERROR,"n_wakeup_config : %d",n_wakeup_config);
// //   APP_LOG(APP_LOG_LEVEL_ERROR,"sizeof(cs_ary) : %d",sizeof(cs_ary));
// //   APP_LOG(APP_LOG_LEVEL_ERROR,"sizeof(cs_ary) : %d",sizeof(cs_ary[0]));


//   //initialize vars for the today time
//   int32_t today_srt_time_t, cur_today_s;
//   // set the start of day time and the today sec time
//   today_srt_time_t_today_s(&today_srt_time_t,&cur_today_s);
//   // initialize the config comparison
//   int32_t min_config_today_s_srt = 84599; // can only decrease from here
//   int16_t config_s_srt = -1;
//   int16_t i_min_config_today_s_srt = -1; // in case no config left today

//   // find the minimal config time and index that is greater than
//   // the current time
// //   config_s_srt = config_t_to_today_s( (cs_ary[0]).srt);
// //   APP_LOG(APP_LOG_LEVEL_ERROR,"config_s_srt : %d",config_s_srt);

//   for(int16_t i =0; i< n_wakeup_config; i++){
//     config_s_srt = config_t_to_today_s(cs_ary[i].srt);
//     // we only accept as valid the start times that are between now
//     // and *at most* midnight*
//     if( (config_s_srt > cur_today_s) && (config_s_srt < min_config_today_s_srt)){
//       min_config_today_s_srt = config_s_srt;
//       i_min_config_today_s_srt = i;
//     }
//   }

//   APP_LOG(APP_LOG_LEVEL_ERROR,"min_config_today_s_srt : %d", (int)min_config_today_s_srt);
//   APP_LOG(APP_LOG_LEVEL_ERROR,"i_min_config_today_s_srt : %d",i_min_config_today_s_srt);

// //   i_min_config_today_s_srt = i_min_config_today_s_srt;

//   time_t wakeup_time_t;

//   wakeup_time_t = today_srt_time_t
//       + min_config_today_s_srt
//       + rand_lw_up_bounds(
//     config_t_to_today_s(cs_ary[i_min_config_today_s_srt].srt),
//     config_t_to_today_s(cs_ary[i_min_config_today_s_srt].end));

//   APP_LOG(APP_LOG_LEVEL_ERROR,"wakeup_time_t : %d", (int)wakeup_time_t);


//   // try to schedule the wake up event
//   // 1. IF the current wakeup id scheduled by a config event is still
//   // valid, then that means we have NOT reached that pinteract yet,
//   // so we dont do anything. REMEMBER, a wakeid is valid ONLY
//   // 2. if there are NO config events whose start times begin after now
//   // at before midnight, we do nothing, because the app worker will make
//   // a call to this function at midnight to schedule the first pinteract
//   // for the day
//   // NOTE!: WakeupId is simply a int32_t
//   if( ( !wakeup_query( ((WakeupId) persist_read_int(CONFIG_WAKEUPID_KEY)), NULL))
//       && (i_min_config_today_s_srt != -1) ){
//     // set the wakeup time
//     wakeup_time_t = today_srt_time_t
//       + min_config_today_s_srt
//       + rand_lw_up_bounds(
//       config_t_to_today_s(cs_ary[i_min_config_today_s_srt].srt),
//       config_t_to_today_s(cs_ary[i_min_config_today_s_srt].end));

//     // schedule the config wakeup handler

//     // schedule the wakeup
//     WakeupId wakeup_id = wakeup_schedule(wakeup_time_t, CONFIG_WAKEUP_COOKIE, false);
// //     wakeup_service_subscribe(config_wakeup_handler);

//     // save the wakeupid that is the next current config wakeup
//     persist_write_int(CONFIG_WAKEUPID_KEY, wakeup_id);
//     // save which config has been scheduled
//     persist_write_int(CONFIG_I_WAKEUP_KEY,i_min_config_today_s_srt);
//     // save what interaction we want to perform when we wakeup
//     persist_write_int(CONFIG_WAKEUP_PINTERACT_CODE_KEY,
//                      cs_ary[i_min_config_today_s_srt].pinteract_code);

//     APP_LOG(APP_LOG_LEVEL_ERROR, "wakeup_query : %d", (int) wakeup_query( wakeup_id,NULL ) );
//     APP_LOG(APP_LOG_LEVEL_ERROR,"wakeup_time_t : %d", (int) wakeup_time_t);
//     APP_LOG(APP_LOG_LEVEL_ERROR, "pinteract_code : %d",
//             (int) persist_read_int(CONFIG_WAKEUP_PINTERACT_CODE_KEY));


//   }


// }

// void config_wakeup_handler(WakeupId wakeup_id, int32_t cookie){

//   APP_LOG(APP_LOG_LEVEL_ERROR, "config_wakeup_handler : %d", (int) wakeup_id );
//   // get the patient code that is supposed to be executed
//   uint16_t pinteract_code = persist_read_int(CONFIG_WAKEUP_PINTERACT_CODE_KEY);
//   APP_LOG(APP_LOG_LEVEL_ERROR, "config_wakeup_handler, pinteract_code : %d", (int) pinteract_code );

//   // get the state of the interaction (ie, if we want to execute it or not)

//   pinteract_driver(pinteract_code);

// }

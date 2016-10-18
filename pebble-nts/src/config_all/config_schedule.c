#include "config_func.h"
#include "../constants.h"
// a function that follows the time_ms example and will write it to two
// variables, a timestamp for start of day and number of seconds since
// start of the day, which is also directly returned


// find the next task in the configuration sheet
//  note, we still have to iterate over all of it because of potential
//  errors in the encoding and because we will have to reset it. Also,
//  the encoding might change during the day
//  Or, hell, just go one by one, whatever.
// NOTE, we use the size of the configuration file to determine


static void zero_out_wakeup_config_ary(WakeupConfig *cs_ary_ps){
  for(int16_t i = 0; i < NUM_CONFIG_WAKEUP; i++){
    cs_ary_ps[i].pinteract_code = 0;
    cs_ary_ps[i].srt = 0;
    cs_ary_ps[i].end = 0;
  }
}

void write_to_config_wakeup_persistant_storage(WakeupConfig * cs_ary_in, int16_t n_cs_ary_el_in ){
  // we know that the maximum numbe of wake wakeup_configs is 6, so we
  // initialize the arrary to be written to all zeros, as pinteract = 0 is our NULL
  // to NOT schedule. BUT, we are trying to make this an easy setup, ie, we
  // want to ensure that we ALWAYS initialize to zero even if we only want to define
  // a few of the config. so, we pass in wakeup_config arry, cs_ary_in, that
  // has c_cs_el_in elements, which MAY be less than 6.

  WakeupConfig cs_ary_ps[NUM_CONFIG_WAKEUP];
  // // initialize all elements of the cs_ary_ps to have 0
  zero_out_wakeup_config_ary(cs_ary_ps);
  memcpy(cs_ary_ps, cs_ary_in, n_cs_ary_el_in*sizeof(WakeupConfig));
  persist_write_data(CONFIG_WAKEUP_PERSIST_KEY,cs_ary_ps, sizeof(cs_ary_ps));
}


WakeupConfig read_config_wakeup_index_persistant_storage( int16_t config_wakeup_i ){
  // we know that the maximum numbe of wake wakeup_configs is 6, so we
  // initialize the arrary to be written to all zeros, as pinteract = 0 is our NULL
  // to NOT schedule. BUT, we are trying to make this an easy setup, ie, we
  // want to ensure that we ALWAYS initialize to zero even if we only want to define
  // a few of the config. so, we pass in wakeup_config arry, cs_ary_in, that
  // has c_cs_el_in elements, which MAY be less than 6.
  // guard against improper inputs
  WakeupConfig cs_ary_ps[NUM_CONFIG_WAKEUP];

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
  if((config_wakeup_i >= 0) && (config_wakeup_i < NUM_TOTAL_WAKEUP)
    && (wakeup_time_t > time(NULL))){
      cw_wakeup_id = read_wakeup_id_at_config_wakeup_index(config_wakeup_i);
      wakeup_cancel(cw_wakeup_id);
      cw_wakeup_id = wakeup_schedule(wakeup_time_t, (int32_t) config_wakeup_i, false);
      // if() equals to out of resources, then notify 
      write_wakeup_id_at_config_wakeup_index(config_wakeup_i, cw_wakeup_id);
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "dealing with config_wakeup_i %d for time_t %d with wakeup_id %d",
          (int) config_wakeup_i,(int) wakeup_time_t, (int)cw_wakeup_id);

  return cw_wakeup_id;
}


void config_wakeup_schedule(){
  // we have a persist buffer that consists of EXACTLY 6

  WakeupConfig cs_ary[NUM_CONFIG_WAKEUP];
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
    // only schedule valid pinteracts, ie, with pinteract codes greater than 0
    if( cs_ary[i].pinteract_code > 0 ){
      wakeup_time_t = today_srt_time_t
        + cs_ary[i].srt + rand_lw_up_bounds(cs_ary[i].srt, cs_ary[i].end);

      // ONLY reschedule IF the current i's wakeup has expired (ie, it doesn't
      // have a wakeup event scheduled later today) AND the current time is NOT
      // AFTER the start time of the period which the current i's wakeup COULD
      // occur in. This second condition prevents re-scheduling within the period
      // once the i's wakeup has been called, the wakeup query expires, and we
      // call to reschedule it.

      APP_LOG(APP_LOG_LEVEL_ERROR, "config wakeup index %d",i); // for debuggin only

      if(!wakeup_query(read_wakeup_id_at_config_wakeup_index(i),NULL)
            && (today_s < cs_ary[i].srt )){

        cw_wakeup_id = reschedule_config_wakeup_index(i, wakeup_time_t);
        APP_LOG(APP_LOG_LEVEL_ERROR, "dealing with index %d for time_t %d with wakeup_id %d",
                i,(int) wakeup_time_t, (int)cw_wakeup_id);
      }
    }
  }

  // 1. wakeup 1 hour from now, only if the 1 hour timer hasn't expired yet
  if(!wakeup_query(read_wakeup_id_at_config_wakeup_index(4),NULL)){
    reschedule_config_wakeup_index(4, time(NULL) + INTERVAL_MINUTES*60);
  }
  // NOTE : Fallback timer, just always cancel and then rewrite
  // 2. wakeup 12 hours from now
  reschedule_config_wakeup_index(5, time(NULL) + 12*60*60);
  // 3.
  // wakeup tomorrow @ 12:01am, for possible time zone changes
  reschedule_config_wakeup_index(6, today_srt_time_t + NUM_SEC_IN_DAY + 60);
  // 4.
  // fallback wakeup, +7 days from now @ 12:01 am,
  reschedule_config_wakeup_index(7, today_srt_time_t + NUM_SEC_IN_WEEK + 60);

  APP_LOG(APP_LOG_LEVEL_ERROR, "rescheduled all timers"); // for debuggin only
}

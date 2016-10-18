#include "helper.h"


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* ++++++++++++++++ STORAGE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


void reset_config_wakeup_persistent_storage(){
  if(persist_exists(CONFIG_WAKEUP_IDS_PERSIST_KEY)){
    persist_delete(CONFIG_WAKEUP_IDS_PERSIST_KEY);
  }

  WakeupConfig cs_ary[2];

  // SUPER ROUGH, but good enough for user testing
  cs_ary[0].pinteract_code = 14;
  cs_ary[0].srt = 540*60;
  cs_ary[0].end = 600*60;
  cs_ary[1].pinteract_code = 11;
  cs_ary[1].srt = 1080*60;
  cs_ary[1].end = 1140*60;

  reset_config_wakeup_schedule();
  write_to_config_wakeup_persistant_storage(cs_ary, 2);
  // call config_wakeup_schedule
  config_wakeup_schedule();

  // DEBUGGING
  // WakeupId wakeup_id = read_wakeup_id_at_config_wakeup_index(3);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "config wakeup i=0, wakeup_id %d ", (int) wakeup_id);
}

void reset_pinteract_states(){

  // initialize the pinteract states to -1
  Pinteract11State pi11_init = { .mood_index= -1 };
  Pinteract14State pi14_init = {.sleep_duration_min= -1, .sleep_quality_index = -1};
  for(int16_t i = 0; i < NUM_DAYS_HISTORY; i++){
    set_pinteract_state(11, (void*) &pi11_init,i);
    set_pinteract_state(14, (void*) &pi14_init,i);
  }
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ FORE APP MASTER TICK FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ CONVIENCE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



ConfigGeneral get_config_general(){
  ConfigGeneral cg;
  persist_read_data(CONFIG_GENERAL_PERSIST_KEY,&cg,sizeof(cg));
  return cg;
}

PinteractStates get_pinteract_state_all(){
  PinteractStates pis;
  persist_read_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
  return pis;
}

void set_pinteract_state(int16_t pi_i, void* state, int16_t index){
  PinteractStates pis = get_pinteract_state_all();
  switch(pi_i){
    case 0 :
      pis.time_last_entry = *((time_t*) state);
      break;
    case 11 :
      pis.pi_11[index].mood_index = ((Pinteract11State*) state)->mood_index;
      break;
    case 14 :
      pis.pi_14[index].sleep_duration_min = ((Pinteract14State*) state)->sleep_duration_min;
      pis.pi_14[index].sleep_quality_index = ((Pinteract14State*) state)->sleep_quality_index;
      break;
    default :
      break;
  }
  persist_write_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
}

void pinteract_state_roll_over_days_entry(time_t time_entry){
  // get the
  PinteractStates pis = get_pinteract_state_all();
  // struct tm* tm_last_entry = localtime(&pis.time_last_entry);
  // struct tm* tm_entry = localtime(&time_entry);
  int last_entry_yday = localtime(&pis.time_last_entry)->tm_yday;
  int last_entry_year = localtime(&pis.time_last_entry)->tm_year;
  int entry_yday = localtime(&time_entry)->tm_yday;
  // to account for roll over in years
  if(  entry_yday < last_entry_yday ){
    // accounts for leap years
    entry_yday = entry_yday + 364 + (last_entry_year%4 == 0);
  }
  // get the difference in days
  int diff_days = entry_yday - last_entry_yday;
  diff_days = (diff_days < NUM_DAYS_HISTORY) ? diff_days : NUM_DAYS_HISTORY;
  // diff_days = 1;
  for(int i = 0; i < diff_days; i++){
    // move the previous 7 days back by 1
    for(int j = NUM_DAYS_HISTORY-1; j > 0 ; j--){
      pis.pi_11[j].mood_index = pis.pi_11[j-1].mood_index;
      pis.pi_14[j].sleep_duration_min = pis.pi_14[j-1].sleep_duration_min;
      pis.pi_14[j].sleep_quality_index = pis.pi_14[j-1].sleep_quality_index;
    }
    // set the current day to NULL
    pis.pi_11[0].mood_index = -1;
    pis.pi_14[0].sleep_duration_min = -1;
    pis.pi_14[0].sleep_quality_index = -1;
  }
  pis.time_last_entry = time_entry;
  persist_write_data(PINTERACT_STATE_PERSIST_KEY, &pis, sizeof(pis));
}




/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIMER FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */




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

  // IF the wakeup_cookie is < NUM_CONFIG_WAKEUP and non-negative, then we know
  // it refers to a config wakeup schedule index
  if(( wakeup_cookie < NUM_CONFIG_WAKEUP) && ( wakeup_cookie >= 0) ) {
    vibes_enqueue_custom_pattern(pinteract_vibe_pat);
    uint16_t config_wakeup_i = (uint16_t)wakeup_cookie; // for clarity
    persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,config_wakeup_i);
    uint16_t pinteract_code = pinteract_code_from_config_wakeup_index(config_wakeup_i); //

    // we assume that pinteract is the most important thing is to have the
    // pinteract, so we close all the windows in favor of opening the pinteract
    // window_stack_pop_all(false);

    // get the pinteract code, pass it to the privacy screen to start pinteract
    pinteract_priv_scrn(pinteract_code);
    // pinteract_driver(pinteract_code);

  }else if(( wakeup_cookie < NUM_TOTAL_WAKEUP) && ( wakeup_cookie >= NUM_CONFIG_WAKEUP) ){
    // Roll over the number of days
    pinteract_state_roll_over_days_entry(time(NULL)+10);
    APP_LOG(APP_LOG_LEVEL_ERROR, "attempt to transmit from wakeup");
    comm_begin_upload_countdown();
  }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ MATHEMATICS FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int32_t rand_lw_up_bounds(int32_t lw_bnd, int32_t up_bnd){
  srand(time(NULL)); // seed a new random generator, comment out for debugging
  return (rand()%(up_bnd - lw_bnd + 1)) ;
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


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIME FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void write_time_to_array_head(time_t ts, uint8_t * buf){
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
/* +++++++++++++++ SORTING FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


// >> the pointer to the array we want the the sorted indicies
static int16_t *sortpt;

// >> directly compares values *a and *b
static int cmpfunc_ascend(const void *a, const void *b){
  return(  (*(int16_t*)a) - (*(int16_t*)b) ); // DESCENDING ORDER
}
static int cmpfunc_descend(const void *a, const void *b){
  return(  (*(int16_t*)b) - (*(int16_t*)a) ); // DESCENDING ORDER
}

// >> compares the values of an array at sortpt, where, a & b are indicies
// into that array. Used with an auxillary array ind_d to modify d_ind such that
// ind_d contains the sorted indicies of the sortpt array
static int cmpfunc_ascend_index(const void *a, const void *b){
  return (  (sortpt[*(int16_t*)a ] ) - (sortpt[*(int16_t*)b ])  ); // ASCENDING ORDER
}
static int cmpfunc_descend_index(const void *a, const void *b){
  return (  (sortpt[*(int16_t*)b ] ) - (sortpt[*(int16_t*)a ])  ); // DESCENDING ORDER
}
// >> takes a
static void maxminval(int16_t d[], int16_t p, int16_t r, int16_t *max, int16_t *min, int (*compar)(const void*,const void*) ){
  // p and r are INDICIES, not sizes, so we must touch all
  // this is p & r inclusive
  *max = d[p]; // the first element
  *min = d[p]; // the first element

  for(int16_t i = p; i <= r; i++ ){
    if(   compar(max,&(d[i])) > 0){  *max = d[i]; }
    if( compar(min,&(d[i])) < 0){  *min = d[i]; }
  }
}

// >> the swap function for qsort
static void swapf(int16_t *d, int16_t i, int16_t j){
  int16_t tmp = d[i];
  d[i] = d[j];
  d[j] = tmp;
}
// >> basic implementation of
static void qsortf(int16_t* d, int16_t p, int16_t r, int (*compar)(const void*,const void*)){
  /* test if base case */
  if( (p >= r) || ( r <= p ) ){
    return;
  }
  /* initialize the variables for pivot and array length */
  int16_t n = r - p + 1;
  int16_t q = 0 + p;
  /* parition into < and >= arrays with first element as pivot */
  for(int16_t j = 0; j < n; j++ ){
    if( compar( &d[p + j], &d[q]) < 0 ){
      swapf(d, p+j ,q+1);
      swapf(d, q+1 ,q);
      q = q + 1;
    }
  }
  /* recursive call */
  qsortf(d, p, q-1, compar);
  qsortf(d, q+1, r, compar);
}

// >> sorts the input array d and puts its sorted indicies into ind_d
static void sort_order_descend(int16_t *d, int16_t *ind_d, int16_t dlen){
  for(int16_t i = 0; i<dlen; i++) { ind_d[i] = i;} // initialize ind_d array
  sortpt = d;
  qsortf(ind_d, 0, (dlen-1), cmpfunc_descend_index);
  qsortf(d, 0, (dlen-1), cmpfunc_descend);
}
static void sort_order_ascend(int16_t *d, int16_t *ind_d, int16_t dlen){
  for(int16_t i = 0; i<dlen; i++) { ind_d[i] = i;} // initialize ind_d array
  sortpt = d;
  qsortf(ind_d, 0, (dlen-1), cmpfunc_ascend_index);
  qsortf(d, 0, (dlen-1), cmpfunc_ascend);
}

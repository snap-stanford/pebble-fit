/* Project Kraepelin, Main file
    Copyright (C) 2015 : Nathaniel T. Stockham

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    This license is taken to apply to any other files in the Project Kraepelin
    Pebble App roject.
*/

// #include <pebble_worker.h>
#include "helper_worker.h"

/* HELPER FUNCTIONS */

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

// TEST : PASSED
int32_t integral_abs(int16_t *d, int16_t srti, int16_t endi){
  /* Integrate the absolute values between given srti and endi index*/
  int32_t int_abs = 0;

  for(int16_t i = srti; i <= endi; i++ ){
    int_abs += abs(d[i]);
  }
  return (int_abs > 0) ? int_abs : 1;
}
// TEST : PASSED
int32_t integral_l2(int16_t *d, int16_t srti, int16_t endi){
  /* Integrate the absolute values between given srti and endi index*/
  int32_t int_l2 = 0;

  for(int16_t i = srti; i <= endi; i++ ){
    // printf("n7.3: \n");
    int_l2 += (d[i]*d[i]);
  }
  // to prevent nasty divide by 0 problems
  return (int_l2 > 0) ? int_l2 : 1;
}



// TEST : PASSED
uint8_t get_angle_i(int16_t x, int16_t y, uint8_t n_ang ){

  // get the angle resolution
  int32_t ang_res = TRIG_MAX_ANGLE/n_ang;
  // Get the angle from the pebble lookup

  // !! MAKE SURE RANGE IS APPROPRIATE, ie -TRIG_MAX_ANGLE/2 to TRIG_MAX_ANGLE/2
  int32_t A = atan2_lookup(y, x);

  // IF the pebble has any consistency whatsoever, the -pi/2 to 0
  // for the atan2 will be mapped to the pi to 2*pi geometric angles.
  // This is the only thing that makes sense for consistency across
  // the various elements

  // BUT, in case it doesn't, here is the transformation to use
  // Shift the negative angles (-TRIG_MAX_ANGLE/2 to 0) so range is 0 to TRIG_MAX_ANGLE
//   A = A > 0 ? A : (A + TRIG_MAX_ANGLE);

  // divide out by ang_res to get the index of the angle
  // we need to make sure that in all cases that the returned
  //   index is at MOST one less that n_ang, cause 0-15

  // shift by (ang_res/2) so rounds int, not floor
  return (uint8_t) ( ((A + (ang_res/2))/ang_res < n_ang) ?
                    ((A + (ang_res/2))/ang_res) : 0);
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ ACTIGRAPHY FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



// TEST: PASSED
uint8_t orient_encode(int16_t *mean_ary, uint8_t n_ang){
  //MAX n_ang is 16, as 16*15 + 15 = 255
  // The range of the theta_i and phi_i is 0 to (n_ang-1)
  // get theta, in the x-y plane. theta relative to +x-axis
  uint8_t theta_i = get_angle_i(mean_ary[0], mean_ary[1], n_ang );

  // get phi, in the xy_vm-z plane
  int16_t xy_vm = isqrt(mean_ary[0]*mean_ary[0] + mean_ary[1]*mean_ary[1]);
  // phi rel to  +z-axis, so z is on hoz-axis and xy_vm is vert-axis
  uint8_t phi_i = get_angle_i(mean_ary[2],xy_vm, n_ang );

  return n_ang*phi_i + theta_i;
}


// TEST : NONE
void fft_mag(int16_t *d, int16_t dlenpwr){
  // NOTE! this function modifies the input array
  int16_t dlen = pow_int(2,dlenpwr);

  // evaluate the fourier coefficent magnitude
  // NOTE: coeff @ index 0 and dlen/2 only have real components
  //    so their magnitude is exactly that
  for(int16_t i = 1; i < (dlen/2); i++){
    // NOTE: eval coeff mag for real and imag : R(i) & I(i)
    d[i] = isqrt(d[i]*d[i] + d[dlen-i]*d[dlen-i]);
  }
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* +++++++++++++++ TIME FUNCTIONS +++++++++++++++ */
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
/* +++++++++++++++ CONVIENCE FUNCTIONS +++++++++++++++ */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */



struct config_general get_config_general(){
  struct config_general cur_config;
  persist_read_data(CONFIG_GENERAL_PERSIST_KEY,&cur_config,sizeof(cur_config));
  return cur_config;
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






//
// /* HELPER FUNCTIONS */
//
// // TEST : PASSED
// int16_t pow_int(int16_t x, int16_t y){
//   // Returns x^y if y>0 and x,y are integers
//   int16_t r = 1; // result
//   for(int16_t i =1; i<= abs(y) ; i++ ){ r = x*r; }
//   return r;
// }
//
//
// // TEST : PASSED
// /* Take square roots */
// uint32_t isqrt(uint32_t x){
//   uint32_t op, res, one;
//
//   op = x;
//   res = 0;
//
//   /* "one" starts at the highest power of four <= than the argument. */
//   one = 1 << 30;  /* second-to-top bit set */
//   while (one > op) one >>= 2;
//
//   while (one != 0) {
//     if (op >= res + one) {
//       op -= res + one;
//       res += one << 1;  // <-- faster than 2 * one
//     }
//     res >>= 1;
//     one >>= 2;
//   }
//   return res;
// }
//
//
// // TEST : NONE
// int32_t integral_abs(int16_t *d, int16_t srti, int16_t endi){
//   /* Integrate the absolute values between given srti and endi index*/
//   int32_t int_abs = 0;
//
//   for(int16_t i = srti; i <= endi; i++ ){
//     int_abs += abs(d[i]);
//   }
//   return int_abs;
// }
//
//
//
// // TEST : PASSED
// uint8_t get_angle_i(int16_t x, int16_t y, uint8_t n_ang ){
//
//   // get the angle resolution
//   int32_t ang_res = TRIG_MAX_ANGLE/n_ang;
//   // Get the angle from the pebble lookup
//
//   // !! MAKE SURE RANGE IS APPROPRIATE, ie -TRIG_MAX_ANGLE/2 to TRIG_MAX_ANGLE/2
//   int32_t A = atan2_lookup(y, x);
//
//   // IF the pebble has any consistency whatsoever, the -pi/2 to 0
//   // for the atan2 will be mapped to the pi to 2*pi geometric angles.
//   // This is the only thing that makes sense for consistency across
//   // the various elements
//
//   // BUT, in case it doesn't, here is the transformation to use
//   // Shift the negative angles (-TRIG_MAX_ANGLE/2 to 0) so range is 0 to TRIG_MAX_ANGLE
// //   A = A > 0 ? A : (A + TRIG_MAX_ANGLE);
//
//   // divide out by ang_res to get the index of the angle
//   // we need to make sure that in all cases that the returned
//   //   index is at MOST one less that n_ang, cause 0-15
//
//   // shift by (ang_res/2) so rounds int, not floor
//   return (uint8_t) ( ((A + (ang_res/2))/ang_res < n_ang) ?
//                     ((A + (ang_res/2))/ang_res) : (n_ang-1));
// }
//
//
// /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
// /* +++++++++++++++ ACTIGRAPHY FUNCTIONS +++++++++++++++ */
// /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
//
//
//
// // TEST: PASSED
// uint8_t orient_encode(int16_t *mean_ary, uint8_t n_ang){
//   //MAX n_ang is 16, as 16*15 + 15 = 255
//   // The range of the theta_i and phi_i is 0 to (n_ang-1)
//   // get theta, in the x-y plane. theta relative to +x-axis
//   uint8_t theta_i = get_angle_i(mean_ary[0], mean_ary[1], n_ang );
//
//   // get phi, in the xy_vm-z plane
//   int16_t xy_vm = isqrt(mean_ary[0]*mean_ary[0] + mean_ary[1]*mean_ary[1]);
//   // phi rel to  +z-axis, so z is on hoz-axis and xy_vm is vert-axis
//   uint8_t phi_i = get_angle_i(mean_ary[2],xy_vm, n_ang );
//
//   return n_ang*phi_i + theta_i;
// }
//
//
// // TEST : NONE
// void fft_mag(int16_t *d, int16_t dlenpwr){
//   // NOTE! this function modifies the input array
//   int16_t dlen = pow_int(2,dlenpwr);
//
//   // evaluate the fourier coefficent magnitude
//   // NOTE: coeff @ index 0 and dlen/2 only have real components
//   //    so their magnitude is exactly that
//   for(int16_t i = 1; i < (dlen/2); i++){
//     // NOTE: eval coeff mag for real and imag : R(i) & I(i)
//     d[i] = isqrt(d[i]*d[i] + d[dlen-i]*d[dlen-i]);
//   }
// }
//
//
// /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
// /* +++++++++++++++ TIME FUNCTIONS +++++++++++++++ */
// /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
//
// void write_time_to_array_head(uint8_t * buf){
//   /* get the current time */
//   uint32_t ts = time(NULL);
//
//   /* put timestamp into 1st 4 bytes of array, assumw little endian encoding*/
//   for(int16_t i=0; i < 4; i ++ ){ buf[i] = (uint8_t) (ts >> (8*i) ); }
// }
//
// time_t  read_time_from_array_head(uint8_t * buf){
//   /* get time from array head */
//   return (time_t)( buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24) );
// }
//
//
//
// int32_t today_srt_time_t_today_s(time_t * today_srt_time_t, int32_t *out_today_s){
//   // time_t * today_srt_t -> the unix timestamp of the start of today
//   // int32_t today_s -> the # of seconds that have elapsed today
//   // return : # of seconds that have elapsed today
//
//   time_t cur_time = time(NULL);
//   struct tm *tt = localtime(&cur_time);
//   int32_t today_s = (int32_t)( (tt->tm_sec) + (tt->tm_min)*60 + (tt->tm_hour)*(60*60) );
//
//   if( out_today_s != NULL){
//     *out_today_s = today_s;
//   }
//   if(today_srt_time_t != NULL){
//     *today_srt_time_t = (time_t) (cur_time - today_s);
//   }
//
//   return today_s;
// }
//
// int32_t today_ms(int32_t * out_today_ms){
//   *out_today_ms = today_srt_time_t_today_s(NULL,NULL)*1000 + time_ms(NULL,NULL);
//   return *out_today_ms;
// }
//
//
// time_t today_s_to_time_t(int32_t today_s){
//   time_t today_srt_time_t;
//   today_srt_time_t_today_s(&today_srt_time_t, NULL);
//   return (int32_t)(today_srt_time_t + today_s);
// }
//
//
// int32_t time_t_to_today_s(time_t t){
//   time_t today_srt_time_t;
//   today_srt_time_t_today_s(&today_srt_time_t, NULL);
//   return (int32_t)(t - today_srt_time_t);
// }
//
//
// uint16_t today_s_to_today_m(int32_t today_s){
//   return today_s/60;
// }
//
// int32_t today_m_to_today_s(uint16_t today_m){
//   return today_m*60;
// }
//
//
//
// /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
// /* +++++++++++++++ CONVIENCE FUNCTIONS +++++++++++++++ */
// /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
//
//
//
// struct config_general get_config_general(){
//   struct config_general cur_config;
//   persist_read_data(CONFIG_GENERAL_PERSIST_KEY,&cur_config,sizeof(cur_config));
//   return cur_config;
// }




// // TEST : PASSED
// void write_time_to_array_head(uint8_t * buf){
//   /* get the current time */
//   uint32_t timestamp = time(NULL);
//   /* stuff the timestamp into first 4 bytes of array, assuming little endian encoding*/
//   buf[0] = (uint8_t) timestamp;
//   buf[1] = (uint8_t) (timestamp >> 8 );
//   buf[2] = (uint8_t) (timestamp >> 16 );
//   buf[3] = (uint8_t) (timestamp >> 24 );
// }

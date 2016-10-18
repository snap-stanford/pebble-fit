#include "data.h"

bool data_to_send_acti(){
  // compares the current time to the end of the previous block of data sent up
  return ( (time(NULL) - persist_read_int(ACTI_LAST_UPLOAD_TIME_PERSIST_KEY)) >= (INTERVAL_MINUTES*60));
}

bool data_to_send_pinteract(){
  // this code tells us if th
  return (persist_read_int(PINTERACT_KEY_COUNT_PERSIST_KEY) > 0);
}

int get_next_pinteract_element_key(){
  return persist_read_int(PINTERACT_KEY_COUNT_PERSIST_KEY);
}

int get_data_size_of_pinteract_element(int pstorage_key){
  // get the head of the pinteract block, which will contain the size of the
  // whole block, so we just write the first two bytes into the
  uint16_t pinteract_header[2];
  persist_read_data(pstorage_key, pinteract_header, 4);
  //get the second first number element of the array, whcih
  return pinteract_header[1];
}

int get_pinteract_code_of_pinteract_element(int pstorage_key){
  // get the head of the pinteract block, which will contain the size of the
  // whole block, so we just write the first two bytes into the
  uint16_t pinteract_header[2];
  persist_read_data(pstorage_key, pinteract_header, 4);
  //get the second first number element of the array, whcih
  return pinteract_header[0];
}

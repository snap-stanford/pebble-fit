#include "pinteract_func.h"


uint16_t write_res_buf_byte_count(uint8_t* res_buf, uint16_t srt_ind, uint16_t byte_count){
  res_buf[srt_ind] = (uint8_t) byte_count;
  res_buf[srt_ind + 1] = (uint8_t) (byte_count >> 8);

  // output the new byte count
  return read_res_buf_byte_count(res_buf,srt_ind);
}

uint16_t read_res_buf_byte_count(uint8_t* res_buf, uint16_t byte_cnt_ind){
  return res_buf[byte_cnt_ind] | (((uint16_t) res_buf[byte_cnt_ind + 1])<<8) ;
  // output the byte count
}



uint16_t write_data_to_res_buf(uint8_t* res_buf, uint8_t* data, uint16_t data_dl){
  // write the data to the buffer???
  uint16_t res_buf_dl = read_res_buf_byte_count(res_buf, 0); // res_buf data length

  memcpy( res_buf + res_buf_dl, data, data_dl); // pointer arithmatic

  uint16_t new_res_buf_dl = res_buf_dl + data_dl;

  if(new_res_buf_dl < MAX_PINTERACT_PS_B_SIZE){
    return write_res_buf_byte_count(res_buf, 0, new_res_buf_dl);
    //     return new_res_buf_dl;
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR,"MAJOR PROBLEM, response buffer length exceeds range of uint8_t" );
    return 0;
  }
}





uint16_t pinteract_write_res_buf_to_pesistant_storage(struct pinteract_func_ll_el* pif_ll_el ){
  // note that the pinteract must be finished, so for redundancy we set this to
  // inactive, ie, -1 active condi
  persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,-1);

  APP_LOG(APP_LOG_LEVEL_ERROR, "made it to pinteract_write_res_buf_to_pesistant_storage");
  // get the response buffer after all pinteract finished
  uint8_t *buf = pif_ll_el->buf;
  // buf_dl is assumed to be the amound of valid data that we want written to
  // response buffer. SO, for the data buf written FROM, we wont count the header
  // as data, but for the response buffer we are writing TO, we do count the header
  // cause it is par of the data we DONT want to overwrite. Hence, IF we are writing
  // a response buffer onto the back of another

  // buf_dl is assumed to be the amound of valid data we want to copy over,
  // so we ignore the first two  bytes that is the counter for the valid data bytes
  uint16_t buf_dl = read_res_buf_byte_count(buf, 0) - PINTERACT_RES_BUF_COUNTER_B_SIZE;

  // get the current pinteract pesist data
  uint8_t ps_buf[MAX_PINTERACT_PS_B_SIZE];
  persist_read_data(PIRPS_B1_PERSIST_KEY, ps_buf,sizeof(ps_buf));

  // just use memcopy
  uint16_t ps_buf_dl = read_res_buf_byte_count(ps_buf, PINTERACT_PS_B_COUNT_IND); // ps_buf data length

  APP_LOG(APP_LOG_LEVEL_ERROR, "ps_buf_dl %d", ps_buf_dl);

  // we start @ buf[1] to not include the data byte  counter
  memcpy( ps_buf + ps_buf_dl, buf + sizeof(MAX_PINTERACT_PS_B_SIZE), buf_dl);
  // we update the data byte counter of the ps_buf, which is after the 4B timestamp
  // and hence at position 5, index 4
  uint16_t new_ps_buf_dl= ps_buf_dl + buf_dl;


  if(new_ps_buf_dl <=  MAX_PINTERACT_PS_B_SIZE){
    // the write_data_to_res_buf does everything to update the size of the buffer,
    // and returns the final size of the ps_buf
    write_res_buf_byte_count(ps_buf, PINTERACT_PS_B_COUNT_IND, new_ps_buf_dl);
    persist_write_data(PIRPS_B1_PERSIST_KEY, ps_buf,sizeof(ps_buf));
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "E: res buffer length exceeds range of uint8_t");
  }
  // !!!!!
  // schedule the next config wakeup event
  config_wakeup_schedule();
  // push the main dash screen
  // psleep(100);
  // display_main_dash();

  return 0;
}



  //   APP_LOG(APP_LOG_LEVEL_ERROR, "GOT TO PINTERACT ALERT");
  //   APP_LOG(APP_LOG_LEVEL_ERROR,
  //           "sample: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
  //           tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],
  //          tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15],tmp[16],tmp[17]);

  // uint8_t *tmp = ps_buf;
  // APP_LOG(APP_LOG_LEVEL_ERROR, "write res_bif to ps: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
  //         tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],
  //         tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15],tmp[16],tmp[17]);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "continued: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
  //         tmp[18],tmp[19],tmp[20],tmp[21],tmp[22],tmp[23],tmp[24],tmp[25],tmp[26],
  //         tmp[27],tmp[28],tmp[29],tmp[30],tmp[31],tmp[32],tmp[33],tmp[34],tmp[35]);
  //
  //
  // APP_LOG(APP_LOG_LEVEL_ERROR, "bytecount %d, timestamps: %d,%d,%d,%d",
  //         (int)read_res_buf_byte_count(tmp,4),
  //         (int)read_time_from_array_head(&tmp[0]),
  //         (int)read_time_from_array_head(&tmp[6]),
  //         (int)read_time_from_array_head(&tmp[10]),
  //        (int)read_time_from_array_head(&tmp[17]));

//     APP_LOG(APP_LOG_LEVEL_ERROR, "write res_bif to ps: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
//           tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],
//           tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15],tmp[16],tmp[17],
//           tmp[18],tmp[19],tmp[20],tmp[21],tmp[22]);

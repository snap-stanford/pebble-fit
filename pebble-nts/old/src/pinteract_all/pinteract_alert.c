#include "pinteract_func.h"

uint16_t pinteract_alert(struct pinteract_func_ll_el* pif_ll_el ){
  uint8_t *tmp = pif_ll_el->buf;
  APP_LOG(APP_LOG_LEVEL_ERROR, "GOT TO PINTERACT ALERT");
  APP_LOG(APP_LOG_LEVEL_ERROR, "sample: %u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
          tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],
         tmp[9],tmp[10],tmp[11],tmp[12],tmp[13],tmp[14],tmp[15],tmp[16],tmp[17],
         tmp[18],tmp[19],tmp[20],tmp[21],tmp[22]);

  APP_LOG(APP_LOG_LEVEL_ERROR, "timestamps: %d,%d,%d",
          (int)read_res_buf_byte_count(tmp,0),
          (int)read_time_from_array_head(&tmp[4]),
          (int)read_time_from_array_head(&tmp[8]) );


  return 0;
}

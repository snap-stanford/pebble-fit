#include "wakeup.h"

static void sent_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, " Sent from Watch!");
}

static void received_server_handler(DictionaryIterator *iter, void *context) {
  if(dict_find(iter, AppKeyServerReceived)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps Received by Server!");
  }
  CommCallback *cb = (CommCallback *) context;
  cb();
}

static void send_data_callback(DictionaryIterator * out) {
  //write the data
  dict_write_data(out, AppKeyStepsData, s_data, sizeof(uint8_t) * s_num_records);
  dict_write_int(out, AppKeyStepsEndDate, &s_start, sizeof(int), true);
}

void send_wakeup_reason() {

}
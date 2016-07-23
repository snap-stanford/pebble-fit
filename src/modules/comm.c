#include "comm.h"

static void js_started_handler(DictionaryIterator *iter, void *context) {
  Tuple *js_ready_t = dict_find(iter, AppKeyJSReady);
  if(js_ready_t) {
    CommCallback *cb = (CommCallback *) context;
    cb();
  }
}

void comm_init(CommCallback *callback) {
  app_message_register_inbox_received(js_started_handler);
  app_message_set_context(callback);
  
  const int inbox_size = APP_MESSAGE_INBOX_SIZE_MINIMUM;
  const int outbox_size = APP_MESSAGE_OUTBOX_SIZE_MINIMUM;
  app_message_open(inbox_size, outbox_size);
}

void comm_deinit() { 
  // Nothing yet
}
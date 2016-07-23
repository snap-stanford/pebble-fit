#include "comm.h"

static CommJSReadyCallback *js_callback;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *js_ready_t = dict_find(iter, AppKeyJSReady);
  if(js_ready_t) {
    js_callback();
  }
}

void comm_init(CommJSReadyCallback *callback) {
  js_callback = callback;
  app_message_register_inbox_received(inbox_received_handler);
  
  const int inbox_size = APP_MESSAGE_INBOX_SIZE_MINIMUM;
  const int outbox_size = APP_MESSAGE_OUTBOX_SIZE_MINIMUM;
  app_message_open(inbox_size, outbox_size);
}

void comm_deinit() { 
  // Nothing yet
}
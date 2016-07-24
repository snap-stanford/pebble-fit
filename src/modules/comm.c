#include "comm.h"

static void js_ready_handler(DictionaryIterator *iter, void *context) {
  if(dict_find(iter, AppKeyJSReady)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Connected to JS!");
    ((CommCallback *) context)();
  }
}

void comm_init(CommCallback *callback) {
  app_message_register_inbox_received(js_ready_handler);
  app_message_set_context(callback);
  
  const int inbox_size = APP_MESSAGE_INBOX_SIZE_MINIMUM;
  const int outbox_size = APP_MESSAGE_OUTBOX_SIZE_MINIMUM;
  app_message_open(inbox_size, outbox_size);
}

void comm_sent_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Sent from Watch!");
}

void comm_server_received_handler(DictionaryIterator *iter, void *context) {
  if(dict_find(iter, AppKeyServerReceived)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Received by Server!");
    ((CommCallback *) context)();
  }
}

void comm_send_data(
  DataWriteCallback data_write_callback,
  AppMessageOutboxSent sent_handler,
  AppMessageInboxReceived received_handler
) {
  
  DictionaryIterator *out;

  // init dict
  if(app_message_outbox_begin(&out) != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message.");
    return;
  }

  // register handlers
  app_message_register_outbox_sent(sent_handler);
  app_message_register_inbox_received(received_handler);

  if (data_write_callback != NULL) data_write_callback(out);

  //check the sending of the message
  if(app_message_outbox_send() != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error starting send of message.");
  }
}

void comm_deinit() { 
  // Nothing yet
}
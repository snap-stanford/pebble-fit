#include "comm.h"

static int s_index, s_num_records;
static time_t end;

static void send_data_item(int index) {
  int *data = data_get_steps_data();

  DictionaryIterator *out;
  if(app_message_outbox_begin(&out) == APP_MSG_OK) {
    dict_write_int(out, AppKeyIndex, &index, sizeof(int), true);
    dict_write_int(out, AppKeyData, &data[s_index], sizeof(int), true);

    // Include the total number of data items
    if(s_index == 0) {
      dict_write_int(out, AppKeyNumDataItems, &s_num_records, sizeof(int), true);
      int end_int = (int) end;
      dict_write_int(out, AppKeyDate, &end_int, sizeof(int), true);
    }

    if(app_message_outbox_send() != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending message");
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning message");
  }
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context) {
  // Last message was successful
  s_index++;

  if(s_index < s_num_records) {
    // Send next item
    send_data_item(s_index);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Upload complete!");
    upload_event();
  }
}

void comm_init(int inbox, int outbox) {
  app_message_register_outbox_sent(outbox_sent_handler);
  app_message_open(inbox, outbox);
}

void comm_begin_upload(int num_records) {
  s_index = 0;
  s_num_records = num_records;
  send_data_item(s_index);
}

void upload_event() {
  if (end == 0) {
    end = time(NULL) - (15 * SECONDS_PER_MINUTE);
  }

  time_t start = end - (MAX_ENTRIES * SECONDS_PER_MINUTE);
  
  // Get last minute data
  int num_records = data_reload_steps(&start, &end);

  end = start;

  if(num_records == 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No new data");
    return;
  }

  // Send to JS
  if(connection_service_peek_pebble_app_connection()) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Beginning upload...");
    comm_begin_upload(num_records);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not send data, connection unavailable");
  }
}
#include "storage.h"

#define MAX_ENTRIES 60
static int s_data[MAX_ENTRIES];
static int s_num_records;
static int latest_key;
static const int AppKeyArrayData = 200;

/* Zero out static array. */
static void clear_static_data() {
  s_num_records = 0;
  for(int i = 0; i < MAX_ENTRIES; i++) {
      s_data[i] = 0;
    }
}

/* Copy persistant array into local array. */
static void load_data_into_local_array(int key) {
  // Clear static data
  clear_static_data();
    
  if(persist_exists(key)) {
    // load data in local array
    s_num_records = persist_read_data(key, s_data, MAX_ENTRIES * sizeof(int)) / sizeof(int);
  }

  // store latest key
  latest_key = key;
}

/* Append new element into persistant array. */
void store_key_data(int key, int element_to_add) {
  load_data_into_local_array(key);

  // add to array
  if(s_num_records < MAX_ENTRIES) {
    s_data[s_num_records] = element_to_add;
  }

  // todo: check if write is successful
  persist_write_data(key, s_data, (s_num_records + 1) * sizeof(int));
}

/* Write local array into dictionary. */
static void data_write(DictionaryIterator * out) {
  //write the data
  dict_write_int(out, AppKeyStorageKey, &latest_key, sizeof(int), true);
  dict_write_int(out, AppKeyArrayLength, &s_num_records, sizeof(int), true);
  dict_write_int(out, AppKeyArrayStart, &AppKeyArrayData, sizeof(int), true);
  for (int i = 0; i < s_num_records; i++) {
    dict_write_int(out, AppKeyArrayData + i, &s_data[i], sizeof(int), true);
  }
}

/* Load persist data associated with key, and send it. */
void transfer_data(int key) {
  load_data_into_local_array(key);
  // todo: check if delete is successful, and delete after data transfer
  persist_delete(key);

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}

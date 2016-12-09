#include "store.h"

#define MAX_ENTRIES 60
//static int s_data[MAX_ENTRIES];
static int s_data[MAX_ENTRIES];
static int s_num_records;
static int latest_key;
//static time_t s_update_time;

static uint8_t s_launch_exit_count;
static uint32_t s_launch_exit_data;

/* 
 * The format of launch-exit record stored persistently (compact to save space).
 * Total size: 4 bytes
 * 11 bits                 | 17 bits                  | 2 bits   | 2 bits  
 * launch (mins since SoD) | exit (secs after launch) | l reason | e reason
 */
#define LAUNCH_EXIT_DATA_SIZE   4
#define LAUNCH_EXIT_COUNT_SIZE  1
#define COMPACT(lm, es, lr, er) (lm<<21 | (es&0x1ffff)<<4 | (lr&0x3)<<2 | (er&0x3))
#define GET_MINUTE(x)           ((x>>21) & 0x07FF)
#define GET_SECOND(x)           ((x>>4)  & 0x01FFFF)
#define GET_LAUNCH_REASON(x)    ((x>>2)  & 0x0003)
#define GET_EXIT_REASON(x)      (x       & 0x0003)

void store_write_launch_exit_event(time_t launch_time, time_t exit_time, uint8_t lr, uint8_t er) {
  time_t minutes, seconds;

  if (persist_exists(PERSIST_KEY_LAUNCH_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCH_COUNT, &s_launch_exit_count, 1);
  }

  if (s_launch_exit_count > PERSIST_KEY_LAUNCH_END - PERSIST_KEY_LAUNCH_START + 1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "s_launch_exit_count less than zero = %u", s_launch_exit_count);
    s_launch_exit_count = 0;
  }

  // Find the number of minutes since SoD (rounded to the nearest minute)
  minutes = (launch_time - time_start_of_today() + SECONDS_PER_MINUTE/2) / SECONDS_PER_MINUTE;
  seconds = exit_time - launch_time;
  s_launch_exit_data = COMPACT(minutes, seconds, lr, er);
  s_launch_exit_count++;

  // DEBUG
  APP_LOG(APP_LOG_LEVEL_INFO, "Write launch and exit events to persistent storage" \
      ". new records count=%d.", s_launch_exit_count);
  APP_LOG(APP_LOG_LEVEL_INFO, "launch_time=%u, exit_time=%u", (unsigned int)launch_time, (unsigned int)exit_time);
  APP_LOG(APP_LOG_LEVEL_INFO, "min=%u, seconds=%u, lr=%d, er=%d, data=%08x", (unsigned int)minutes, (unsigned int)seconds, lr, er, (unsigned int)s_launch_exit_data);
  
  // Write data first. Can tolerate losing data to avoid uploading wrong data to the server.
  persist_write_data(PERSIST_KEY_LAUNCH_START+s_launch_exit_count-1, &s_launch_exit_data, LAUNCH_EXIT_DATA_SIZE);
  persist_write_data(PERSIST_KEY_LAUNCH_COUNT, &s_launch_exit_count, LAUNCH_EXIT_COUNT_SIZE);
}

void store_send_launch_exit_event() {
  // FIXME: consider using a single key and sequential storage location
  uint32_t key;
  time_t launch_time, exit_time;
  if (persist_exists(PERSIST_KEY_LAUNCH_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCH_COUNT, &s_launch_exit_count, LAUNCH_EXIT_COUNT_SIZE);

    // Read each record and try to resend to the server; decrement count.
    // FIXME: instead of sending all records to the server, only send one at a time.
    // When ACK comes back, this function will be called again.
    //for (key = PERSIST_KEY_LAUNCH_START+s_launch_exit_count-1; key >= PERSIST_KEY_LAUNCH_START; key--) {
    key = PERSIST_KEY_LAUNCH_START+s_launch_exit_count-1;
      if (key>=PERSIST_KEY_LAUNCH_START && key<=PERSIST_KEY_LAUNCH_END &&  persist_exists(key)) {
        persist_read_data(key, &s_launch_exit_data, LAUNCH_EXIT_DATA_SIZE);

        launch_time = time_start_of_today() + GET_MINUTE(s_launch_exit_data) * SECONDS_PER_MINUTE;
        exit_time = launch_time + GET_SECOND(s_launch_exit_data);

        //launch_resend(launch_time, GET_LAUNCH_REASON(s_launch_exit_data), true);
        //launch_resend(exit_time, GET_EXIT_REASON(s_launch_exit_data), false);
        launch_resend(launch_time, exit_time, 
          GET_LAUNCH_REASON(s_launch_exit_data), GET_EXIT_REASON(s_launch_exit_data));

        persist_delete(key);
        s_launch_exit_count--;


        // DEBUG
        APP_LOG(APP_LOG_LEVEL_INFO, "Resend launch and exit events to the server" \
            ". new records count=%d.", s_launch_exit_count);
        APP_LOG(APP_LOG_LEVEL_INFO, "launch_time=%u, exit_time=%u, lr=%d, er=%d", (unsigned int)launch_time, (unsigned int)exit_time, (int)GET_LAUNCH_REASON(s_launch_exit_data), (int)GET_EXIT_REASON(s_launch_exit_data));
      }
    //}

    persist_write_data(PERSIST_KEY_LAUNCH_COUNT, &s_launch_exit_count, LAUNCH_EXIT_COUNT_SIZE);
  }
}

//void store_write_update_time(time_t timestamp) {
//  s_update_time = time(NULL);
//  persist_write_data(PERSIST_KEY_UPDATE_TIME, &s_update_time, sizeof(time_t));
//}
//int store_read_update_time(time_t timestamp) {
//  if (persist_exists(PERSIST_KEY_UPDATE_TIME)) {
//    return persist_read_data(PERSIST_KEY_UPDATE_TIME, &s_update_time, sizeof(time_t));
//  } else {
//    return -1; // 0xffffffff
//  }
//}

/* Old implementations. */

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
void store_add_key_data(int key, int element_to_add) {
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
void store_transfer_data(int key) {
  load_data_into_local_array(key);
  // todo: check if delete is successful, and delete after data transfer
  persist_delete(key);

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}

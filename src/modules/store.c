#include "store.h"

//static int s_data[MAX_ENTRIES];
static int s_num_records;
static int latest_key;

static uint8_t s_launchexit_count;
static uint32_t s_launchexit_data;

/* 
 * The format of launch-exit record stored persistently (compact to save space).
 * Total size: 4 bytes
 * 11 bits                 | 17 bits                  | 2 bits   | 2 bits  
 * launch (mins since SoD) | exit (secs after launch) | l reason | e reason
 */
#define LAUNCHEXIT_DATA_SIZE    4
#define LAUNCHEXIT_COUNT_SIZE   1
#define COMPACT(lm, es, lr, er) (lm<<21 | (es&0x1ffff)<<4 | (lr&0x3)<<2 | (er&0x3))
#define GET_MINUTE(x)           ((x>>21) & 0x07FF)
#define GET_SECOND(x)           ((x>>4)  & 0x01FFFF)
#define GET_LAUNCH_REASON(x)    ((x>>2)  & 0x0003)
#define GET_EXIT_REASON(x)      (x       & 0x0003)

void store_write_launchexit_event(time_t launch_time, time_t exit_time, uint8_t lr, uint8_t er) {
  time_t minutes, seconds;

  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, 1);
  }

  if (s_launchexit_count > PERSIST_KEY_LAUNCH_END - PERSIST_KEY_LAUNCH_START + 1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "s_launchexit_count less than zero = %u", s_launchexit_count);
    s_launchexit_count = 0;
  }

  // Find the number of minutes since SoD (rounded to the nearest minute)
  // Always round down for consistency.
  //minutes = (launch_time - time_start_of_today() + SECONDS_PER_MINUTE/2) / SECONDS_PER_MINUTE;
  minutes = (launch_time - time_start_of_today()) / SECONDS_PER_MINUTE;
  seconds = exit_time - launch_time;
  s_launchexit_data = COMPACT(minutes, seconds, lr, er);
  s_launchexit_count++;

  // DEBUG
  APP_LOG(APP_LOG_LEVEL_INFO, "Write launch and exit events to persistent storage" \
      ". new records count=%d.", s_launchexit_count);
  APP_LOG(APP_LOG_LEVEL_INFO, "launch_time=%u, exit_time=%u", (unsigned int)launch_time, (unsigned int)exit_time);
  APP_LOG(APP_LOG_LEVEL_INFO, "min=%u, seconds=%u, lr=%d, er=%d, data=%08x", (unsigned int)minutes, (unsigned int)seconds, lr, er, (unsigned int)s_launchexit_data);
  
  // Write data first. Can tolerate losing data to avoid uploading wrong data to the server.
  persist_write_data(PERSIST_KEY_LAUNCH_START+s_launchexit_count-1, &s_launchexit_data, LAUNCHEXIT_DATA_SIZE);
  persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);
}

/* Return whether we finish resending launch/exit event (only when we do not send any data 
 * to the server. */
bool store_resend_launchexit_event() {
  // FIXME: consider using a single key and sequential storage location
  uint32_t key;
  time_t launch_time, exit_time;
  if (persist_exists(PERSIST_KEY_LAUNCHEXIT_COUNT)) {
    persist_read_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);

    if (s_launchexit_count <= 0) {
      return true;
    }

    // Read each record and try to resend to the server; decrement count.
    // FIXME: instead of sending all records to the server, only send one at a time.
    // When ACK comes back, this function will be called again.
    key = PERSIST_KEY_LAUNCH_START+s_launchexit_count-1;
    if (persist_exists(key) && key >= PERSIST_KEY_LAUNCH_START && key <= PERSIST_KEY_LAUNCH_END) {
      persist_read_data(key, &s_launchexit_data, LAUNCHEXIT_DATA_SIZE);

      launch_time = time_start_of_today() + GET_MINUTE(s_launchexit_data) * SECONDS_PER_MINUTE;
      exit_time = launch_time + GET_SECOND(s_launchexit_data);

      //launch_resend(launch_time, GET_LAUNCH_REASON(s_launchexit_data), true);
      //launch_resend(exit_time, GET_EXIT_REASON(s_launchexit_data), false);
      launch_resend(launch_time, exit_time, 
        GET_LAUNCH_REASON(s_launchexit_data), GET_EXIT_REASON(s_launchexit_data));

      persist_delete(key);
      s_launchexit_count--;


      // DEBUG
      APP_LOG(APP_LOG_LEVEL_INFO, "Resend launch and exit events to the server" \
          ". new records count=%d.", s_launchexit_count);
      APP_LOG(APP_LOG_LEVEL_INFO, "launch_time=%u, exit_time=%u, lr=%d, er=%d", (unsigned int)launch_time, (unsigned int)exit_time, (int)GET_LAUNCH_REASON(s_launchexit_data), (int)GET_EXIT_REASON(s_launchexit_data));
    }

    persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &s_launchexit_count, LAUNCHEXIT_COUNT_SIZE);
    return false;
  } else {
    return true;
  }
}

void store_write_update_time(time_t time) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Store update time = %d", (int) time);
  persist_write_data(PERSIST_KEY_UPDATE_TIME, &time, sizeof(time_t));
}

/* Return whether we finish resending steps data (only when we do not send any data 
 * to the server. Later in steps_send_latest()  we might resend some data that we are 
 * sending in here.
 */
bool store_resend_steps(time_t curr_time) {
  time_t last_update_time; 
  time_t interval_seconds = MAX_ENTRIES * SECONDS_PER_MINUTE;
  //last_update_time = (last_update_time < time_start_of_today());

  if (persist_exists(PERSIST_KEY_UPDATE_TIME)) {
    persist_read_data(PERSIST_KEY_UPDATE_TIME, &last_update_time, sizeof(time_t));
    if (last_update_time >= curr_time - interval_seconds) {
      // Data in the lastest 60 minutes will be sent by the normal data upload routine.
      return true;
    }

    steps_send_in_between(last_update_time, last_update_time + interval_seconds, true);

    last_update_time += interval_seconds;
    persist_write_data(PERSIST_KEY_UPDATE_TIME, &last_update_time, sizeof(time_t));
    //return last_update_time < curr_time - interval_seconds;
    return false;
  } else {
    return true;
  }
}
//int store_read_update_time(time_t timestamp) {
//  if (persist_exists(PERSIST_KEY_UPDATE_TIME)) {
//    return persist_read_data(PERSIST_KEY_UPDATE_TIME, &s_update_time, sizeof(time_t));
//  } else {
//    return -1; // 0xffffffff
//  }
//}

/* Old implementations. */

/* Zero out static array. */
/*
static void clear_static_data() {
  s_num_records = 0;
  for(int i = 0; i < MAX_ENTRIES; i++) {
      s_data[i] = 0;
    }
}
*/

/* Copy persistant array into local array. */
/*
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
*/

/* Append new element into persistant array. */
/*
void store_add_key_data(int key, int element_to_add) {
  load_data_into_local_array(key);

  // add to array
  if(s_num_records < MAX_ENTRIES) {
    s_data[s_num_records] = element_to_add;
  }

  // todo: check if write is successful
  persist_write_data(key, s_data, (s_num_records + 1) * sizeof(int));
}
*/

/* Write local array into dictionary. */
/*
static void data_write(DictionaryIterator * out) {
  //write the data
  dict_write_int(out, AppKeyStorageKey, &latest_key, sizeof(int), true);
  dict_write_int(out, AppKeyArrayLength, &s_num_records, sizeof(int), true);
  dict_write_int(out, AppKeyArrayStart, &AppKeyArrayData, sizeof(int), true);
  for (int i = 0; i < s_num_records; i++) {
    dict_write_int(out, AppKeyArrayData + i, &s_data[i], sizeof(int), true);
  }
}
*/

/* Load persist data associated with key, and send it. */
/*
void store_transfer_data(int key) {
  load_data_into_local_array(key);
  // todo: check if delete is successful, and delete after data transfer
  persist_delete(key);

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}
*/

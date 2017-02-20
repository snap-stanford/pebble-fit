#include "launch.h"

time_t e_launch_time;
int e_launch_reason;
int e_exit_reason;

static time_t s_launch_time, s_exit_time, s_curr_time;
static int s_config_request;
static const char *s_random_message_id, *s_random_message;
char random_message[RANDOM_MSG_SIZE_MAX];

/* Add launch reason and date to out dict. */
// FIXME: could combine both launch and exit packets, since they have very similar format.
static void prv_launch_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyDate, &e_launch_time, sizeof(int), true);
  dict_write_int(out, AppKeyLaunchReason, &e_launch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyConfigRequest, &s_config_request, sizeof(int), true);
  dict_write_cstring(out, AppKeyRandomMessage, s_random_message_id);
}
/* Add exit reason and date to out dict. */
static void prv_exit_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyExitReason, &e_exit_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_exit_time, sizeof(int), true);
}

/* Add both launch and exit reasons and dates to out dict. */
static void prv_launch_exit_data_write(DictionaryIterator * out) {
  s_curr_time = time(NULL);
  dict_write_int(out, AppKeyLaunchReason, &e_launch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyLaunchTime, &e_launch_time, sizeof(int), true);
  dict_write_int(out, AppKeyExitReason, &e_exit_reason, sizeof(int), true);
  dict_write_int(out, AppKeyExitTime, &s_exit_time, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_curr_time, sizeof(int), true);
}

/**
 * Upload the current launch event. Might associate a request for the new configuration.
 */
void launch_send_launch_notification() {
  s_config_request = store_resend_config_request(e_launch_time)? 1 : 0;

  comm_send_data(prv_launch_data_write, comm_sent_handler, comm_server_received_handler);
}

/* Upload the current exit event to phone. */
void launch_send_exit_notification(time_t time) {
  s_exit_time = time;
  comm_send_data(prv_exit_data_write, comm_sent_handler, NULL);
}

/**
 * Resend the previous launch and exit events.
 */
void launch_resend(time_t launch_time, time_t exit_time, int launch_reason, int exit_reason) {
  // The current launch record has been uploaded and the current exit reason has 
  // not yet been collected, so it is safe to modify these two varaibles here.
  APP_LOG(APP_LOG_LEVEL_INFO, "launch_resend");
  s_launch_time = launch_time;
  s_exit_time = exit_time;
  e_launch_reason = launch_reason;
  e_exit_reason = exit_reason;

  comm_send_data(prv_launch_exit_data_write, comm_sent_handler, comm_server_received_handler);

  //if (is_launch) {
  //  comm_send_data(prv_launch_data_write, comm_sent_handler, comm_server_received_handler);
  //} else {
  //  comm_send_data(prv_exit_data_write, comm_sent_handler, NULL);
  //}
}

/**
 * Read a random message from the persistent storage into memory.
 */
void launch_set_random_message() {
  char *c;

  snprintf(random_message, sizeof(random_message), store_read_random_message());

  for (s_random_message_id = c = random_message; *c++ != ':'; ) {} // Assuming ':' exists.
  *(--c) = '\0';
  s_random_message = ++c;
} 

/**
 * Return the random message previouly read from the persistent memory. 
 */
const char* launch_get_random_message() {
  return s_random_message;
}

#include "launch.h"

int e_exit_reason;
int e_launch_reason;

static time_t s_launch_time, s_exit_time, s_curr_time;
static int s_config_request;
static int s_test;

/* Add launch reason and date to out dict. */
// FIXME: could combine both launch and exit packets, since they have very similar format.
static void prv_launch_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyLaunchReason, &e_launch_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_launch_time, sizeof(int), true);
  dict_write_int(out, AppKeyConfigRequest, &s_config_request, sizeof(int), true);
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
  dict_write_int(out, AppKeyLaunchTime, &s_launch_time, sizeof(int), true);
  dict_write_int(out, AppKeyExitReason, &e_exit_reason, sizeof(int), true);
  dict_write_int(out, AppKeyExitTime, &s_exit_time, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_curr_time, sizeof(int), true);
}

/* Test function for debug purpose. */
static void prv_test_data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyTest0, &s_test, sizeof(int), true);
}

void launch_send_test(int test) {
  s_test = test; 
  comm_send_data(prv_test_data_write, comm_sent_handler, comm_server_received_handler);
}

/* Send launch event to phone. */
void launch_send_launch_notification(time_t time) {
  s_launch_time = time;
  s_config_request = store_resend_config_request(time)? 1 : 0;
  /*
  switch (launch_reason()) {
    case APP_LAUNCH_USER:
    case APP_LAUNCH_QUICK_LAUNCH:
      e_launch_reason = USER_LAUNCH;
      break;
    case APP_LAUNCH_WAKEUP:
      e_launch_reason = WAKEUP_LAUNCH;
      break;
    case APP_LAUNCH_PHONE:
      e_launch_reason = PHONE_LAUNCH;
      break;
    default:
      e_launch_reason = OTHER_LAUNCH;
  }
  */

  comm_send_data(prv_launch_data_write, comm_sent_handler, comm_server_received_handler);
}

/* Send exited event to phone. */
void launch_send_exit_notification(time_t time) {
  s_exit_time = time;
  comm_send_data(prv_exit_data_write, comm_sent_handler, NULL);
}

/* TODO. */
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

